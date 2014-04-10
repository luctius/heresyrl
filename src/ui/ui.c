#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <sys/param.h>
#include <string.h>

#include "ui.h"
#include "ui_common.h"

#include "options.h"

#include "tiles.h"
#include "fight.h"
#include "logging.h"
#include "player.h"
#include "inventory.h"
#include "input.h"
#include "dowear.h"
#include "game.h"
#include "ai/ai_utils.h"
#include "fov/sight.h"
#include "monster/monster.h"
#include "monster/monster_action.h"
#include "items/items.h"
#include "dungeon/dungeon_map.h"

static int hdr_lines = 0;
static int hdr_cols = 0;
bool ui_create(int cols, int lines) {
    win_generate_colours();

    if ( (hdr_lines != lines) || (hdr_cols != cols) ) {
        hdr_lines = lines;
        hdr_cols = cols;

        if ( (lines < 24) || (cols < 40) ) {
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
            msgwin_log_refresh(gbl_log, NULL);
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

void update_screen(void) {
    if (gbl_game->running) {
        mapwin_display_map(gbl_game->current_map, &gbl_game->player_data.player->pos);
        charwin_refresh();

        wrefresh(map_win->win);
        wrefresh(char_win->win);
    }
}

static void mapwin_display_map_noref(struct dm_map *map, coord_t *player) {
    coord_t scr_c = cd_create(0,0);

    if (map_win == NULL) return;
    if (dm_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;

    int x_max = (map_win->cols < map->size.x) ? map_win->cols : map->size.x;
    int y_max = (map_win->lines < map->size.y) ? map_win->lines : map->size.y;
    werase(map_win->win);
    curs_set(0);

    /* Only change viewport if the difference between the last change is big enough */
    coord_t ppos = *player;
    int view_pos_diff = MAX(map_win->cols, map_win->lines) / 10;
    if (cd_pyth(&last_ppos, &ppos) > view_pos_diff) {
        last_ppos = ppos;
    }
    else { ppos = last_ppos; }

    // Calculate top left of camera position
    scr_c.x = get_viewport(ppos.x, map_win->cols,  map->size.x);
    scr_c.y = get_viewport(ppos.y, map_win->lines, map->size.y);

    bool map_see = options.debug_show_map;

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            coord_t map_c = cd_create(xi+scr_c.x, yi+scr_c.y);
            struct dm_map_entity *me = dm_get_map_me(&map_c, map);
            struct tl_tile *tile = me->tile;

            if ( (me->visible == true) || (me->discovered == true) || (map_see == true) ) {
                int attr_mod = get_colour(TERM_COLOUR_L_DARK);
                char icon = tile->icon;
                bool modified = false;

                if (me->icon_override != -1) icon = me->icon_override;
                if (me->icon_attr_override != -1) {
                    if (me->visible == true) {
                        attr_mod = me->icon_attr_override;
                        modified = true;
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
                        if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) == true) {
                            if (inv_inventory_size(me->inventory) > 0) {
                                struct itm_item *i = inv_get_next_item(me->inventory, NULL);
                                icon = i->icon;
                                attr_mod = i->icon_attr;
                                modified = true;
                            }
                        }
                    }
                }

                if (modified == false) {
                    if (me->visible == true) {
                        if (me->light_level > 0) {
                            { //if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) ){
                                attr_mod = get_colour(TERM_COLOUR_YELLOW);
                                modified = true;
                            }
                        }
                    }
                }

                /* Otherwise visible traversable tiles */
                if (modified == false) {
                    if (me->visible == true) {
                        attr_mod = tile->icon_attr;
                        modified = true;
                    }
                }

                /* test colours */
                {
                    if (me->test_var == 2) {
                        attr_mod = get_colour(TERM_COLOUR_BLUE);
                        modified = true;
                    }
                    if (me->test_var == 1) {
                        attr_mod = get_colour(TERM_COLOUR_RED);
                        modified = true;
                    }
                }

                if (has_colors() == TRUE) wattron(map_win->win, attr_mod);
                mvwaddch(map_win->win, yi, xi, icon);
                if (has_colors() == TRUE) wattroff(map_win->win, attr_mod);
            }
        }
    }
}

void mapwin_display_map(struct dm_map *map, coord_t *player) {
    if (map_win == NULL) return;
    if (dm_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "update mapwin");
    mapwin_display_map_noref(map, player);
    if (options.refresh) wrefresh(map_win->win);
}

