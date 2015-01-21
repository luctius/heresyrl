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

void show_msg(struct hrl_window *window);

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
        int char_lines = map_lines -1;
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
            show_msg(msg_win);
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

    lg_debug("update mapwin");
    mapwin_display_map_noref(map, player);
    if (options.refresh) wrefresh(map_win->win);

    show_msg(msg_win);
}

static void mapwin_examine(struct dm_map_entity *me) {
    if (char_win == NULL) return;
    if (me == NULL) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;

    werase(char_win->win);
    ui_print_reset(char_win);

    if (me->visible || me->in_sight) {
        ui_printf(char_win, "%s.\n", me->tile->ld_name);
        ui_printf(char_win, "\n");

        if (me->visible) {
            if (me->monster != NULL) {

                if (me->monster->is_player == true) {
                    ui_printf(char_win, "You see yourself.\n");

                    if (me->monster->cur_wounds < 0) ui_printf(char_win, "You are criticly wounded.\n");
                    else if (me->monster->cur_wounds != me->monster->max_wounds) ui_printf(char_win, "You are wounded.\n");
                } else {
                    ui_printf(char_win, "You see %s.\n", me->monster->ld_name);

                    if (me->monster->cur_wounds < 0) ui_printf(char_win, "%s is criticly wounded.\n", msr_gender_name(me->monster, false) );
                    else if (me->monster->cur_wounds != me->monster->max_wounds) ui_printf(char_win, "%s is wounded.\n", msr_gender_name(me->monster, false) );
                }

                if (se_list_size(me->monster->status_effects) > 0) {
                    ui_printf(char_win, "\nThis one is affected by:\n");

                    struct status_effect *c = NULL;
                    while ( (c = se_list_get_next_status_effect(me->monster->status_effects, c) ) != NULL) {
                        ui_printf(char_win, "- %s\n", c->name);
                    }
                }
            }

            ui_printf(char_win, "\n");

            if ( (inv_inventory_size(me->inventory) > 0) && (TILE_HAS_ATTRIBUTE(me->tile, TILE_ATTR_TRAVERSABLE) ) ) {
                ui_printf(char_win, "The %s contains:\n", me->tile->sd_name);
                struct itm_item *i = NULL;
                while ( (i = inv_get_next_item(me->inventory, i) ) != NULL) {
                    ui_printf(char_win, " - %s\n", i->ld_name);
                }
            }
        }
    }
    else ui_printf(char_win, "You can not see this place.\n");

    wrefresh(char_win->win);
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

        lg_debug("examining pos: (%d,%d), plr (%d,%d)", e_pos.x, e_pos.y, p_pos->x, p_pos->y);
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
    ui_print_reset(window);

    if (me->monster != NULL) {
        ui_printf(window,"Target: %s.\n", msr_ldname(me->monster) );

        int tohit = fght_ranged_calc_tohit(player, pos, FGHT_MAIN_HAND);
        ui_printf(window,"Total change of hitting: %d.\n", tohit);
    }
    else ui_printf(window,"No Target.\n");

    ui_printf(window,"Ballistic Skill: %d\n\n", msr_calculate_characteristic(player, MSR_CHAR_BALISTIC_SKILL) );

    int idx = 0;
    struct tohit_desc *thd = NULL;
    while ( (thd = fght_get_tohit_mod_description(idx++) ) != NULL) {
        ui_printf(window,"%c %s (%d).\n", (thd->modifier > 0) ? '+' : '-', thd->description, thd->modifier);
    }

    ui_printf(window,"\n");

    ui_print_reset(window);
    ui_printf(window, "Calculated: %s.\n", witem->sd_name);
    if (wpn_is_catergory(witem, WEAPON_CATEGORY_THROWN_GRENADE) ) {
        ui_printf(window,"Timer: %d.%d.\n", witem->energy / TT_ENERGY_TURN, witem->energy % TT_ENERGY_TURN);
    }

    wrefresh(window->win);
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

void show_log(struct hrl_window *window, bool input);

static bool active = false;
void msgwin_log_callback(struct logging *lg, struct log_entry *entry, void *priv) {
    FIX_UNUSED(entry);
    FIX_UNUSED(priv);
    FIX_UNUSED(lg);

    if (active) return;
    active = true;

    show_msg(msg_win);
    active = false;
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

    ui_print_reset(char_win);

    struct msr_monster *player = plr->player;

    ui_printf(char_win, "Name      %s\n", player->unique_name);
    ui_printf(char_win, "Career    %s\n", "Thug");
    ui_printf(char_win, "Turn      %d.%d\n", gbl_game->turn / TT_ENERGY_TURN, gbl_game->turn % TT_ENERGY_TURN);
    ui_printf(char_win, "\n");

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
    ui_printf(char_win, "WS   %d   BS   %d\n", ws,bs);
    ui_printf(char_win, "Str  %d   Tgh  %d\n", str, tgh);
    ui_printf(char_win, "Agi  %d   Int  %d\n", agi, intel);
    ui_printf(char_win, "Per  %d   Wil  %d\n", per, wil);
    //ui_printf(char_win, "Fellowship   [%d]%d", chr/10, chr%10);
    ui_printf(char_win, "\n");

    ui_printf(char_win, "Wounds    [%2d/%2d]\n", player->cur_wounds, player->max_wounds);
    ui_printf(char_win, "Armour [%d][%d][%d][%d][%d][%d]\n", 
                                            msr_calculate_armour(player, MSR_HITLOC_HEAD),
                                            msr_calculate_armour(player, MSR_HITLOC_BODY),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_ARM),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_LEG),
                                            msr_calculate_armour(player, MSR_HITLOC_RIGHT_LEG) );
    ui_printf(char_win, "\n");

    struct itm_item *item;
    for (int i = 0; i<2; i++) {
        bitfield_t loc = INV_LOC_MAINHAND_WIELD;
        if (i == 1) loc = INV_LOC_OFFHAND_WIELD;
        if ( (item == inv_get_item_from_location(player->inventory, loc) ) ) continue; /*ignore off=hand when wielding 2h weapon.*/

        if ( (item = inv_get_item_from_location(player->inventory, loc) ) != NULL) {
            if (item->item_type == ITEM_TYPE_WEAPON) {
                struct item_weapon_specific *wpn = &item->specific.weapon;
                ui_printf(char_win, "%s Wpn: %s\n", (i==0) ? "Main" : "Sec.", item->sd_name);
                if (wpn->nr_dmg_die == 0) ui_printf(char_win, " Dmg 1D5");
                else ui_printf(char_win, " Dmg %dD10", wpn->nr_dmg_die);
                ui_printf(char_win, "+%d,%d", wpn->dmg_addition, wpn->penetration);

                if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                    if (wpn->jammed == false) {
                        ui_printf(char_win, "  Ammo %d/%d\n", wpn->magazine_left, wpn->magazine_sz);
                    } 
                    else ui_printf(char_win, "  jammed\n");

                    int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                    int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                    int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                    const char *set = (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) ? "single" : 
                                (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                    char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                    char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                    ui_printf(char_win, " Setting: %s (%s/%s/%s)\n", set, 
                            (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                }
                else ui_printf(char_win, "\n");
            }
        }
        ui_printf(char_win, "\n");
    }

    if ( ( (item = inv_get_item_from_location(player->inventory, INV_LOC_MAINHAND_WIELD) ) != NULL) ||
         ( (item = inv_get_item_from_location(player->inventory, INV_LOC_OFFHAND_WIELD) ) != NULL) ) {
        switch (player->wpn_sel) {
            case MSR_WEAPON_SELECT_OFF_HAND:
                ui_printf(char_win, "Using off-hand.\n");
                break;
            case MSR_WEAPON_SELECT_MAIN_HAND:
                ui_printf(char_win, "Using main-hand.\n");
                break;
            case MSR_WEAPON_SELECT_BOTH_HAND:
            case MSR_WEAPON_SELECT_DUAL_HAND:
                ui_printf(char_win, "Using both hands.\n");
                break;
            case MSR_WEAPON_SELECT_CREATURE1:
                ui_printf(char_win, "Unarmed.\n");
            default: break;
        }
    }
    ui_printf(char_win, "\n");

    wrefresh(char_win->win);
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
    ui_print_reset(window);

    max = MIN(max, INP_KEY_MAX_IDX);
    if (start >= max) return -1;

    for (int i = 0; i < max; i++) {
        struct itm_item *item = list[i+start].item;
        ui_printf(window, "%c)  %c%s", inp_key_translate_idx(i), list[i+start].location[0], item->sd_name);
        if (item->quality != ITEM_QLTY_AVERAGE) ui_printf(window, ", %s quality", itm_quality_string(item) );
        if (item->stacked_quantity > 1) ui_printf(window, " x%d", item->stacked_quantity);
        ui_printf(window, "\n");
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

void invwin_examine(struct hrl_window *window, struct itm_item *item) {
    if (window == NULL) return;
    if (itm_verify_item(item) == false) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;

    werase(window->win);
    ui_print_reset(window);

    if (strlen(item->description) > 0) {
        ui_printf(window, "%s.\n", item->description);
    }
    else ui_printf(window, "No description available.\n");

    ui_printf(window, "\n");

    switch (item->item_type) {
        case ITEM_TYPE_WEAPON: {
            struct item_weapon_specific *wpn = &item->specific.weapon;
            ui_printf(char_win, "Weapon statistics\n");
            ui_printf(char_win, "- Dmg ");
            if (wpn->nr_dmg_die == 0) ui_printf(char_win, "1D5");
            else ui_printf(char_win, "%dD10", wpn->nr_dmg_die);
                ui_printf(char_win, "+%d, pen %d\n", wpn->dmg_addition, wpn->penetration);

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) || wpn_is_type(item, WEAPON_TYPE_THROWN) ) {
                ui_printf(char_win, "- Range %d\n", wpn->range);
            }

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) {
                ui_printf(char_win, "- Magazine size %d\n", wpn->magazine_sz);
                ui_printf(char_win, "- Uses %s\n", wpn_ammo_string(wpn->ammo_type) );

                int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];
                char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                ui_printf(char_win, "- Rate of Fire (%s/%s/%s)\n", 
                        (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");

                ui_printf(char_win, "\n");
                if (wpn->magazine_left == 0) {
                    ui_printf(char_win, "The weapon is empty.\n");
                } 
                else {
                    struct itm_item *ammo = itm_create(wpn->ammo_used_template_id);
                    ui_printf(char_win, "It is currently loaded with %s.\n\n", ammo->ld_name);
                    itm_destroy(ammo);
                }

                if (wpn->jammed == true) {
                    ui_printf(char_win, "It is jammed.\n");
                }
            }

            if (wpn->wpn_talent != TLT_NONE) {
                ui_printf(char_win, "This weapon requires the %s talent.\n", msr_talent_names(wpn->wpn_talent) );
            }

            if (wpn->special_quality != 0) {
                ui_printf(char_win, "\n");
                ui_printf(char_win, "Weapon qualities:\n");

                for (int i = 0; i < WPN_SPCQLTY_MAX; i++) {
                    if (wpn_has_spc_quality(item, i) )  {
                        ui_printf(char_win, "- %s.\n", wpn_spec_quality_name(i) );
                    }
                }
            }
        } break;
        case ITEM_TYPE_WEARABLE: {
            struct item_wearable_specific *wrbl = &item->specific.wearable;
            ui_printf(char_win, "Wearable statistics\n");

            ui_printf(char_win, "- Armour: %d\n", wrbl->damage_reduction);

            ui_printf(char_win, "- Locations: ");
            bool first = true;
            for (enum inv_locations i = 1; i < INV_LOC_MAX; i <<= 1) {
                if ( (wrbl->locations & i) > 0) {
                    if (first == false) ui_printf(char_win, "/");
                    ui_printf(char_win, "%s", inv_location_name(wrbl->locations & i) );
                    first = false;
                }
            }

            if (wrbl->special_quality != 0) {
                ui_printf(char_win, "\n");
                ui_printf(char_win, "Wearable qualities:\n");

                for (int i = 0; i < WBL_SPCQLTY_MAX; i++) {
                    if (wbl_has_spc_quality(item, i) )  {
                        ui_printf(char_win, "- %s.\n", wbl_spec_quality_name(i) );
                    }
                }
            }

            ui_printf(char_win, "\n");
        } break;
        case ITEM_TYPE_TOOL: {
            struct item_tool_specific *tool = &item->specific.tool;
            if (tool->energy > 0) {
                int energy_pc = (tool->energy_left * 100) / tool->energy;
                ui_printf(char_win, "Energy left %d\%\n", energy_pc);
            }

        }break;
        case ITEM_TYPE_AMMO: {
            struct item_ammo_specific *ammo = &item->specific.ammo;
            ui_printf(char_win, "Provides %s\n", wpn_ammo_string(ammo->ammo_type) );

            if (ammo->energy > 0) {
                int energy_pc = (ammo->energy_left * 100) / ammo->energy;
                ui_printf(char_win, "Energy left %d\%\n", energy_pc);
            }
        } break;
        case ITEM_TYPE_FOOD: break;
        default: break;
    }
    wrefresh(char_win->win);
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
    unsigned int names_len = 0;

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

    ui_print_reset(&pad);
    ui_printf(&pad, "Name:       %s\n", mon->unique_name);
    ui_printf(&pad, "Gender      %s\n", msr_gender_string(mon) );
    ui_printf(&pad, "Career:     %s\n", "tester");
    ui_printf(&pad, "Rank:       %s\n", "beginner");
    ui_printf(&pad, "Origin:     %s\n", "computer");
    ui_printf(&pad, "Divination: %s\n", "You will die...");

    ui_printf(&pad, "Wounds:   %d/%d\n", mon->cur_wounds, mon->max_wounds);
    ui_printf(&pad, "Fatique:  %d\n", mon->fatique);
    ui_printf(&pad, "XP:        %d\n", plr->xp_current / TT_ENERGY_TURN);

    ui_printf(&pad, "Fate:       %d/%d\n", mon->fate_points,0);
    ui_printf(&pad, "Corruption:    %d\n", mon->corruption_points);
    ui_printf(&pad, "Spend:         %d\n", plr->xp_spend);

    /* Characteristics */
    const char *char_names[] = {"Ws", "Bs", "Str", "Tgh", "Agi", "Int", "Per", "Wil", "Per"};

    y += 1;
    for (int i = 0; i < MSR_CHAR_MAX -1; i++) {
        ui_printf(&pad, "[%3s] ", char_names[i]);
    }

    y += 1;
    for (int i = 0; i < MSR_CHAR_MAX -1; i++) {
        ui_printf(&pad, "[%3d] ", msr_calculate_characteristic(mon, i) );
    }

    y += 2;

    /* Armour  */
    ui_printf(&pad, "Armour\n");
    ui_printf(&pad, "------\n");

    /* Armour */
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
        if ( (inv_item_worn(mon->inventory, item) == true) && 
             (inv_item_wielded(mon->inventory, item) == false) ) {

            ui_printf(&pad, "%s", item->ld_name);

            if (names_len < strlen(item->ld_name) ) {
                names_len = strlen(item->ld_name);
            }
        }
    }

            ui_printf(&pad, "AP\n");
            ui_printf(&pad, "--\n");
            /* Armour */
            item = NULL;
            while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
                if ( (inv_item_worn(mon->inventory, item) == true) && 
                     (inv_item_wielded(mon->inventory, item) == false) ) {
                    int armour = 0;

                    if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
                        armour = item->specific.wearable.damage_reduction;
                    }

                    ui_printf(&pad, "%3d", armour);
                }
            }

            ui_printf(&pad, "Location\n");
            ui_printf(&pad, "--------\n");

            y += y_sub;

            y_sub = 0;
            item = NULL;
            while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
                if ( (inv_item_worn(mon->inventory, item) == true) && 
                     (inv_item_wielded(mon->inventory, item) == false) ) {
                    bitfield_t locs = inv_get_item_locations(mon->inventory, item);

                    bool first = true;
                    for (enum inv_locations i = 1; i < INV_LOC_MAX; i <<= 1) {
                        if ( (locs & i) > 0) {
                            if (first == false) ui_printf(&pad, "/");
                            ui_printf(&pad, "%s", inv_location_name(locs & i) );
                            first = false;
                        }
                    }
                }
            }
    y += y_sub +1;

    /* Skills */
    ui_printf(&pad, "Skills\n");
    ui_printf(&pad, "------\n");

    for (unsigned int i = 0; i < MSR_SKILLS_MAX; i++) {
        if (names_len < strlen(msr_skill_names(i) ) ) {
            names_len = strlen(msr_skill_names(i) );
        }
        ui_printf(&pad, "%s\n", msr_skill_names(i) );
    }

            ui_printf(&pad, "Proficiency\n");
            ui_printf(&pad, "-----------\n");
            for (unsigned int i = 0; i < MSR_SKILLS_MAX; i++) {
                enum msr_skill_rate skillrate = msr_has_skill(mon,  i);
                lg_debug("skill rate: %d", skillrate);
                ui_printf(&pad, "%s\n", msr_skillrate_names(skillrate));
            }
    y += y_sub +1;

    /* Talents */
    ui_printf(&pad, "Talents\n");
    ui_printf(&pad, "-------\n");

    names_len = 0;
    for (unsigned int i = 1; i < MSR_TALENTS_MAX; i++) {
        if (msr_has_talent(mon, i) ) {
            if (names_len < strlen(msr_talent_names(i) ) ) {
                names_len = strlen(msr_talent_names(i) );
            }
            ui_printf(&pad, "%s\n", msr_talent_names(i) );
        }
    }

    y += y_sub +1;


    /* Conditions */
    ui_printf(&pad, "Conditions\n");
    ui_printf(&pad, "----------\n");

    names_len = 0;
    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(mon->status_effects, c) ) != NULL) {
        ui_printf(&pad, "%s\n", c->name);
    }

    y += y_sub +1;



    /* Controls */

    int line = 0;
    bool watch = true;
    while(watch == true) {
        prefresh(pad.win, line,0,1,1,pad.lines -4,pad.cols);

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
        if (line > (y - pad.lines +4) ) line = y - pad.lines +4;
    }

    delwin(pad.win);
}

