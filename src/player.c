#include <unistd.h>
#include <ncurses.h>

#include "enums.h"
#include "player.h"
#include "fight.h"
#include "input.h"
#include "inventory.h"
#include "fov/sight.h"
#include "game.h"
#include "random.h"
#include "tiles.h"
#include "ui/ui.h"
#include "ai/ai_utils.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "monster/monster_static.h"
#include "monster/monster_action.h"
#include "items/items.h"
#include "careers/careers.h"

static bool plr_action_loop(struct msr_monster *player);

static int starting_wounds[MSR_RACE_MAX][4]= {
                    /*     <=3, <=6, <=9, ==10*/
    [MSR_RACE_DWARF]   = {  11,  12,  13,   14,},
    [MSR_RACE_ELF]     = {   9,  10,  11,   12,},
    [MSR_RACE_HALFLING]= {   8,   9,  10,   11,},
    [MSR_RACE_HUMAN]   = {  10,  11,  12,   13,},
};

static int gen_starting_wnds(enum msr_race race, int roll_d10) {
    if (roll_d10 <= 3)  return starting_wounds[race][0];
    if (roll_d10 <= 6)  return starting_wounds[race][1];
    if (roll_d10 <= 9)  return starting_wounds[race][2];
    if (roll_d10 <= 10) return starting_wounds[race][3];
    return 0;
}

static int starting_fatepoints[MSR_RACE_MAX][3]= {
                    /*     <=4, <=7, <=10*/
    [MSR_RACE_DWARF]   = {   1,   2,   3, },
    [MSR_RACE_ELF]     = {   1,   2,   2, },
    [MSR_RACE_HALFLING]= {   2,   2,   3, },
    [MSR_RACE_HUMAN]   = {   2,   3,   3, },
};

static int gen_starting_fp(enum msr_race race, int roll_d10) {
    if (roll_d10 <= 4)  return starting_fatepoints[race][0];
    if (roll_d10 <= 7)  return starting_fatepoints[race][1];
    if (roll_d10 <= 10) return starting_fatepoints[race][2];
    return 0;
}

static enum msr_talents random_talent[]=
{ TLT_ACUTE_SIGHT, TLT_AMBIDEXTROUS, TLT_COOL_HEADED, TLT_EXCELENT_VISION, TLT_FLEET_FOOTED, 
  TLT_HARDY, TLT_LIGHTNING_REFLEXES, TLT_LUCKY, TLT_MARKSMAN, TLT_NIGHT_VISION, TLT_RESITANCE_TO_MAGIC, 
  TLT_RESITANCE_TO_DISEASE, TLT_RESITANCE_TO_POISON, TLT_SAVVY, TLT_SIXTH_SENSE, TLT_STRONG_MINDED, 
  TLT_STURDY, TLT_VERY_RESILIENT, TLT_VERY_STRONG, TLT_WARRIOR_BORN,};

void plr_create(struct pl_player *plr, char *name, uint32_t template_id, enum msr_gender gender) {
    if (plr->player != NULL) {
        msr_destroy(plr->player, NULL);
        plr->player = NULL;
    }

    plr->player = msr_create(template_id);
    struct msr_monster *player = plr->player;

    player->unique_name = name;
    player->gender      = gender;
    player->is_player   = true;

    player->characteristic[MSR_CHAR_WEAPON_SKILL].base_value   += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_STRENGTH].base_value       += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_TOUGHNESS].base_value      += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_AGILITY].base_value        += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_INTELLIGENCE].base_value   += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_WILLPOWER].base_value      += random_xd10(gbl_game->random, 2);
    player->characteristic[MSR_CHAR_PERCEPTION].base_value     += random_xd10(gbl_game->random, 2);
    player->max_wounds    = gen_starting_wnds(player->race, random_xd10(gbl_game->random, 1) );
    player->cur_wounds    = player->max_wounds;
    player->fate_points   = gen_starting_fp(player->race, random_xd10(gbl_game->random, 1) );

    /* give humans and halflings a random talent */
    if (player->race == MSR_RACE_HALFLING || player->race == MSR_RACE_HUMAN) {
        int talents = 1;
        if (player->race == MSR_RACE_HUMAN) talents = 3;

        int trys = 10;
        for (int i = 0; i < trys; i++) {
            int ri = random_int32(gbl_game->random) % ARRAY_SZ(random_talent);
            enum msr_talents t = random_talent[ri];
            if (msr_set_talent(player, t) == true) talents--;
            if (talents == 0) break;
        }
    }

    plr->career = cr_get_career_by_id(cr_spawn(random_float(gbl_game->random), player->race) );
    cr_give_trappings_to_player(plr->career, player);

    plr->player_map_pos = cd_create(0,0);
    plr->xp_spend = 0;
    plr->xp_current = 300;

    System_msg("Welcome %s %s.", plr->career->title, player->unique_name);
}

