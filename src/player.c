/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <assert.h>

#include <uncursed.h>

#include "enums.h"
#include "player.h"
#include "fight.h"
#include "input.h"
#include "inventory.h"
#include "fov/sight.h"
#include "game.h"
#include "random.h"
#include "ui/ui.h"
#include "options.h"
#include "ai/ai_utils.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "monster/monster_static.h"
#include "monster/monster_action.h"
#include "items/items.h"
#include "careers/careers.h"
#include "quests/quests.h"
#include "wizard/wizard_mode.h"

static bool plr_action_loop(struct msr_monster *player);

bool plr_init(struct pl_player *plr) {
    struct monster_controller mc = {
        .ai = {
            .ai_ctx = plr,
        },
        .controller_cb = plr_action_loop,
    };

    if (plr->player != NULL) {
        msr_assign_controller(plr->player, &mc);

        plr->player->icon = '@';
        plr->player->icon_attr = TERM_COLOUR_WHITE;
        plr->player->faction = MSR_FACTION_PLAYER;
        plr->exit_map = false;
        plr->retire = false;
        return true;
    }
    return false;
}

struct pf_context *plr_map(struct pl_player *plr, struct dm_map *map) {
    if (cd_equal(&plr->player_map_pos, & plr->player->pos) == false) {
        if (aiu_generate_dijkstra(&plr->player_map, map, &plr->player->pos, 0) == true) {
            plr->player_map_pos = plr->player->pos;
        }
    }
    return plr->player_map;
}

static bool player_running = false;
static coord_t player_running_dir;
static bool interrupt_running(struct msr_monster *player, struct dm_map *map, coord_t *pos, coord_t *dir) {
    coord_t next = cd_add(pos, dir);
    if (dm_get_map_tile(pos, map)->id != dm_get_map_tile(&next, map)->id) return true;

    if (aiu_get_nearest_enemy(player, 0, map) != NULL) return true;
    return false;
}

