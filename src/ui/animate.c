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

#include <unistd.h>

#include "heresyrl_def.h"
#include "animate.h"
#include "ui_common.h"
#include "options.h"
#include "dungeon/dungeon_map.h"

void ui_animate_explosion(struct dm_map *map, coord_t path[], int path_len) {
    if (gbl_game == NULL) return;
    if (gbl_game->player_data.player == NULL) return;
    if (options.refresh == false) return;

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

    chtype chlist[path_len];

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            chlist[i] = mvwinch(map_win->win, path[i].y - scr_y, path[i].x - scr_x);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, (chlist[i] & (A_CHARTEXT) ) | get_colour(TERM_COLOUR_BG_YELLOW) );
        }
    }

    wrefresh(map_win->win);
    usleep(50000);

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, chlist[i] | get_colour(TERM_COLOUR_BG_RED) );
        }
    }

    if (options.refresh) {
        wrefresh(map_win->win);
        usleep(50000);
    }

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, chlist[i]);
        }
    }
}

void ui_animate_projectile(struct dm_map *map, coord_t path[], int path_len) {
    if (gbl_game == NULL) return;
    if (gbl_game->player_data.player == NULL) return;
    if (options.refresh == false) return;

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->sett.size.y);

    for (int i = 1; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            chtype oldch = mvwinch(map_win->win, path[i].y - scr_y, path[i].x - scr_x);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            if (options.refresh) wrefresh(map_win->win);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, oldch);
            if (options.refresh) usleep(20000);
        }
    }
}

