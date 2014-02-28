#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#include "heresyrl_def.h"
#include "map_display.h"
#include "logging.h"
#include "monster.h"
#include "game.h"
#include "items.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

//int main(int argc, char *argv[])
int main(void)
{
    int ch;
    int xpos = 0;
    int ypos = 0;

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

    xpos = game->player_data.player->x_pos;
    ypos = game->player_data.player->y_pos;
    struct itm_items *item = itm_create_specific(0);
    if (item != NULL) itm_insert_item(item, game->current_map, xpos, ypos);

    game_new_turn();
    do {
        int new_xpos = xpos;
        int new_ypos = ypos;
    
        if (ch == KEY_UP) { new_ypos--; }
        if (ch == KEY_RIGHT) { new_xpos++; }
        if (ch == KEY_DOWN) { new_ypos++; }
        if (ch == KEY_LEFT) { new_xpos--; }
        if (ch == 'g') {
            if ( (item = SD_GET_INDEX(game->player_data.player->x_pos, game->player_data.player->y_pos, game->current_map).item) != NULL ) {
                if (msr_give_item(game->player_data.player, item) == true) {
                    SD_GET_INDEX(game->player_data.player->x_pos, game->player_data.player->y_pos, game->current_map).item = NULL;
                }
            }
            else lg_printf("There is nothing there");
        }
        if (ch == 'u') {
            msr_use_item(game->player_data.player, game->player_data.player->inventory);
        }
        if (ch == 'd') {
            item = game->player_data.player->inventory;
            msr_remove_item(game->player_data.player, item);
            itm_insert_item(item, game->current_map, 
                    game->player_data.player->x_pos,game->player_data.player->y_pos);
        }

        if (msr_move_monster(game->player_data.player, game->current_map, new_xpos, new_ypos) == true) {
            xpos = new_xpos;
            ypos = new_ypos;
        }

        game_new_turn();
        create_ui(COLS, LINES, &map_win, &char_win, &msg_win);
        win_display_map(map_win, game->current_map, xpos, ypos);
    }
    while((ch = getch()) != 27 && ch != 'q');

    destroy_ui(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    //clear();
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
