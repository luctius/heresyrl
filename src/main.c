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
    int x = 120;
    int y = 100;
    struct sd_map *map = NULL;
    int xpos = 0;
    int ypos = 0;

    gbl_log = lg_init(LG_DEBUG_LEVEL_DEBUG, 100);
    map = sd_alloc_map(x,y);
    sd_generate_map(map);

    initscr(); /*  Start curses mode         */
    refresh(); /*  Print it on to the real screen */
    create_ui(COLS, LINES, &map_win, &char_win, &msg_win);

    cbreak();
    noecho();
    timeout(1);
    keypad(stdscr, TRUE);

    do {
        if (ch == KEY_UP && ypos > 0) { ypos--; lg_printf("key up"); }
        if (ch == KEY_RIGHT && xpos < map->x_sz-1) { xpos++; lg_printf("key right"); }
        if (ch == KEY_DOWN && ypos < map->y_sz-1) { ypos++; lg_printf("key down"); }
        if (ch == KEY_LEFT && xpos > 0) { xpos--; lg_printf("key left"); }

        create_ui(COLS, LINES, &map_win, &char_win, &msg_win);

        SD_GET_INDEX(xpos, ypos, map).has_player = true;
        win_display_map(map_win, map, xpos, ypos);
        SD_GET_INDEX(xpos, ypos, map).has_player = false;
    }
    while((ch = getch()) != 27 && ch != 'q');

    destroy_ui(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    lg_set_callback(gbl_log, NULL, NULL);
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    sd_free_map(map);

    return 0;
}
