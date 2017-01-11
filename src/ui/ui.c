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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <sys/param.h>
#include <string.h>

#include "ui.h"
#include "ui_common.h"

#include "options.h"

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
#include "careers/careers.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "quests/quests.h"
#include "random.h"
#include "wizard/wizard_mode.h"

void show_msg(struct hrl_window *window);

static int hdr_lines = 0;
static int hdr_cols = 0;
bool ui_create(int cols, int lines) {
    win_generate_colours();

    if ( (hdr_lines != lines) || (hdr_cols != cols) ) {
        hdr_lines = lines;
        hdr_cols = cols;

        if ( (lines < LINES_MIN) || (cols < COLS_MIN) ) {
            endwin();           /*  End curses mode       */
            fprintf(stderr, "Terminal is too small, minimum is %dx%d, this terminal is %dx%d.\n", COLS_MIN, LINES_MIN, cols, lines);
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
        int msg_lines = (lines) - map_lines;
        if (options.wz_mode) msg_lines -= 1;
        if (msg_lines < MSG_MIN_LINES) msg_lines = MSG_MIN_LINES;
        if ( (msg_lines > MSG_MAX_LINES) && (MSG_MAX_LINES != 0) ) msg_lines = MSG_MAX_LINES;

        int char_cols = cols - map_cols;
        if (char_cols < CHAR_MIN_COLS) char_cols = CHAR_MIN_COLS;
        if ( (char_cols > CHAR_MAX_COLS) && (CHAR_MAX_COLS != 0) ) char_cols = CHAR_MAX_COLS;
        int char_lines = map_lines -1;
        if ( (char_lines > CHAR_MAX_LINES) && (CHAR_MAX_LINES != 0) ) char_lines = CHAR_MAX_LINES;

        int wz_lines = 0;
        if (options.wz_mode) wz_lines = 1;

        int total_lines = map_lines + msg_lines +wz_lines;
        if (total_lines < char_lines) total_lines = char_lines;
        int total_cols = map_cols + char_cols;
        if (total_cols < msg_cols) total_cols = msg_cols;

        if (total_lines > lines) { fprintf(stderr, "Too many lines used!\n"); exit(1); }
        if (total_cols > cols) { fprintf(stderr, "Too many cols used!\n"); exit(1); }

        int l_margin = lines / LINES_MIN;
        int c_margin = cols / COLS_MIN;
        if (l_margin > 2) l_margin = 2;
        if (c_margin > 2) c_margin = 2;

        if ( (map_win == NULL) || (char_win == NULL) ||(msg_win == NULL)  ) {
            main_win = win_create(lines - l_margin, cols - c_margin, l_margin, c_margin, HRL_WINDOW_TYPE_MAIN);
            map_win = win_create(map_lines - l_margin, map_cols - l_margin, l_margin, c_margin, HRL_WINDOW_TYPE_MAP);
            char_win = win_create(char_lines - l_margin, char_cols - c_margin, l_margin, map_cols+1, HRL_WINDOW_TYPE_CHARACTER);
            msg_win = win_create(msg_lines - l_margin, msg_cols - c_margin, map_lines, c_margin, HRL_WINDOW_TYPE_MESSAGE);
            if (options.wz_mode) wz_win = win_create(wz_lines, msg_cols - c_margin, map_lines + (msg_lines - l_margin), c_margin, HRL_WINDOW_TYPE_WIZARD);
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
    if (main_win != NULL) win_destroy(main_win);
    if (map_win != NULL) win_destroy(map_win);
    if (char_win != NULL) win_destroy(char_win);
    if (msg_win != NULL) win_destroy(msg_win);
}

void update_screen(void) {
    if (gbl_game->running) {
        mapwin_display_map(gbl_game->current_map, &gbl_game->player_data.player->pos);
        charwin_refresh();
    }
}

static void mapwin_display_map_noref(struct dm_map *map, coord_t *player) {
    coord_t scr_c = cd_create(0,0);

    if (map_win == NULL) return;
    if (dm_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;
    if (se_has_effect(gbl_game->player_data.player, EF_DEAD) ) return;
    if (options.refresh == false) return;

    struct msr_monster *plr = dm_get_map_me(player, map)->monster;

    int x_max = (map_win->cols < map->sett.size.x) ? map_win->cols : map->sett.size.x;
    int y_max = (map_win->lines < map->sett.size.y) ? map_win->lines : map->sett.size.y;
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
    scr_c.x = get_viewport(ppos.x, map_win->cols,  map->sett.size.x);
    scr_c.y = get_viewport(ppos.y, map_win->lines, map->sett.size.y);

    bool map_see = options.debug_show_map;
    bool monster_see = options.debug_show_map;

    for (int xi = 0; xi < x_max; xi++) {
        for (int yi = 0; yi < y_max; yi++) {
            coord_t map_c = cd_create(xi+scr_c.x, yi+scr_c.y);
            struct dm_map_entity *me = dm_get_map_me(&map_c, map);
            struct tl_tile *tile = me->tile;

            if ( (me->visible == true) || (me->discovered == true) || (map_see == true) || me->icon_override != -1) {
                int attr_mod = TERM_COLOUR_L_DARK;
                icon_t icon = tile->icon;

                /* Otherwise visible traversable tiles */
                if (map_see || me->visible == true) {
                    attr_mod = tile->icon_attr;
                    if (tile->type == TILE_TYPE_FLOOR) {
                        if (me->light_level >= 5) {
                            attr_mod = TERM_COLOUR_YELLOW;
                        }
                        else if (me->light_level > 0) {
                            attr_mod = TERM_COLOUR_L_YELLOW;
                        }
                    }
                }
                /* Else see items */
                if ( (me->visible == true) || (map_see) ) {
                    if (TILE_HAS_ATTRIBUTE(tile, TILE_ATTR_TRAVERSABLE) == true) {
                        if (inv_inventory_size(me->inventory) > 0) {
                            struct itm_item *i = inv_get_next_item(me->inventory, NULL);
                            icon = i->icon;
                            attr_mod = i->icon_attr;
                        }
                    }
                }
                /* Then see ground effects */
                if ( (me->visible == true) || (map_see) ) {
                    if (me->effect != NULL) {
                        icon = me->effect->icon;
                        attr_mod = me->effect->icon_attr;
                    }
                }
                /* First see monster */
                if ( (me->in_sight == true) || (map_see) ) {
                    if (me->monster != NULL) {
                        if (monster_see || fght_can_see(map, plr, me->monster) ) {
                            icon = me->monster->icon;
                            attr_mod = me->monster->icon_attr;
                        }
                    }
                }

                if (me->icon_override != -1) {
                    icon = me->icon_override;
                    if (me->icon_attr_override != -1) {
                        attr_mod = me->icon_attr_override;
                    }
                }

                /* test colours */
                {
                    if (me->test_var > 0) {
                        attr_mod = me->test_var;
                    }
                }

                if (has_colors() == TRUE) wattron(map_win->win, get_colour(attr_mod) );
                mvwaddch(map_win->win, yi, xi, icon);
                if (has_colors() == TRUE) wattroff(map_win->win, get_colour(attr_mod) );
            }
        }
    }
}

void mapwin_display_map(struct dm_map *map, coord_t *player) {
    if (map_win == NULL) return;
    if (dm_verify_map(map) == false) return;
    if (player == NULL) return;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return;
    if (options.refresh == false) return;

    mapwin_display_map_noref(map, player);
    if (options.refresh) wrefresh(map_win->win);

    //show_msg(msg_win);
}

static void mapwin_examine(struct dm_map_entity *me) {
    if (char_win == NULL) return;
    if (me == NULL) return;
    if (char_win->type != HRL_WINDOW_TYPE_CHARACTER) return;
    if (options.refresh == false) return;

    wclear(char_win->win);
    werase(char_win->win);
    ui_print_reset(char_win);

    if (me->visible || me->in_sight) {
        ui_printf(char_win, "%s.\n", me->tile->ld_name);
        ui_printf(char_win, "\n");

        if (me->visible) {
            if (me->monster != NULL) {
                bool sees = false;

                if (me->monster->is_player == true) {
                    ui_printf(char_win, cs_PLAYER "You" cs_CLOSE " see yourself.\n");

                    if (me->monster->wounds.curr < 0) ui_printf(char_win, cs_PLAYER "You" cs_CLOSE " are criticly wounded.\n");
                    else if (me->monster->wounds.curr != me->monster->wounds.max) ui_printf(char_win, cs_PLAYER "You" cs_CLOSE " are wounded.\n");

                    sees = true;
                } else if (fght_can_see(gbl_game->current_map, gbl_game->player_data.player, me->monster) ) {
                    ui_printf(char_win, cs_PLAYER "You" cs_CLOSE " see %s.\n", me->monster->ld_name);

                    if (me->monster->wounds.curr < 0) ui_printf(char_win, "%s is criticly wounded.\n", msr_gender_name(me->monster, false) );
                    else if (me->monster->wounds.curr != me->monster->wounds.max) ui_printf(char_win, "%s is wounded.\n", msr_gender_name(me->monster, false) );


                    if (inv_loc_empty(me->monster->inventory, INV_LOC_MAINHAND_WIELD) == false) {
                        struct itm_item *witem = inv_get_item_from_location(me->monster->inventory, INV_LOC_MAINHAND_WIELD);
                        if (witem != NULL) ui_printf(char_win, "%s wields %s.\n", msr_gender_name(me->monster, false), witem->ld_name);
                    }
                    if (inv_loc_empty(me->monster->inventory, INV_LOC_OFFHAND_WIELD) == false) {
                        struct itm_item *witem = inv_get_item_from_location(me->monster->inventory, INV_LOC_OFFHAND_WIELD);
                        if (witem != NULL) ui_printf(char_win, "%s wields %s in his off-hand.\n", msr_gender_name(me->monster, false), witem->ld_name);
                    }

                    sees = true;
                }

                if (sees && se_list_size(me->monster->status_effects) > 0) {
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
    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

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

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES:
            case INP_KEY_EXAMINE:    examine_mode = false; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        if (options.refresh) {
            lg_debug("examining pos: (%d,%d), plr (%d,%d)", e_pos.x, e_pos.y, p_pos->x, p_pos->y);
            chtype oldch = mvwinch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x);
            mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, (oldch & 0xFF) | get_colour(TERM_COLOUR_BG_RED) );
            if (options.refresh) wrefresh(map_win->win);
            mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
        }

        mapwin_examine(dm_get_map_me(&e_pos, map) );
    }
    while( ( (ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE) && (examine_mode == true) );

    wrefresh(map_win->win);
}

void targetwin_examine(struct hrl_window *window, struct dm_map *map, struct msr_monster *player, coord_t *pos, struct itm_item *witem, enum item_weapon_type wtype) {
    if (window == NULL) return;
    if (witem == NULL) return;
    if (msr_verify_monster(player) == false) return;
    if (itm_verify_item(witem) == false) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;
    if (options.refresh == false) return;
    struct dm_map_entity *me = dm_get_map_me(pos, map);
    bool item_is_weapon = true;

    if (me->in_sight == false) {
        charwin_refresh();
        return;
    }

    wclear(window->win);
    werase(window->win);
    ui_print_reset(window);

    if (me->monster != NULL) {
        if (me->monster->is_player == true) {
            ui_printf(window, "Target: " cs_PLAYER "you" cs_CLOSE ".\n");

            if (me->monster->wounds.curr < 0) ui_printf(window, "You are criticly wounded.\n");
            else if (me->monster->wounds.curr != me->monster->wounds.max) ui_printf(window, "You are wounded.\n");
        } else if (fght_can_see(map, player, me->monster) ) {
            ui_printf(window, "Target %s.\n", msr_ldname(me->monster) );

            if (me->monster->wounds.curr < 0) ui_printf(window, "\n%s is criticly wounded.\n", msr_gender_name(me->monster, false) );
            else if (me->monster->wounds.curr != me->monster->wounds.max) ui_printf(window, "\n%s is wounded.\n", msr_gender_name(me->monster, false) );
        }

    }
    else ui_printf(window,"No Target.\n");
    ui_printf(window,"\n");

    int tohit = 0;
    if (wtype == WEAPON_TYPE_MELEE && (cd_pyth(&player->pos, pos) == 1) ) {
        tohit = fght_melee_calc_tohit(player, pos, witem, FGHT_MAIN_HAND);
        ui_printf(window,"Melee Skill: %d\n\n", msr_calculate_skill(player, MSR_SKILLS_MELEE) );
        item_is_weapon = true;
    }
    else if (wtype == WEAPON_TYPE_RANGED) {
        tohit = fght_ranged_calc_tohit(player, pos, witem, FGHT_MAIN_HAND, false);
        ui_printf(window,"Ranged Skill: %d\n\n", msr_calculate_skill(player, MSR_SKILLS_RANGED) );
        item_is_weapon = true;
    }
    else if (wtype == WEAPON_TYPE_THROWN) {
        tohit = fght_ranged_calc_tohit(player, pos, witem, FGHT_MAIN_HAND, true);
        ui_printf(window,"Ranged Skill: %d\n\n", msr_calculate_skill(player, MSR_SKILLS_RANGED) );
        item_is_weapon = true;
    }
    else item_is_weapon = false;

    if (item_is_weapon) {
        ui_printf(window,"Total change of hitting: " cs_DAMAGE "%d" cs_CLOSE ".\n", tohit);

        int idx = 0;
        struct tohit_desc *thd = NULL;
        while ( (thd = fght_get_tohit_mod_description(idx++) ) != NULL) {
            ui_printf(window,"%c %s (%d).\n", (thd->modifier > 0) ? '+' : '-', thd->description, thd->modifier);
        }

        ui_printf(window,"\n");
    }

    ui_printf(window, "Calculated: %s.\n", witem->ld_name);
    if (wpn_is_catergory(witem, WEAPON_CATEGORY_THROWN_GRENADE) ) {
        ui_printf(window,"Timer: %d.%d.\n", witem->energy / TT_ENERGY_TURN, witem->energy % TT_ENERGY_TURN);
    }

    if (options.refresh) wrefresh(window->win);
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

    if (msr_weapon_type_check(plr->player, WEAPON_TYPE_MELEE) && (cd_pyth(&plr->player->pos, p_pos) > 1)) {
        You(plr->player, "cannot reach your target with your weapon.");
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

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

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
                else if(ma_do_melee(plr->player, &e_pos) == true) {
                    mapwin_display_map(map, p_pos);
                    return true;
                }
                else if (cd_equal(&e_pos, &plr->player->pos) ) {
                    You(plr->player, "dare not attack yourself.");
                }
                else Your(plr->player, "weapon(s) failed to fire.");
                fire_mode=false;
            }
            break;
            default: break;
        }
        if (fire_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        lg_debug("entering fire_mode (%d,%d) -> (%d,%d)", p_pos->x, p_pos->y, e_pos.x, e_pos.y);

        if (options.refresh) {
            path_len = sgt_los_path(gbl_game->current_map, p_pos, &e_pos, &path, false);
            for (int i = 1; i < path_len; i++) {
                lg_debug("point[%d] in projectile path: (%d,%d)", i, path[i].x, path[i].y);
                mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            }
            if (path_len > 0) free(path);

            mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            wrefresh(map_win->win);
        }

        struct itm_item *witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_RANGED, FGHT_MAIN_HAND);
        if (witem == NULL) witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_RANGED, FGHT_OFF_HAND);
        if (witem != NULL) targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, witem, WEAPON_TYPE_RANGED);
        else {
            witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_MELEE, FGHT_MAIN_HAND);
            if (witem == NULL) witem = fght_get_working_weapon(plr->player, WEAPON_TYPE_MELEE, FGHT_OFF_HAND);
            if (witem != NULL) targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, witem, WEAPON_TYPE_MELEE);
        }
    }
    while((ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE && fire_mode);

    return false;
}

bool mapwin_overlay_throw_item_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos) {
    int ch = '0';
    bool fire_mode = true;
    if (map_win == NULL) return false;
    if (g == NULL) return false;
    if (dm_verify_map(map) == false) return false;
    if (p_pos == NULL) return false;
    if (map_win->type != HRL_WINDOW_TYPE_MAP) return false;

    struct pl_player *plr = &g->player_data;
    if (plr == NULL) return false;
    coord_t e_pos = *p_pos;

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

    coord_t *path;
    int path_len = 0;

    int item_idx = 0;
    struct itm_item *item = aiu_next_unused_item(plr->player, item_idx);
    if (item == NULL) return false;
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
            case INP_KEY_WEAPON_SETTING:
                if (item_idx > 0) {
                    item_idx -= 0;
                    item = aiu_next_unused_item(plr->player, item_idx);
                }
                break;
            case INP_KEY_WEAPON_SELECT:
                if (aiu_next_unused_item(plr->player, item_idx+1) != NULL) {
                    item_idx += 1;
                    item = aiu_next_unused_item(plr->player, item_idx);
                }
                break;
            case INP_KEY_YES:
            case INP_KEY_THROW_ITEM:
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
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        if (options.refresh) {
            path_len = sgt_los_path(gbl_game->current_map, p_pos, &e_pos, &path, false);
            for (int i = 1; i < path_len; i++) {
                mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            }
            if (path_len > 0) free(path);

            mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            wrefresh(map_win->win);
        }

        targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, item, WEAPON_TYPE_THROWN);
    }
    while((ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE && fire_mode);

    return false;
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
    if (aiu_next_thrown_weapon(plr->player, 0) == NULL) {
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

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

    coord_t *path;
    int path_len = 0;

    int weapon_idx = 0;
    struct itm_item *item = aiu_next_thrown_weapon(plr->player, weapon_idx);
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
                    item = aiu_next_thrown_weapon(plr->player, weapon_idx);
                    item->energy = TT_ENERGY_TURN;
                }
                break;
            case INP_KEY_WEAPON_SELECT:
                if (aiu_next_thrown_weapon(plr->player, weapon_idx+1) != NULL) {
                    weapon_idx += 1;
                    item = aiu_next_thrown_weapon(plr->player, weapon_idx);
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
            case INP_KEY_THROW_ITEM:
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
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        if (options.refresh) {
            path_len = sgt_los_path(gbl_game->current_map, p_pos, &e_pos, &path, false);
            for (int i = 1; i < path_len; i++) {
                mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            }
            if (path_len > 0) free(path);

            mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            wrefresh(map_win->win);
        }

        targetwin_examine(char_win, gbl_game->current_map, plr->player, &e_pos, item, WEAPON_TYPE_THROWN);
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
    if (options.refresh == false) return;
    int starty = 1;

    werase(char_win->win);
    ui_print_reset(char_win);

    if (options.play_recording) {
        ui_printf_ext(char_win, 1,1, cs_ATTR "playback x%d" cs_CLOSE "\n", options.play_delay);
        starty += 2;
    }

    ui_print_reset(char_win);

    struct msr_monster *player = plr->player;

    ui_printf(char_win, cs_ATTR "Name      " cs_CLOSE " %-14s\n", player->unique_name);
    ui_printf(char_win, cs_ATTR "Background" cs_CLOSE " %-14s\n", cr_get_background_by_id(plr->career.b_tid)->name);
    ui_printf(char_win, cs_ATTR "Role      " cs_CLOSE " %-14s\n", cr_get_role_by_id(plr->career.r_tid)->name);

    ui_printf(char_win, cs_ATTR "Turn      " cs_CLOSE " %d.%d\n", gbl_game->turn / TT_ENERGY_TURN, gbl_game->turn % TT_ENERGY_TURN);
    ui_printf(char_win, "\n");

    int cmb = msr_calculate_characteristic(player, MSR_CHAR_COMBAT);
    int str = msr_calculate_characteristic(player, MSR_CHAR_STRENGTH);
    int tgh = msr_calculate_characteristic(player, MSR_CHAR_TOUGHNESS);
    int agi = msr_calculate_characteristic(player, MSR_CHAR_AGILITY);
    int intel = msr_calculate_characteristic(player, MSR_CHAR_INTELLIGENCE);
    int per = msr_calculate_characteristic(player, MSR_CHAR_PERCEPTION);
    int wil = msr_calculate_characteristic(player, MSR_CHAR_WILLPOWER);

    ui_printf(char_win, cs_ATTR "CMB" cs_CLOSE "  %3d\n", cmb);
    ui_printf(char_win, cs_ATTR "Str" cs_CLOSE "  %3d  " cs_ATTR "Tgh" cs_CLOSE  "  %3d\n", str, tgh);
    ui_printf(char_win, cs_ATTR "Agi" cs_CLOSE "  %3d  " cs_ATTR "Int" cs_CLOSE  "  %3d\n", agi, intel);
    ui_printf(char_win, cs_ATTR "Per" cs_CLOSE "  %3d  " cs_ATTR "Wil" cs_CLOSE  "  %3d\n", per, wil);

    ui_printf(char_win, "\n");

    ui_printf(char_win, cs_ATTR "Wounds" cs_CLOSE "    [%2d/%2d]\n", player->wounds.curr, player->wounds.max);
    ui_printf(char_win, cs_ATTR "Armour" cs_CLOSE " [%d][%d][%d][%d]\n",
                                            msr_calculate_armour(player, MSR_HITLOC_HEAD),
                                            msr_calculate_armour(player, MSR_HITLOC_BODY),
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_ARM), /* Left and Right should be the same*/
                                            msr_calculate_armour(player, MSR_HITLOC_LEFT_LEG) );
    ui_printf(char_win, "\n");

    struct itm_item *item;
    for (int i = 0; i<2; i++) {
        bitfield32_t loc = INV_LOC_MAINHAND_WIELD;
        if (i == 1) loc = INV_LOC_OFFHAND_WIELD;

        if ( (item = inv_get_item_from_location(player->inventory, loc) ) != NULL) {
            if ( (loc == INV_LOC_MAINHAND_WIELD) && (item == inv_get_item_from_location(player->inventory, INV_LOC_OFFHAND_WIELD) ) )  continue; /*ignore off=hand when wielding 2h weapon.*/

            if (item->item_type == ITEM_TYPE_WEAPON) {
                struct item_weapon_specific *wpn = &item->specific.weapon;
                ui_printf(char_win, cs_ATTR "%s Wpn:" cs_CLOSE " %s\n", (i==0) ? "Main" : "Sec.", item->sd_name);
                if (wpn->nr_dmg_die == 0) ui_printf(char_win, " Dmg 1D5");
                else ui_printf(char_win, cs_ATTR " Dmg" cs_CLOSE " %dD10", wpn->nr_dmg_die);
                int add = wpn->dmg_addition;
                if (wpn_is_type(item, WEAPON_TYPE_MELEE) ) add += msr_calculate_characteristic_bonus(player, MSR_CHAR_STRENGTH);
                char sign = ' ';
                if (add > 0) sign = '+';
                ui_printf(char_win, "%c%d", sign, add);
                if (wpn->penetration > 0) ui_printf(char_win, ", pen %d", wpn->penetration);

                if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                    if (wpn->jammed == false) {
                        ui_printf(char_win, "  " cs_ATTR "Ammo" cs_CLOSE " %d/%d\n", wpn->magazine_left, wpn->magazine_sz);
                    }
                    else ui_printf(char_win, "  " cs_ATTR "jammed" cs_CLOSE "\n");

                    int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                    int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                    int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];

                    if (semi > 0 || aut > 0) {
                        const char *set = (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) ? "single":
                                    (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) ? "semi": "auto";
                        char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                        char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                        ui_printf(char_win, " " cs_ATTR "Setting:" cs_CLOSE " %s (%s/%s/%s)\n", set,
                                (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                    }
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
                ui_printf(char_win, cs_ATTR "Using off-hand." cs_CLOSE "\n");
                break;
            case MSR_WEAPON_SELECT_MAIN_HAND:
                ui_printf(char_win, cs_ATTR "Using main-hand." cs_CLOSE "\n");
                break;
            case MSR_WEAPON_SELECT_BOTH_HAND:
            case MSR_WEAPON_SELECT_DUAL_HAND:
                ui_printf(char_win, cs_ATTR "Using both hands." cs_CLOSE "\n");
                break;
            case MSR_WEAPON_SELECT_CREATURE1:
                ui_printf(char_win, cs_ATTR "Unarmed." cs_CLOSE "\n");
            default: break;
        }
        ui_printf(char_win, "\n");
    }

    int cnt = 0;
    struct msr_monster *target = NULL;
    while ( (target = aiu_get_nearest_enemy(player, cnt, gbl_game->current_map) ) != NULL) {
        int y = ui_printf(char_win, "    %s", msr_ldname(target));
        if (target->controller.ai.emo_state != NULL) ui_printf(char_win, " (%s)", target->controller.ai.emo_state);
        ui_printf(char_win, "\n");
        wattron(char_win->win, get_colour(target->icon_attr) );
        mvwaddch(char_win->win, y, 0, target->icon);
        wattroff(char_win->win, get_colour(target->icon_attr) );

        int tgt_status = get_colour(TERM_COLOUR_BG_GREEN);
        if (target->wounds.curr < 0) tgt_status = get_colour(TERM_COLOUR_BG_RED);
        else if ( ( (target->wounds.curr * 100) / target->wounds.max) < 50) tgt_status = get_colour(TERM_COLOUR_BG_YELLOW);

        wattron(char_win->win, tgt_status);
        mvwaddch(char_win->win, y, 2, ' ');
        wattroff(char_win->win, tgt_status);

        cnt++;
    }
    if (cnt > 0) ui_printf(char_win, "\n");

    if (options.refresh) wrefresh(char_win->win);
    enum status_effect_flags seef[] = { EF_BLEEDING, EF_BLINDED,
            EF_DEAFENED, EF_DISABLED_LLEG, EF_DISABLED_RLEG,
            EF_DISABLED_LARM, EF_DISABLED_RARM, EF_DISABLED_EYE, EF_ENCUMBERED,
            EF_EXHAUSTED, EF_GRAPPLED, EF_HELPLESS, EF_INVISIBLE,
            EF_ON_FIRE, EF_PARALYZED, EF_PINNED, EF_PRONE,
            EF_SWIMMING, EF_STAGGERED, EF_STUNNED, EF_UNCONSCIOUS, };
    for (unsigned int i = 0; i < ARRAY_SZ(seef); i++) {
        if (se_has_effect(player, seef[i]) ) {
            ui_printf(char_win, "[%s]", se_effect_names(seef[i]) );
            lg_debug("[%s:  %d]", se_effect_names(seef[i]), seef[i]);
        }
    }
}

/* Beware of dragons here..... */

struct inv_show_item {
    const char *location;
    struct itm_item *item;
};

static int invwin_printlist(struct hrl_window *window, struct inv_show_item list[], int list_sz, int start, int end) {
    int max = MIN(list_sz, end);

    /*
    werase(window->win);
    ui_print_reset(window);
    */

    if (list_sz == 0) {
        ui_printf_ext(window, 1, 1, "Your inventory is empty");
        return 0;
    }

    max = MIN(max, INP_KEY_MAX_IDX);
    if (start >= max) return -1;

    for (int i = 0; i < max; i++) {
        struct itm_item *item = list[i+start].item;
        int y = ui_printf(window, "%c)  %c%s", inp_key_translate_idx(i), list[i+start].location[0], item->sd_name);
        if (item->quality != ITEM_QLTY_AVERAGE) ui_printf(window, "(%s)", itm_quality_string(item) );

        int weight = item->weight;
        if (item->stacked_quantity > 1) {
            weight *= item->stacked_quantity;
            ui_printf(window, "(x%llu)", (unsigned long long int) item->stacked_quantity);
        }

        ui_printf_ext(window, y, 34, " (%4.1f kg)\n", (weight * INV_WEIGHT_MODIFIER) );
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

    wclear(window->win);
    werase(window->win);
    ui_print_reset(window);

    struct msr_monster *player = gbl_game->player_data.player;
    if (msr_verify_monster(player) == false) return;

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

            char mod = '+';
            int add = wpn->dmg_addition;
            if (wpn_is_type(item, WEAPON_TYPE_MELEE) ) add += msr_calculate_characteristic_bonus(player, MSR_CHAR_STRENGTH);
            if (add < 0) mod = '-';
            ui_printf(char_win, "%c%d\n", mod, abs(add) );

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) || wpn_is_type(item, WEAPON_TYPE_THROWN) ) {
                ui_printf(char_win, "- Range %d\n", wpn->range);
            }

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) {
                ui_printf(char_win, "- Magazine size %d\n", wpn->magazine_sz);
                if (wpn_uses_ammo(item) ) ui_printf(char_win, "- Uses %s\n", wpn_ammo_string(wpn->ammo_type) );

                int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];

                if (semi > 0 || aut > 0) {
                    char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                    char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                    ui_printf(char_win, "- Rate of Fire (%s/%s/%s)\n",
                            (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                }

                ui_printf(char_win, "\n");
                if (wpn->magazine_left == 0) {
                    ui_printf(char_win, "The weapon is empty.\n");
                }
                else {
                    struct itm_item *ammo = itm_create(wpn->ammo_used_tid);
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

                if (wpn->penetration > 0) {
                    ui_printf(char_win, "- Armour Penetration (%d).\n", wpn->penetration);
                }

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
                int energy_pc = (item->energy * 100) / tool->energy;
                ui_printf(char_win, "Energy left %d\%\n", energy_pc);
            }

        }break;
        case ITEM_TYPE_AMMO: {
            struct item_ammo_specific *ammo = &item->specific.ammo;
            if (ammo->energy > 0) {
                int energy_pc = (ammo->energy_left * 100) / ammo->energy;
                ui_printf(char_win, "Energy left %d\%\n", energy_pc);
            }
            ui_printf(char_win, "Provides %s\n", wpn_ammo_string(ammo->ammo_type) );
        } break;
        case ITEM_TYPE_FOOD: break;
        default: break;
    }
    if (options.refresh) wrefresh(window->win);
}