static void mapwin_examine(struct dm_map_entity *me) {
    if (char_win == NULL) return;
    if (me == NULL) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;

    werase(char_win->win);
    textwin_init(char_win,1,0,0,0);

    if (me->visible || me->in_sight) {
        textwin_add_text(char_win, "%s.\n", me->tile->ld_name);
        textwin_add_text(char_win, "\n");

        if (me->visible) {
            if (me->monster != NULL) {

                if (me->monster->is_player == true) {
                    textwin_add_text(char_win, "You see yourself.\n");
                } else {
                    textwin_add_text(char_win, "You see %s.\n", me->monster->ld_name);
                }
            }

            textwin_add_text(char_win, "\n");

            if ( (inv_inventory_size(me->inventory) > 0) && (TILE_HAS_ATTRIBUTE(me->tile, TILE_ATTR_TRAVERSABLE) ) ) {
                textwin_add_text(char_win, "The %s contains:\n", me->tile->sd_name);
                struct itm_item *i = NULL;
                while ( (i = inv_get_next_item(me->inventory, i) ) != NULL) {
                    textwin_add_text(char_win, " - %s\n", i->ld_name);
                }
            }
        }
    }
    else textwin_add_text(char_win, "You can not see this place.\n");

    textwin_display_text(char_win);
}

void mapwin_overlay_examine_cursor(struct dm_map *map, coord_t *p_pos) {
    int ch = '0';
    bool examine_mode = true;

    if (map_win == NULL) return;
    if (char_win == NULL) return;
    if (dm_verify_map(map) == false) return;
    if (p_pos == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;

    coord_t e_pos = *p_pos;
    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

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
            case INP_KEY_YES:
            case INP_KEY_EXAMINE:    examine_mode = false; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->size.y) e_pos.y = map->size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->size.x) e_pos.x = map->size.x -1;

        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ui", "examining pos: (%d,%d), plr (%d,%d)", e_pos.x, e_pos.y, p_pos->x, p_pos->y);
        chtype oldch = mvwinch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, (oldch & 0xFF) | get_colour(TERM_COLOUR_BG_RED) );
        wrefresh(map_win->win);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);

        mapwin_examine(dm_get_map_me(&e_pos, map) );
    }
    while( ( (ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE) && (examine_mode == true) );

    wrefresh(map_win->win);
}

void targetwin_examine(struct hrl_window *window, struct dm_map *map, struct msr_monster *player, coord_t *pos, struct itm_item *witem) {
    if (window == NULL) return;
    if (witem == NULL) return;
    if (msr_verify_monster(player) == false) return;
    if (itm_verify_item(witem) == false) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;
    struct dm_map_entity *me = dm_get_map_me(pos, map);

    if (me->in_sight == false) {
        charwin_refresh();
        return;
    }

    werase(window->win);
    textwin_init(window,1,0,0,window->lines -4);

    if (me->monster != NULL) {
        textwin_add_text(window,"Target: %s.\n", msr_ldname(me->monster) );
    }
    else textwin_add_text(window,"No Target.\n");

    int tohit = fght_ranged_calc_tohit(player, pos, FGHT_MAIN_HAND);
    textwin_add_text(window,"Total change of hitting: %d.\n", tohit);
    textwin_add_text(window,"Ballistic Skill: %d\n\n", msr_calculate_characteristic(player, MSR_CHAR_BALISTIC_SKILL) );

    int idx = 0;
    struct tohit_desc *thd = NULL;
    while ( (thd = fght_get_tohit_mod_description(idx++) ) != NULL) {
        textwin_add_text(window,"%c %s (%d).\n", (thd->modifier > 0) ? '+' : '-', thd->description, thd->modifier);
    }

    textwin_add_text(window,"\n");
    textwin_display_text(window);

    textwin_init(window,1,window->lines -4,0,0);
    textwin_add_text(window, "Calculated: %s.\n", witem->sd_name);
    if (wpn_is_catergory(witem, WEAPON_CATEGORY_THROWN_GRENADE) ) {
        textwin_add_text(window,"Timer: %d.%d.\n", witem->energy / TT_ENERGY_TURN, witem->energy % TT_ENERGY_TURN);
    }

    textwin_display_text(window);
}

