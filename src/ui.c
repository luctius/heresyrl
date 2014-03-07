#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <unistd.h>
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
#include "input.h"
#include "dowear.h"

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

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "generating colours");

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
                /*
                else if (sd_get_map_me(&map_c, map)->item != NULL) {
                    icon = sd_get_map_me(&map_c, map)->item->icon;
                    attr_mod = sd_get_map_me(&map_c, map)->item->icon_attr;
                }
                */
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

static WINDOW *mapwin_examine(struct hrl_window *window, struct dc_map_entity *me) {
    if (window == NULL) return NULL;
    if (me == NULL) return NULL;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return NULL;
    int y_ctr = 0;

    WINDOW *mapwin_ex = derwin(window->win, 0,0,0,0);
    touchwin(window->win);
    wclear(mapwin_ex);

    mvwprintw(mapwin_ex, y_ctr++, 1, "Upon a %s.", me->tile->ld_name);

    if (me->monster != NULL) {
        y_ctr++;

        if (me->monster->is_player == true) {
            mvwprintw(mapwin_ex, y_ctr++, 1, "You see yourself.");
        } else {
            mvwprintw(mapwin_ex, y_ctr++, 1, "You see %s.", me->monster->ld_name);
            y_ctr++;

            char **desc;
            int *len_lines;
            int len = strwrap(me->monster->description, window->cols, &desc, &len_lines);
            for (int i = 0; i < len; i++) {
                mvwprintw(mapwin_ex, y_ctr++, 0, desc[i]);
            }
            free(desc);
            free(len_lines);
        }
    }

    if ( (inv_inventory_size(me->inventory) > 0) && (TILE_HAS_ATTRIBUTE(me->tile, TILE_ATTR_TRAVERSABLE) ) ) {
        y_ctr++;
        mvwprintw(mapwin_ex, y_ctr++, 1, "The %s contains:", me->tile->sd_name);
        struct itm_item *i = NULL;
        while ( (i = inv_get_next_item(me->inventory, i) ) != NULL) {
            mvwprintw(mapwin_ex, y_ctr++, 1, " - %s", i->ld_name);
        }
    }

    wrefresh(mapwin_ex);
    return mapwin_ex;
}

