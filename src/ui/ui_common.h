/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UI_COMMON_H
#define UI_COMMON_H

//#include <ncurses.h>
#include <uncursed.h>

#include "heresyrl_def.h"
#include "coord.h"

#define LINES_MIN 22
#define COLS_MIN  60

#define MAP_MIN_COLS 20
#define MAP_MAX_COLS 150
#define MAP_COLS_FACTOR 0.90f

#define MAP_MIN_LINES 22
#define MAP_MAX_LINES 0
#define MAP_LINES_FACTOR 0.90f

#define MSG_MIN_COLS 20
#define MSG_MAX_COLS 150
#define MSG_COLS_FACTOR 0.90f

#define MSG_MIN_LINES 2
#define MSG_MAX_LINES 0
#define MSG_LINES_FACTOR 0.10f

#define CHAR_MIN_COLS 23
#define CHAR_MAX_COLS 40
#define CHAR_COLS_FACTOR 0.05f

#define CHAR_MIN_LINES 30
#define CHAR_MAX_LINES 80
#define CHAR_LINES_FACTOR 1.00f

enum window_type {
    HRL_WINDOW_TYPE_MAIN,
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

    int text_x;
    int text_y;
};

extern struct hrl_window *main_win;
extern struct hrl_window *map_win;
extern struct hrl_window *char_win;
extern struct hrl_window *msg_win;

extern coord_t last_ppos;
int get_viewport(int p, int vps, int mps);
void win_generate_colours(void);

int ui_printf_ext(struct hrl_window *win, int y_start, int x_start, const char *format, ...);
#define ui_print_reset(win) do { (win)->text_x = 0; (win)->text_y = 0; } while(0)
#define ui_printf(win, f, a...) ui_printf_ext(win, -1, -1, f, ##a)

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);
#endif /* UI_COMMON_H */