bool mapwin_overlay_fire_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (map_win == NULL) return false;
    if (g == NULL) return false;
    if (dm_verify_map(map) == false) return false;
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
    struct msr_monster *target = aiu_get_nearest_enemy(plr->player, ign_cnt, map);
    if (target != NULL) {
        e_pos = target->pos;
        ign_cnt++;
    }

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    coord_t *path;
    int path_len = 0;

    do {
        mapwin_display_map_noref(map, &plr->player->pos);

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

                if ( (target = aiu_get_nearest_enemy(plr->player, ign_cnt, map) ) != NULL) {
                    e_pos = target->pos; 
                    ign_cnt++; 
                }
                else {
                    ign_cnt = 0;
                    target = aiu_get_nearest_enemy(plr->player, ign_cnt, map);
                    if (target != NULL) {
                        e_pos = target->pos; 
                        ign_cnt++;
                    }
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

        lg_debug("entering fire_mode (%d,%d) -> (%d,%d)", p_pos->x, p_pos->y, e_pos.x, e_pos.y);

        path_len = sgt_los_path(gbl_game->sight, gbl_game->current_map, p_pos, &e_pos, &path, false);
        for (int i = 1; i < path_len; i++) {
            lg_debug("point[%d] in projectile path: (%d,%d)", i, path[i].x, path[i].y);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
        }
        if (path_len > 0) free(path);

        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
        wrefresh(map_win->win);

        struct itm_item *witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_RANGED, FGHT_MAIN_HAND);
        if (witem == NULL) witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_RANGED, FGHT_OFF_HAND);
        targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, witem);
    }
    while((ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE && fire_mode);

    return false;
}

struct itm_item *find_throw_weapon(struct msr_monster *player, int idx) {
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(player->inventory, item) ) != NULL) {
        if (wpn_is_type(item, WEAPON_TYPE_THROWN) ) {
            if (idx == 0) {
                break;
            }
            idx--;
        }
    }
    return item;
}

bool mapwin_overlay_throw_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (map_win == NULL) return false;
    if (g == NULL) return false;
    if (dm_verify_map(map) == false) return false;
    if (p_pos == NULL) return false;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return false;

    struct pl_player *plr = &g->player_data;
    if (plr == NULL) return false;
    if (find_throw_weapon(plr->player, 0) == NULL) {
        You(plr->player, "do not have a throwing weapon.");
        return false;
    }
    coord_t e_pos = *p_pos;

    /*find nearest enemy....*/
    int ign_cnt = 0;
    struct msr_monster *target = aiu_get_nearest_enemy(plr->player, ign_cnt, map);
    if (target != NULL) {
        e_pos = target->pos;
        ign_cnt++;
    }

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    coord_t *path;
    int path_len = 0;

    int weapon_idx = 0;
    struct itm_item *item = find_throw_weapon(plr->player, weapon_idx);
    item->energy = TT_ENERGY_TURN;

    do {
        mapwin_display_map_noref(map, &plr->player->pos);

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

                if ( (target = aiu_get_nearest_enemy(plr->player, ign_cnt, map) ) != NULL) {
                    e_pos = target->pos; 
                    ign_cnt++; 
                }
                else {
                    ign_cnt = 0;
                    target = aiu_get_nearest_enemy(plr->player, ign_cnt, map);
                    if (target != NULL) {
                        e_pos = target->pos; 
                        ign_cnt++;
                    }
                }
            } 
            break;
            case INP_KEY_WEAPON_SETTING: 
                if (weapon_idx > 0) {
                    weapon_idx -= 0;
                    item = find_throw_weapon(plr->player, weapon_idx);
                    item->energy = TT_ENERGY_TURN;
                }
                break;
            case INP_KEY_WEAPON_SELECT: 
                if (find_throw_weapon(plr->player, weapon_idx+1) != NULL) {
                    weapon_idx += 1;
                    item = find_throw_weapon(plr->player, weapon_idx);
                    item->energy = TT_ENERGY_TURN;
                }
                break;
            case INP_KEY_MINUS:
                    item->energy -= TT_ENERGY_TURN;
                    if (item->energy <= 0) item->energy = TT_ENERGY_TICK;
                break;
            case INP_KEY_PLUS:
                    item->energy += TT_ENERGY_TURN;
                    if (item->energy >= (TT_ENERGY_TURN * 10) ) item->energy = TT_ENERGY_TURN * 10;
                    if ((item->energy % TT_ENERGY_TURN) > 0) item->energy = (item->energy / TT_ENERGY_TURN) * TT_ENERGY_TURN;
                break;
            case INP_KEY_YES:
            case INP_KEY_THROW: {
                if (ma_do_throw(plr->player, &e_pos, item) == true) {
                    mapwin_display_map(map, p_pos);
                    return true;
                }
                else Your(plr->player, "unable to throw that.");
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

        path_len = sgt_los_path(gbl_game->sight, gbl_game->current_map, p_pos, &e_pos, &path, false);
        for (int i = 1; i < path_len; i++) {
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
        }
        if (path_len > 0) free(path);

        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
        wrefresh(map_win->win);

        targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, item);
    }
    while((ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE && fire_mode);

    return false;
}

