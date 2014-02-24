#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>

#include "map_display.h"

struct hrl_window {
    WINDOW *win;
    int cols;
    int lines;
    int y;
    int x;
    enum window_type type;
};

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

