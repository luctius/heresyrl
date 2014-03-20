#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/param.h>

#include "ui.h"
#include "cmdline.h"
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
#include "game.h"
#include "ai.h"
#include "los.h"
#include "monster_action.h"

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
};

static struct hrl_window *map_win = NULL;
static struct hrl_window *char_win = NULL;
static struct hrl_window *msg_win = NULL;
static bool colours_generated = false;

static struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
static void win_destroy(struct hrl_window *window);

static void win_generate_colours(void) {
    int i = 1;

    if (colours_generated == false) {
        colours_generated = true;
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
}

static int hdr_lines = 0;
static int hdr_cols = 0;
bool ui_create(int cols, int lines) {
    win_generate_colours();

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

        if ( (map_win == NULL) || (char_win == NULL) ||(msg_win == NULL)  ) {
            map_win = win_create(map_lines-2, map_cols-2, 1, 1, HRL_WINDOW_TYPE_MAP);
            char_win = win_create(char_lines, char_cols, 1, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
            msg_win = win_create(msg_lines, msg_cols-1, map_lines, 1, HRL_WINDOW_TYPE_MESSAGE);
            lg_set_callback(gbl_log, NULL, msgwin_log_callback);
            msgwin_log_refresh(gbl_log);
            return true;
        }
        else {
            lg_set_callback(gbl_log, NULL, NULL);
            ui_destroy();
            map_win = NULL;
            char_win = NULL;
            msg_win = NULL;
            hdr_lines = 0;
            hdr_cols = 0;
            return ui_create(cols, lines);
        }
    }
    return false;
}

void ui_destroy() {
    lg_set_callback(gbl_log, NULL, NULL);
    if (map_win != NULL) win_destroy(map_win);
    if (char_win != NULL) win_destroy(char_win);
    if (msg_win != NULL) win_destroy(msg_win);
}

static struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type) {
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

void update_screen(void) {
    mapwin_display_map(gbl_game->current_map, &gbl_game->player_data.player->pos);
    charwin_refresh();
}

static int get_viewport(int p, int vps, int mps) {
    int hvps = round(vps / 2.0f);

    if (mps < vps) return 0;
    if (p < hvps) return 0;
    if (p > (mps - hvps) ) return mps - vps;
    return p - hvps;
}

static coord_t last_ppos = {0,0};
static void mapwin_display_map_noref(struct dc_map *map, coord_t *player) {
    coord_t scr_c = cd_create(0,0);

    if (map_win == NULL) return;
    if (dc_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;

    int x_max = (map_win->cols < map->size.x) ? map_win->cols : map->size.x;
    int y_max = (map_win->lines < map->size.y) ? map_win->lines : map->size.y;
    werase(map_win->win);
    curs_set(0);

    /* Only change viewport if the difference between the last change is big enough */
    coord_t ppos = *player;
    if (cd_pyth(&last_ppos, &ppos) > 10) {
        last_ppos = ppos;
    }
    else { ppos = last_ppos; }

    // Calculate top left of camera position
    scr_c.x = get_viewport(ppos.x, map_win->cols,  map->size.x);
    scr_c.y = get_viewport(ppos.y, map_win->lines, map->size.y);

    bool map_see = gbl_game->args_info->map_flag;

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            coord_t map_c = cd_create(xi+scr_c.x, yi+scr_c.y);
            struct dc_map_entity *me = sd_get_map_me(&map_c, map);
            struct tl_tile *tile = me->tile;

            if ( (me->visible == true) || (me->discovered == true) || (map_see == true) ) {
                int attr_mod = tile->icon_attr;
                char icon = tile->icon;
                bool modified = false;

                /* Modify wall colour */
                if (modified == false) {
                    if (me->visible == true) {
                        if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) == false) {
                            attr_mod = COLOR_PAIR(DPL_COLOUR_FG_YELLOW);
                            modified = true;
                        }
                    }
                }

                /* First see monster */
                if (modified == false) {
                    if ( (me->visible == true) || (map_see) ) {
                        if (me->monster != NULL) {
                            icon = me->monster->icon;
                            attr_mod = me->monster->icon_attr;
                            modified = true;
                        }
                    }
                }
                /* Else see items */
                if (modified == false) {
                    if ( (me->visible == true) || (map_see) ) {
                        if (inv_inventory_size(me->inventory) > 0) {
                            struct itm_item *i = inv_get_next_item(me->inventory, NULL);
                            icon = i->icon;
                            attr_mod = i->icon_attr;
                            modified = true;
                        }
                    }
                }

                if (modified == false) {
                    if (me->test_var == 2) {
                        attr_mod = COLOR_PAIR(DPL_COLOUR_BGB_RED);
                        modified = true;
                    }
                    if (me->test_var == 1) {
                        attr_mod = COLOR_PAIR(DPL_COLOUR_BGB_BLUE);
                        modified = true;
                    }
                }

                /* Otherwise visible traversable tiles */
                if (modified == false) {
                    if (me->visible == true) {
                        if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) == true){
                            attr_mod |= A_BOLD;
                            modified = true;
                        }
                    }
                }

                /* finaly, if out of direct sight, dim tile */
                if (modified == false) {
                    if (me->visible == false) {
                        if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) ){
                            attr_mod |= A_DIM;
                        }
                    }
                }

                if (has_colors() == TRUE) wattron(map_win->win, attr_mod);
                //mvwprintw(map_win->win, yi, xi, "%c", icon);
                mvwaddch(map_win->win, yi, xi, icon);
                if (has_colors() == TRUE) wattroff(map_win->win, attr_mod);
            }
        }
    }
}