void plr_init(struct pl_player *plr) {
    struct monster_controller mc = {
        .ai = {
            .ai_ctx = plr,
        },
        .controller_cb = plr_action_loop,
    };

    if (plr->player != NULL) {
        msr_assign_controller(plr->player, &mc);

        plr->player->icon = '@';
        plr->player->icon_attr = get_colour(TERM_COLOUR_WHITE);
        plr->player->faction = 0;
    }
}

struct pf_context *plr_map(struct pl_player *plr, struct dm_map *map) {
    if (cd_equal(&plr->player_map_pos, & plr->player->pos) == false) {
        if (aiu_generate_dijkstra(&plr->player_map, map, &plr->player->pos, 0) == true) {
            plr->player_map_pos = plr->player->pos;
        }
    }
    return plr->player_map;
}

static bool low_wounds_warning = false;
static bool critical_wounds_warning = false;
static bool plr_action_loop(struct msr_monster *player) {
    if (player == NULL) return false;
    struct dm_map *map = gbl_game->current_map;
    int ch;
    bool has_action = false;

    coord_t zero = cd_create(0,0);
    dm_clear_map_visibility(map, &zero, &map->size);
    sgt_calculate_all_light_sources(map);
    sgt_calculate_player_sight(map, player);

    coord_t pos = player->pos;
    coord_t *player_pos = &player->pos;

    if (player->dead) {
        if (player->fate_points > 0) {
            player->fate_points -= 1;
            player->dead = false;

            msr_remove_monster(player, map);
            if (dm_tile_instance(map, TILE_TYPE_STAIRS_DOWN, 0, &pos) == false) exit(1);
            if (cd_equal(player_pos, &pos) == false) {
                if (msr_insert_monster(player, map, &pos) == false) exit(1);
            }
            clear();
            refresh();

            se_add_status_effect(player, SEID_FATEHEALTH);
            se_remove_all_non_permanent(player);

            You(player, "would have died if fate did not intervene...");

            return true;
        }
    }
    
    if (player->dead) {
        gbl_game->running = false;
        usleep(40000);
        return true;
    }

    if ( ( (player->cur_wounds * 100) / player->max_wounds) < 10) {
        if (critical_wounds_warning == false) {
            Warning("hitpoints critical.");
            critical_wounds_warning = true;
        }
    }
    else if ( ( (player->cur_wounds * 100) / player->max_wounds) < 50) {
        if (low_wounds_warning == false) {
            Warning("low hitpoints.");
            low_wounds_warning = true;
        }
        critical_wounds_warning = false;
    }
    else if ( ( (player->cur_wounds * 100) / player->max_wounds) > 50) {
        low_wounds_warning = false;
    }

    lg_debug("plr_action_loop");
    while (gbl_game->running && (has_action == false) ) {
        mapwin_display_map(map, player_pos);
        charwin_refresh();

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
                has_action = invwin_inventory(gbl_game->current_map, &gbl_game->player_data); break;
            case INP_KEY_CHARACTER:
                character_window(); break;
            case INP_KEY_LOG:
                log_window(); break;
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(gbl_game->current_map, player_pos); break;
            case INP_KEY_FIRE:
                has_action = mapwin_overlay_fire_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_THROW:
                has_action = mapwin_overlay_throw_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_STAIRS_DOWN:
                if (dm_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_DOWN) {
                    You(player, "win."); 
                    gbl_game->running = false;
                } 
                break;
            case INP_KEY_STAIRS_UP:
                if (dm_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    You(player, "see a broken stairway."); } break;
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

            case INP_KEY_UP_LEFT:    pos.y--; pos.x--; break;
            case INP_KEY_UP:         pos.y--; break;
            case INP_KEY_UP_RIGHT:   pos.y--; pos.x++; break;
            case INP_KEY_RIGHT:      pos.x++; break;
            case INP_KEY_DOWN_RIGHT: pos.y++; pos.x++; break;
            case INP_KEY_DOWN:       pos.y++; break;
            case INP_KEY_DOWN_LEFT:  pos.y++; pos.x--; break;
            case INP_KEY_LEFT:       pos.x--; break;
            
            default:
                break;
        }

        if (has_action == false) {
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

    dm_clear_map_visibility(map, &zero, &map->size);
    sgt_calculate_all_light_sources(map);
    sgt_calculate_player_sight(map, player);

    return has_action;
}
