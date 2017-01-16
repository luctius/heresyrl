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

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>

#include "ui_common.h"
#include "options.h"

struct hrl_window *main_win = NULL;
struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;
struct hrl_window *wz_win = NULL;

coord_t last_ppos = {0,0};
int get_viewport(int p, int vps, int mps) {
    int hvps = round(vps / 2.0f);

    if (mps < vps) return 0;
    if (p < hvps) return 0;
    if (p > (mps - hvps) ) return mps - vps;
    return p - hvps;
}

static bool colours_generated = false;
void win_generate_colours(void) {
    if (colours_generated == false) {
        colours_generated = true;
        lg_debug("generating colours, we do %d, max is %d", TERM_COLOUR_MAX, COLOR_PAIRS);

        generate_colours();
    }
}

static inline uint16_t ui_idx_to_real_idx(char *txt, int pidx) {
    for (int i = 0; i < (int) strlen(txt); i++) {
        if (clrstr_is_colour(&txt[i]) || clrstr_is_close(&txt[i]) ) {
            int l = clrstr_len(&txt[i]);
            i += l-1;
        }
        else pidx--;

        if (pidx <= 0) return i+1;
    }
    return -1;
}

static inline uint16_t ui_strlen(char *txt) {
    int max_txt_sz = strlen(txt);
    int txt_len = max_txt_sz;

    for (int i = 0; i < max_txt_sz; i++) {
        if (clrstr_is_colour(&txt[i]) || clrstr_is_close(&txt[i]) ) {
            int l = clrstr_len(&txt[i]);
            txt_len -= l;
            i += l-1;
        }
    }
    return txt_len;
}

#define STRING_MAX 1024
#define MAX_CLRSTR_LEN 20
#define MAX_CLR_DEPTH 5
int ui_printf_ext(struct hrl_window *win, int y_start, int x_start, const char *format, ...) {
    if (win == NULL) return 0;
    if (x_start > -1) win->text_x = x_start;
    if (y_start > -1) win->text_y = y_start;

    char buf[STRING_MAX+1];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, STRING_MAX, format, args);
    va_end(args);
    int print_txt_sz = ui_strlen(buf);
    //int real_txt_sz = strlen(buf);

    int max_line_sz = win->cols -1;

    int attr_mod[MAX_CLR_DEPTH];
    attr_mod[0] = get_colour(TERM_COLOUR_L_WHITE);
    int attr_mod_ctr = 0;
    wattrset(win->win, attr_mod[0]);

    if (win->text_x != 0) {
        cchar_t space = { attr_mod[0], {' ', 0} };
        if (options.refresh) mvwadd_wch(win->win, win->text_y, win->text_x++, &space);
    }

    int print_txt_idx = 0;
    int real_txt_idx = 0;
    while (print_txt_idx < print_txt_sz) {
        bool new_line = false;

        int p_line_sz = MIN(max_line_sz - win->text_x, print_txt_sz - print_txt_idx);
        int r_line_sz = ui_idx_to_real_idx(buf, p_line_sz);
        if ( (print_txt_sz - print_txt_idx) > p_line_sz) {
            //new_line = true;

            int tmp_line_sz = r_line_sz;
            r_line_sz = 0;

            for (int i = tmp_line_sz; i > 0; i--) {
                if (buf[real_txt_idx + i] == ' ') {
                    r_line_sz = i;
                    break;
                }
            }
        }

        for (int i = 0; i < r_line_sz; i++) {
            if (buf[real_txt_idx] == '\n') {
                real_txt_idx++;
                print_txt_idx++;

                new_line = true;
                break;
            }
            else if (clrstr_is_close(&buf[real_txt_idx]) ) {
                wattrset(win->win, attr_mod[attr_mod_ctr]);
                attr_mod_ctr--;
                assert(attr_mod_ctr >= 0);
                wattrset(win->win, attr_mod[attr_mod_ctr]);

                assert(attr_mod_ctr < MAX_CLR_DEPTH);

                int l = clrstr_len(&buf[real_txt_idx]);
                real_txt_idx += l;
                i += (l-1);
            }
            else if (clrstr_is_colour(&buf[real_txt_idx]) ) {
                int cstr_len = clrstr_len(&buf[real_txt_idx]);

                int c = clrstr_to_attr(&buf[real_txt_idx]);

                attr_mod_ctr++;
                attr_mod[attr_mod_ctr]= c;
                wattrset(win->win, attr_mod[attr_mod_ctr]);

                assert(attr_mod_ctr < MAX_CLR_DEPTH);
                real_txt_idx += cstr_len;
                i += (cstr_len-1);
            }
            else {
                if (options.refresh) {
                    cchar_t b = { attr_mod[attr_mod_ctr], {buf[real_txt_idx], 0} };
                    mvwadd_wch(win->win, win->text_y, win->text_x++, &b);
                }

                real_txt_idx++;
                print_txt_idx++;
            }
        }

        if (new_line || (win->text_x > (0.7 * win->cols) ) ) {
            win->text_y++;
            win->text_x = 0;
            if (buf[real_txt_idx] == ' ') {
                real_txt_idx++;
                print_txt_idx++;
            }
        }
    }

    wattrset(win->win, attr_mod[attr_mod_ctr]);
    if (options.refresh) wrefresh(win->win);

    return win->text_y;
}

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = calloc(1, sizeof(struct hrl_window) );

    if (retval != NULL) {
        //clear();
        //if (options.refresh) refresh();

        retval->text_x = 0;
        retval->text_y = 0;

        retval->cols = width;
        retval->lines =height;
        retval->y = starty;
        retval->x = startx;
        retval->type = type;
        retval->win = newwin(retval->lines, retval->cols, starty, startx);
        if (options.refresh) {
            wrefresh(retval->win);
        }
    }

    return retval;
}

void win_destroy(struct hrl_window *window) {
    if (window != NULL) {
        if (options.refresh) {
            wrefresh(window->win);
        }
        delwin(window->win);
        free(window);
    }
}