void mapwin_display_map(struct dc_map *map, coord_t *player) {
    if (map_win == NULL) return;
    if (dc_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "update mapwin");
    mapwin_display_map_noref(map, player);
    wrefresh(map_win->win);
}

static WINDOW *mapwin_examine(struct dc_map_entity *me) {
    if (char_win == NULL) return NULL;
    if (me == NULL) return NULL;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return NULL;
    int y_ctr = 0;

    WINDOW *mapwin_ex = derwin(char_win->win, 0,0,0,0);
    touchwin(char_win->win);
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
            int len = strwrap(me->monster->description, char_win->cols, &desc, &len_lines);
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

void mapwin_overlay_examine_cursor(struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool examine_mode = true;

    if (map_win == NULL) return;
    if (char_win == NULL) return;
    if (dc_verify_map(map) == false) return;
    if (p_pos == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;

    coord_t e_pos = *p_pos;
    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    WINDOW *map_win_ex = NULL;

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
            case INP_KEY_ESCAPE:
            case INP_KEY_YES:
            case INP_KEY_EXAMINE:    examine_mode = false; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->size.y) e_pos.y = map->size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.x) e_pos.x = map->size.x -1;

        delwin(map_win_ex);
        map_win_ex = mapwin_examine(sd_get_map_me(&e_pos, map) );

        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "examining pos: (%d,%d), plr (%d,%d)", e_pos.x, e_pos.y, p_pos->x, p_pos->y);
        chtype oldch = mvwinch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwchgat(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
        wrefresh(map_win->win);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
    }
    while( ( (ch = inp_get_input()) != INP_KEY_ESCAPE) && (examine_mode == true) );

    delwin(map_win_ex);
    wrefresh(map_win->win);
}

void ui_animate_projectile(struct dc_map *map, coord_t path[], int path_len, char p) {
    if (gbl_game == NULL) return;
    if (gbl_game->player_data.player == NULL) return;

    coord_t *p_pos = &gbl_game->player_data.player->pos;
    int scr_x = get_viewport(p_pos->x, map_win->cols, map->size.x);
    int scr_y = get_viewport(p_pos->y, map_win->lines, map->size.y);

    for (int i = 1; i < path_len; i++) {
        if (sd_get_map_me(&path[i],map)->visible == true) {
            chtype oldch = mvwinch(map_win->win, path[i].y - scr_y, path[i].x - scr_x);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, p);
            wrefresh(map_win->win);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, oldch);
            usleep(50000);
        }
    }
}

