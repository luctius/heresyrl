#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <ncurses.h>

#include "heresyrl_def.h"
#include "coord.h"

#define MAP_MIN_COLS 20
#define MAP_MAX_COLS 100
#define MAP_COLS_FACTOR 0.90f

#define MAP_MIN_LINES 22
#define MAP_MAX_LINES 0
#define MAP_LINES_FACTOR 0.90f

#define MSG_MIN_COLS 40
#define MSG_MAX_COLS 100
#define MSG_COLS_FACTOR 0.95f

#define MSG_MIN_LINES 2
#define MSG_MAX_LINES 0
#define MSG_LINES_FACTOR 0.10f

#define CHAR_MIN_COLS 31
#define CHAR_MAX_COLS 31
#define CHAR_COLS_FACTOR 0.05f

#define CHAR_MIN_LINES 30
#define CHAR_MAX_LINES 30
#define CHAR_LINES_FACTOR 1.00f

enum window_type {
    HRL_WINDOW_TYPE_MAP,
    HRL_WINDOW_TYPE_CHARACTER,
    HRL_WINDOW_TYPE_MESSAGE,
    HRL_WINDOW_TYPE_MAX,
};

struct hrl_window {
    WINDOW *win;
    int cols;
    int lines;
    int y;
    int x;
    enum window_type type;

    char *text;
    int text_sz;
    int text_idx;
    int text_sx;
    int text_sy;
    int text_ex;
    int text_ey;
};

extern struct hrl_window *map_win;
extern struct hrl_window *char_win;
extern struct hrl_window *msg_win;

extern coord_t last_ppos;
int get_viewport(int p, int vps, int mps);
void win_generate_colours(void);

void textwin_init(struct hrl_window *win, int sx, int sy, int ex, int ey);
void textwin_add_text(struct hrl_window *win, const char *format, ...);
int textwin_display_text(struct hrl_window *win);

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);
#endif /* UI_COMMON_H */
