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
    struct itm_item *item = itm_create_specific(0);
    if (item != NULL) itm_insert_item(item, game->current_map, &pos);

    game_new_turn();

    coord_t *player_pos = &game->player_data.player->pos;
    mapwin_display_map(map_win, game->current_map, player_pos);
    charwin_refresh(char_win, &game->player_data);

    do {
        pos = *player_pos;
    
        switch (ch) { 
            case KEY_HOME: pos.y--; pos.x--; break;
            case KEY_UP: pos.y--; break;
            case KEY_NPAGE: pos.y--; pos.x++; break;
            case KEY_RIGHT: pos.x++; break;
            case KEY_PPAGE: pos.y++; pos.x++; break;
            case KEY_DOWN: pos.y++; break;
            case KEY_END: pos.y++; pos.x--; break;
            case KEY_LEFT: pos.x--; break;
            
            case 'g':
                if ( (item = sd_get_map_me(player_pos, game->current_map)->item) != NULL ) {
                    if (msr_give_item(game->player_data.player, item) == true) {
                        sd_get_map_me(player_pos, game->current_map)->item = NULL;
                    }
                }
                else You("see nothing there.");
                break;
            case 'i':
                invwin_inventory(map_win, char_win, game->current_map, &game->player_data);
                break;
            case 'x':
                mapwin_overlay_examine_cursor(map_win, char_win, game->current_map, player_pos);
                break;
            case 'f':
                mapwin_overlay_fire_cursor(map_win, &game->player_data, game->current_map, player_pos);
                break;
            case '<':
                if (sd_get_map_tile(player_pos, game->current_map)->type == TILE_TYPE_STAIRS_DOWN) {
                    You("see a broken stairway.");
                }
                break;
            case '>':
                if (sd_get_map_tile(player_pos, game->current_map)->type == TILE_TYPE_STAIRS_UP) {
                    You("see a broken stairway.");
                }
                break;
            default:
                lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "main", "key pressed: %d.", ch);
                break;
        }

        msr_move_monster(game->player_data.player, game->current_map, &pos);

        game_new_turn();
        ui_create(COLS, LINES, &map_win, &char_win, &msg_win);
        charwin_refresh(char_win, &game->player_data);
        mapwin_display_map(map_win, game->current_map, player_pos);
    }
    while((ch = getch()) != 27 && ch != 'q');

    ui_destroy(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    clear();
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