static bool low_wounds_warning = false;
static bool critical_wounds_warning = false;
static bool plr_action_loop(struct msr_monster *player) {
    if (player == NULL) return false;
    struct dm_map *map = gbl_game->current_map;
    int ch;
    bool has_action = false;

    if (options.debug) {
        itm_dbg_check_all();
        msr_dbg_check_all();
        se_dbg_check_all();

        struct inv_inventory *inv = player->inventory;
        struct itm_item *item = NULL;
        while ( ( (item = inv_get_next_item(inv, item) ) != NULL) ) {
            itm_verify_item(item);
        }
    }

    gbl_game->plr_last_turn = gbl_game->turn;
    game_save();

    coord_t pos = player->pos;
    coord_t zero = cd_create(0,0);
    coord_t change_pos = zero;
    coord_t *player_pos = &player->pos;

    if (se_has_effect(player, EF_DEAD) ) {
        if (player->fate_points > 0) {
            player->fate_points -= 1;
            se_remove_status_effects_by_effect(player, EF_DEAD);

            msr_remove_monster(player, map);
            if (dm_tile_instance(map, TILE_TYPE_STAIRS_UP, 0, &pos) == false) exit(1);
            if (cd_equal(player_pos, &pos) == false) {
                if (msr_insert_monster(player, map, &pos) == false) exit(1);
            }
            clear();
            refresh();

            se_add_status_effect(player, SEID_FATEHEALTH, NULL);
            se_remove_all_non_permanent(player);

            GM_msg( cs_PLAYER "You" cs_CLOSE " would have died if fate did not intervene...");

            return true;
        }
        else {
            gbl_game->running = false;
            usleep(40000);
            return true;
        }
    }

    if ( ( (player->wounds.curr * 100) / player->wounds.max) < 10) {
        if (critical_wounds_warning == false) {
            Warning("hitpoints critical.");
            critical_wounds_warning = true;
        }
    }
    else if ( ( (player->wounds.curr * 100) / player->wounds.max) < 50) {
        if (low_wounds_warning == false) {
            Warning("low hitpoints.");
            low_wounds_warning = true;
        }
        critical_wounds_warning = false;
    }
    else if ( ( (player->wounds.curr * 100) / player->wounds.max) > 50) {
        low_wounds_warning = false;
    }

    qst_process_quest_during(gbl_game->player_data.quest, gbl_game->current_map);

    if (player_running == true) {
        if (interrupt_running(player, map, &pos, &player_running_dir) == false) {
            pos = cd_add(&pos, &player_running_dir);
            if (ma_do_move(player, &pos) == true) {
                has_action = true;
            }
        }
        else player_running = false;
    }

    lg_debug("plr_action_loop");
    while (gbl_game->running && (has_action == false) ) {
        /* Update screen dimensions if needed*/
        int lines, cols;
        getmaxyx(stdscr, lines, cols);
        ui_create(cols, lines);

        update_screen();
        //mapwin_display_map(map, player_pos);
        //charwin_refresh();

        switch (ch = inp_get_input(gbl_game->input) ) {
            case INP_KEY_PICKUP: {
                    struct inv_inventory *inv = dm_get_map_me(&pos, map)->inventory;
                    if ( (inv_inventory_size(inv) ) > 0) {
                        struct itm_item *item = NULL;
                        struct itm_item *item_list[inv_inventory_size(inv)];
                        int item_list_sz = 0;
                        bool stop = false;
                        bool pickup_all = false;

                        if (inv_inventory_size(inv) > 1) {
                            while ( ( (item = inv_get_next_item(inv, item) ) != NULL) && (stop == false) ){
                                bool pickup = false;

                                if (pickup_all == false) {
                                    System_msg("Pickup %s? (o)k/(c)ancel/(a)ll/(q)uit", item->ld_name);
                                    switch (inp_get_input(gbl_game->input) ) {
                                        case INP_KEY_ESCAPE: stop = true; break;
                                        case INP_KEY_APPLY:
                                        case INP_KEY_ALL:    pickup_all = true; break;
                                        case INP_KEY_YES:    pickup = true; break;
                                        case INP_KEY_NO:
                                        default: break;
                                    }
                                }

                                if (pickup || pickup_all) {
                                    item_list[item_list_sz++] = item;
                                }
                            }
                        }
                        else {
                            item_list[item_list_sz++] = inv_get_next_item(inv, item);
                        }

                        if ( (stop == true) || (item == NULL) ) {
                            if (item_list_sz > 0) {
                                has_action = ma_do_pickup(player, item_list, item_list_sz);
                            }
                            if (stop) System_msg("Stop.");
                            else System_msg("Done.");
                        }
                    }
                    else You(player, "see nothing there.");
                }
                break;
            case INP_KEY_INVENTORY:
                has_action = invwin_inventory(gbl_game->current_map, &gbl_game->player_data);
                update_screen();
                break;
            case INP_KEY_CHARACTER:
                character_window(); break;
            case INP_KEY_LOG:
                log_window(); break;
            case INP_KEY_HELP:
                help_window(); break;
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(gbl_game->current_map, player_pos); break;
            case INP_KEY_FIRE:
                has_action = mapwin_overlay_fire_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_THROW_ITEM:
                has_action = mapwin_overlay_throw_item_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_THROW:
                has_action = mapwin_overlay_throw_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_STAIRS_DOWN:
                break;
            case INP_KEY_STAIRS_UP:
                if (dm_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    bool exit = false;
                    if (qst_is_quest_done(gbl_game->player_data.quest, gbl_game->current_map) ) {
                        qst_process_quest_end(gbl_game->player_data.quest, gbl_game->current_map);
                        exit = true;
                    }
                    else {
                        Your(player, "quest is not finished, are you sure? (o)k/(c)ancel");
                        if ( (ch = inp_get_input(gbl_game->input) ) == INP_KEY_YES) {
                            exit = true;
                        }
                        else System_msg("Cancel.");
                    }

                    if (exit) {
                        has_action = gbl_game->player_data.exit_map = exit;
                        se_remove_all_non_permanent(player);
                        village_screen();
                    }
                } break;
            case INP_KEY_RELOAD:
                has_action = ma_do_reload_carried(player, NULL); break;
            case INP_KEY_WAIT:
                has_action = ma_do_idle(player); break; //ma_do_guard(player); break;
            case INP_KEY_UNLOAD:
                has_action = ma_do_unload(player, NULL); break;
            case INP_KEY_WEAPON_SETTING:
                if ( (player->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
                     (player->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ||
                     (player->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
                    wpn_ranged_next_rof_set(inv_get_item_from_location(player->inventory, INV_LOC_MAINHAND_WIELD) );
                }
                if ( (player->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
                     (player->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
                    wpn_ranged_next_rof_set(inv_get_item_from_location(player->inventory, INV_LOC_OFFHAND_WIELD) );
                }
                break;
            case INP_KEY_WEAPON_SELECT:
                msr_weapon_next_selection(player);
                break;

            case INP_KEY_QUIT:       gbl_game->running = false; break;

            case INP_KEY_UP_LEFT:    change_pos = cd_create(-1,-1); break;
            case INP_KEY_UP:         change_pos = cd_create( 0,-1); break;
            case INP_KEY_UP_RIGHT:   change_pos = cd_create( 1,-1); break;
            case INP_KEY_RIGHT:      change_pos = cd_create( 1, 0); break;
            case INP_KEY_DOWN_RIGHT: change_pos = cd_create( 1, 1); break;
            case INP_KEY_DOWN:       change_pos = cd_create( 0, 1); break;
            case INP_KEY_DOWN_LEFT:  change_pos = cd_create(-1, 1); break;
            case INP_KEY_LEFT:       change_pos = cd_create(-1, 0); break;

            case INP_KEY_RUN: {
                    has_action = true;
                    player_running_dir = cd_create(0,0);
                    System_msg("Run into which direction?");
                    switch (ch = inp_get_input(gbl_game->input) ) {
                        case INP_KEY_UP_LEFT:    player_running_dir = cd_create(-1,-1); break;
                        case INP_KEY_UP:         player_running_dir = cd_create( 0,-1); break;
                        case INP_KEY_UP_RIGHT:   player_running_dir = cd_create( 1,-1); break;
                        case INP_KEY_RIGHT:      player_running_dir = cd_create( 1, 0); break;
                        case INP_KEY_DOWN_RIGHT: player_running_dir = cd_create( 1, 1); break;
                        case INP_KEY_DOWN:       player_running_dir = cd_create( 0, 1); break;
                        case INP_KEY_DOWN_LEFT:  player_running_dir = cd_create(-1, 1); break;
                        case INP_KEY_LEFT:       player_running_dir = cd_create(-1, 0); break;
                        default: has_action = false; System_msg("Abort"); break;
                    }

                    if (has_action == true) {
                        player_running = true;
                        pos = player_running_dir;
                    }
                } break;

            case INP_KEY_REDRAW: update_screen(); lg_debug("redraw"); break;
            case INP_KEY_WIZARD: wz_mode(); break;
            default:
                break;
        }

        if (has_action == false) {
            pos = cd_add(&pos, &change_pos);
            if (cd_equal(&pos, player_pos) == false) {
                /* test for a move */
                if (ma_do_move(player, &pos) == true) {
                    has_action = true;
                }
                else {
                    has_action = ma_do_melee(player, &pos);
                }
            }
        }

        pos = player->pos;
    }

    return has_action;
}

