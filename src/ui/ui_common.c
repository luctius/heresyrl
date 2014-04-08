#include <assert.h>

#include "ui_common.h"
#include "linewrap.h"

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
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "generating colours, we do %d, max is %d", TERM_COLOUR_MAX, COLOR_PAIRS);

        generate_colours();
    }
}

void textwin_init(struct hrl_window *win) {
    free(win->text);
    win->text = NULL;
    win->text_sz = win->cols * win->lines * sizeof(char);
    win->text = malloc(win->text_sz);
    win->text_idx = 0;
}

void textwin_add_text(struct hrl_window *win, const char *format, ...) {
    assert(win->text != NULL);
    assert (win->text_idx < win->text_sz);

    va_list args;
    va_start(args, format);
    win->text_idx += vsnprintf(&win->text[win->text_idx], win->text_sz - win->text_idx, format, args);
    va_end(args);
}

void textwin_display_text(struct hrl_window *win) {
    if (win == NULL) return;
    werase(win->win);

    assert(win->text != NULL);

    char **desc;
    int *len_lines;
    int len = strwrap(win->text, win->cols -1, &desc, &len_lines);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len_lines[i]; j++) {
                mvwaddch(win->win, i, j, desc[i][j]);
            }
        }
    }

    free(desc);
    free(len_lines);

    wrefresh(win->win);
    free(win->text);
    win->text = NULL;
    win->text_sz = win->text_idx = 0;
}

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = malloc(sizeof(struct hrl_window) );

    if (retval != NULL) {
        clear();
        refresh();

        retval->text = NULL;
        retval->text_sz = 0;
        retval->text_idx = 0;

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

