#include <ncurses.h>
#include "player.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "monster_static.h"
#include "items.h"
#include "fight.h"
#include "inventory.h"
#include "ui.h"
#include "input.h"
#include "game.h"
#include "monster_turn.h"
#include "tiles.h"

static bool plr_action_loop(struct gm_game *g);

static bool plr_action_done_callback(struct msr_monster *player, void *controller) {
    struct gm_game *g = controller;
    plr_action_loop(g);
    return true;
}

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    if (plr->player == NULL) {
        plr->name = name;
        plr->player = msr_create(MSR_ID_BASIC_FERAL);
    }

    plr->player->icon = '@';
    plr->player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;
    plr->player->faction = 0;

    plr->player->energy = MT_ENERGY_FULL;
    mt_do_guard(plr->player, plr_action_done_callback, gbl_game);
}

bool plr_action_loop(struct gm_game *g) {
    struct msr_monster *player = g->player_data.player;
    struct dc_map *map = g->current_map;
    int ch;
    bool has_action = false;

    coord_t pos = player->pos;
    coord_t *player_pos = &player->pos;

    while ( (has_action == false) && (g->running) ) {
        switch (ch = inp_get_input() ) { 
            case INP_KEY_QUIT:       g->running = false; break;

            case INP_KEY_UP_LEFT:    pos.y--; pos.x--; break;
            case INP_KEY_UP:         pos.y--; break;
            case INP_KEY_UP_RIGHT:   pos.y--; pos.x++; break;
            case INP_KEY_RIGHT:      pos.x++; break;
            case INP_KEY_DOWN_RIGHT: pos.y++; pos.x++; break;
            case INP_KEY_DOWN:       pos.y++; break;
            case INP_KEY_DOWN_LEFT:  pos.y++; pos.x--; break;
            case INP_KEY_LEFT:       pos.x--; break;
            
            case INP_KEY_PICKUP: {
                    struct inv_inventory *inv = sd_get_map_me(&pos, map)->inventory;
                    if ( (inv_inventory_size(inv) ) > 0) {
                        int nr_picked = 0;
                        struct itm_item *item = NULL;
                        struct itm_item *prev = NULL;
                        item = NULL;
                        bool pickup_all = false;
                        bool stop = false;
                        while ( ( (item = inv_get_next_item(inv, item) ) != NULL) && (stop == false) ){
                            bool pickup = false;
                            if (pickup_all != true) {
                                lg_printf("Pickup %s? (Y)es/(N)o/(A)ll/(q)uit", item->ld_name);
                                switch (inp_get_input() ) {
                                    case INP_KEY_ESCAPE: stop = false; break;
                                    case INP_KEY_ALL:    pickup_all = true; break;
                                    case INP_KEY_YES:    pickup = true; break;
                                    case INP_KEY_NO:
                                    default: break;
                                }
                            }

                            if (pickup || pickup_all) {
                                if (msr_give_item(player, item) == true) {
                                    inv_remove_item(inv, item);
                                    item = prev;
                                    nr_picked++;
                                }
                            }
                            prev = item;
                        }
                        lg_printf("Done.");

                        if (nr_picked > 0) {
                        }
                    }
                    else You("see nothing there.");
                }
                break;
            case INP_KEY_INVENTORY:
                invwin_inventory(gbl_game->current_map, &gbl_game->player_data);
                break;
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(gbl_game->current_map, player_pos);
                break;
            case INP_KEY_FIRE:
                mapwin_overlay_fire_cursor(gbl_game, gbl_game->current_map, player_pos);
                break;
            case INP_KEY_STAIRS_DOWN:
                if (sd_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_DOWN) {
                    You("see a broken stairway.");
                }
                break;
            case INP_KEY_STAIRS_UP:
                if (sd_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    You("see a broken stairway.");
                }
                break;
            case INP_KEY_RELOAD: break;
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
            default:
                break;
        }

        if (mt_do_move(player, &pos, plr_action_done_callback, gbl_game) == true) {
            has_action = true;
        }
    }
    return true;
}