int log_channel_to_colour(enum lg_channel ch) {
    switch (ch) {
        case LG_CHANNEL_WARNING: return get_colour(TERM_COLOUR_RED);
        case LG_CHANNEL_GM: return get_colour(TERM_COLOUR_L_YELLOW);
        case LG_CHANNEL_SAY: return get_colour(TERM_COLOUR_WHITE);
        case LG_CHANNEL_NUMBER: return get_colour(TERM_COLOUR_PURPLE);
        case LG_CHANNEL_SYSTEM: return get_colour(TERM_COLOUR_L_PURPLE);
        default:
        case LG_CHANNEL_DEBUG:
        case LG_CHANNEL_MAX:
        case LG_CHANNEL_PLAIN: return get_colour(TERM_COLOUR_SLATE);
    }
}

void msgwin_log_refresh(struct logging *lg, struct log_entry *new_entry) {
    struct queue *q = lg_queue(lg);
    int log_sz = queue_size(q);
    int win_sz = msg_win->lines -1;
    struct log_entry *tmp_entry = NULL;
    struct log_entry *tmpgame_entry = NULL;

    /* TODO: make msg_win into a pad */

    if (msg_win == NULL) return;
    if (msg_win->type != HRL_WINDOW_TYPE_MESSAGE) return;
    if ( (new_entry != NULL) && (new_entry->level > LG_DEBUG_LEVEL_GAME) ) return;

    int max = MIN(win_sz, log_sz);
    int log_start = 0;

    int game_lvl_sz = 0;
    for (int i = log_sz; i > 0; i--) {
        tmp_entry = (struct log_entry *) queue_peek_nr(q, i-1);
        if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
            game_lvl_sz++;
            log_start = i -1;
            if (game_lvl_sz == max) i = 0;
            if (tmpgame_entry == NULL) tmpgame_entry = tmp_entry;
        }
    }

    if (game_lvl_sz > 0) {
        int y = 0;
        int x = 1;

        wclear(msg_win->win);
        for (int i = log_start; i < log_sz; i++) {
            tmp_entry = (struct log_entry *) queue_peek_nr(q, i);
            if ( (tmp_entry != NULL) && (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) ) {
                for (int l = 0; l < tmp_entry->atom_lst_sz; l++) {
                    struct log_atom *a = &tmp_entry->atom_lst[l];
                    if (a != NULL) {
                        if (x + strlen(a->string) >= msg_win->cols) { y++; x = 1; }
                        int colour = log_channel_to_colour(a->channel);

                        wattron(msg_win->win, colour);
                        mvwprintw(msg_win->win, y,x, a->string);
                        wattroff(msg_win->win, colour);
                        x += strlen(a->string);
                    }
                }
                if (tmp_entry->repeat > 1) {
                    mvwprintw(msg_win->win, y,x, " (x%d)", tmp_entry->repeat);
                }
                y++; 
                x = 1;
            }
        }

        if (options.refresh) wrefresh(msg_win->win);
    }
}

void msgwin_log_callback(struct logging *lg, struct log_entry *entry, void *priv) {
    FIX_UNUSED(entry);
    FIX_UNUSED(priv);

    msgwin_log_refresh(lg, entry);
}

