#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>

#include "map_display.h"
#include "tiles.h"
#include "monster.h"

static bool generated_colours = false;

struct hrl_window {
    WINDOW *win;
    int cols;
    int lines;
    int y;
    int x;
    enum window_type type;
};

static void win_generate_colours(void) {
    if (generated_colours == false) {
        generated_colours = true;
        int i = 1;

        lg_printf("generating colours");

        start_color();
        init_pair(i++, COLOR_WHITE, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_NORMAL);
        init_pair(i++, COLOR_RED, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_RED);
        init_pair(i++, COLOR_GREEN, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_GREEN);
        init_pair(i++, COLOR_YELLOW, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_YELLOW);
        init_pair(i++, COLOR_BLUE, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_BLUE);
        init_pair(i++, COLOR_MAGENTA, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_MAGENTA);
        init_pair(i++, COLOR_CYAN, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_FG_CYAN);

        init_pair(i++, COLOR_BLACK, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_INVERSE);
        init_pair(i++, COLOR_RED, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_RED);
        init_pair(i++, COLOR_GREEN, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_GREEN);
        init_pair(i++, COLOR_YELLOW, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_YELLOW);
        init_pair(i++, COLOR_BLUE, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_BLUE);
        init_pair(i++, COLOR_MAGENTA, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_MAGENTA);
        init_pair(i++, COLOR_CYAN, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_FGW_CYAN);

        init_pair(i++, COLOR_BLACK, COLOR_RED);
        assert(i-1 == DPL_COLOUR_BGB_RED);
        init_pair(i++, COLOR_BLACK, COLOR_GREEN);
        assert(i-1 == DPL_COLOUR_BGB_GREEN);
        init_pair(i++, COLOR_BLACK, COLOR_YELLOW);
        assert(i-1 == DPL_COLOUR_BGB_YELLOW);
        init_pair(i++, COLOR_BLACK, COLOR_BLUE);
        assert(i-1 == DPL_COLOUR_BGB_BLUE);
        init_pair(i++, COLOR_BLACK, COLOR_MAGENTA);
        assert(i-1 == DPL_COLOUR_BGB_MAGENTA);
        init_pair(i++, COLOR_BLACK, COLOR_CYAN);
        assert(i-1 == DPL_COLOUR_BGB_CYAN);

        init_pair(i++, COLOR_WHITE, COLOR_RED);
        assert(i-1 == DPL_COLOUR_BGW_RED);
        init_pair(i++, COLOR_WHITE, COLOR_GREEN);
        assert(i-1 == DPL_COLOUR_BGW_GREEN);
        init_pair(i++, COLOR_WHITE, COLOR_YELLOW);
        assert(i-1 == DPL_COLOUR_BGW_YELLOW);
        init_pair(i++, COLOR_WHITE, COLOR_BLUE);
        assert(i-1 == DPL_COLOUR_BGW_BLUE);
        init_pair(i++, COLOR_WHITE, COLOR_MAGENTA);
        assert(i-1 == DPL_COLOUR_BGW_MAGENTA);
        init_pair(i++, COLOR_WHITE, COLOR_CYAN);
        assert(i-1 == DPL_COLOUR_BGW_CYAN);

        init_pair(i++, COLOR_RED, COLOR_RED);
        assert(i-1 == DPL_COLOUR_ALL_RED);
        init_pair(i++, COLOR_GREEN, COLOR_GREEN);
        assert(i-1 == DPL_COLOUR_ALL_GREEN);
        init_pair(i++, COLOR_YELLOW, COLOR_YELLOW);
        assert(i-1 == DPL_COLOUR_ALL_YELLOW);
        init_pair(i++, COLOR_BLUE, COLOR_BLUE);
        assert(i-1 == DPL_COLOUR_ALL_BLUE);
        init_pair(i++, COLOR_MAGENTA, COLOR_MAGENTA);
        assert(i-1 == DPL_COLOUR_ALL_MAGENTA);
        init_pair(i++, COLOR_CYAN, COLOR_CYAN);
        assert(i-1 == DPL_COLOUR_ALL_CYAN);
        init_pair(i++, COLOR_BLACK, COLOR_BLACK);
        assert(i-1 == DPL_COLOUR_ALL_BLACK);
        init_pair(i++, COLOR_WHITE, COLOR_WHITE);
        assert(i-1 == DPL_COLOUR_ALL_WHITE);
    }
}