void status_effect_examine(struct hrl_window *window, struct status_effect *se) {
    if (window == NULL) return;
    if (se_verify_status_effect(se) == false) return;
    if (window->type != HRL_WINDOW_TYPE_CHARACTER) return;
    if (options.refresh == false) return;

    wclear(window->win);
    werase(window->win);
    ui_print_reset(window);

    if (se->name != NULL) {
        ui_printf(window, "%s.\n",se->name);
    }
    else ui_printf(window, "Unknown.\n");

    ui_printf(window, "\n");

    if (se->description != NULL) {
        ui_printf(window, "%s.\n",se->description);
    }
    else ui_printf(window, "No description available.\n");

    ui_printf(window, "\n");
    if (options.refresh) wrefresh(window->win);
}


bool invwin_inventory(struct dm_map *map, struct pl_player *plr) {
    FIX_UNUSED(map);

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
        struct inv_show_item invlist[invsz];
        inv_create_list(plr->player->inventory, invlist, invsz);

        werase(map_win->win);
        ui_print_reset(map_win);

        /*mapwin_display_map_noref(map, &plr->player->pos);
        touchwin(map_win->win);*/

        ui_printf(map_win, "Inventory:         [Total Weight: %3.0f/%3.0f kg]\n", (inv_get_weight(plr->player->inventory) * INV_WEIGHT_MODIFIER), (msr_calculate_carrying_capacity(plr->player) * INV_WEIGHT_MODIFIER) );

        if ( (dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz) ) == -1) {
            invstart = 0;
            dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz);
        }
        ui_printf_ext(map_win, winsz +1, 1, cs_ATTR "[a]" cs_CLOSE " apply             " cs_ATTR "[d]    " cs_CLOSE " drop.");
        ui_printf_ext(map_win, winsz +2, 1, cs_ATTR "[w]" cs_CLOSE " wield/wear/remove " cs_ATTR "[x]    " cs_CLOSE " examine.");
        ui_printf_ext(map_win, winsz +3, 1, cs_ATTR "[q]" cs_CLOSE " exit              " cs_ATTR "[space]" cs_CLOSE " next page.");
        if (options.refresh) wrefresh(map_win->win);
        bool examine = false;

        /* TODO clean this shit up */
        switch (ch) {
            case INP_KEY_YES: invstart += dislen; break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_INVENTORY: inventory = false; break;
            case INP_KEY_ALL:
            case INP_KEY_APPLY: {
                ui_printf_ext(map_win, winsz, 1, "Use which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;

                if (ma_do_use(plr->player, item) ) return true;
            } break;
            case INP_KEY_WEAR: {
                ui_printf_ext(map_win, winsz, 1, "Wear which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;

                if (inv_item_worn(plr->player->inventory, item) == true) {
                    if (ma_do_remove(plr->player, item) ) return true;
                }
                else {
                    if (ma_do_wear(plr->player, item) ) return true;
                }
            } break;
            case INP_KEY_EXAMINE: {
                ui_printf_ext(map_win, winsz, 1, "Examine which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;

                invwin_examine(char_win, invlist[item_idx +invstart].item);
                examine = true;
            } break;
            case INP_KEY_DROP: {
                ui_printf_ext(map_win, winsz, 1, "Drop which item?");
                if (options.refresh) wrefresh(map_win->win);

                invsz = inv_inventory_size(plr->player->inventory);
                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if ((item_idx + invstart) >= invsz) break;
                item = invlist[item_idx +invstart].item;
                struct itm_item *items[1] = {item};
                if (ma_do_drop(plr->player, items, 1) ) return true;
            } break;
            default: break;
        }

        wmove(map_win->win, winsz, 0);
        wclrtoeol(map_win->win);
        if (options.refresh) wrefresh(map_win->win);

        if (examine == false) charwin_refresh();

    } while((inventory != false) && (ch = inp_get_input(gbl_game->input) ) != INP_KEY_ESCAPE);

    charwin_refresh();

    return false;
}

void character_window(void) {
    int y = 0;
    struct pl_player *plr = &gbl_game->player_data;
    struct msr_monster *mon = plr->player;

    struct hrl_window *window = main_win;
    struct hrl_window pad;
    memmove(&pad, window, sizeof(struct hrl_window) );
    pad.lines -= 4;
    pad.win = newpad(pad.lines, pad.cols);
    assert(pad.win != NULL);

    if (options.refresh) {
        wclear(window->win);
        werase(window->win);
    }

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
    ui_printf(&pad, cs_ATTR "Name:      " cs_CLOSE " %-20s\n", mon->unique_name);
    ui_printf(&pad, cs_ATTR "Gender:    " cs_CLOSE " %-20s\n", msr_gender_string(mon) );
    ui_printf(&pad, cs_ATTR "Homeworld: " cs_CLOSE " %-20s\n", cr_get_homeworld_by_id(plr->career.h_tid)->name );
    ui_printf(&pad, cs_ATTR "Background " cs_CLOSE " %-20s\n", cr_get_background_by_id(plr->career.b_tid)->name);
    ui_printf(&pad, cs_ATTR "Role       " cs_CLOSE " %-20s\n", cr_get_role_by_id(plr->career.r_tid)->name);

    ui_printf(&pad, cs_ATTR "Wounds:    " cs_CLOSE " %d/%d\n", mon->wounds.curr, mon->wounds.max);
    ui_printf(&pad, cs_ATTR "XP:        " cs_CLOSE " %d\n", plr->career.xp_current);
    ui_printf(&pad, cs_ATTR "XP Spend:  " cs_CLOSE " %d\n", plr->career.xp_spend);

    int quest_desc_len = 100;
    char quest_desc[quest_desc_len];
    qst_get_description(plr->quest, quest_desc, quest_desc_len);
    ui_printf(&pad, cs_ATTR "Quest:" cs_CLOSE " %-20s\n", quest_desc);
    //ui_printf(&pad, cs_ATTR "Corruption:" cs_CLOSE "    %d\n", mon->corruption_points);

    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad,           cs_ATTR "Cmb" cs_CLOSE "  %-2d\n", msr_calculate_characteristic(mon, MSR_CHAR_COMBAT) );
    ui_printf(&pad,           cs_ATTR "Str" cs_CLOSE "  %-2d", msr_calculate_characteristic(mon, MSR_CHAR_STRENGTH) );
        ui_printf(&pad, "   " cs_ATTR "Tgh" cs_CLOSE "   %-2d\n", msr_calculate_characteristic(mon, MSR_CHAR_TOUGHNESS) );
    ui_printf(&pad,           cs_ATTR "Agi" cs_CLOSE "  %-2d", msr_calculate_characteristic(mon, MSR_CHAR_AGILITY) );
        ui_printf(&pad, "   " cs_ATTR "Int" cs_CLOSE "   %-2d\n", msr_calculate_characteristic(mon, MSR_CHAR_INTELLIGENCE) );
    ui_printf(&pad,           cs_ATTR "Per" cs_CLOSE "  %-2d", msr_calculate_characteristic(mon, MSR_CHAR_PERCEPTION) );
        ui_printf(&pad, "   " cs_ATTR "Wil" cs_CLOSE "   %-2d\n", msr_calculate_characteristic(mon, MSR_CHAR_WILLPOWER) );

    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");


    /* Armour  */
    ui_printf(&pad, cs_ATTR "Armour              DR    Locations" cs_CLOSE "\n");
    ui_printf(&pad, cs_ATTR "------              ---   ---------" cs_CLOSE "\n");

    /* Armour */
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(mon->inventory, item) ) != NULL) {
        if ( (inv_item_worn(mon->inventory, item) == true) &&
             (inv_item_wielded(mon->inventory, item) == false) ) {
            int armour = 0;
            bitfield32_t locs = inv_get_item_locations(mon->inventory, item);

            if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
                armour = item->specific.wearable.damage_reduction;
            }

            y = ui_printf(&pad, "%s", item->ld_name);
            ui_printf_ext(&pad,y, 20, "%-2d  ", armour);
            ui_printf_ext(&pad,y, 24, "");

            bool first = true;
            for (enum inv_locations i = 1; i < INV_LOC_MAX; i <<= 1) {
                if ( (locs & i) > 0) {
                    if (first == false) ui_printf(&pad, "/");
                    ui_printf(&pad,"%s", inv_location_name(locs & i) );
                    first = false;
                }
            }
            ui_printf(&pad, "\n");
        }
    }
    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");

    /* Skills */
    ui_printf(&pad, cs_ATTR "Skills" cs_CLOSE "               "cs_ATTR "Rate" cs_CLOSE "\n");
    ui_printf(&pad, cs_ATTR "------" cs_CLOSE "               "cs_ATTR "----" cs_CLOSE "\n");

    for (unsigned int i = 0; i < MSR_SKILLS_MAX; i++) {
        if (msr_has_skill(mon, i) ) {
            enum msr_skill_rate skillrate = msr_has_skill(mon,  i);
            lg_debug("skill rate: %d", skillrate);
            ui_printf(&pad, "%-20s %s\n", msr_skill_names(i),  msr_skillrate_names(skillrate));
        }
    }
    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");
    /* Talents */
    ui_printf(&pad, cs_ATTR "Talents" cs_CLOSE "\n");
    ui_printf(&pad, cs_ATTR "-------" cs_CLOSE "\n");

    for (int i = MSR_TALENT_TIER_T1; i < MSR_TALENT_TIER_MAX; i++) {
        for (int x = 0; x < MSR_TALENTS_PER_TIER; x++) {
            enum msr_talents t = MSR_TALENT(i, x);
            if (t == TLT_NONE) continue;

            if (msr_has_talent(mon, t) ) {
                ui_printf(&pad, "%s\n", msr_talent_names(t) );
            }
        }
    }
    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");


    /* Status Effects */
    ui_printf(&pad, cs_ATTR "Status Effects" cs_CLOSE "\n");
    ui_printf(&pad, cs_ATTR "--------------" cs_CLOSE "\n");

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(mon->status_effects, c) ) != NULL) {
        ui_printf(&pad, "%s\n", c->name);
    }
    ui_printf(&pad, "\n");

    y = ui_printf(&pad, "\n");


    /* Controls */
    int line = 0;
    bool watch = true;
    while(watch == true) {
        ui_print_reset(window);
        ui_printf_ext(window, window->lines -3, 1, cs_ATTR " [q]" cs_CLOSE " exit. " );
        ui_printf_ext(window, window->lines -2, 1, cs_ATTR "[up]" cs_CLOSE " up,  "  cs_ATTR "  [down]" cs_CLOSE " down.");
        if (options.refresh) wrefresh(window->win);
        prefresh(pad.win, line,0,pad.y,pad.x, pad.y + pad.lines -5, pad.x + pad.cols);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_UP_RIGHT:   line += 20; break;
            case INP_KEY_DOWN_RIGHT: line -= 20; break;
            case INP_KEY_UP:         line--; break;
            case INP_KEY_DOWN:       line++; break;

            case INP_KEY_CHARACTER:
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
    wclear(window->win);
    werase(window->win);
    if (options.refresh) {
        wrefresh(window->win);
    }

    show_msg(msg_win);
}

