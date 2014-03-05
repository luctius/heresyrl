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
#include "items_static.h"
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
    coord_t pos = cd_create(0,0);

    gbl_log = lg_init(LG_DEBUG_LEVEL_DEBUG, 100);
 	srand(time(NULL));

    initscr(); /*  Start curses mode         */
    if (has_colors() == FALSE) exit(1);
    if (start_color() == ERR) exit(1);
    win_generate_colours();
    refresh(); /*  Print it on to the real screen */

    ui_create(COLS, LINES, &map_win, &char_win, &msg_win);

    cbreak();
    noecho();
    //timeout(1);
    keypad(stdscr, TRUE);

    game_init(NULL, rand());

    pos = game->player_data.player->pos;
    struct itm_item *item = itm_create_specific(ITEM_ID_AVERAGE_TORCH);
    if (item != NULL) itm_insert_item(item, game->current_map, &pos);
    item = itm_create_specific(ITEM_ID_AVERAGE_STUB_AUTOMATIC);
    if (item != NULL) itm_insert_item(item, game->current_map, &pos);

    game_new_turn();

    coord_t *player_pos = &game->player_data.player->pos;
    mapwin_display_map(map_win, game->current_map, player_pos);
    charwin_refresh(char_win, &game->player_data);

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
                    struct inv_inventory *inv = sd_get_map_me(player_pos, game->current_map)->inventory;
                    if ( (inv_inventory_size(inv) ) > 0) {
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
                                if (msr_give_item(game->player_data.player, item) == true) {
                                    inv_remove_item(inv, item);
                                }
                            }
                        }
                    }
                    else You("see nothing there.");
                }
                break;
            case INP_KEY_INVENTORY:
                invwin_inventory(map_win, char_win, game->current_map, &game->player_data);
                break;
            case INP_KEY_EXAMINE:
                mapwin_overlay_examine_cursor(map_win, char_win, game->current_map, player_pos);
                break;
            case INP_KEY_FIRE:
                mapwin_overlay_fire_cursor(map_win, &game->player_data, game->current_map, player_pos);
                break;
            case INP_KEY_STAIRS_DOWN:
                if (sd_get_map_tile(player_pos, game->current_map)->type == TILE_TYPE_STAIRS_DOWN) {
                    You("see a broken stairway.");
                }
                break;
            case INP_KEY_STAIRS_UP:
                if (sd_get_map_tile(player_pos, game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    You("see a broken stairway.");
                }
                break;
            case INP_KEY_RELOAD: break;
            case INP_KEY_WEAPON_SETTING: 
                if (inv_loc_empty(game->player_data.player->inventory, INV_LOC_RIGHT_WIELD) == false) {
                    game->player_data.rof_setting_rhand++;
                    /*check if valid here, else ++*/
                    game->player_data.rof_setting_rhand %= WPN_ROF_SETTING_MAX;
                }
                if (inv_loc_empty(game->player_data.player->inventory, INV_LOC_LEFT_WIELD) == false) {
                    game->player_data.rof_setting_lhand++;
                    /*check if valid here, else ++*/
                    game->player_data.rof_setting_lhand %= WPN_ROF_SETTING_MAX;
                }
                break;
            case INP_KEY_WEAPON_SELECT: 
                game->player_data.weapon_selection++;
                game->player_data.weapon_selection %= FGHT_WEAPON_SELECT_MAX;
                if (inv_loc_empty(game->player_data.player->inventory, INV_LOC_RIGHT_WIELD) == true) {
                    game->player_data.weapon_selection = FGHT_WEAPON_SELECT_LHAND;
                }
                if (inv_loc_empty(game->player_data.player->inventory, INV_LOC_LEFT_WIELD) == true) {
                    game->player_data.weapon_selection = FGHT_WEAPON_SELECT_RHAND;
                }
                break;
            default:
                lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "main", "key pressed: %d.", ch);
                break;
        }

        if (msr_move_monster(game->player_data.player, game->current_map, &pos) == false) {
        }

        game_new_turn();
        ui_create(COLS, LINES, &map_win, &char_win, &msg_win);
        charwin_refresh(char_win, &game->player_data);
        mapwin_display_map(map_win, game->current_map, player_pos);
    }
    while((ch = inp_get_input()) != INP_KEY_ESCAPE);

    ui_destroy(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    clear();
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