static int hdr_lines = 0;
static int hdr_cols = 0;
bool create_ui(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win) {
    if ( (map_win != NULL) && (char_win != NULL) && (msg_win != NULL)  )
    {
        if ( (hdr_lines != lines) || (hdr_cols != cols) ) {
            hdr_lines = lines;
            hdr_cols = cols;

            if ( (lines < 25) || (cols < 40) ) {
                endwin();           /*  End curses mode       */
                fprintf(stderr, "Terminal is too small, minimum is 40x25, this terminal is %dx%d.\n", cols, lines);
                exit(1);
            }

            /* Calculate 3 windows sizes */
            int map_cols = cols - CHAR_MIN_COLS;
            if (map_cols > MAP_MIN_COLS) map_cols *= MAP_COLS_FACTOR;
            if ( (map_cols > MAP_MAX_COLS) && (MAP_MAX_COLS != 0) ) map_cols = MAP_MAX_COLS;
            int map_lines = (lines -1) - MSG_MIN_LINES;
            if (map_lines > MAP_MIN_LINES) map_lines = (lines - MSG_MIN_LINES) * MAP_LINES_FACTOR;
            if ( (map_lines > MAP_MAX_LINES) && (MAP_MAX_LINES != 0) ) map_lines = MAP_MAX_LINES;

            int char_cols = cols - map_cols;
            if (char_cols < CHAR_MIN_COLS) char_cols = CHAR_MIN_COLS;
            if ( (char_cols > CHAR_MAX_COLS) && (CHAR_MAX_COLS != 0) ) char_cols = CHAR_MAX_COLS;
            int char_lines = lines * CHAR_LINES_FACTOR;
            if ( (char_lines > CHAR_MAX_LINES) && (CHAR_MAX_LINES != 0) ) char_lines = CHAR_MAX_LINES;

            int msg_cols = cols;
            if ( (msg_cols > MSG_MAX_COLS) && (MSG_MAX_COLS != 0) ) msg_cols = MSG_MAX_COLS;
            int msg_lines = (lines -1) - map_lines;
            if (msg_lines < MSG_MIN_LINES) msg_lines = MSG_MIN_LINES;
            if ( (msg_lines > MSG_MAX_LINES) && (MSG_MAX_LINES != 0) ) msg_lines = MSG_MAX_LINES;

            int total_lines = map_lines + msg_lines;
            if (total_lines < char_lines) total_lines = char_lines;
            int total_cols = map_cols + char_cols;
            if (total_cols < msg_cols) total_cols = msg_cols;

            if (total_lines > lines) { fprintf(stderr, "Too many lines used!\n"); exit(1); }
            if (total_cols > cols) { fprintf(stderr, "Too many cols used!\n"); exit(1); }

            if ( (*map_win == NULL) || (*char_win == NULL) ||(*msg_win == NULL)  ) {
                *map_win = win_create(map_lines, map_cols, 0, 0, HRL_WINDOW_TYPE_MAP);
                *char_win = win_create(char_lines, char_cols, 0, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
                *msg_win = win_create(msg_lines, msg_cols, map_lines+1, 0, HRL_WINDOW_TYPE_MESSAGE);
                lg_set_callback(gbl_log, *msg_win, win_log_callback);
                win_log_refresh(*msg_win, gbl_log);
                return true;
            }
            else {
                lg_set_callback(gbl_log, NULL, NULL);
                destroy_ui(*map_win, *char_win, *msg_win);
                *map_win = NULL;
                *char_win = NULL;
                *msg_win = NULL;
                hdr_lines = 0;
                hdr_cols = 0;
                return create_ui(cols, lines, map_win, char_win, msg_win);
            }
        }
    }
    return false;
}

void destroy_ui(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win) {
    lg_set_callback(gbl_log, NULL, NULL);
    if (map_win != NULL) win_destroy(map_win);
    if (char_win != NULL) win_destroy(char_win);
    if (msg_win != NULL) win_destroy(msg_win);
}

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = malloc(sizeof(struct hrl_window) );

    if (retval != NULL) {
        clear();
        refresh();

        retval->cols = width;
        retval->lines =height;
        retval->y = starty;
        retval->x = startx;
        retval->type = type;
        retval->win = newwin(retval->lines, retval->cols, starty, startx);
        wrefresh(retval->win);

        if (has_colors() == TRUE) win_generate_colours();
    }

    return retval;
}

