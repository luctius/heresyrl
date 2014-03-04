#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#include "heresyrl_def.h"
#include "ui.h"
#include "logging.h"
#include "monster.h"
#include "game.h"
#include "items.h"
#include "coord.h"

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

    create_ui(COLS, LINES, &map_win, &char_win, &msg_win);

    cbreak();
    noecho();
    //timeout(1);
    keypad(stdscr, TRUE);

    game_init(rand());

    pos = game->player_data.player->pos;
    struct itm_items *item = itm_create_specific(0);
    if (item != NULL) itm_insert_item(item, game->current_map, &pos);

    game_new_turn();

    coord_t *player_pos = &game->player_data.player->pos;
    do {
        pos = *player_pos;
    
        switch (ch) { 
            case KEY_UP: pos.y--; break;
            case KEY_DOWN: pos.y++; break;
            case KEY_LEFT: pos.x--; break;
            case KEY_RIGHT: pos.x++; break;
            
            case 'g':
                if ( (item = sd_get_map_me(player_pos, game->current_map)->item) != NULL ) {
                    if (msr_give_item(game->player_data.player, item) == true) {
                        sd_get_map_me(player_pos, game->current_map)->item = NULL;
                    }
                }
                else You("see nothing there.");
                break;
            case 'u':
                msr_use_item(game->player_data.player, game->player_data.player->inventory);
                break;
            case 'd':
                item = game->player_data.player->inventory;
                msr_remove_item(game->player_data.player, item);
                itm_insert_item(item, game->current_map, player_pos);
                break;
            case 'x':
                win_overlay_examine_cursor(map_win, game->current_map, player_pos);
                break;
            case 'f':
                win_overlay_fire_cursor(map_win, game->current_map, player_pos);
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
                break;
        }

        msr_move_monster(game->player_data.player, game->current_map, &pos);

        game_new_turn();
        create_ui(COLS, LINES, &map_win, &char_win, &msg_win);
        win_display_map(map_win, game->current_map, player_pos);
    }
    while((ch = getch()) != 27 && ch != 'q');

    destroy_ui(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    clear();
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