void mapwin_overlay_examine_cursor(struct hrl_window *mapwin, struct hrl_window *charwin, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool examine_mode = true;

    if (mapwin == NULL) return;
    if (charwin == NULL) return;
    if (map == NULL) return;
    if (p_pos == NULL) return;
    if (mapwin->type != HRL_WINDOW_TYPE_MAP) return;
    if (charwin->type != HRL_WINDOW_TYPE_CHARACTER) return;

    coord_t e_pos = *p_pos;
    int scr_x = get_viewport(p_pos->x, mapwin->cols, map->size.x);
    int scr_y = get_viewport(p_pos->y, mapwin->lines, map->size.y);

    WINDOW *mapwin_ex = NULL;

    do {
        switch (ch) {
            case INP_KEY_UP_LEFT:    e_pos.y--; e_pos.x--; break;
            case INP_KEY_UP:         e_pos.y--; break;
            case INP_KEY_UP_RIGHT:   e_pos.y--; e_pos.x++; break;
            case INP_KEY_RIGHT:      e_pos.x++; break;
            case INP_KEY_DOWN_RIGHT: e_pos.y++; e_pos.x++; break;
            case INP_KEY_DOWN:       e_pos.y++; break;
            case INP_KEY_DOWN_LEFT:  e_pos.y++; e_pos.x--; break;
            case INP_KEY_LEFT:       e_pos.x--; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->size.y -1) e_pos.y = map->size.y;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.y -1) e_pos.x = map->size.x;

        delwin(mapwin_ex);
        mapwin_ex = mapwin_examine(charwin, sd_get_map_me(&e_pos, map) );

        chtype oldch = mvwinch(mapwin->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwchgat(mapwin->win, e_pos.y - scr_y, e_pos.x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
        wrefresh(mapwin->win);
        mvwaddch(mapwin->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
    }
    while((ch = inp_get_input()) != INP_KEY_ESCAPE && examine_mode);

    delwin(mapwin_ex);
    wrefresh(mapwin->win);
}

void mapwin_overlay_fire_cursor(struct hrl_window *window, struct pl_player *plr, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (window == NULL) return;
    if (map == NULL) return;
    if (p_pos == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MAP) return;
    if (fght_ranged_weapons_check(plr->player, plr->weapon_selection) == false) {
        You("wield no ranged weapon.");
        return;
    }

    coord_t e_pos = *p_pos;
    /*find nearest enemy....*/


    int scr_x = get_viewport(p_pos->x, window->cols, map->size.x);
    int scr_y = get_viewport(p_pos->y, window->lines, map->size.y);

    int length = 0;
    coord_t path[MAX(map->size.x, map->size.y)];
    int path_len = 0;

    do {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "entering fire_mode (%d,%d) -> (%d,%d)", p_pos->x, p_pos->y, e_pos.x, e_pos.y);
        switch (ch) {
            case INP_KEY_UP_LEFT:    e_pos.y--; e_pos.x--; break;
            case INP_KEY_UP:         e_pos.y--; break;
            case INP_KEY_UP_RIGHT:   e_pos.y--; e_pos.x++; break;
            case INP_KEY_RIGHT:      e_pos.x++; break;
            case INP_KEY_DOWN_RIGHT: e_pos.y++; e_pos.x++; break;
            case INP_KEY_DOWN:       e_pos.y++; break;
            case INP_KEY_DOWN_LEFT:  e_pos.y++; e_pos.x--; break;
            case INP_KEY_LEFT:       e_pos.x--; break;
            case INP_KEY_YES:
            case INP_KEY_FIRE: {
                bool blocked = false;
                You("fire (%d,%d)", e_pos.x, e_pos.y);
                path_len = fght_shoot(plr->player, map, plr->rof_setting_rhand, plr->rof_setting_lhand, plr->weapon_selection, p_pos, &e_pos, path, ARRAY_SZ(path) );
                for (int i = 1; (i < path_len) && (blocked == false); i++) {
                    chtype oldch = mvwinch(window->win, path[i].y - scr_y, path[i].x - scr_x);
                    mvwaddch(window->win, path[i].y - scr_y, path[i].x - scr_x, '*');
                    wrefresh(window->win);
                    mvwaddch(window->win, path[i].y - scr_y, path[i].x - scr_x, oldch);
                    usleep(50000);
                }
                fire_mode=false;
            }
            break;
            default: break;
        }
        if (fire_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->size.y -1) e_pos.y = map->size.y;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.y -1) e_pos.x = map->size.x;

        length = cd_pyth(p_pos, &e_pos) +1;
        path_len = fght_calc_lof_path(p_pos, &e_pos, path, ARRAY_SZ(path));
        for (int i = 0; i < MIN(path_len, length); i++) {
            mvwchgat(window->win, path[i].y - scr_y, path[i].x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "fire_mode: [%d/%d] c (%d,%d) -> (%d,%d)", i, path_len, path[i].x, path[i].y, path[i].x - scr_x, path[i].y - scr_y);
        }

        wrefresh(window->win);
        mapwin_display_map_noref(window, map, p_pos);
    }
    while((ch = inp_get_input()) != INP_KEY_ESCAPE && fire_mode);
}

