#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include "simple_dungeon.h"

#include "map_display.h"
#include "char_display.h"
#include "msg_display.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

void create_ui(int cols, int lines) {
    if (map_win != NULL) win_destroy(map_win);
    if (char_win != NULL) win_destroy(char_win);
    if (msg_win != NULL) win_destroy(msg_win);
    cols -=1;
    lines -=1;

    /* Calculate 3 windows sizes */
    int map_cols = cols - CHAR_MIN_COLS;
    if (map_cols > MAP_MIN_COLS) map_cols *= MAP_COLS_FACTOR;
    if ( (map_cols > MAP_MAX_COLS) && (MAP_MAX_COLS != 0) ) map_cols = MAP_MAX_COLS;
    int map_lines = lines - MSG_MIN_LINES;
    if (map_lines > MAP_MIN_LINES) map_lines = (lines - MSG_MIN_LINES) * MAP_LINES_FACTOR;
    if ( (map_lines > MAP_MAX_LINES) && (MAP_MAX_LINES != 0) ) map_lines = MAP_MAX_LINES;

    int char_cols = cols - map_cols;
    if (char_cols < CHAR_MIN_COLS) char_cols = CHAR_MIN_COLS;
    if ( (char_cols > CHAR_MAX_COLS) && (CHAR_MAX_COLS != 0) ) char_cols = CHAR_MAX_COLS;
    int char_lines = lines * CHAR_LINES_FACTOR;
    if ( (char_lines > CHAR_MAX_LINES) && (CHAR_MAX_LINES != 0) ) char_lines = CHAR_MAX_LINES;

    int msg_cols = cols;
    if ( (msg_cols > MSG_MAX_COLS) && (MSG_MAX_COLS != 0) ) msg_cols = MSG_MAX_COLS;
    int msg_lines = lines - map_lines;
    if (msg_lines < MSG_MIN_LINES) msg_lines = MSG_MIN_LINES;
    if ( (msg_lines > MSG_MAX_LINES) && (MSG_MAX_LINES != 0) ) msg_lines = MSG_MAX_LINES;

    int total_lines = map_lines + msg_lines;
    if (total_lines < char_lines) total_lines = char_lines;
    int total_cols = map_cols + char_cols;
    if (total_cols < msg_cols) total_cols = msg_cols;

    if (total_lines > lines) { fprintf(stderr, "Too many lines used!\n"); exit(1); }
    if (total_cols > cols) { fprintf(stderr, "Too many cols used!\n"); exit(1); }

    map_win = win_create(map_lines, map_cols, 0, 0, HRL_WINDOW_TYPE_MAP);
    char_win = win_create(char_lines, char_cols, 0, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
    msg_win = win_create(msg_lines, msg_cols, map_lines+1, 0, HRL_WINDOW_TYPE_MESSAGE);
}

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

    printw("Hello World !!!\n");  /*  Print Hello World         */
    refresh();          /*  Print it on to the real screen */

    create_ui(cols, lines);

    int x = 120;
    int y = 50;
    struct sd_map *map = NULL;

    map = sd_alloc_map(x,y);

    sd_generate_map(map);

    int xpos = 0;
    int ypos = 0;
    win_display_map(map_win, map, 100, 50);

    cbreak();
    keypad(stdscr, TRUE);
    while((ch = getch()) != KEY_F(1)) {
        if (ch == KEY_UP && ypos > 0) ypos--;
        if (ch == KEY_RIGHT && xpos < map->x_sz-1) xpos++;
        if (ch == KEY_DOWN && ypos < map->y_sz-1) ypos++;
        if (ch == KEY_LEFT && xpos > 0) xpos--;

        if (cols != COLS || lines != LINES) {
            cols = COLS;
            lines = LINES;
            create_ui(cols, lines);
        }

        SD_GET_INDEX(xpos, ypos, map).has_player = true;
        win_display_map(map_win, map, xpos, ypos);
        SD_GET_INDEX(xpos, ypos, map).has_player = false;
    }

    endwin();           /*  End curses mode       */

    sd_free_map(map);

    return 0;
}