void win_destroy(struct hrl_window *window) {
    wrefresh(window->win);
    delwin(window->win);
    free(window);
}

void win_display_map(struct hrl_window *window, struct dc_map *map, int player_x, int player_y) {
    // Calculate top left of camera position
    int cx = 0;
    int cy = 0;
    int x_max = (window->cols < map->x_sz) ? window->cols : map->x_sz;
    int y_max = (window->lines < map->y_sz) ? window->lines : map->y_sz;
    werase(window->win);

    if (window->cols < map->x_sz) {
        cx = player_x - (window->cols / 2);
        if (cx < 0) cx = 0;
        if (cx + window->cols > map->x_sz) cx = map->x_sz - window->cols;
    }
    if (window->lines < map->y_sz) {
        cy = player_y - (window->lines / 2);
        if (cy < 0) cy = 0;
        if (cy + window->lines > map->y_sz) cy = map->y_sz - window->lines;
    }

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            if (SD_GET_INDEX(xi+cx, yi+cy, map).monster != NULL) {
                if (has_colors() == TRUE) attron(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).monster->colour ) );
                mvwprintw(window->win, yi, xi, "%c", SD_GET_INDEX(xi+cx, yi+cy, map).monster->icon);
                if (has_colors() == TRUE) attroff(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).monster->colour ) );
            }
            else if (SD_GET_INDEX(xi+cx, yi+cy, map).item != NULL) {
                if (has_colors() == TRUE) attron(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).item->colour ) );
                mvwprintw(window->win, yi, xi, "%c", SD_GET_INDEX(xi+cx, yi+cy, map).item->icon);
                if (has_colors() == TRUE) attroff(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).item->colour ) );
            }
            else {
                if (has_colors() == TRUE) attron(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).tile.colour ) );
                mvwprintw(window->win, yi, xi, "%c", SD_GET_INDEX_TYPE(xi+cx, yi+cy, map) );
                if (has_colors() == TRUE) attroff(COLOR_PAIR(SD_GET_INDEX(xi+cx, yi+cy, map).tile.colour ) );
            }
        }
    }
    wrefresh(window->win);
}

void win_log_refresh(struct hrl_window *window, struct logging *log) {
    struct queue *q = lg_logging_queue(log);
    int log_sz = queue_size(q);
    int win_sz = window->lines;

    if (window == NULL) return;
    werase(window->win);

    int max = (win_sz < log_sz) ? win_sz : log_sz;
    int log_start = log_sz - win_sz;
    if (log_start < 0) log_start = 0;

    for (int i = 0; i < max; i++) {
        struct log_entry *tmp_entry = queue_peek_nr(q, log_start +i);
        if (tmp_entry != NULL) {
            mvwprintw(window->win, i, 0, tmp_entry->string);
        }
    }
    wrefresh(window->win);
}

void win_log_callback(struct logging *log, struct log_entry *entry, void *priv) {
    struct hrl_window *window = priv;
    if (window != NULL) win_log_refresh(window, log);
}

