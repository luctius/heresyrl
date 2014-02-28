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

#include "heresyrl_def.h"
#include "dungeon_creator.h"
#include "logging.h"

enum window_type {
    HRL_WINDOW_TYPE_MAP,
    HRL_WINDOW_TYPE_CHARACTER,
    HRL_WINDOW_TYPE_MESSAGE,
    HRL_WINDOW_TYPE_MAX,
};

struct hrl_window;

bool create_ui(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win);
void destroy_ui(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win);

void win_generate_colours(void);
struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);

void win_display_map(struct hrl_window *window, struct dc_map *map, int player_x, int player_y);
void win_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void win_log_refresh(struct hrl_window *window, struct logging *log);
#endif /*MAP_DISPLAY_H_*/