void show_log(struct hrl_window *window, bool input) {
    int y = 0;
    int log_sz = lg_size(gbl_log);
    struct log_entry *tmp_entry = NULL;

    struct hrl_window pad;
    memmove(&pad, window, sizeof(struct hrl_window) );
    pad.win = newpad(MAX(log_sz, window->lines) , window->cols);
    assert(pad.win != NULL);

    touchwin(pad.win);
    werase(pad.win);


    char pre_format_buf[100];
    ui_print_reset(&pad);
    if (log_sz > 0) {
        for (int i = log_sz; i > 0; i--) {
            tmp_entry = lg_peek(gbl_log, i-1);
            if (tmp_entry != NULL && tmp_entry->string != NULL) {
                bool print = false;
                bool old = false;

                pre_format_buf[0] = '\0';

                if (options.debug) {
                    const char *pre_format;
                    print = true;

                    switch (tmp_entry->level) {
                        case LG_DEBUG_LEVEL_GAME:
                            pre_format = "[%s:%d]" "[Game][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_GAME_INFO:
                            pre_format = "[%s:%d]" "[Game Info][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_DEBUG:
                            pre_format = "[%s:%d]" "[Debug][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_INFORMATIONAL:
                            pre_format = "[%s:%d]" "[Info][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_WARNING:
                            pre_format = "[%s:%d]" "[Warning][%d] ";
                            break;
                        case LG_DEBUG_LEVEL_ERROR:
                            pre_format = "[%s:%d]" "[Error][%d] ";
                            break;
                        default:
                            pre_format ="[%s:%d]" "[Unknown][%d] ";
                            break;
                    }

                    sprintf(pre_format_buf, pre_format, tmp_entry->module, tmp_entry->line, tmp_entry->turn);
                }
                else if (tmp_entry->level == LG_DEBUG_LEVEL_GAME) {
                    print = true;
                }
                else if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME_INFO) {
                    print = true;
                    old = true;
                }

                if ( (tmp_entry->turn +(TT_ENERGY_TURN+TT_ENERGY_TICK) ) < gbl_game->turn) {
                    old = true;
                }

                if (print) {
                    char *old_str = "";
                    if (old) old_str = cs_OLD;

                    if (tmp_entry->repeat > 1) {
                        y = ui_printf(&pad, "%s%s%s (x%d)%s\n", old_str, pre_format_buf, tmp_entry->string, tmp_entry->repeat, old_str);
                    }
                    else y = ui_printf(&pad, "%s%s%s%s\n", old_str, pre_format_buf, tmp_entry->string, old_str);
                }
            }
        }
    }
    else ui_printf(&pad, "Empty\n");

    if (input) {
        int line = 0;
        bool watch = true;
        while(watch == true) {
            prefresh(pad.win, line,0,pad.y,pad.x, pad.y + pad.lines -1, pad.x + pad.cols);

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
    }
    else prefresh(pad.win, 0,0, pad.y, pad.x, pad.y + pad.lines -1, pad.x + pad.cols);

    delwin(pad.win);
}