void show_log(struct hrl_window *window, bool input) {
    int y = 0;
    int log_sz = lg_size(gbl_log);
    struct log_entry *tmp_entry = NULL;

    struct hrl_window pad;
    memmove(&pad, window, sizeof(struct hrl_window) );
    pad.win = newpad(MAX(log_sz, window->lines) , window->cols);
    assert(pad.win != NULL);

    wclear(window->win);
    werase(window->win);

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

                if (tmp_entry->turn < gbl_game->plr_last_turn) {
                    old = true;
                }

                if (print) {
                    if (old) ui_printf(&pad, cs_OLD);
                    y = ui_printf(&pad, "%s%s", pre_format_buf, tmp_entry->string);
                    if (tmp_entry->repeat > 1) {
                        ui_printf(&pad, " (x%d)", tmp_entry->repeat);
                    }
                    ui_printf(&pad, "\n");
                    if (old) ui_printf(&pad, cs_CLOSE);
                }
            }
        }
    }
    else ui_printf(&pad, "Empty\n");

    if (input) {
        int line = 0;
        bool watch = true;
        while(watch == true) {
            ui_print_reset(window);
            ui_printf_ext(window, window->lines -3, 1, cs_ATTR "[up]" cs_CLOSE " newer,  " cs_ATTR "[down]" cs_CLOSE " older.");
            ui_printf_ext(window, window->lines -2, 1, cs_ATTR " [q]" cs_CLOSE " exit.");
            if (options.refresh) wrefresh(window->win);
            prefresh(pad.win, line,0,pad.y,pad.x, pad.y + pad.lines -5, pad.x + pad.cols);

            switch (inp_get_input(gbl_game->input) ) {
                case INP_KEY_UP_RIGHT:   line += 20; break;
                case INP_KEY_DOWN_RIGHT: line -= 20; break;
                case INP_KEY_UP:         line--; break;
                case INP_KEY_DOWN:       line++; break;

                case INP_KEY_LOG:
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

    wclear(window->win);
    werase(window->win);
    if (options.refresh) {
        wrefresh(window->win);
    }

    show_msg(msg_win);
}

#define SHOW_MAX_MSGS (20)
void show_msg(struct hrl_window *window) {
    int log_sz = lg_size(gbl_log);
    struct log_entry *tmp_entry = NULL;
    if (options.refresh == false) return;

    werase(window->win);
    scrollok(window->win, TRUE);
    ui_print_reset(window);

    int ctr = 0;
    if (log_sz > 0) {
        int min = 0;
        for (int i = log_sz -1; i >= 0; i--) {
            tmp_entry = lg_peek(gbl_log, i);
            if (tmp_entry != NULL) {
                if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) {
                    ctr++;
                }
            }
            if (ctr >= window->lines) {
                min = i;
                i = 0;
            }
        }

        if (ctr > 0) {
            for (int i = min; i < log_sz; i++) {
                bool old = false;

                tmp_entry = lg_peek(gbl_log, i);
                if (tmp_entry != NULL) {
                    if (tmp_entry->level <= LG_DEBUG_LEVEL_GAME) {

                        if (tmp_entry->turn < gbl_game->plr_last_turn) {
                            old = true;
                        }

                        if (old) ui_printf(window, "%s", cs_OLD);
                        ui_printf(window, "%s", tmp_entry->string);
                        if (tmp_entry->repeat > 1) {
                            ui_printf(window, " (x%d)", tmp_entry->repeat);
                        }
                        if (old) ui_printf(window, "%s", cs_CLOSE);
                        ui_printf(window, "\n");
                    }
                }
            }
        }
    }
}

void log_window(void) {
    show_log(main_win, true);
}

void charwin_examine(const char *type, const char *name, const char *description) {
    //if (options.refresh == false) return;

    werase(char_win->win);
    ui_print_reset(char_win);

    ui_printf(char_win, cs_ATTR "%s:" cs_CLOSE, type);
    if (name) ui_printf(char_win, " %s\n", name);
    else ui_printf(char_win, "\n");
    ui_printf(char_win, "\n");

    ui_printf(char_win, "%s", description);
    wrefresh(char_win->win);
}

enum lvlup_type {
    LVLUP_NONE,
    LVLUP_CHAR,
    LVLUP_SKILL,
    LVLUP_TLT_T1,
    LVLUP_TLT_T2,
    LVLUP_TLT_T3,
};

struct lvl_struct {
    enum lvlup_type type;
    bitfield64_t id;
    int cost;

    const char *name;
    const char *description;
};

void levelup_aquire_window(struct lvl_struct *list, int start, int sz, const char *type_str) {
    struct hrl_window *window = map_win;

    struct msr_monster *player = gbl_game->player_data.player;
    if (msr_verify_monster(player) == false) return;
    charwin_refresh();

    bool lvl_up_done = false;
    while(lvl_up_done == false) {
        wclear(window->win);
        werase(window->win);
        ui_print_reset(window);

        ui_printf(window,"%s\n", type_str);
        ui_printf(window, cs_ATTR "XP left:" cs_CLOSE " %d\n", gbl_game->player_data.career.xp_current);
        ui_printf(window,"\n");

        enum lvlup_type type = list[start].type;
        for (int i = start; i < sz && list[i+1].type == type; i++) {
            int y = ui_printf(window, cs_ATTR " %c) " cs_CLOSE "%s", inp_key_translate_idx(i-start), list[i].name);
            ui_printf_ext(window, y, 30, "%dxp\n", list[i].cost);
        }

        ui_printf_ext(map_win, map_win->lines - 3, 1, cs_ATTR "[a]" cs_CLOSE " acquire, " cs_ATTR "  [x]" cs_CLOSE " examine.");
        ui_printf_ext(map_win, map_win->lines - 2, 1, cs_ATTR "[q]" cs_CLOSE " exit,    " cs_ATTR "  [?]" cs_CLOSE " help.");
        if (options.refresh) wrefresh(window->win);

        int ch = inp_get_input(gbl_game->input);
        switch (ch) {
            case INP_KEY_EXAMINE: {
                    ui_printf_ext(map_win, map_win->lines - 4, 1, "Examine which selection?");
                    if (options.refresh) wrefresh(window->win);

                    int tidx = inp_get_input_idx(gbl_game->input) + start;
                    charwin_examine(type_str, list[tidx].name, list[tidx].description);
                }
                break;
            case INP_KEY_APPLY: {
                    charwin_refresh();

                    ui_printf_ext(map_win, map_win->lines - 4, 1, "Upgrade which selection?");
                    if (options.refresh) wrefresh(window->win);

                    int tidx = inp_get_input_idx(gbl_game->input) + start;
                    if (gbl_game->player_data.career.xp_current < list[tidx].cost) {
                        GM_msg(cs_PLAYER "You" cs_CLOSE " do not have enough experience points.");
                        break;
                    }

                    switch(list[tidx].type){
                        case LVLUP_NONE: assert(false);
                        case LVLUP_CHAR: assert(false);
                        case LVLUP_SKILL: {
                            enum msr_skill_rate sr = msr_has_skill(player, list[tidx].id) +1;
                            assert(msr_set_skill(player, list[tidx].id, sr) );
                            GM_msg(cs_PLAYER "You" cs_CLOSE " trained the %s skill to %s level.", msr_skill_names(list[tidx].id), msr_skillrate_names(sr) );
                            gbl_game->player_data.career.xp_current -= list[tidx].cost;
                            gbl_game->player_data.career.xp_spend   += list[tidx].cost;
                        } break;
                        case LVLUP_TLT_T1:
                        case LVLUP_TLT_T2:
                        case LVLUP_TLT_T3: {
                            assert(msr_set_talent(player, list[tidx].id) );
                            GM_msg(cs_PLAYER "You" cs_CLOSE " acquired the %s talent.", msr_talent_names(list[tidx].id));
                            gbl_game->player_data.career.xp_current -= list[tidx].cost;
                            gbl_game->player_data.career.xp_spend   += list[tidx].cost;
                        } break;
                        default: assert(false); break;
                    }
                    lvl_up_done = true;
                }
                break;
            case INP_KEY_HELP:
                help_window();
                charwin_refresh();
                break;

            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_ESCAPE:
                lvl_up_done = true;
            default: break;
        }
    }
}

void levelup_selection_window(void) {
    //if (options.refresh == false) return;

    struct hrl_window *window = map_win;

    struct msr_monster *player = gbl_game->player_data.player;
    if (msr_verify_monster(player) == false) return;
    charwin_refresh();

    int lvl_list_sz = MSR_CHAR_MAX + MSR_SKILLS_MAX +(3 * MSR_TALENTS_PER_TIER);
    struct lvl_struct lvl_list[lvl_list_sz];

    bool lvl_up_done = false;
    int ch = INP_KEY_NONE;
    do {
        wclear(window->win);
        werase(window->win);
        ui_print_reset(window);

        int lvl_list_idx = 0;

        int skill_start = lvl_list_idx;
        for (int i = MSR_SKILLS_NONE+1; i < MSR_SKILLS_MAX; i++) {
            if (msr_has_skill(player, i) != MSR_SKILL_RATE_VETERAN) {
                lvl_list[lvl_list_idx].type = LVLUP_SKILL;
                lvl_list[lvl_list_idx].id = i;
                lvl_list[lvl_list_idx].name = msr_skill_names(i);
                lvl_list[lvl_list_idx].description = msr_skill_descriptions(i);
                lvl_list[lvl_list_idx].cost = cr_skill_cost(&gbl_game->player_data, i);

                lvl_list_idx++;
            }
        }

        int talent_start[MSR_TALENT_TIER_MAX];
        //for (int i = MSR_TALENT_TIER_T1; i < MSR_TALENT_TIER_MISC; i++) {
        int i = 0;
            talent_start[i] = lvl_list_idx;
            for (int x = 1; x < MSR_TALENTS_PER_TIER; x++) {
                enum msr_talents t = MSR_TALENT(i, x);
                if ( (msr_has_talent(player, t) == false) && (cr_talent_cost(&gbl_game->player_data, t) > 0) ) {
                    lvl_list[lvl_list_idx].type = LVLUP_TLT_T1 +i;
                    lvl_list[lvl_list_idx].id = t;
                    lvl_list[lvl_list_idx].name = msr_talent_names(t);
                    lvl_list[lvl_list_idx].description = msr_talent_descriptions(t);
                    lvl_list[lvl_list_idx].cost = cr_talent_cost(&gbl_game->player_data, t);

                    lvl_list_idx++;
                }
            }
        //}

        ui_printf(window,"Spending experience points\n");
        ui_printf(window, cs_ATTR "XP left:" cs_CLOSE " %d\n", gbl_game->player_data.career.xp_current);
        ui_printf(window,"\n");

        ui_printf(window, cs_ATTR " a)" cs_CLOSE " Level Characteristics\n");
        ui_printf(window, cs_ATTR " b)" cs_CLOSE " Train Skills\n");
        ui_printf(window, cs_ATTR " c)" cs_CLOSE " Acquire Tier 1 Talent\n");

        ui_printf_ext(map_win, map_win->lines - 3, 1, cs_ATTR "[a]" cs_CLOSE " acquire." );
        ui_printf_ext(map_win, map_win->lines - 2, 1, cs_ATTR "[q]" cs_CLOSE " exit,    " cs_ATTR "  [?]" cs_CLOSE " help.");
        if (options.refresh) wrefresh(window->win);

        ch = inp_get_input(gbl_game->input);
        switch (ch) {
            case INP_KEY_APPLY: {
                    charwin_refresh();

                    ui_printf_ext(map_win, map_win->lines - 4, 1, "Upgrade what?");
                    if (options.refresh) wrefresh(window->win);

                    int tidx = inp_get_input_idx(gbl_game->input);
                    switch(tidx) {
                        case 0: break;
                        case 1: levelup_aquire_window(lvl_list, skill_start, lvl_list_sz, "Skills"); break;
                        case 2: levelup_aquire_window(lvl_list, talent_start[0], lvl_list_sz, "Talents"); break;
                        default: break;
                    }
                }
                break;
            case INP_KEY_HELP:
                help_window();
                charwin_refresh();
                break;

            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_ESCAPE:
                lvl_up_done = true;
            default: break;
        }
    } while((lvl_up_done == false) && ch != INP_KEY_ESCAPE);
}

void show_help(struct hrl_window *window, bool input) {
    int y = 0;

    int help_sz = 60;

    struct hrl_window pad;
    memmove(&pad, window, sizeof(struct hrl_window) );
    pad.win = newpad(MAX(help_sz, window->lines) , window->cols);
    assert(pad.win != NULL);

    wclear(window->win);
    werase(window->win);

    touchwin(pad.win);
    werase(pad.win);
    ui_print_reset(&pad);

    ui_printf(&pad, "   " cs_ATTR "HeresyRL" cs_CLOSE " help.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "      VI movement:\n");
    ui_printf(&pad, "        " cs_ATTR "<h/j/k/l>:" cs_CLOSE " left/down/up/right.\n");
    ui_printf(&pad, "        " cs_ATTR "<y/u/b/n>:" cs_CLOSE " left-up/right-up/left-down/right-down.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "  Keypad movement:\n");
    ui_printf(&pad, "        " cs_ATTR "<4/2/8/6>:" cs_CLOSE " left/down/up/right.\n");
    ui_printf(&pad, "        " cs_ATTR "<7/9/1/3>:" cs_CLOSE " left-up/right-up/left-down/right-down.\n");
    ui_printf(&pad, "\n");

    /* <Do not touch this evil magic....> */
    ui_printf(&pad, cs_ATTR                "         7  8  9           y  k  u" cs_CLOSE "\n");
    ui_printf(&pad,                      "          \\ | /             \\ | /    \n");
    ui_printf(&pad, "        " cs_ATTR "4" cs_CLOSE " - " cs_ATTR "5" cs_CLOSE " - " cs_ATTR "6" cs_CLOSE "         " cs_ATTR "h" cs_CLOSE " - " cs_ATTR "." cs_CLOSE " - " cs_ATTR "l" cs_CLOSE "\n");
    ui_printf(&pad,                       "          / | \\             / | \\   \n");
    ui_printf(&pad, cs_ATTR                "         1  2  3           b  j  n" cs_CLOSE "\n");
    ui_printf(&pad, "\n");
    /* </Do not touch this evil magic....> */

    ui_printf(&pad, " General Controls:\n");
    ui_printf(&pad, "         " cs_ATTR "<ctrl-X>:" cs_CLOSE " Save and Quit.\n");
    ui_printf(&pad, "            " cs_ATTR "<q/Q>:" cs_CLOSE " Quit Window.\n");
    ui_printf(&pad, "            " cs_ATTR "<x/X>:" cs_CLOSE " eXamine.\n");
    ui_printf(&pad, "            " cs_ATTR "<a/A>:" cs_CLOSE " Apply/Choose.\n");
    ui_printf(&pad, "            " cs_ATTR "<c/C>:" cs_CLOSE " Cancel.\n");
    ui_printf(&pad, "            " cs_ATTR "<o/O>:" cs_CLOSE " Ok.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "          Windows:\n");
    ui_printf(&pad, "              " cs_ATTR "<@>:" cs_CLOSE " Character overview.\n");
    ui_printf(&pad, "              " cs_ATTR "<?>:" cs_CLOSE " This Help window.\n");
    ui_printf(&pad, "              " cs_ATTR "<L>:" cs_CLOSE " message Log.\n");
    ui_printf(&pad, "            " cs_ATTR "<i/I>:" cs_CLOSE " Inventory.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "Main Window Controls:\n");
    ui_printf(&pad, "            " cs_ATTR "<f/F>:" cs_CLOSE " Fire or Fight.\n");
    ui_printf(&pad, "            " cs_ATTR "<t/T>:" cs_CLOSE " Throw.\n");
    ui_printf(&pad, "              " cs_ATTR "<r>:" cs_CLOSE " reload weapon.\n");
    ui_printf(&pad, "              " cs_ATTR "<R>:" cs_CLOSE " unload weapon.\n");
    ui_printf(&pad, "              " cs_ATTR "<[>:" cs_CLOSE " Change weapon fire setting.\n");
    ui_printf(&pad, "              " cs_ATTR "<]>:" cs_CLOSE " Change weapon select.\n");
    ui_printf(&pad, "            " cs_ATTR "<,/g>:" cs_CLOSE " Pickup.\n");
    ui_printf(&pad, "            " cs_ATTR "<./5>:" cs_CLOSE " Wait.\n");
    ui_printf(&pad, "              " cs_ATTR "</>:" cs_CLOSE " Run into a direction.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "Inventory Controls:\n");
    ui_printf(&pad, "            " cs_ATTR "<d/D>:" cs_CLOSE " Drop.\n");
    ui_printf(&pad, "\n");
    ui_printf(&pad, "Fire Mode Controls:\n");
    ui_printf(&pad, "            " cs_ATTR "<Tab>:" cs_CLOSE " Next Target.\n");
    y = ui_printf(&pad, "\n");

    if (input) {
        int line = 0;
        bool watch = true;
        while(watch == true) {
            ui_printf_ext(window, window->lines -3, 1, cs_ATTR "[up]" cs_CLOSE " up,  " cs_ATTR "[down]" cs_CLOSE " down.");
            ui_printf_ext(window, window->lines -2, 1, cs_ATTR " [q]" cs_CLOSE " exit.");
            if (options.refresh) wrefresh(window->win);
            prefresh(pad.win, line,0,pad.y,pad.x, pad.y + pad.lines -5, pad.x + pad.cols);

            switch (inp_get_input(gbl_game->input) ) {
                case INP_KEY_UP_RIGHT:   line += 20; break;
                case INP_KEY_DOWN_RIGHT: line -= 20; break;
                case INP_KEY_UP:         line--; break;
                case INP_KEY_DOWN:       line++; break;

                case INP_KEY_HELP:
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

    wclear(window->win);
    werase(window->win);
    if (options.refresh) {
        wrefresh(window->win);
    }

    show_msg(msg_win);
}

void help_window(void) {
    show_help(main_win, true);
}

void vs_shop(int32_t randint, const char *shop_name, enum item_group *grplst, int grplst_sz){
    struct random *r = random_init_genrand(randint);
    bool buying = true;

    lg_debug("shop: %s", shop_name);

    int32_t sz = (random_int32(r) % 16) + 10;
    int32_t list[sz];

    for (int i = 0; i < sz; i++) {
        bool unique = false;

        for (int j = 0; unique == false && j < 2000; j++) {
            int32_t grp = random_int32(r) % grplst_sz;
            int level_add = random_xd5(r, 1);
            int idx = itm_spawn(random_int32(r), gbl_game->player_data.level +level_add, grplst[grp], NULL);
            list[i] = idx;

            unique = true;
            if (list[i] == IID_NONE) unique = false;
            for (int k = 0; unique && k < i; k++) {
                if (list[i] == list[k]) unique = false;
            }
        }
        if (!unique) sz = i;
    }

    while(buying) {
        struct msr_monster *monster = gbl_game->player_data.player;
        werase(map_win->win);
        ui_print_reset(map_win);

        struct itm_item *money_item = inv_get_item_by_tid(monster->inventory, IID_MONEY);
        unsigned int money = 0;
        if (money_item != NULL) money = money_item->stacked_quantity;

        int y = ui_printf(map_win, "%s", shop_name);
        ui_printf_ext(map_win, y, 30, "(You have %lld gold)\n", money);

        for (int i = 0; i < sz; i++) {
            if (list[i] == -1) continue;

            struct itm_item *itm = itm_create(list[i]);
            y = ui_printf(map_win, cs_ATTR " %c) " cs_CLOSE "%s", inp_key_translate_idx(i), itm->ld_name);
            if (itm->quality != ITEM_QLTY_AVERAGE) ui_printf_ext(map_win, y, 30, " (%s)", itm_quality_string(itm) );
            ui_printf_ext(map_win, y, 40, " (%d gold)\n", itm->cost);
            itm_destroy(itm);
        }

        ui_printf_ext(map_win, map_win->lines -4, 1, cs_ATTR " [a]" cs_CLOSE " acquire" "  " cs_ATTR " [d]" cs_CLOSE " sell.");
        ui_printf_ext(map_win, map_win->lines -3, 1, cs_ATTR " [x]" cs_CLOSE " examine" "  " cs_ATTR " [q]" cs_CLOSE " exit.");
        if (options.refresh) wrefresh(map_win->win);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_APPLY: {
                ui_printf_ext(map_win, map_win->lines -5, 1, "Acquire which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if (item_idx >= sz) break;

                struct itm_item *itm = itm_create(list[item_idx]);
                if (itm->cost <= money) {
                    money_item->stacked_quantity -= itm->cost;
                    msr_give_item(monster, itm);
                    You(monster, "acquired %s", itm->ld_name);
                }
                else {
                    You(monster, "do not have enough money for %s", itm->ld_name);
                    itm_destroy(itm);
                }
            } break;

            case INP_KEY_DROP: {
                int invsz = inv_inventory_size(monster->inventory);
                struct inv_show_item invlist[invsz];
                inv_create_list(monster->inventory, invlist, invsz);

                /* TODO: implement large inventory support */
                /* TODO: implement amount */
                /* TODO: implement barthering */

                werase(map_win->win);
                ui_print_reset(map_win);

                ui_printf(map_win, "Your Inventory:            [Total Weight: %4.0f/%4.0f kg]\n", (inv_get_weight(monster->inventory) * INV_WEIGHT_MODIFIER), (msr_calculate_carrying_capacity(monster) * INV_WEIGHT_MODIFIER) );

                int dislen = 0;
                int invstart = 0;
                int winsz = map_win->lines;
                if ( (dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz) ) == -1) {
                    invstart = 0;
                    dislen = invwin_printlist(map_win, invlist, invsz, invstart, invstart +winsz);
                }

                ui_printf_ext(map_win, map_win->lines -5, 1, "Sell which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if (item_idx >= sz) break;
                struct itm_item *item = invlist[item_idx +invstart].item;
                msr_remove_item(monster, item);
                if (money_item == NULL) {
                    money_item = itm_create(IID_MONEY);
                    msr_give_item(monster, money_item);
                    money_item->stacked_quantity = 0;
                }
                money_item->stacked_quantity += MAX(item->cost / 10, 1);
            } break;

            case INP_KEY_EXAMINE: {
                ui_printf_ext(map_win, map_win->lines -5, 1, "Examine which item?");
                if (options.refresh) wrefresh(map_win->win);

                int item_idx = inp_get_input_idx(gbl_game->input);
                if (item_idx < 0) break;
                if (item_idx == INP_KEY_ESCAPE) break;
                if (item_idx >= sz) break;

                struct itm_item *itm = itm_create(list[item_idx]);
                invwin_examine(char_win, itm);
                itm_destroy(itm);
            } break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: buying = false; break;
            default: break;
        }
    }
    charwin_refresh();
}


void vs_healer(void) {
    bool healing = true;

    while(healing) {
        struct msr_monster *monster = gbl_game->player_data.player;
        werase(map_win->win);
        ui_print_reset(map_win);

        se_remove_all_non_permanent(monster);

        struct itm_item *money_item = inv_get_item_by_tid(monster->inventory, IID_MONEY);
        int money = 0;
        if (money_item != NULL) money = money_item->stacked_quantity;

        ui_printf(map_win, "Healer:      (You have %lld gold)\n", money);

        int listsz = se_list_size(monster->status_effects);
        struct status_effect *se_lst[listsz];

        int i = 0;
        struct status_effect *c = NULL;
        while ( (c = se_list_get_next_status_effect(monster->status_effects, c) ) != NULL) {
            ui_printf(map_win, cs_ATTR " %c) " cs_CLOSE "%s", inp_key_translate_idx(i), c->name);
            ui_printf(map_win, "(cost %d)\n", c->heal_cost);
            se_lst[i++] = c;
        }

        ui_printf_ext(map_win, map_win->lines -4, 1, cs_ATTR " [a]" cs_CLOSE " heal" "  " cs_ATTR " [x]" cs_CLOSE " examine.");
        ui_printf_ext(map_win, map_win->lines -3, 1, cs_ATTR " [q]" cs_CLOSE " exit.");
        if (options.refresh) wrefresh(map_win->win);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_APPLY: {
                ui_printf_ext(map_win, map_win->lines -5, 1, "Heal which condition?");
                if (options.refresh) wrefresh(map_win->win);

                int se_idx = inp_get_input_idx(gbl_game->input);
                if (se_idx < 0) break;
                if (se_idx == INP_KEY_ESCAPE) break;
                if (se_idx >= listsz) break;

                c = se_lst[se_idx];
                if (c->heal_cost <= money) {
                    if (se_heal_status_effect(monster, NULL, c, false) ) {
                        money_item->stacked_quantity -= c->heal_cost;
                        You(monster, "healed %s", c->name);
                    }
                }
                else {
                    You(monster, "do not have enough money to heal %s", c->name);
                }
            } break;

            case INP_KEY_EXAMINE: {
                ui_printf_ext(map_win, map_win->lines -5, 1, "Examine which condition?");
                if (options.refresh) wrefresh(map_win->win);

                int se_idx = inp_get_input_idx(gbl_game->input);
                if (se_idx < 0) break;
                if (se_idx == INP_KEY_ESCAPE) break;
                if (se_idx >= listsz) break;
                status_effect_examine(char_win, se_lst[se_idx]);
            } break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: healing = false; break;
            default: break;
        }
    }
    charwin_refresh();
}

void pay_loan(void) {
    bool payment = true;

    while (payment) {
        struct msr_monster *monster = gbl_game->player_data.player;
        werase(map_win->win);
        ui_print_reset(map_win);

        struct itm_item *money_item = inv_get_item_by_tid(monster->inventory, IID_MONEY);
        int money = 0;
        if (money_item != NULL) money = money_item->stacked_quantity;

        ui_printf_ext(map_win, map_win->lines -4, 1, cs_ATTR " [a]" cs_CLOSE " pay loan.");
        ui_printf_ext(map_win, map_win->lines -3, 1, cs_ATTR " [q]" cs_CLOSE " exit.");
        ui_printf_ext(map_win, 1, 1, "");

        ui_printf(map_win, "Loanshark:      (You have %lld gold)\n", money);
        ui_printf(map_win, "Credit: %d\n",  gbl_game->player_data.loan);
        ui_printf(map_win, "Payment: ");

        if (options.refresh) wrefresh(map_win->win);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_APPLY: {
                int pay = 0;
                ui_printf_ext(map_win, map_win->lines -5, 1, "Pay how much?");
                if (options.refresh) wrefresh(map_win->win);

                bool b = true;
                enum inp_keys key = 0;
                int amount = 0;
                while ( b && (key = inp_get_input_digit(gbl_game->input) ) ) {
                    switch (key) {
                        case '\n':
                        case 24: b = false; break;
                        case INP_KEY_BACKSPACE:
                            amount /= 10;
                            break;
                        default: {
                            amount = inp_input_to_digit(key);
                            if (amount >= 0) {
                                ui_printf(map_win, "%d",  amount);
                                pay *= 10;
                                pay += amount;
                            }
                        }
                    }
                }

                if (pay <= money && pay > 0) {
                    money_item->stacked_quantity -= pay;
                    You(monster, "payed the loanshark %d gp.", pay);
                }
                else {
                    You(monster, "do not have enough money to pay  %d", pay);
                }
            } break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: payment = false; break;
            default: break;
        }
    }
}

void quest_selection(int32_t randint) {
    struct msr_monster *monster = gbl_game->player_data.player;
    struct random *r = random_init_genrand(randint);
    bool quest = true;

    int32_t sz = (random_int32(r) % 3) + 2;
    uint32_t list[sz];
    for (int i = 0; i < sz; i++) {
        bool unique = false;

        for (int j = 0; unique == false && j < 20; j++) {
            struct quest *q = qst_spawn(gbl_game->player_data.level, random_int32(r) );
            if (q == NULL) continue;
            list[i] = q->tid;

            unique = true;
            if (list[i] == QSTID_NONE) unique = false;
            for (int k = 0; unique && k < i; k++) {
                if (list[i] == list[k]) unique = false;
            }
        }
        if (!unique) sz = i;
    }

    while (quest) {
        werase(map_win->win);
        ui_print_reset(map_win);

        ui_printf(map_win, "These are the available quests:\n");
        for (int i = 0; i < sz; i++) {
            struct quest *q = qst_by_tid(list[i]);
            assert(q != NULL);
            int y = ui_printf(map_win, cs_ATTR " %c) " cs_CLOSE "%s", inp_key_translate_idx(i), q->description);
            ui_printf_ext(map_win, y, 40, "%dgp", q->gp_reward);
            ui_printf_ext(map_win, y, 45, "%dxp\n", q->xp_reward);
        }

        ui_printf_ext(map_win, map_win->lines -4, 1, cs_ATTR " [a]" cs_CLOSE " accept quest.");
        ui_printf_ext(map_win, map_win->lines -3, 1, cs_ATTR " [q]" cs_CLOSE " exit.");

        if (options.refresh) wrefresh(map_win->win);

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_APPLY: {
                ui_printf_ext(map_win, map_win->lines -5, 1, "Acquire which quest?");
                if (options.refresh) wrefresh(map_win->win);

                int quest_idx = inp_get_input_idx(gbl_game->input);
                if (quest_idx < 0) break;
                if (quest_idx == INP_KEY_ESCAPE) break;
                if (quest_idx >= sz) break;

                gbl_game->player_data.quest = qst_by_tid(list[quest_idx]);
                You(monster, "selected '%s' as your next quest.\n", gbl_game->player_data.quest->description);
            } break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: quest = false; break;
            default: break;
        }
    }
    charwin_refresh();
}

void village_screen() {
    int32_t r_ws = random_int32(gbl_game->random);
    int32_t r_as = random_int32(gbl_game->random);
    int32_t r_ap = random_int32(gbl_game->random);
    int32_t r_qst = random_int32(gbl_game->random);

    bool watch = true;
    while(watch == true) {
        werase(map_win->win);

        ui_print_reset(map_win);

        ui_printf(map_win, "Village:\n");
        ui_printf(map_win, "\n");
        ui_printf(map_win, cs_ATTR " a)" cs_CLOSE " Weapon Smith\n");
        ui_printf(map_win, cs_ATTR " b)" cs_CLOSE " Armour Smith\n");
        ui_printf(map_win, cs_ATTR " c)" cs_CLOSE " Apothecary\n");
        ui_printf(map_win, cs_ATTR " d)" cs_CLOSE " Healer\n");
        ui_printf(map_win, cs_ATTR " e)" cs_CLOSE " Pay Loan\n");
        ui_printf(map_win, cs_ATTR " f)" cs_CLOSE " Train\n");
        ui_printf(map_win, cs_ATTR " g)" cs_CLOSE " Quests\n");
        ui_printf(map_win, cs_ATTR " h)" cs_CLOSE " Retire\n");
        ui_printf(map_win, "\n");

        ui_printf_ext(map_win, map_win->lines -4, 1, cs_ATTR " [a]" cs_CLOSE " travel to.");
        ui_printf_ext(map_win, map_win->lines -3, 1, cs_ATTR " [q]" cs_CLOSE " exit.");
        if (options.refresh) wrefresh(map_win->win);

        show_msg(msg_win);
        charwin_refresh();

        switch (inp_get_input(gbl_game->input) ) {
            case INP_KEY_APPLY:
                ui_printf_ext(map_win, map_win->lines-5, 1, "Travel were?");
                if (options.refresh) wrefresh(map_win->win);

                enum item_group ws_grplst[] = {
                    ITEM_GROUP_RANGED,
                    ITEM_GROUP_THROWING,
                    ITEM_GROUP_1H_MELEE,
                    ITEM_GROUP_2H_MELEE,
                    /*ITEM_GROUP_GUNPOWDER,*/
                };

                enum item_group as_grplst[] = {
                    ITEM_GROUP_ARMOUR,
                    /*ITEM_GROUP_SHIELD,*/
                };

                enum item_group ap_grplst[] = {
                    ITEM_GROUP_POTION,
                };

                int idx = inp_get_input_idx(gbl_game->input);
                switch (idx) {
                    case 0: vs_shop(r_ws, "Weapon Smith", ws_grplst, ARRAY_SZ(ws_grplst) ); break;
                    case 1: vs_shop(r_as, "Armour Smith", as_grplst, ARRAY_SZ(as_grplst) ); break;
                    case 2: vs_shop(r_ap, "Apothecary",   ap_grplst, ARRAY_SZ(ap_grplst) ); break;
                    case 3: vs_healer(); break;
                    case 4: pay_loan(); break;
                    case 5: levelup_selection_window(); break;
                    case 6: quest_selection(r_qst); break;
                    case 7: {
                            System_msg("Are you sure you want to retire? (o)k/(c)ancel");
                            switch (inp_get_input(gbl_game->input) ) {
                                default:
                                case INP_KEY_ESCAPE:
                                case INP_KEY_QUIT:
                                case INP_KEY_NO: break;
                                case INP_KEY_YES:
                                    watch = false;
                                    gbl_game->player_data.exit_map = true;
                                    gbl_game->player_data.retire = true;
                                    gbl_game->running = false;
                                break;
                            }
                        } break;
                    default: break;
                }
                break;
            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES: watch = false; break;
            default: break;
        }
    }

    werase(map_win->win);
    if (options.refresh) {
        wrefresh(map_win->win);
    }

    show_msg(msg_win);
}

