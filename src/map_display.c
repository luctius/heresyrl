#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>

#include "map_display.h"

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

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = malloc(sizeof(struct hrl_window) );

    if (retval != NULL) {
        retval->cols = width;
        retval->lines =height;
        retval->y = starty;
        retval->x = startx;
        retval->type = type;
        retval->win = newwin(retval->lines, retval->cols, starty, startx);
        wrefresh(retval->win);

        win_generate_colours();
    }

    return retval;
}

void win_destroy(struct hrl_window *window) {
    wrefresh(window->win);
    delwin(window->win);
    free(window);
}

void win_display_map(struct hrl_window *window, struct sd_map *map, int player_x, int player_y) {

    // Calculate top left of camera position
    int cx = 0;
    int cy = 0;
    int x_max = (window->cols < map->x_sz) ? window->cols : map->x_sz;
    int y_max = (window->lines < map->y_sz) ? window->lines : map->y_sz;

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
            if (SD_GET_INDEX(xi+cx, yi+cy, map).has_player) mvwprintw(window->win, yi, xi, "@", SD_GET_INDEX(xi+cx, yi+cy, map).type );
            else mvwprintw(window->win, yi, xi, "%c", SD_GET_INDEX(xi+cx, yi+cy, map).type );
        }
    }
    wrefresh(window->win);
}

void win_log_callback(struct logging *log, struct log_entry *entry, void *priv) {
    struct hrl_window *window = priv;
    struct queue *q = lg_logging_queue(log);
    int log_sz = queue_size(q);

    int max = (window->lines < log_sz) ? window->lines : log_sz;
    int log_start = log_sz - window->lines;
    if (log_start < 0) log_start = 0;

    for (int i = 0; i < max; i++) {
        struct log_entry *tmp_entry = queue_peek_nr(q, log_start +i);
        if (tmp_entry != NULL) {
            mvwprintw(window->win, i, 0, tmp_entry->string);
        }
    }
    wrefresh(window->win);
}