#define SHOW_MAX_MSGS (20)
void show_msg(struct hrl_window *window) {
    int y = 0;
    int log_sz = lg_size(gbl_log);
    struct log_entry *tmp_entry = NULL;

    struct hrl_window pad;
    memmove(&pad, window, sizeof(struct hrl_window) );
    pad.win = newpad(window->lines, window->cols);
    assert(pad.win != NULL);

    touchwin(pad.win);
    werase(pad.win);

    ui_print_reset(&pad);

    int ctr = 0;
    if (log_sz > 0) {
        int min = 0;
        for (int i = log_sz; i > 0; i--) {
            tmp_entry = lg_peek(gbl_log, i-1);
            if (tmp_entry != NULL) {
                bool print = false;
                if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) print = true;

                if (print) ctr++;
            }
            if (ctr >= window->lines) {
                min = i;
                i = 0;
            }
        }

        int last_turn = -1;
        if (ctr > 0) {
            for (int i = min; i < log_sz; i++) {
                bool old = false;
                bool print = false;

                tmp_entry = lg_peek(gbl_log, i);
                if (tmp_entry != NULL) {
                    if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) {
                        print = true;
                        if ( (tmp_entry->turn +(TT_ENERGY_TURN+TT_ENERGY_TICK) ) < gbl_game->turn) {
                            old = true;
                        }
                    }

                    if (print) {
                        char *old_str = "";
                        if (old) old_str = cs_OLD;

                        if (tmp_entry->repeat > 1) {
                            y = ui_printf(&pad, "%s%s (x%d)%s\n", old_str, tmp_entry->string, tmp_entry->repeat, old_str);
                        }
                        else y = ui_printf(&pad, "%s%s%s\n", old_str, tmp_entry->string, old_str);

                        last_turn = tmp_entry->turn;
                    }
                }
            }
        }
    }

    if (ctr == 0) y = ui_printf(&pad, "Empty");

    prefresh(pad.win, y - pad.lines + 1,0, pad.y, pad.x, pad.y + pad.lines, pad.x + pad.cols);
    delwin(pad.win);
}

void log_window(void) {
    /* clear bottom messages to avoid confusion*/
    wclear(msg_win->win);
    wrefresh(msg_win->win);

    show_log(map_win, true);
    show_msg(msg_win); /* refresh log messages at log exit*/
}

void levelup_selection_window(void) {
/*

*/
}