void msgwin_log_refresh(struct hrl_window *window, struct logging *lg) {
    struct queue *q = lg_logging_queue(lg);
    int log_sz = queue_size(q);
    int win_sz = window->lines;
    struct log_entry *tmp_entry = NULL;

    if (window == NULL) return;
    if (window->type != HRL_WINDOW_TYPE_MESSAGE) return;
    wclear(window->win);

    int max = MIN(win_sz, log_sz);
    int log_start = 0;

    int game_lvl_sz = 0;
    for (int i = log_sz -1; i > 0; i--) {
        tmp_entry = (struct log_entry *) queue_peek_nr(q, i);
        if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
            game_lvl_sz++;
            log_start = i;
            if (game_lvl_sz == max) i = 0;
        }
    }

    if (game_lvl_sz > 0) {
        int y = 0;
        for (int i = log_start; i < log_sz; i++) {
            tmp_entry = (struct log_entry *) queue_peek_nr(q, i);
            if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
                mvwprintw(window->win, y++,1, "%s\n", tmp_entry->string);
            }
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
    int ws, bs, str, tgh, agi, intel, per, wil, fel;
    ws = msr_calculate_characteristic(player, MSR_CHAR_WEAPON_SKILL);
    bs = msr_calculate_characteristic(player, MSR_CHAR_BALISTIC_SKILL);
    str = msr_calculate_characteristic(player, MSR_CHAR_STRENGTH);
    tgh = msr_calculate_characteristic(player, MSR_CHAR_TOUCHNESS);
    agi = msr_calculate_characteristic(player, MSR_CHAR_AGILITY);
    intel = msr_calculate_characteristic(player, MSR_CHAR_INTELLIGENCE);
    per = msr_calculate_characteristic(player, MSR_CHAR_PERCEPTION);
    wil = msr_calculate_characteristic(player, MSR_CHAR_WILLPOWER);
    fel = msr_calculate_characteristic(player, MSR_CHAR_FELLOWSHIP);
    mvwprintw(window->win, y++,x, "WS     %d   BS    %d", ws,bs);
    mvwprintw(window->win, y++,x, "Str  [%d]%d   Tgh [%d]%d", str/10, str%10, tgh/10, tgh%10);
    mvwprintw(window->win, y++,x, "Agi  [%d]%d   Int [%d]%d", agi/10, agi%10, intel/10, intel%10);
    mvwprintw(window->win, y++,x, "Per  [%d]%d   Wil [%d]%d", per/10, per%10, wil/10, wil%10);
    //mvwprintw(window->win, y++,x, "Fellowship   [%d]%d", chr/10, chr%10);

    y++;
    mvwprintw(window->win, y++,x, "Wounds    [%2d/%2d]", player->cur_wounds, player->max_wounds);
    mvwprintw(window->win, y++,x, "Armour [%d][%d][%d][%d][%d][%d]", 10,10,10,10,10,10);

    y++;
    struct itm_item *item;
    if ( (item = inv_get_item_from_location(player->inventory, INV_LOC_RIGHT_WIELD) ) != NULL) {
        if (item->item_type == ITEM_TYPE_WEAPON) {
            struct item_weapon_specific *wpn = &item->specific.weapon;
            mvwprintw(window->win, y++,x, "Right Wpn: %s", item->sd_name);
            mvwprintw(window->win, y++,x, "  Dmg: %dD10 +%d", wpn->nr_dmg_die, wpn->dmg_addition);
            if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                mvwprintw(window->win, y++,x, "  Ammo: %d/%d", wpn->magazine_left, wpn->magazine_sz);
                int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                char *set = (plr->rof_setting_rhand == WEAPON_ROF_SETTING_SINGLE) ? "single" : 
                            (plr->rof_setting_rhand == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                mvwprintw(window->win, y++,x, "  Setting: %s (%s/%s/%s)", set, 
                        (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
            }
        }
    }

    y++;
    if ( (item = inv_get_item_from_location(player->inventory, INV_LOC_LEFT_WIELD) ) != NULL) {
        if (item->item_type == ITEM_TYPE_WEAPON) {
            struct item_weapon_specific *wpn = &item->specific.weapon;
            mvwprintw(window->win, y++,x, "Left Wpn: %s", item->sd_name);
            mvwprintw(window->win, y++,x, "  Damage: %dD10 +%d", wpn->nr_dmg_die, wpn->dmg_addition);
            if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                mvwprintw(window->win, y++,x, "  Ammo: %d/%d", wpn->magazine_left, wpn->magazine_sz);
                int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                char *set = (plr->rof_setting_lhand == WEAPON_ROF_SETTING_SINGLE) ? "single" : 
                            (plr->rof_setting_lhand == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                mvwprintw(window->win, y++,x, "  Setting: %s (%s/%s/%s)", set, 
                        (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
            }
        }
    }

    if ( ( (item = inv_get_item_from_location(player->inventory, INV_LOC_RIGHT_WIELD) ) != NULL) ||
         ( (item = inv_get_item_from_location(player->inventory, INV_LOC_LEFT_WIELD) ) != NULL) ) {
        y++;
        switch (plr->weapon_selection) {
            case FGHT_WEAPON_SELECT_LEFT_HAND:
                mvwprintw(window->win, y++,x, "Using left hand.");
                break;
            case FGHT_WEAPON_SELECT_RIGHT_HAND:
                mvwprintw(window->win, y++,x, "Using right hand.");
                break;
            case FGHT_WEAPON_SELECT_BOTH_HAND:
            case FGHT_WEAPON_SELECT_DUAL_HAND:
                mvwprintw(window->win, y++,x, "Using both hands.");
                break;
            default: break;
        }
        y++;
    }

    wrefresh(window->win);
}

/* Beware of dragons here..... */

struct inv_show_item {
    char *location;
    struct itm_item *item;
};

static int invwin_printlist(WINDOW *win, struct inv_show_item list[], int list_sz, int start, int end) {
    int max = MIN(list_sz, end);
    if (list_sz == 0) {
        mvwprintw(win, 1, 1, "%s", "Your inventory is empty");
        return 0;
    }

    max = MIN(max, INP_KEY_MAX_IDX);
    if (start >= max) return -1;

    for (int i = 0; i < max; i++) {
        mvwprintw(win, i, 1, "%c  %c%s", inp_key_translate_idx(i), list[i+start].location[0], list[i+start].item->sd_name);
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

static WINDOW *invwin_examine(struct hrl_window *window, struct itm_item *item) {
    if (window == NULL) return NULL;
    if (item == NULL) return NULL;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return NULL;

    char **desc;
    int *len_lines;
    int len = strwrap(item->description, window->cols, &desc, &len_lines);
    if (len > 0) {
        WINDOW *invwin_ex = derwin(window->win, 0,0,0,0);
        touchwin(window->win);
        wclear(invwin_ex);

        mvwprintw(invwin_ex, 0, 1, "Description of %s.", item->ld_name);
        for (int i = 0; i < len; i++) {
            mvwprintw(invwin_ex, 2+i, 0, desc[i]);
        }
        free(desc);
        free(len_lines);
        wrefresh(invwin_ex);
        return invwin_ex;
    }
    return NULL;
}

void invwin_inventory(struct hrl_window *mapwin, struct hrl_window *charwin, struct dc_map *map, struct pl_player *plr) {
    if (mapwin == NULL) return;
    if (plr == NULL) return;
    if (mapwin->type != HRL_WINDOW_TYPE_MAP) return;
    int invstart = 0;
    int ch = 0;

    WINDOW *invwin = derwin(mapwin->win, mapwin->lines, mapwin->cols / 2, 0, 0);
    WINDOW *invwin_ex = NULL;

    int winsz = mapwin->lines -4;

    int dislen = winsz;
    do {
        int invsz = inv_inventory_size(plr->player->inventory);
        struct inv_show_item *invlist = calloc(invsz, sizeof(struct inv_show_item) );
        inv_create_list(plr->player->inventory, invlist, invsz);

        /* TODO clean this shit up */
        switch (ch) {
            case INP_KEY_YES: invstart += dislen; break;
            case INP_KEY_USE: {
                    mvwprintw(invwin, winsz, 1, "Use which item?.");
                    wrefresh(invwin);
                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;

                    msr_use_item(plr->player, invlist[item_idx +invstart].item);
                }
                break;
            case INP_KEY_WEAR: {
                    mvwprintw(invwin, winsz, 1, "Wear which item?.");
                    wrefresh(invwin);
                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;
                    if (inv_get_item_location(plr->player->inventory, invlist[item_idx+invstart].item) == INV_LOC_INVENTORY) {
                        dw_wear_item(plr->player, invlist[item_idx+invstart].item);
                    }
                    else {
                        dw_remove_item(plr->player, invlist[item_idx+invstart].item);
                    }
                    inv_create_list(plr->player->inventory, invlist, invsz);
                } 
                break;
            case INP_KEY_EXAMINE: {
                    mvwprintw(invwin, winsz, 1, "Examine which item?.");
                    wrefresh(invwin);
                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;

                    delwin(invwin_ex);
                    invwin_ex = invwin_examine(charwin, invlist[item_idx +invstart].item);
                } 
                break;
            case INP_KEY_DROP: {
                    mvwprintw(invwin, winsz, 1, "Drop which item?.");
                    wrefresh(invwin);
                    invsz = inv_inventory_size(plr->player->inventory);
                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;

                    if (msr_remove_item(plr->player, invlist[item_idx +invstart].item ) == true) {
                        itm_insert_item(invlist[item_idx +invstart].item, map, &plr->player->pos);
                        free(invlist);
                        invsz = inv_inventory_size(plr->player->inventory);
                        invlist = calloc(invsz, sizeof(struct inv_show_item) );
                        inv_create_list(plr->player->inventory, invlist, invsz);
                    }
                }
                break;
            default: break;
        }

        charwin_refresh(charwin, plr);
        mapwin_display_map_noref(mapwin, map, &plr->player->pos);
        touchwin(mapwin->win);
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
    while((ch = inp_get_input() ) != INP_KEY_ESCAPE);

    delwin(invwin_ex);
    delwin(invwin);

    mapwin_display_map_noref(mapwin, map, &plr->player->pos);
    wrefresh(mapwin->win);
    charwin_refresh(charwin, plr);
    wrefresh(charwin->win);
}

