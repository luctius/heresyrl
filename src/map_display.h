#pragma once
#ifndef MAP_DISPLAY_H_
#define MAP_DISPLAY_H_

#define MAP_MIN_COLS 20
#define MAP_MAX_COLS 100
#define MAP_COLS_FACTOR 0.95f

#define MAP_MIN_LINES 22
#define MAP_MAX_LINES 0
#define MAP_LINES_FACTOR 0.90f

#define MSG_MIN_COLS 40
#define MSG_MAX_COLS 40
#define MSG_COLS_FACTOR 0.95f

#define MSG_MIN_LINES 2
#define MSG_MAX_LINES 0
#define MSG_LINES_FACTOR 0.10f

#define CHAR_MIN_COLS 20
#define CHAR_MAX_COLS 20
#define CHAR_COLS_FACTOR 0.05f

#define CHAR_MIN_LINES 20
#define CHAR_MAX_LINES 20
#define CHAR_LINES_FACTOR 1.00f

#include "simple_dungeon.h"

enum window_type {
    HRL_WINDOW_TYPE_MAP,
    HRL_WINDOW_TYPE_CHARACTER,
    HRL_WINDOW_TYPE_MESSAGE,
};

struct hrl_window;

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);

void win_display_map(struct hrl_window *window, struct sd_map *map, int player_x, int player_y);
#endif /*MAP_DISPLAY_H_*/