bool mapwin_overlay_fire_cursor(struct gm_game *g, struct dc_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (map_win == NULL) return false;
    if (g == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    if (p_pos == NULL) return false;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return false;

    struct pl_player *plr = &g->player_data;
    if (plr == NULL) return false;
    if (msr_weapon_type_check(plr->player, WEAPON_TYPE_RANGED) == false) {
        You(plr->player, "do not wield a ranged weapon.");
        return false;
    }

    coord_t e_pos = *p_pos;

    /*find nearest enemy....*/
    int ign_cnt = 0;
    struct msr_monster *target = ai_get_nearest_enemy(plr->player, ign_cnt, map);
    if (target != NULL) {
        e_pos = target->pos;
        ign_cnt++;
    }

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    int length = 0;
    coord_t path[MAX(map->size.x, map->size.y)];
    int path_len = 0;

    do {
        mapwin_display_map_noref(map, &plr->player->pos);
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
            case INP_KEY_TAB: {

                if ( (target = ai_get_nearest_enemy(plr->player, ign_cnt, map) ) != NULL) {
                    e_pos = target->pos; 
                    ign_cnt++; 
                }
                else {
                    ign_cnt = 0;
                    target = ai_get_nearest_enemy(plr->player, ign_cnt, map);
                    e_pos = target->pos; 
                    ign_cnt++;
                }
            } 
            break;
            case INP_KEY_YES:
            case INP_KEY_FIRE: {
                if (ma_do_fire(plr->player, &e_pos) == true) {
                    mapwin_display_map(map, p_pos);
                    return true;
                }
                else Your(plr->player, "weapon(s) failed to fire.");
                fire_mode=false;
            }
            break;
            default: break;
        }
        if (fire_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->size.y) e_pos.y = map->size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.x) e_pos.x = map->size.x -1;

        length = cd_pyth(p_pos, &e_pos) +1;
        path_len = lof_calc_path(p_pos, &e_pos, path, ARRAY_SZ(path));
        for (int i = 0; i < MIN(path_len, length); i++) {
            mvwchgat(map_win->win, path[i].y - scr_y, path[i].x - scr_x, 1, A_NORMAL, DPL_COLOUR_BGB_RED, NULL);
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "mapwin", "fire_mode: [%d/%d] c (%d,%d) -> (%d,%d)", i, path_len, path[i].x, path[i].y, path[i].x - scr_x, path[i].y - scr_y);
        }
        wrefresh(map_win->win);
    }
    while((ch = inp_get_input()) != INP_KEY_ESCAPE && fire_mode);

    return false;
}

void msgwin_log_refresh(struct logging *lg) {
    struct queue *q = lg_logging_queue(lg);
    int log_sz = queue_size(q);
    int win_sz = msg_win->lines;
    struct log_entry *tmp_entry = NULL;

    if (msg_win == NULL) return;
    if (msg_win->type != HRL_WINDOW_TYPE_MESSAGE) return;
    wclear(msg_win->win);

    int max = MIN(win_sz, log_sz);
    int log_start = 0;

    int game_lvl_sz = 0;
    for (int i = log_sz; i > 0; i--) {
        tmp_entry = (struct log_entry *) queue_peek_nr(q, i-1);
        if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
            game_lvl_sz++;
            log_start = i -1;
            if (game_lvl_sz == max) i = 0;
        }
    }

    if (game_lvl_sz > 0) {
        int y = 0;
        for (int i = log_start; i < log_sz; i++) {
            tmp_entry = (struct log_entry *) queue_peek_nr(q, i);
            if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
                mvwprintw(msg_win->win, y++,1, "%s\n", tmp_entry->string);
            }
        }
    }
    wrefresh(msg_win->win);
}

void msgwin_log_callback(struct logging *lg, struct log_entry *entry, void *priv) {
    FIX_UNUSED(entry);
    FIX_UNUSED(priv);

    msgwin_log_refresh(lg);
}