void charwin_refresh() {
    if (char_win == NULL) return;
    struct pl_player *plr = &gbl_game->player_data;
    if (plr == NULL) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;
    int starty = 1;

    werase(char_win->win);

    if (options.play_recording) {
        int attr_mod = get_colour(TERM_COLOUR_RED);
        if (has_colors() == TRUE) wattron(char_win->win, attr_mod);
        mvwprintw(char_win->win, 1,1, "playback x%d", options.play_delay);
        if (has_colors() == TRUE) wattroff(char_win->win, attr_mod);
        starty += 2;
    }

    textwin_init(char_win,1,starty,0,0);

    struct msr_monster *player = plr->player;

    textwin_add_text(char_win, "Name      %s\n", player->unique_name);
    textwin_add_text(char_win, "Career    %s\n", "Thug");
    textwin_add_text(char_win, "Turn      %d.%d\n", gbl_game->turn / TT_ENERGY_TURN, gbl_game->turn % TT_ENERGY_TURN);
    textwin_add_text(char_win, "\n");

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
    textwin_add_text(char_win, "WS   %d   BS   %d\n", ws,bs);
    textwin_add_text(char_win, "Str  %d   Tgh  %d\n", str, tgh);
    textwin_add_text(char_win, "Agi  %d   Int  %d\n", agi, intel);
    textwin_add_text(char_win, "Per  %d   Wil  %d\n", per, wil);
    //textwin_add_text(char_win, "Fellowship   [%d]%d", chr/10, chr%10);
    textwin_add_text(char_win, "\n");

    textwin_add_text(char_win, "Wounds    [%2d/%2d]\n", player->cur_wounds, player->max_wounds);
    textwin_add_text(char_win, "Armour [%d][%d][%d][%d][%d][%d]\n", 
                                            msr_calculate_armour(player, MSR_HITLOC_HEAD),
                                            msr_calculate_armour(player, MSR_HITLOC_CHEST),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_LEG),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_LEG) );
    textwin_add_text(char_win, "\n");

    struct itm_item *item;
    for (int i = 0; i<2; i++) {
        bitfield_t loc = INV_LOC_MAINHAND_WIELD;
        if (i == 1) loc = INV_LOC_OFFHAND_WIELD;
        if ( (item == inv_get_item_from_location(player->inventory, loc) ) ) continue; /*ignore off=hand when wielding 2h weapon.*/

        if ( (item = inv_get_item_from_location(player->inventory, loc) ) != NULL) {
            if (item->item_type == ITEM_TYPE_WEAPON) {
                struct item_weapon_specific *wpn = &item->specific.weapon;
                textwin_add_text(char_win, "%s Wpn: %s\n", (i==0) ? "Main" : "Sec.", item->sd_name);
                textwin_add_text(char_win, " Dmg %dD10+%d,%d", wpn->nr_dmg_die, wpn->dmg_addition, wpn->penetration);
                if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                    textwin_add_text(char_win, "  Ammo %d/%d\n", wpn->magazine_left, wpn->magazine_sz);

                    int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                    int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                    int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                    const char *set = (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) ? "single" : 
                                (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                    char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                    char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                    textwin_add_text(char_win, " Setting: %s (%s/%s/%s)\n", set, 
                            (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                }
                else textwin_add_text(char_win, "\n");
            }
        }
        textwin_add_text(char_win, "\n");
    }

    if ( ( (item = inv_get_item_from_location(player->inventory, INV_LOC_MAINHAND_WIELD) ) != NULL) ||
         ( (item = inv_get_item_from_location(player->inventory, INV_LOC_OFFHAND_WIELD) ) != NULL) ) {
        switch (player->wpn_sel) {
            case MSR_WEAPON_SELECT_OFF_HAND:
                textwin_add_text(char_win, "Using off-hand.\n");
                break;
            case MSR_WEAPON_SELECT_MAIN_HAND:
                textwin_add_text(char_win, "Using main-hand.\n");
                break;
            case MSR_WEAPON_SELECT_BOTH_HAND:
            case MSR_WEAPON_SELECT_DUAL_HAND:
                textwin_add_text(char_win, "Using both hands.\n");
                break;
            case MSR_WEAPON_SELECT_CREATURE1:
                textwin_add_text(char_win, "Unarmed.\n");
            default: break;
        }
    }
    textwin_add_text(char_win, "\n");

    textwin_display_text(char_win);
}

/* Beware of dragons here..... */

struct inv_show_item {
    const char *location;
    struct itm_item *item;
};

static int invwin_printlist(struct hrl_window *window, struct inv_show_item list[], int list_sz, int start, int end) {
    int max = MIN(list_sz, end);

    if (list_sz == 0) {
        mvwprintw(window->win, 1, 1, "Your inventory is empty");
        return 0;
    }

    werase(window->win);
    textwin_init(window,1,0,0,0);

    max = MIN(max, INP_KEY_MAX_IDX);
    if (start >= max) return -1;

    for (int i = 0; i < max; i++) {
        struct itm_item *item = list[i+start].item;
        textwin_add_text(window, "%c)  %c%s", inp_key_translate_idx(i), list[i+start].location[0], item->sd_name);
        if (item->quality != ITEM_QUALITY_AVERAGE) textwin_add_text(window, ", %s quality", itm_quality_string(item) );
        if (item->stacked_quantity > 1) textwin_add_text(window, " x%d", item->stacked_quantity);
        textwin_add_text(window, "\n");
    }

    textwin_display_text(window);
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

void invwin_examine(struct hrl_window *window, struct itm_item *item) {
    if (window == NULL) return;
    if (itm_verify_item(item) == false) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;

    werase(window->win);
    textwin_init(window,1,0,0,0);

    if (strlen(item->description) > 0) {
        textwin_add_text(window, "%s.\n", item->description);
    }
    else textwin_add_text(window, "No description available.\n");

    textwin_add_text(window, "\n");

    switch (item->item_type) {
        case ITEM_TYPE_WEAPON: break;
        case ITEM_TYPE_WEARABLE: break;
        case ITEM_TYPE_TOOL: break;
        case ITEM_TYPE_AMMO: break;
        case ITEM_TYPE_FOOD: break;
        default: break;
    }

    textwin_display_text(window);
}

bool invwin_inventory(struct dm_map *map, struct pl_player *plr) {
    if (map_win == NULL) return false;
    if (plr == NULL) return false;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return false;
    int invstart = 0;
    struct itm_item *item = NULL;
    bool inventory = true;

    int winsz = map_win->lines -4;
    int dislen = winsz;

    int ch = INP_KEY_NONE;
    do {
        int invsz = inv_inventory_size(plr->player->inventory);
        struct inv_show_item *invlist = calloc(invsz, sizeof(struct inv_show_item) );
        inv_create_list(plr->player->inventory, invlist, invsz);

        mapwin_display_map_noref(map, &plr->player->pos);
        touchwin(map_win->win);
        if ( (dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz) ) == -1) {
            invstart = 0;
            dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz);
        }
        mvwprintw(map_win->win, winsz +1, 1, "[q] exit, [space] next page.");
        mvwprintw(map_win->win, winsz +2, 1, "[d] drop, [x] examine.");
        mvwprintw(map_win->win, winsz +3, 1, "[U] use,  [w] wield/wear.");
        wrefresh(map_win->win);
        bool examine = false;

        /* TODO clean this shit up */
        switch (ch) {
            case INP_KEY_YES: invstart += dislen; break;
            case INP_KEY_INVENTORY: inventory = false; break;
            case INP_KEY_UP_RIGHT: 
            case INP_KEY_USE: {
                mvwprintw(map_win->win, winsz, 1, "Use which item?.");
                wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;
                free(invlist);
                mapwin_display_map(map, &plr->player->pos);

                return ma_do_use(plr->player, item);
            } break;
            case INP_KEY_WEAR: {
                mvwprintw(map_win->win, winsz, 1, "Wear which item?.");
                wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;
                free(invlist);
                mapwin_display_map(map, &plr->player->pos);

                if (inv_item_worn(plr->player->inventory, item) == true) {
                    return ma_do_remove(plr->player, item);
                }
                else {
                    return ma_do_wear(plr->player, item);
                }
            } break;
            case INP_KEY_EXAMINE: {
                mvwprintw(map_win->win, winsz, 1, "Examine which item?.");
                wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;
                free(invlist);

                invwin_examine(char_win, invlist[item_idx +invstart].item);
                examine = true;
            } break;
            case INP_KEY_DROP: {
                mvwprintw(map_win->win, winsz, 1, "Drop which item?.");
                wrefresh(map_win->win);

                invsz = inv_inventory_size(plr->player->inventory);
                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;
                free(invlist);
                struct itm_item *items[1] = {item};
                return ma_do_drop(plr->player, items, 1);
            } break;
            default: break;
        }

        wmove(map_win->win, winsz, 0);
        wclrtoeol(map_win->win);
        wrefresh(map_win->win);

        if (examine == false) charwin_refresh();

    } while((inventory != false) && (ch = inp_get_input(gbl_game->input) ) != INP_KEY_ESCAPE);

    mapwin_display_map(map, &plr->player->pos);
    charwin_refresh();

    return false;
}

