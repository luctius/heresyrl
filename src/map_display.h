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
#include "logging.h"

enum window_type {
    HRL_WINDOW_TYPE_MAP,
    HRL_WINDOW_TYPE_CHARACTER,
    HRL_WINDOW_TYPE_MESSAGE,
};

enum dpl_colours {
    DPL_COLOUR_NORMAL  =1, /*FG_WHITE,BG_BLACK*/
    DPL_COLOUR_FG_RED,      /* FG_RED,BG_BLACK */
    DPL_COLOUR_FG_GREEN,    /* FG_GREEN,BG_BLACK */
    DPL_COLOUR_FG_YELLOW,   /* FG_YELLOW,BG_BLACK */
    DPL_COLOUR_FG_BLUE,     /* FG_BLUE,BG_BLACK */
    DPL_COLOUR_FG_MAGENTA,  /* FG_MAGENTA,BG_BLACK */
    DPL_COLOUR_FG_CYAN,     /* FG_CYAN,BG_BLACK */

    DPL_COLOUR_FGW_INVERSE, /* BG_WHITE,FG_BLACK */
    DPL_COLOUR_FGW_RED, /* BG_RED,FG_WHITE */
    DPL_COLOUR_FGW_GREEN, /* BG_GREEN,FG_WHITE */
    DPL_COLOUR_FGW_YELLOW, /* BG_YELLOW,FG_WHITE */
    DPL_COLOUR_FGW_BLUE, /* BG_BLUE,FG_WHITE */
    DPL_COLOUR_FGW_MAGENTA, /* BG_MAGENTA,FG_WHITE */
    DPL_COLOUR_FGW_CYAN, /* BG_CYAN,FG_WHITE */

    DPL_COLOUR_BGB_RED, /* BG_RED,FG_BLACK */
    DPL_COLOUR_BGB_GREEN, /* BG_GREEN,FG_BLACK */
    DPL_COLOUR_BGB_YELLOW, /* BG_YELLOW,FG_BLACK */
    DPL_COLOUR_BGB_BLUE, /* BG_BLUE,FG_BLACK */
    DPL_COLOUR_BGB_MAGENTA, /* BG_MAGENTA,FG_BLAC */
    DPL_COLOUR_BGB_CYAN, /* BG_CYAN,FG_BLACK */

    DPL_COLOUR_BGW_RED, /* BG_RED,FG_WHITE */
    DPL_COLOUR_BGW_GREEN, /* BG_GREEN,FG_WHITE */
    DPL_COLOUR_BGW_YELLOW, /* BG_YELLOW,FG_WHITE */
    DPL_COLOUR_BGW_BLUE, /* BG_BLUE,FG_WHITE */
    DPL_COLOUR_BGW_MAGENTA, /* BG_MAGENTA,FG_BLAC */
    DPL_COLOUR_BGW_CYAN, /* BG_CYAN,FG_WHITE */

    DPL_COLOUR_ALL_RED, /* BG_RED,FG_RED */
    DPL_COLOUR_ALL_GREEN, /* BG_GREEN,FG_GREEN */
    DPL_COLOUR_ALL_YELLOW, /* BG_YELLOW,FG_YELLOW */
    DPL_COLOUR_ALL_BLUE, /* BG_BLUE,FG_BLUE */
    DPL_COLOUR_ALL_MAGENTA, /* BG_MAGENTA,FG_MAGENTA */
    DPL_COLOUR_ALL_CYAN, /* BG_CYAN,FG_CYAN */
    DPL_COLOUR_ALL_BLACK, /* BG_CYAN,FG_CYAN */
    DPL_COLOUR_ALL_WHITE, /* BG_CYAN,FG_CYAN */
};

struct hrl_window;

bool create_ui(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win);
void destroy_ui(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win);

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);

void win_display_map(struct hrl_window *window, struct sd_map *map, int player_x, int player_y);
void win_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void win_log_refresh(struct hrl_window *window, struct logging *log);
#endif /*MAP_DISPLAY_H_*/
