#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>

#include "map_display.h"
#include "tiles.h"
#include "monster.h"
#include "items.h"
#include "fight.h"

struct hrl_window {
    WINDOW *win;
    int cols;
    int lines;
    int y;
    int x;
    enum window_type type;
};

void win_generate_colours(void) {
    int i = 1;

    lg_printf("generating colours");

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

static int hdr_lines = 0;
static int hdr_cols = 0;
bool create_ui(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win) {
    if ( (map_win != NULL) && (char_win != NULL) && (msg_win != NULL)  )
    {
        if ( (hdr_lines != lines) || (hdr_cols != cols) ) {
            hdr_lines = lines;
            hdr_cols = cols;

            if ( (lines < 25) || (cols < 40) ) {
                endwin();           /*  End curses mode       */
                fprintf(stderr, "Terminal is too small, minimum is 40x25, this terminal is %dx%d.\n", cols, lines);
                exit(1);
            }

            curs_set(0);

            /* Calculate 3 windows sizes */
            int map_cols = cols - CHAR_MIN_COLS;
            if (map_cols > MAP_MIN_COLS) map_cols *= MAP_COLS_FACTOR;
            if ( (map_cols > MAP_MAX_COLS) && (MAP_MAX_COLS != 0) ) map_cols = MAP_MAX_COLS;
            int map_lines = (lines -1) - MSG_MIN_LINES;
            if (map_lines > MAP_MIN_LINES) map_lines = (lines - MSG_MIN_LINES) * MAP_LINES_FACTOR;
            if ( (map_lines > MAP_MAX_LINES) && (MAP_MAX_LINES != 0) ) map_lines = MAP_MAX_LINES;

            int char_cols = cols - map_cols;
            if (char_cols < CHAR_MIN_COLS) char_cols = CHAR_MIN_COLS;
            if ( (char_cols > CHAR_MAX_COLS) && (CHAR_MAX_COLS != 0) ) char_cols = CHAR_MAX_COLS;
            int char_lines = lines * CHAR_LINES_FACTOR;
            if ( (char_lines > CHAR_MAX_LINES) && (CHAR_MAX_LINES != 0) ) char_lines = CHAR_MAX_LINES;

            int msg_cols = cols;
            if ( (msg_cols > MSG_MAX_COLS) && (MSG_MAX_COLS != 0) ) msg_cols = MSG_MAX_COLS;
            int msg_lines = (lines -1) - map_lines;
            if (msg_lines < MSG_MIN_LINES) msg_lines = MSG_MIN_LINES;
            if ( (msg_lines > MSG_MAX_LINES) && (MSG_MAX_LINES != 0) ) msg_lines = MSG_MAX_LINES;

            int total_lines = map_lines + msg_lines;
            if (total_lines < char_lines) total_lines = char_lines;
            int total_cols = map_cols + char_cols;
            if (total_cols < msg_cols) total_cols = msg_cols;

            if (total_lines > lines) { fprintf(stderr, "Too many lines used!\n"); exit(1); }
            if (total_cols > cols) { fprintf(stderr, "Too many cols used!\n"); exit(1); }

            if ( (*map_win == NULL) || (*char_win == NULL) ||(*msg_win == NULL)  ) {
                *map_win = win_create(map_lines-1, map_cols-1, 1, 1, HRL_WINDOW_TYPE_MAP);
                *char_win = win_create(char_lines-1, char_cols, 1, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
                *msg_win = win_create(msg_lines, msg_cols-1, map_lines+1, 1, HRL_WINDOW_TYPE_MESSAGE);
                lg_set_callback(gbl_log, *msg_win, win_log_callback);
                win_log_refresh(*msg_win, gbl_log);
                return true;
            }
            else {
                lg_set_callback(gbl_log, NULL, NULL);
                destroy_ui(*map_win, *char_win, *msg_win);
                *map_win = NULL;
                *char_win = NULL;
                *msg_win = NULL;
                hdr_lines = 0;
                hdr_cols = 0;
                return create_ui(cols, lines, map_win, char_win, msg_win);
            }
        }
    }
    return false;
}

void destroy_ui(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win) {
    lg_set_callback(gbl_log, NULL, NULL);
    if (map_win != NULL) win_destroy(map_win);
    if (char_win != NULL) win_destroy(char_win);
    if (msg_win != NULL) win_destroy(msg_win);
}

struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
    struct hrl_window *retval = malloc(sizeof(struct hrl_window) );

    if (retval != NULL) {
        clear();
        refresh();

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

static void win_display_map_noref(struct hrl_window *window, struct dc_map *map, coord_t *player) {
    // Calculate top left of camera position
    coord_t scr_c = cd_create(0,0);
    int x_max = (window->cols < map->size.x) ? window->cols : map->size.x;
    int y_max = (window->lines < map->size.x) ? window->lines : map->size.y;
    werase(window->win);
    curs_set(0);

    if (window->cols < map->size.x) {
        scr_c.x = player->x - (window->cols / 2);
        if (scr_c.x < 0) scr_c.x = 0;
        if (scr_c.x + window->cols > map->size.x) scr_c.x = map->size.x - window->cols;
    }

    if (window->lines < map->size.y) {
        scr_c.y = player->y - (window->lines / 2);
        if (scr_c.y < 0) scr_c.y = 0;
        if (scr_c.y + window->lines > map->size.y) scr_c.y = map->size.y - window->lines;
    }

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            coord_t map_c = cd_create(xi+scr_c.x, yi+scr_c.y);
            if ( (SD_GET_INDEX(&map_c, map).visible == true) || (SD_GET_INDEX(&map_c, map).discovered == true) ) {
                int attr_mod = SD_GET_INDEX(&map_c, map).tile->icon_attr;
                char icon = SD_GET_INDEX_ICON(&map_c, map);

                if (SD_GET_INDEX(&map_c, map).monster != NULL) {
                    icon = SD_GET_INDEX(&map_c, map).monster->icon;
                    attr_mod = SD_GET_INDEX(&map_c, map).monster->icon_attr;
                }
                else if (SD_GET_INDEX(&map_c, map).item != NULL) {
                    icon = SD_GET_INDEX(&map_c, map).item->icon;
                    attr_mod = SD_GET_INDEX(&map_c, map).item->icon_attr;
                }
                else if (TILE_HAS_ATTRIBUTE(SD_GET_INDEX(&map_c, map).tile, TILE_ATTR_TRAVERSABLE) == false) {
                    if (SD_GET_INDEX(&map_c, map).visible == true) {
                        attr_mod = COLOR_PAIR(DPL_COLOUR_FG_YELLOW);
                    }
                }
                else if (TILE_HAS_ATTRIBUTE(SD_GET_INDEX(&map_c, map).tile, TILE_ATTR_TRAVERSABLE) ){
                    if (SD_GET_INDEX(&map_c, map).visible == false) attr_mod |= A_DIM;
                    else if (SD_GET_INDEX(&map_c, map).visible == true) attr_mod |= A_BOLD;
                }
                if (has_colors() == TRUE) wattron(window->win, attr_mod);
                mvwprintw(window->win, yi, xi, "%c", icon);
                if (has_colors() == TRUE) wattroff(window->win, attr_mod);
            }
        }
    }
}

void win_display_map(struct hrl_window *window, struct dc_map *map, coord_t *player) {
    win_display_map_noref(window, map, player);
    wrefresh(window->win);
}

void win_overlay_examine_cursor(struct hrl_window *window, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool examine_mode = true;

    coord_t e_pos = *p_pos;

    do {
        switch (ch) {
            case KEY_UP: e_pos.y--; break;
            case KEY_RIGHT: e_pos.x++; break;
            case KEY_DOWN: e_pos.y++; break;
            case KEY_LEFT: e_pos.x--; break;
            case '\n':
            case 'x':
                You("examine (%d,%d)", e_pos.x, e_pos.y);
                examine_mode=false;
                break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= window->lines -1) e_pos.y = window->lines -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= window->cols -1) e_pos.x = window->cols -1;

        chtype oldch = mvwinch(window->win, e_pos.y, e_pos.x);
        mvwchgat(window->win, e_pos.y, e_pos.x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
        wrefresh(window->win);
        mvwaddch(window->win, e_pos.y, e_pos.x, oldch);
    }
    while((ch = getch()) != 27 && ch != 'q' && examine_mode);
}

void win_overlay_fire_cursor(struct hrl_window *window, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;

    coord_t e_pos = *p_pos;
    /*find nearest enemy....*/

    do {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "entering fire_mode (%d,%d) -> (%d,%d)", p_pos->x, p_pos->y, e_pos.x, e_pos.y);
        switch (ch) {
            case KEY_UP: e_pos.y--; break;
            case KEY_RIGHT: e_pos.x++; break;
            case KEY_DOWN: e_pos.y++; break;
            case KEY_LEFT: e_pos.x--; break;
            case '\n':
            case 'f':
                You("fire (%d,%d)", e_pos.x, e_pos.y);
                fire_mode=false;
                break;
            default: break;
        }
        if (fire_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= window->lines -1) e_pos.y = window->lines -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= window->cols -1) e_pos.x = window->cols -1;

        int length = cd_pyth(p_pos, &e_pos) +1;
        coord_t path[length];
        int path_len = fght_calc_lof_path(p_pos, &e_pos, path, length);
        for (int i = 0; i < path_len; i++) {
            mvwchgat(window->win, path[i].y, path[i].x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "fire_mode: [%d/%d] c (%d,%d)", i, path_len, path[i].x, path[i].y);
        }
        mvwchgat(window->win, e_pos.y, e_pos.x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);

        wrefresh(window->win);
        win_display_map_noref(window, map, p_pos);
    }
    while((ch = getch()) != 27 && ch != 'q' && fire_mode);
}

void win_log_refresh(struct hrl_window *window, struct logging *log) {
    struct queue *q = lg_logging_queue(log);
    int log_sz = queue_size(q);
    int win_sz = window->lines;

    if (window == NULL) return;
    werase(window->win);

    int max = (win_sz < log_sz) ? win_sz : log_sz;
    int log_start = log_sz - win_sz;
    if (log_start < 0) log_start = 0;

    for (int i = 0; i < max; i++) {
        struct log_entry *tmp_entry = (struct log_entry *) queue_peek_nr(q, log_start +i);
        if (tmp_entry != NULL) {
            //mvwprintw(window->win, i, 0, tmp_entry->string);
            waddstr(window->win, tmp_entry->string);
        }
    }
    wrefresh(window->win);
}

void win_log_callback(struct logging *log, struct log_entry *entry, void *priv) {
    struct hrl_window *window = priv;
    if (window != NULL) win_log_refresh(window, log);
}