void character_window(void) {
    int y = 0;
    int y_sub = 0;
    struct pl_player *plr = &gbl_game->player_data;
    struct msr_monster *mon = plr->player;

    struct hrl_window pad;
    memmove(&pad, map_win, sizeof(struct hrl_window) );
    pad.win = newpad(map_win->lines * 10, map_win->cols);
    assert(pad.win != NULL);

    touchwin(pad.win);
    werase(pad.win);

/*
Name: 
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

    /* General Stats */

    textwin_init(&pad,1,0,0,0);
    textwin_add_text(&pad, "Name:       %s\n", mon->unique_name);
    textwin_add_text(&pad, "Gender      %s\n", msr_gender_string(mon) );
    textwin_add_text(&pad, "Career:     %s\n", "tester");
    textwin_add_text(&pad, "Rank:       %s\n", "beginner");
    textwin_add_text(&pad, "Origin:     %s\n", "computer");
    textwin_add_text(&pad, "Divination: %s\n", "die");
    y += textwin_display_text(&pad) +1;

    textwin_init(&pad,1,y,20,10);
    textwin_add_text(&pad, "Wounds:   %d/%d\n", mon->cur_wounds, mon->max_wounds);
    textwin_add_text(&pad, "Insanity:  %d\n", mon->insanity_points);
    textwin_add_text(&pad, "XP:        %d\n", plr->xp_current / TT_ENERGY_TURN);
    y_sub = textwin_display_text(&pad);

            textwin_init(&pad,22,y,20,y_sub);
            textwin_add_text(&pad, "Fate:       %d/%d\n", mon->fate_points,0);
            textwin_add_text(&pad, "Corruption:    %d\n", mon->corruption_points);
            textwin_add_text(&pad, "Spend:         %d\n", plr->xp_spend);
            textwin_display_text(&pad);
    y += y_sub;

    /* Characteristics */
    const char *char_names[] = {"Ws", "Bs", "Str", "Tgh", "Agi", "Int", "Per", "Wil", "Per"};

    y += 1;
    for (int i = 0; i < MSR_CHAR_MAX -1; i++) {
        textwin_init(&pad,(i * 6),y,6,1);
        textwin_add_text(&pad, "[%3s] ", char_names[i]);
        textwin_display_text(&pad);
    }

    y += 1;
    for (int i = 0; i < MSR_CHAR_MAX -1; i++) {
        textwin_init(&pad,(i * 6),y,6,1);
        textwin_add_text(&pad, "[%3d] ", msr_calculate_characteristic(mon, i) );
        textwin_display_text(&pad);
    }

    y += 2;

    /* Skills */
    textwin_init(&pad,1,y,0,0);
    textwin_add_text(&pad, "Skills\n");
    textwin_add_text(&pad, "------\n");
    const char *skill_names[] = {"Awareness",       "Barter", 
                                 "Chem Use",        "Common Lore", 
                                "Concealment",      "Demolition", 
                                "Disguise",         "Dodge", 
                                "Evaluate",         "Forbidden Lore", 
                                "Invocation",       "Logic", 
                                "Medicae",          "Psyscience", 
                                "Scholastic Lore",  "Search", 
                                "Security",         "Silent Move", 
                                "Survival",         "Tech Use", 
                                "Tracking"};

    unsigned int names_len = 0;
    for (unsigned int i = 0; i < ARRAY_SZ(skill_names); i++) {
        if (names_len < strlen(skill_names[i]) ) {
            names_len = strlen(skill_names[i]);
        }
        textwin_add_text(&pad, "%s\n", skill_names[i]);
    }
    y_sub = textwin_display_text(&pad);

            textwin_init(&pad,names_len + 3,y,0,0);
            textwin_add_text(&pad, "Proficiency\n");
            textwin_add_text(&pad, "-----------\n");
            const char *skill_rate_names[] = { "untrained", "basic", "advanced", "expert" };
            for (unsigned int i = 0; i < ARRAY_SZ(skill_names); i++) {
                enum skill_rate skillrate = msr_has_skill(mon,  (1<<i));
                lg_debug("skill rate: %d", skillrate);
                textwin_add_text(&pad, "%s\n", skill_rate_names[skillrate]);
            }
            y_sub = textwin_display_text(&pad);
    y += y_sub +1;


    /* Armour  */
    textwin_init(&pad,1,y,0,0);
    textwin_add_text(&pad, "Armour\n");
    textwin_add_text(&pad, "------\n");
    y_sub = textwin_display_text(&pad);

    names_len = 0;
    /* Armour */
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
        if ( (inv_item_worn(mon->inventory, item) == true) && 
             (inv_item_wielded(mon->inventory, item) == false) ) {

            textwin_init(&pad,1,y+y_sub,0,0);
            textwin_add_text(&pad, "%s", item->ld_name);
            textwin_display_text(&pad);

            if (names_len < strlen(item->ld_name) ) {
                names_len = strlen(item->ld_name);
            }
        }
    }

            textwin_init(&pad,names_len +3,y,0,0);
            textwin_add_text(&pad, "AP\n");
            textwin_add_text(&pad, "--\n");
            y_sub = textwin_display_text(&pad);
            /* Armour */
            item = NULL;
            while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
                if ( (inv_item_worn(mon->inventory, item) == true) && 
                     (inv_item_wielded(mon->inventory, item) == false) ) {
                    int armour = 0;

                    if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
                        armour = item->specific.wearable.damage_reduction;
                    }

                    textwin_init(&pad,names_len +3,y+y_sub,0,0);
                    textwin_add_text(&pad, "%3d", armour);
                    textwin_display_text(&pad);
                }
            }

            textwin_init(&pad,names_len +7 ,y,0,0);
            textwin_add_text(&pad, "Location\n");
            textwin_add_text(&pad, "--------\n");
            textwin_display_text(&pad);

            y += y_sub;

            y_sub = 0;
            item = NULL;
            while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
                if ( (inv_item_worn(mon->inventory, item) == true) && 
                     (inv_item_wielded(mon->inventory, item) == false) ) {
                    bitfield_t locs = inv_get_item_locations(mon->inventory, item);

                    textwin_init(&pad,names_len +7,y,0,0);
                    bool first = true;
                    for (enum inv_locations i = 1; i < INV_LOC_MAX; i <<= 1) {
                        if ( (locs & i) > 0) {
                            if (first == false) textwin_add_text(&pad, "/");
                            textwin_add_text(&pad, "%s", inv_location_name(locs & i) );
                            first = false;
                        }
                    }
                    y_sub += textwin_display_text(&pad);
                }
            }
    y += y_sub;




    /* Controls */

    int line = 0;
    bool watch = true;
    while(watch == true) {
        prefresh(pad.win, line,0,1,1,pad.lines,pad.cols);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_UP_RIGHT:   line += 20; break;
            case INP_KEY_DOWN_RIGHT: line -= 20; break;
            case INP_KEY_UP:         line--; break;
            case INP_KEY_DOWN:       line++; break;
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: watch = false; break;
            default: break;
        }

        if (line < 0) line = 0;
        if (line > (y - pad.lines) ) line = y - pad.lines;
    }

    delwin(pad.win);
}


