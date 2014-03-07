#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#include "heresyrl_def.h"
#include "ui.h"
#include "dungeon_creator.h"
#include "logging.h"
#include "monster.h"
#include "game.h"
#include "items.h"
#include "coord.h"
#include "tiles.h"
#include "inventory.h"
#include "input.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

//int main(int argc, char *argv[])
int main(void)
{
    int ch;
    struct itm_item *item;
    coord_t pos = cd_create(0,0);

    gbl_log = lg_init(LG_DEBUG_LEVEL_DEBUG, 10000);
 	srand(time(NULL));
    game_init(NULL, rand());

    initscr(); //  Start curses mode
    if (has_colors() == FALSE) exit(1);
    if (start_color() == ERR) exit(1);
    win_generate_colours();
    refresh(); //  Print it on to the real screen

    ui_create(COLS, LINES, &map_win, &char_win, &msg_win);

    cbreak();
    noecho();
    //timeout(1);
    keypad(stdscr, TRUE);

    game_init_map();

    game_new_turn();

    coord_t *player_pos = &gbl_game->player_data.player->pos;
    mapwin_display_map(map_win, gbl_game->current_map, player_pos);
    charwin_refresh(char_win, &gbl_game->player_data);

    do {
        pos = *player_pos;
    
        switch (ch) { 
            case INP_KEY_UP_LEFT:    pos.y--; pos.x--; break;
            case INP_KEY_UP:         pos.y--; break;
            case INP_KEY_UP_RIGHT:   pos.y--; pos.x++; break;
            case INP_KEY_RIGHT:      pos.x++; break;
            case INP_KEY_DOWN_RIGHT: pos.y++; pos.x++; break;
            case INP_KEY_DOWN:       pos.y++; break;
            case INP_KEY_DOWN_LEFT:  pos.y++; pos.x--; break;
            case INP_KEY_LEFT:       pos.x--; break;
            
            case INP_KEY_PICKUP: {
                    struct inv_inventory *inv = sd_get_map_me(player_pos, gbl_game->current_map)->inventory;
                    if ( (inv_inventory_size(inv) ) > 0) {
                        struct itm_item *prev = NULL;
                        item = NULL;
                        bool pickup_all = false;
                        bool stop = false;
                        while ( ( (item = inv_get_next_item(inv, item) ) != NULL) && (stop == false) ){
                            bool pickup = false;
                            if (pickup_all != true) {
                                lg_printf("Pickup %s? (y)es/(n)o/(a)ll/(q)uit", item->ld_name);
                                switch (inp_get_input() ) {
                                    case INP_KEY_ESCAPE: stop = false; break;
                                    case INP_KEY_ALL:    pickup_all = true; break;
                                    case INP_KEY_YES:    pickup = true; break;
                                    case INP_KEY_NO:
                                    default: break;
                                }
                            }

                            if (pickup || pickup_all) {
                                if (msr_give_item(gbl_game->player_data.player, item) == true) {
                                    inv_remove_item(inv, item);
                                    item = prev;
                                }
                            }
                            prev = item;
                        }
                        lg_printf("Done.");
                    }
                    else You("see nothing there.");
                }
                break;
            case INP_KEY_INVENTORY:
                invwin_inventory(map_win, char_win, gbl_game->current_map, &gbl_game->player_data);
                break;
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(map_win, char_win, gbl_game->current_map, player_pos);
                break;
            case INP_KEY_FIRE:
                mapwin_overlay_fire_cursor(map_win, &gbl_game->player_data, gbl_game->current_map, player_pos);
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
                if ( (gbl_game->player_data.weapon_selection == FGHT_WEAPON_SELECT_RIGHT_HAND) || 
                     (gbl_game->player_data.weapon_selection == FGHT_WEAPON_SELECT_DUAL_HAND) || 
                     (gbl_game->player_data.weapon_selection == FGHT_WEAPON_SELECT_BOTH_HAND) ) {
                    if (inv_loc_empty(gbl_game->player_data.player->inventory, INV_LOC_RIGHT_WIELD) == false) {
                        item = inv_get_item_from_location(gbl_game->player_data.player->inventory, INV_LOC_RIGHT_WIELD);
                        if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) {
                            do {
                                gbl_game->player_data.rof_setting_rhand++;
                                gbl_game->player_data.rof_setting_rhand %= WEAPON_ROF_SETTING_MAX;
                            } while (wpn_ranged_weapon_setting_check(item, gbl_game->player_data.rof_setting_rhand) == false);
                        }
                    }
                }
                if ( (gbl_game->player_data.weapon_selection == FGHT_WEAPON_SELECT_LEFT_HAND) || 
                     (gbl_game->player_data.weapon_selection == FGHT_WEAPON_SELECT_DUAL_HAND) ) {
                    if (inv_loc_empty(gbl_game->player_data.player->inventory, INV_LOC_LEFT_WIELD) == false) {
                        item = inv_get_item_from_location(gbl_game->player_data.player->inventory, INV_LOC_LEFT_WIELD);
                        if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) {
                            do {
                                gbl_game->player_data.rof_setting_lhand++;
                                gbl_game->player_data.rof_setting_lhand %= WEAPON_ROF_SETTING_MAX;
                            } while (wpn_ranged_weapon_setting_check(item, gbl_game->player_data.rof_setting_lhand) == false);
                        }
                    }
                }
                break;
            case INP_KEY_WEAPON_SELECT: 
                if ( (inv_loc_empty(gbl_game->player_data.player->inventory, INV_LOC_RIGHT_WIELD) == true) &&
                     (inv_loc_empty(gbl_game->player_data.player->inventory, INV_LOC_LEFT_WIELD) == true) ) break;

                do {
                    gbl_game->player_data.weapon_selection++;
                    gbl_game->player_data.weapon_selection %= FGHT_WEAPON_SELECT_MAX;
                } while (fght_weapons_check(gbl_game->player_data.player, gbl_game->player_data.weapon_selection) == false);
                break;
            default:
                break;
        }

        if (msr_move_monster(gbl_game->player_data.player, gbl_game->current_map, &pos) == false) {
        }

        game_new_turn();
        ui_create(COLS, LINES, &map_win, &char_win, &msg_win);
        charwin_refresh(char_win, &gbl_game->player_data);
        mapwin_display_map(map_win, gbl_game->current_map, player_pos);
    }
    while((ch = inp_get_input()) != INP_KEY_ESCAPE);

    game_exit();

    ui_destroy(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;

    clear();
    refresh();          //  Print it on to the real screen
    endwin();           //  End curses mode

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
