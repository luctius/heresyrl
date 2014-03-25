#include <ncurses.h>

#include "player.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "monster_static.h"
#include "items.h"
#include "fight.h"
#include "inventory.h"
#include "ui.h"
#include "sight.h"
#include "input.h"
#include "game.h"
#include "monster_action.h"
#include "tiles.h"
#include "ai_utils.h"
#include "spawn.h"

static bool plr_action_loop(struct msr_monster *player, void *controller);

struct spwn_monster_item items[] = {  
    {.id=ITEM_ID_FLAK_JACKET,.min=1,.max=1,.wear=true}, 
    {.id=ITEM_ID_LAS_GUN,.min=1,.max=1,.wear=true}, 
    {.id=ITEM_ID_BASIC_AMMO_LAS,.min=1,.max=3,.wear=false},
    {0,0,0,0,} };

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    struct monster_controller mc = {
        .controller_ctx = plr,
        .controller_cb = plr_action_loop,
    };

    if (plr->player == NULL) {
        plr->name = name;
        plr->player = msr_create(MSR_ID_BASIC_FERAL);
        plr->player->unique_name = name;

        int i = 0;
        while (items[i].max != 0) {
            spwn_add_item_to_monster(plr->player, &items[i], gbl_game->spawn_random);
            i++;
        }
    }

    msr_assign_controller(plr->player, &mc);

    plr->player->is_player = true;
    plr->player->icon = '@';
    plr->player->icon_attr = get_colour(TERM_COLOUR_WHITE);
    plr->player->faction = 0;
    plr->player_map_pos = cd_create(0,0);
    plr->player->characteristic[MSR_CHAR_PERCEPTION].base_value = 80;
    plr->player->characteristic[MSR_CHAR_AGILITY].base_value = 40;
    plr->player->characteristic[MSR_CHAR_TOUGHNESS].base_value = 40;
}

struct pf_context *plr_map(struct pl_player *plr, struct dc_map *map) {
    if (cd_equal(&plr->player_map_pos, & plr->player->pos) == false) {
        if (aiu_generate_dijkstra(&plr->player_map, map, &plr->player->pos, 0) == true) {
            plr->player_map_pos = plr->player->pos;
        }
    }
    return plr->player_map;
}

static bool plr_action_loop(struct msr_monster *player, void *controller) {
    if (player == NULL) return false;
    if (controller == NULL) return false;
    struct dc_map *map = gbl_game->current_map;
    struct pl_player *plr = controller;
    int ch;
    bool has_action = false;

    coord_t zero = cd_create(0,0);
    //dc_clear_map_visibility(map, &zero, &map->size);
    sgt_calculate_all_light_sources(gbl_game->sight, map);
    sgt_calculate_player_sight(gbl_game->sight, map, player);

    coord_t pos = player->pos;
    coord_t *player_pos = &player->pos;

    if (player->dead) {
        gbl_game->running = false;
    }

    lg_debug("plr_action_loop");
    while (gbl_game->running && (has_action == false) ) {
        mapwin_display_map(map, player_pos);
        charwin_refresh();

        switch (ch = inp_get_input() ) { 
            case INP_KEY_PICKUP: {
                    struct inv_inventory *inv = sd_get_map_me(&pos, map)->inventory;
                    if ( (inv_inventory_size(inv) ) > 0) {
                        struct itm_item *item = NULL;
                        struct itm_item *item_list[inv_inventory_size(inv)];
                        int item_list_sz = 0;
                        bool stop = false;
                        bool pickup_all = false;

                        while ( ( (item = inv_get_next_item(inv, item) ) != NULL) && (stop == false) ){
                            bool pickup = false;

                            if (pickup_all == false) {
                                System_msg("Pickup %s? (Y)es/(N)o/(A)ll/(q)uit", item->ld_name);
                                switch (inp_get_input() ) {
                                    case INP_KEY_ESCAPE: stop = true; break;
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
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(gbl_game->current_map, player_pos); break;
            case INP_KEY_FIRE:
                has_action = mapwin_overlay_fire_cursor(gbl_game, gbl_game->current_map, player_pos); break;
            case INP_KEY_STAIRS_DOWN:
                if (sd_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_DOWN) {
                    You(player, "see a broken stairway."); } break;
            case INP_KEY_STAIRS_UP:
                if (sd_get_map_tile(player_pos, gbl_game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    You(player, "see a broken stairway."); } break;
            case INP_KEY_RELOAD: 
                has_action = ma_do_reload_carried(player, NULL); break;
            case INP_KEY_WAIT: 
                has_action = ma_do_guard(player); break;
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

    dc_clear_map_visibility(map, &zero, &map->size);
    sgt_calculate_all_light_sources(gbl_game->sight, map);
    sgt_calculate_player_sight(gbl_game->sight, map, player);

    return has_action;
}