void log_window(void) {
    struct queue *q = lg_queue(gbl_log);
    int y = 0;
    int log_sz = queue_size(q);
    struct log_entry *tmp_entry = NULL;

    struct hrl_window pad;
    memmove(&pad, map_win, sizeof(struct hrl_window) );
    pad.win = newpad(MAX(log_sz * 2, map_win->lines) , map_win->cols);
    assert(pad.win != NULL);

    touchwin(pad.win);
    werase(pad.win);

    textwin_init(&pad,1,0,0,log_sz);
    if (log_sz > 0) {
        for (int i = log_sz; i > 0; i--) {
            tmp_entry = (struct log_entry *) queue_peek_nr(q, i-1);
            if (tmp_entry != NULL) {
                bool print = false;

                if (options.debug) {
                    const char *pre_format;
                    print = true;

                    switch (tmp_entry->level) {
                        case LG_DEBUG_LEVEL_GAME:
                            pre_format = "[%s" ":Game][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_DEBUG:
                            pre_format = "[%s" ":Debug][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_INFORMATIONAL:
                            pre_format = "[%s" ":Info][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_WARNING:
                            pre_format = "[%s" ":Warning][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_ERROR:
                            pre_format = "[%s" ":Error][%d] ";
                            break;
                        default:
                            pre_format ="[%s" ":Unknown][%d] ";
                            break;
                    }

                    textwin_add_text(&pad, pre_format, tmp_entry->module, tmp_entry->turn);
                }
                else if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) print = true;

                if (print) {
                    for (int l = 0; l < tmp_entry->atom_lst_sz; l++) {
                        struct log_atom *a = &tmp_entry->atom_lst[l];
                        if (a != NULL) {
                            textwin_add_text(&pad, "%s", a->string);
                        }
                    }
                    if (tmp_entry->repeat > 1) {
                        textwin_add_text(&pad, " (x%d)", tmp_entry->repeat);
                    }
                    textwin_add_text(&pad, "\n");
                }
            }
        }
    }
    else textwin_add_text(&pad, "Empty\n");
    y += textwin_display_text(&pad) +1;

    int line = 0;
    bool watch = true;
    while(watch == true) {
        prefresh(pad.win, line,0,1,1,pad.lines,pad.cols);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_UP_RIGHT:   line += 20; break;
            case INP_KEY_DOWN_RIGHT: line -= 20; break;
            case INP_KEY_UP:         line--; break;
            case INP_KEY_DOWN:       line++; break;
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: watch = false; break;
            default: break;
        }

        if (line < 0) line = 0;
        if (line > (y - pad.lines) ) line = y - pad.lines;
    }

    delwin(pad.win);
}

void levelup_selection_window(void) {
/*

*/
}