void charwin_refresh() {
    if (char_win == NULL) return;
     struct pl_player *plr = &gbl_game->player_data;
     if (plr == NULL) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;
    werase(char_win->win);

    struct msr_monster *player = plr->player;

    int y = 0;
    int x = 0;
    mvwprintw(char_win->win, y++,x, "Name      %s", plr->name);
    mvwprintw(char_win->win, y++,x, "Gender    %s", msr_gender_string(player) );
    mvwprintw(char_win->win, y++,x, "Homeworld %s", "Void Born");
    mvwprintw(char_win->win, y++,x, "Career    %s", "Thug");
    mvwprintw(char_win->win, y++,x, "Turn      %d.%d", gbl_game->turn / MSR_ENERGY_TURN, gbl_game->turn % MSR_ENERGY_TURN);

    y++;
    int ws, bs, str, tgh, agi, intel, per, wil/*, fel*/;
    ws = msr_calculate_characteristic(player, MSR_CHAR_WEAPON_SKILL);
    bs = msr_calculate_characteristic(player, MSR_CHAR_BALISTIC_SKILL);
    str = msr_calculate_characteristic(player, MSR_CHAR_STRENGTH);
    tgh = msr_calculate_characteristic(player, MSR_CHAR_TOUGHNESS);
    agi = msr_calculate_characteristic(player, MSR_CHAR_AGILITY);
    intel = msr_calculate_characteristic(player, MSR_CHAR_INTELLIGENCE);
    per = msr_calculate_characteristic(player, MSR_CHAR_PERCEPTION);
    wil = msr_calculate_characteristic(player, MSR_CHAR_WILLPOWER);
    /*fel = msr_calculate_characteristic(player, MSR_CHAR_FELLOWSHIP);*/
    mvwprintw(char_win->win, y++,x, "WS   %d   BS   %d", ws,bs);
    mvwprintw(char_win->win, y++,x, "Str  %d   Tgh  %d", str, tgh);
    mvwprintw(char_win->win, y++,x, "Agi  %d   Int  %d", agi, intel);
    mvwprintw(char_win->win, y++,x, "Per  %d   Wil  %d", per, wil);
    //mvwprintw(char_win->win, y++,x, "Fellowship   [%d]%d", chr/10, chr%10);

    y++;
    mvwprintw(char_win->win, y++,x, "Wounds    [%2d/%2d]", player->cur_wounds, player->max_wounds);
    mvwprintw(char_win->win, y++,x, "Armour [%d][%d][%d][%d][%d][%d]", 
                                            msr_calculate_armour(player, MSR_HITLOC_HEAD),
                                            msr_calculate_armour(player, MSR_HITLOC_CHEST),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_LEG),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_LEG) );

    struct itm_item *item;
    for (int i = 0; i<2; i++) {
        bitfield_t loc = INV_LOC_MAINHAND_WIELD;
        if (i == 1) loc = INV_LOC_OFFHAND_WIELD;
        if ( (item == inv_get_item_from_location(player->inventory, loc) ) ) continue; /*ignore off=hand when wielding 2h weapon.*/

        y++;
        if ( (item = inv_get_item_from_location(player->inventory, loc) ) != NULL) {
            if (item->item_type == ITEM_TYPE_WEAPON) {
                struct item_weapon_specific *wpn = &item->specific.weapon;
                mvwprintw(char_win->win, y++,x, "%s Wpn: %s", (i==0) ? "Main" : "Secondary", item->sd_name);
                mvwprintw(char_win->win, y++,x, "  Dmg: %dD10 +%d   Pen: %d", wpn->nr_dmg_die, wpn->dmg_addition, wpn->penetration);
                if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                    mvwprintw(char_win->win, y++,x, "  Ammo: %d/%d", wpn->magazine_left, wpn->magazine_sz);
                    int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                    int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                    int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                    char *set = (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) ? "single" : 
                                (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                    char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                    char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                    mvwprintw(char_win->win, y++,x, "  Setting: %s (%s/%s/%s)", set, 
                            (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                }
            }
        }
    }

    if ( ( (item = inv_get_item_from_location(player->inventory, INV_LOC_MAINHAND_WIELD) ) != NULL) ||
         ( (item = inv_get_item_from_location(player->inventory, INV_LOC_OFFHAND_WIELD) ) != NULL) ) {
        y++;
        switch (player->wpn_sel) {
            case MSR_WEAPON_SELECT_OFF_HAND:
                mvwprintw(char_win->win, y++,x, "Using off-hand.");
                break;
            case MSR_WEAPON_SELECT_MAIN_HAND:
                mvwprintw(char_win->win, y++,x, "Using main-hand.");
                break;
            case MSR_WEAPON_SELECT_BOTH_HAND:
            case MSR_WEAPON_SELECT_DUAL_HAND:
                mvwprintw(char_win->win, y++,x, "Using both hands.");
                break;
            case MSR_WEAPON_SELECT_CREATURE1:
                mvwprintw(char_win->win, y++,x, "Unarmed.");
            default: break;
        }
    }

    wrefresh(char_win->win);
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
        mvwprintw(win, i, 1, "%c)  %c%s", inp_key_translate_idx(i), list[i+start].location[0], list[i+start].item->sd_name);
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
            if (inv_item_worn(inventory, item) == true) {
                invlist[i].location = "*";
            }
         }
    }
}

static WINDOW *invwin_examine(struct hrl_window *window, struct itm_item *item) {
    if (window == NULL) return NULL;
    if (itm_verify_item(item) == false) return NULL;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return NULL;

    WINDOW *invwin_ex = derwin(window->win, 0,0,0,0);
    touchwin(window->win);
    wclear(invwin_ex);
    mvwprintw(invwin_ex, 0, 1, "Description of %s.", item->ld_name);

    char **desc;
    int *len_lines;
    int len = strwrap(item->description, window->cols, &desc, &len_lines);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            mvwprintw(invwin_ex, 2+i, 0, desc[i]);
        }
    }
    else {
        mvwprintw(invwin_ex, 2, 0, "No description available.");
    }

    free(desc);
    free(len_lines);
    wrefresh(invwin_ex);
    return invwin_ex;
}

