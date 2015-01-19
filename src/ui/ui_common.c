#include <assert.h>
#include <string.h>
#include <sys/param.h>

#include "ui_common.h"
#include "options.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

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
    int txt_sz = strlen(buf);
    
    int max_line_sz = win->cols -1;

    if (win->text_x != 0) {
        mvwaddch(win->win, win->text_y, win->text_x++, ' ');
    }

    bool colour_parse = false;
    char colour_buf[MAX_CLRSTR_LEN+1];
    int attr_mod[MAX_CLR_DEPTH];
    attr_mod[0] = get_colour(TERM_COLOUR_L_WHITE);
    int attr_mod_ctr = 0;

    int try = 0;
    int t = 0;
    while (t < txt_sz) {
        bool new_line = false;

        int line_sz = MIN(max_line_sz - win->text_x, txt_sz - t);
        if ( (txt_sz - t) > line_sz) {
            new_line = true;

            int tmp_line_sz = line_sz;
            line_sz = 0;

            for (int i = tmp_line_sz; i > tmp_line_sz * 0.9; i--) {
                if (buf[t + i] == ' ') {
                    line_sz = i;
                    break;
                }
            }
        }

        int x = 0;
        int colour_ctr = 0;
        for (int i = 0; i < line_sz; i++) {
            if (buf[t] == '\n') {
                t++;
                new_line = true;
                line_sz = i;
                break;
            }
            else if (buf[t] == '<') {
                colour_parse = true;
                colour_buf[colour_ctr++] = buf[t];
            }
            else if (colour_parse) {
                colour_buf[colour_ctr++] = buf[t];

                if (buf[t] == '>') {
                    colour_buf[colour_ctr] = '\0';
                    colour_parse = false;
                    int c = clrstr_to_attr(colour_buf);
                    if (attr_mod[attr_mod_ctr] == c) {
                        attr_mod_ctr--;
                        assert(attr_mod_ctr >= 0);
                        if (attr_mod_ctr < 0) attr_mod_ctr = 0;
                    }
                    else {
                        attr_mod_ctr++;
                        attr_mod[attr_mod_ctr]= c;
                    }
                    colour_ctr = 0;
                }
                assert(colour_ctr   < MAX_CLRSTR_LEN);
                assert(attr_mod_ctr < MAX_CLR_DEPTH);
            }
            else {
                if (has_colors() == TRUE) wattron(win->win, attr_mod[attr_mod_ctr]);
                mvwaddch(win->win, win->text_y, win->text_x + x, buf[t]);
                if (has_colors() == TRUE) wattroff(win->win, attr_mod[attr_mod_ctr]);
                x++;
            }

            t++;
        }
        win->text_x += x;

        if (new_line) {
            if (line_sz == 0 && try == 1) return win->text_y;
            win->text_y++;
            win->text_x = 0;
            if (buf[t] == ' ') t++;
            if (line_sz == 0) try++;
        }
    }

    return win->text_y;
}

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = malloc(sizeof(struct hrl_window) );

    if (retval != NULL) {
        clear();
        refresh();

        retval->text = NULL;
        retval->text_sz = 0;
        retval->text_idx = 0;

        retval->text_x = 0;
        retval->text_y = 0;

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
    if (window != NULL) {
        free(window->text);
        wrefresh(window->win);
        delwin(window->win);
        free(window);
    }
}

