#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <sys/param.h>

#include "ui.h"
#include "tiles.h"
#include "monster.h"
#include "items.h"
#include "fight.h"
#include "dungeon_creator.h"
#include "logging.h"
#include "player.h"
#include "inventory.h"
#include "linewrap.h"

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
bool ui_create(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win) {
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

            int msg_cols = cols;
            if ( (msg_cols > MSG_MAX_COLS) && (MSG_MAX_COLS != 0) ) msg_cols = MSG_MAX_COLS;
            int msg_lines = (lines -1) - map_lines;
            if (msg_lines < MSG_MIN_LINES) msg_lines = MSG_MIN_LINES;
            if ( (msg_lines > MSG_MAX_LINES) && (MSG_MAX_LINES != 0) ) msg_lines = MSG_MAX_LINES;

            int char_cols = cols - map_cols;
            if (char_cols < CHAR_MIN_COLS) char_cols = CHAR_MIN_COLS;
            if ( (char_cols > CHAR_MAX_COLS) && (CHAR_MAX_COLS != 0) ) char_cols = CHAR_MAX_COLS;
            int char_lines = lines - msg_lines;
            if ( (char_lines > CHAR_MAX_LINES) && (CHAR_MAX_LINES != 0) ) char_lines = CHAR_MAX_LINES;

            int total_lines = map_lines + msg_lines;
            if (total_lines < char_lines) total_lines = char_lines;
            int total_cols = map_cols + char_cols;
            if (total_cols < msg_cols) total_cols = msg_cols;

            if (total_lines > lines) { fprintf(stderr, "Too many lines used!\n"); exit(1); }
            if (total_cols > cols) { fprintf(stderr, "Too many cols used!\n"); exit(1); }

            if ( (*map_win == NULL) || (*char_win == NULL) ||(*msg_win == NULL)  ) {
                *map_win = win_create(map_lines-2, map_cols-2, 1, 1, HRL_WINDOW_TYPE_MAP);
                *char_win = win_create(char_lines, char_cols, 1, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
                *msg_win = win_create(msg_lines, msg_cols-1, map_lines, 1, HRL_WINDOW_TYPE_MESSAGE);
                lg_set_callback(gbl_log, *msg_win, msgwin_log_callback);
                msgwin_log_refresh(*msg_win, gbl_log);
                return true;
            }
            else {
                lg_set_callback(gbl_log, NULL, NULL);
                ui_destroy(*map_win, *char_win, *msg_win);
                *map_win = NULL;
                *char_win = NULL;
                *msg_win = NULL;
                hdr_lines = 0;
                hdr_cols = 0;
                return ui_create(cols, lines, map_win, char_win, msg_win);
            }
        }
    }
    return false;
}

void ui_destroy(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win) {
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

static int get_viewport(int p, int vps, int mps) {
    int hvps = vps / 2;
    if (mps < vps) return 0;
    if (p < hvps) return 0;
    if (p > (mps - hvps) ) return mps - vps;
    return p - hvps;
}

static void mapwin_display_map_noref(struct hrl_window *window, struct dc_map *map, coord_t *player) {
    // Calculate top left of camera position
    coord_t scr_c = cd_create(0,0);

    if (window == NULL) return;
    if (map == NULL) return;
    if (player == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;

    int x_max = (window->cols < map->size.x) ? window->cols : map->size.x;
    int y_max = (window->lines < map->size.y) ? window->lines : map->size.y;
    werase(window->win);
    curs_set(0);

    scr_c.x = get_viewport(player->x, window->cols, map->size.x);
    scr_c.y = get_viewport(player->y, window->lines, map->size.y);

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            coord_t map_c = cd_create(xi+scr_c.x, yi+scr_c.y);
            if ( (sd_get_map_me(&map_c, map)->visible == true) || (sd_get_map_me(&map_c, map)->discovered == true) ) {
                int attr_mod = sd_get_map_tile(&map_c, map)->icon_attr;
                char icon = sd_get_map_tile(&map_c, map)->icon;

                if (sd_get_map_me(&map_c, map)->monster != NULL) {
                    icon = sd_get_map_me(&map_c, map)->monster->icon;
                    attr_mod = sd_get_map_me(&map_c, map)->monster->icon_attr;
                }
                else if (sd_get_map_me(&map_c, map)->item != NULL) {
                    icon = sd_get_map_me(&map_c, map)->item->icon;
                    attr_mod = sd_get_map_me(&map_c, map)->item->icon_attr;
                }
                else if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&map_c, map), TILE_ATTR_TRAVERSABLE) == false) {
                    if (sd_get_map_me(&map_c, map)->visible == true) {
                        attr_mod = COLOR_PAIR(DPL_COLOUR_FG_YELLOW);
                    }
                }
                else if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&map_c, map), TILE_ATTR_TRAVERSABLE) ){
                    if (sd_get_map_me(&map_c, map)->visible == false) attr_mod |= A_DIM;
                    else if (sd_get_map_me(&map_c, map)->visible == true) attr_mod |= A_BOLD;
                }
                if (has_colors() == TRUE) wattron(window->win, attr_mod);
                mvwprintw(window->win, yi, xi, "%c", icon);
                if (has_colors() == TRUE) wattroff(window->win, attr_mod);
            }
        }
    }
}