bool invwin_inventory(struct dc_map *map, struct pl_player *plr) {
    if (map_win == NULL) return false;
    if (plr == NULL) return false;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return false;
    int invstart = 0;
    struct itm_item *item = NULL;
    bool inventory = true;

    WINDOW *invwin = derwin(map_win->win, map_win->lines, map_win->cols / 2, 0, 0);
    WINDOW *invwin_ex = NULL;

    int winsz = map_win->lines -4;
    int dislen = winsz;

    int ch = INP_KEY_NONE;
    do {
        int invsz = inv_inventory_size(plr->player->inventory);
        struct inv_show_item *invlist = calloc(invsz, sizeof(struct inv_show_item) );
        inv_create_list(plr->player->inventory, invlist, invsz);

        mapwin_display_map_noref(map, &plr->player->pos);
        touchwin(map_win->win);
        wclear(invwin);
        if ( (dislen = invwin_printlist(invwin, invlist, invsz, invstart, invstart +winsz) ) == -1) {
            invstart = 0;
            dislen = invwin_printlist(invwin, invlist, invsz, invstart, invstart +winsz);
        }
        mvwprintw(invwin, winsz +1, 1, "[q] exit, [space] next page.");
        mvwprintw(invwin, winsz +2, 1, "[d] drop, [x] examine.");
        mvwprintw(invwin, winsz +3, 1, "[U] use,  [w] wield/wear.");
        wrefresh(invwin);

        /* TODO clean this shit up */
        switch (ch) {
            case INP_KEY_YES: invstart += dislen; break;
            case INP_KEY_INVENTORY: inventory = false; break;
            case INP_KEY_UP_RIGHT: 
            case INP_KEY_USE: {
                    mvwprintw(invwin, winsz, 1, "Use which item?.");
                    wrefresh(invwin);
                    delwin(invwin_ex);

                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;
                    item = invlist[item_idx +invstart].item;
                    free(invlist);
                    delwin(invwin);
                    charwin_refresh();
                    mapwin_display_map(map, &plr->player->pos);

                    return ma_do_use(plr->player, item);
                }
                break;
            case INP_KEY_WEAR: {
                    mvwprintw(invwin, winsz, 1, "Wear which item?.");
                    wrefresh(invwin);
                    delwin(invwin_ex);
                    charwin_refresh();

                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;
                    item = invlist[item_idx +invstart].item;
                    free(invlist);
                    delwin(invwin);
                    charwin_refresh();
                    mapwin_display_map(map, &plr->player->pos);

                    if (inv_item_worn(plr->player->inventory, item) == true) {
                        return ma_do_remove(plr->player, item);
                    }
                    else {
                        return ma_do_wear(plr->player, item);
                    }
                } 
                break;
            case INP_KEY_EXAMINE: {
                    mvwprintw(invwin, winsz, 1, "Examine which item?.");
                    wrefresh(invwin);
                    delwin(invwin_ex);
                    charwin_refresh();

                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;
                    item = invlist[item_idx +invstart].item;
                    free(invlist);

                    invwin_ex = invwin_examine(char_win, invlist[item_idx +invstart].item);
                } 
                break;
            case INP_KEY_DROP: {
                    mvwprintw(invwin, winsz, 1, "Drop which item?.");
                    wrefresh(invwin);
                    delwin(invwin_ex);
                    charwin_refresh();

                    invsz = inv_inventory_size(plr->player->inventory);
                    int item_idx = inp_get_input_idx();
                    if (item_idx == INP_KEY_ESCAPE) break;
                    if ((item_idx + invstart) >= invsz) break;
                    item = invlist[item_idx +invstart].item;
                    free(invlist);
                    struct itm_item *items[1] = {item};
                    return ma_do_drop(plr->player, items, 1);
                }
                break;
            default: break;
        }

        if (invwin_ex == NULL) {
            charwin_refresh();
        }

    } while((inventory != false) && (ch = inp_get_input() ) != INP_KEY_ESCAPE);

    delwin(invwin_ex);
    delwin(invwin);

    mapwin_display_map(map, &plr->player->pos);
    charwin_refresh();

    return false;
}

void character_window(void) {

/*
Career: 
Rank:
Origin: 
Divination: 
Wounds  0/0         Fate:  0/0
Insanity: 0         Corruption: 0
XP: 100             Spend: 3000

[WS BS ... ]
[      ... ]

Amour                 AP          location
guard flak      

Condition                   Note
....                        ....

Skill       Level
dodge       basic
...         ...
                                                 |
Talent                      Note                 |
Basic weapon traning SP     ...                  |

*/
}

void levelup_selection_window(void) {
/*

*/
}

