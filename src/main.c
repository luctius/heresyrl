#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include "simple_dungeon.h"

#include "logging.h"
#include "map_display.h"
//#include "char_display.h"
//#include "msg_display.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

int main(int argc, char *argv[])
{
    int ch;
    int cols, lines;

    initscr();          /*  Start curses mode         */

    cols = COLS;
    lines = LINES;
    if ( (lines < 25) || (cols < 40) ) {
        endwin();           /*  End curses mode       */
        fprintf(stderr, "Terminal is too small, minimum is 40x25.\n");
        exit(1);
    }

    refresh();          /*  Print it on to the real screen */

    create_ui(cols, lines, &map_win, &char_win, &msg_win);
    struct logging *log = lg_init(LG_DEBUG_LEVEL_DEBUG, 100);
    lg_set_callback(log, msg_win, win_log_callback);


    int x = 120;
    int y = 100;
    struct sd_map *map = NULL;

    map = sd_alloc_map(x,y);

    sd_generate_map(map);

    int xpos = 0;
    int ypos = 0;

    cbreak();
    noecho();
    timeout(1);
    keypad(stdscr, TRUE);

    do {
        if (ch == KEY_UP && ypos > 0) { ypos--; lg_printf(log, "key up"); }
        if (ch == KEY_RIGHT && xpos < map->x_sz-1) { xpos++;  lg_printf(log, "key right"); }
        if (ch == KEY_DOWN && ypos < map->y_sz-1) { ypos++;  lg_printf(log, "key down"); }
        if (ch == KEY_LEFT && xpos > 0) { xpos--;  lg_printf(log, "key left"); }

        if (cols != COLS || lines != LINES) {
            cols = COLS;
            lines = LINES;
            create_ui(cols, lines, &map_win, &char_win, &msg_win);
        }

        SD_GET_INDEX(xpos, ypos, map).has_player = true;
        win_display_map(map_win, map, xpos, ypos);
        SD_GET_INDEX(xpos, ypos, map).has_player = false;
    }
    while((ch = getch()) != 27 && ch != 'q');

    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf(log, "Goodbye :) \n ");
    lg_exit(log);
    sd_free_map(map);

    return 0;
}