void mapwin_display_map(struct hrl_window *window, struct dc_map *map, coord_t *player) {
    if (window == NULL) return;
    if (map == NULL) return;
    if (player == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;

    mapwin_display_map_noref(window, map, player);
    wrefresh(window->win);
}

void mapwin_overlay_examine_cursor(struct hrl_window *window, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool examine_mode = true;

    if (window == NULL) return;
    if (map == NULL) return;
    if (p_pos == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;

    coord_t e_pos = *p_pos;
    int scr_x = get_viewport(p_pos->x, window->cols, map->size.x);
    int scr_y = get_viewport(p_pos->y, window->lines, map->size.y);

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
        if (e_pos.y >= map->size.y -1) e_pos.y = map->size.y;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.y -1) e_pos.x = map->size.x;

        chtype oldch = mvwinch(window->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwchgat(window->win, e_pos.y - scr_y, e_pos.x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
        wrefresh(window->win);
        mvwaddch(window->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
    }
    while((ch = getch()) != 27 && ch != 'q' && examine_mode);
}

void mapwin_overlay_fire_cursor(struct hrl_window *window, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (window == NULL) return;
    if (map == NULL) return;
    if (p_pos == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;

    coord_t e_pos = *p_pos;
    /*find nearest enemy....*/

    int scr_x = get_viewport(p_pos->x, window->cols, map->size.x);
    int scr_y = get_viewport(p_pos->y, window->lines, map->size.y);

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
        if (e_pos.y >= map->size.y -1) e_pos.y = map->size.y;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.y -1) e_pos.x = map->size.x;

        int length = cd_pyth(p_pos, &e_pos);
        coord_t path[length+1];
        int path_len = fght_calc_lof_path(p_pos, &e_pos, path, ARRAY_SZ(path) );
        for (int i = 0; i < path_len; i++) {
            mvwchgat(window->win, path[i].y - scr_y, path[i].x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "fire_mode: [%d/%d] c (%d,%d) -> (%d,%d)", i, path_len, path[i].x, path[i].y, path[i].x - scr_x, path[i].y - scr_y);
        }
        mvwchgat(window->win, e_pos.y -scr_y, e_pos.x -scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);

        wrefresh(window->win);
        mapwin_display_map_noref(window, map, p_pos);
    }
    while((ch = getch()) != 27 && ch != 'q' && fire_mode);
}

void msgwin_log_refresh(struct hrl_window *window, struct logging *lg) {
    struct queue *q = lg_logging_queue(lg);
    int log_sz = queue_size(q);
    int win_sz = window->lines;

    if (window == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MESSAGE) return;
    werase(window->win);

    int max = (win_sz < log_sz) ? win_sz : log_sz;
    int log_start = log_sz - win_sz;
    if (log_start < 0) log_start = 0;

    for (int i = 0; i < max; i++) {
        struct log_entry *tmp_entry = (struct log_entry *) queue_peek_nr(q, log_start +i);
        if (tmp_entry != NULL) {
            waddstr(window->win, tmp_entry->string);
        }
    }
    wrefresh(window->win);
}

void msgwin_log_callback(struct logging *lg, struct log_entry *entry, void *priv) {
    struct hrl_window *window = priv;
    if (window != NULL) msgwin_log_refresh(window, lg);
}

void charwin_refresh(struct hrl_window *window, struct pl_player *plr) {
    if (window == NULL) return;
    if (plr == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;
    werase(window->win);

    struct msr_monster *player = plr->player;

    int y = 0;
    int x = 0;
    mvwprintw(window->win, y++,x, "Name      %s", plr->name);
    mvwprintw(window->win, y++,x, "Gender    %s", msr_gender_string(player) );
    mvwprintw(window->win, y++,x, "Homeworld %s", "Void Born");
    mvwprintw(window->win, y++,x, "Career    %s", "Thug");

    y++;
    int chr = msr_calculate_characteristic(player, MSR_CHAR_WEAPON_SKILL);
    mvwprintw(window->win, y++,x, "WS             %d", chr);
    chr = msr_calculate_characteristic(player, MSR_CHAR_BALISTIC_SKILL);
    mvwprintw(window->win, y++,x, "BS             %d", chr);
    chr = msr_calculate_characteristic(player, MSR_CHAR_STRENGTH);
    mvwprintw(window->win, y++,x, "Strength     [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_TOUCHNESS);
    mvwprintw(window->win, y++,x, "Toughness    [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_AGILITY);
    mvwprintw(window->win, y++,x, "Agility      [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_INTELLIGENCE);
    mvwprintw(window->win, y++,x, "Intelligence [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_PERCEPTION);
    mvwprintw(window->win, y++,x, "Perception   [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_WILLPOWER);
    mvwprintw(window->win, y++,x, "Willpower    [%d]%d", chr/10, chr%10);
    chr = msr_calculate_characteristic(player, MSR_CHAR_FELLOWSHIP);
    mvwprintw(window->win, y++,x, "Fellowship   [%d]%d", chr/10, chr%10);

    y++;
    mvwprintw(window->win, y++,x, "Wounds    [%2d/%2d]", player->cur_wounds, player->max_wounds);

    y++;
    mvwprintw(window->win, y++,x, "Armour [%2d]", 1);
    mvwprintw(window->win, y++,x, "       [%2d]", 1);
    mvwprintw(window->win, y++,x, "   [%2d]    [%2d]", 1, 1);
    mvwprintw(window->win, y++,x, "     [%2d][%2d]", 1, 1);

    wrefresh(window->win);
}

/* Beware of dragons here..... */

struct inv_show_item {
    char *location;
    struct itm_item *item;
};
static char invid[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9'};
static int get_invid(char c) {
    unsigned int i = 0;
    while (invid[i] != c && i < ARRAY_SZ(invid) ) {
        i++;
    }
    if (invid[i] == c) return i;
    return -1;
}

static int invwin_printlist(WINDOW *win, struct inv_show_item list[], int list_sz, int start, int end) {
    int max = MIN(list_sz, end);
    max = MIN(max, (int) ARRAY_SZ(invid) );
    if (start >= max) return -1;

    for (int i = 0; i < max; i++) {
        mvwprintw(win, i, 1, "%c  %c%s", invid[i], list[i+start].location[0], list[i+start].item->sd_name);
    }
    return max;
}

static void inv_create_list(struct inv_inventory *inventory, struct inv_show_item invlist[], int list_sz) {
    if (inventory == NULL) return;

    struct itm_item *item = NULL;
    for (int i = 0; i < list_sz; i++) {
         item = inv_get_next_item(inventory, item);
         if (item != NULL) {
            invlist[i].item = item;
            invlist[i].location = " ";
            if (inv_get_item_location(inventory, item) != INV_LOC_INVENTORY) {
                invlist[i].location = "*";
            }
         }
    }
}

WINDOW *invwin_examine(struct hrl_window *window, struct itm_item *item) {
    if (window == NULL) return NULL;
    if (item == NULL) return NULL;
    if (window->type != HRL_WINDOW_TYPE_MAP) return NULL;

    char **desc;
    int *len_lines;
    int len = strwrap(item->description, (window->cols / 2) -2, &desc, &len_lines);
    if (len > 0) {
        WINDOW *invwin_ex = derwin(window->win, window->lines, window->cols / 2, 0, window->cols / 2);
        touchwin(window->win);
        wclear(invwin_ex);

        mvwprintw(invwin_ex, 0, 1, "Description of %s.", item->ld_name);
        for (int i = 0; i < len; i++) {
            mvwprintw(invwin_ex, 2+i, 1, desc[i]);
        }
        free(desc);
        free(len_lines);
        wrefresh(invwin_ex);
        return invwin_ex;
    }
    return NULL;
}

void invwin_inventory(struct hrl_window *window, struct dc_map *map, struct pl_player *plr) {
    if (window == NULL) return;
    if (plr == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;
    int invstart = 0;
    char ch;

    WINDOW *invwin = derwin(window->win, window->lines, window->cols / 2, 0, 0);
    WINDOW *invwin_ex = NULL;

    int winsz = window->lines -4;

    int dislen = winsz;
    do {
        int invsz = inv_inventory_size(plr->player->inventory);
        struct inv_show_item *invlist = calloc(invsz, sizeof(struct inv_show_item) );
        inv_create_list(plr->player->inventory, invlist, invsz);

        /* TODO clean this shit up */
        switch (ch) {
            case ' ': invstart += dislen; break;
            case 'u': {
                    int item_idx;
                    ch = getch();
                    if (ch == 'q' || ch == 27) break;
                    if ( (item_idx = get_invid(ch) ) != -1) {
                        if ((item_idx + invstart) >= invsz) break;
                        msr_use_item(plr->player, invlist[item_idx +invstart].item);
                    }
                }
                break;
            case 'x': {
                    int item_idx;
                    delwin(invwin_ex);
                    ch = getch();
                    if (ch == 'q' || ch == 27) break;
                    if ( (item_idx = get_invid(ch) ) != -1) {
                        if ((item_idx + invstart) >= invsz) break;
                        invwin_ex = invwin_examine(window, invlist[item_idx +invstart].item);
                    }
                } 
                break;
            case 'd': {
                    int item_idx;
                    ch = getch();
                    if (ch == 'q' || ch == 27) break;
                    if ( (item_idx = get_invid(ch) ) != -1) {
                        if ((item_idx + invstart) >= invsz) break;
                        if (msr_remove_item(plr->player, invlist[item_idx +invstart].item ) == true) {
                            itm_insert_item(invlist[item_idx +invstart].item, map, &plr->player->pos);
                            free(invlist);
                            invsz = inv_inventory_size(plr->player->inventory);
                            invlist = calloc(invsz, sizeof(struct inv_show_item) );
                            inv_create_list(plr->player->inventory, invlist, invsz);
                        }
                    }
                }
                break;
            default: break;
        }

        touchwin(window->win);
        wclear(invwin);
        if ( (dislen = invwin_printlist(invwin, invlist, invsz, invstart, invstart +winsz) ) == -1) {
            invstart = 0;
            dislen = invwin_printlist(invwin, invlist, invsz, invstart, invstart +winsz);
        }
        mvwprintw(invwin, winsz +1, 1, "[q] exit, [space] next page.");
        mvwprintw(invwin, winsz +2, 1, "[d] drop, [x] examine.");
        mvwprintw(invwin, winsz +3, 1, "[u] use,  [w] wear.");
        wrefresh(invwin);
        free(invlist);
    }
    while((ch = getch()) != 27 && ch != 'q');

    delwin(invwin_ex);
    delwin(invwin);
    wrefresh(window->win);
}

