#include <unistd.h>

#include "heresyrl_def.h"
#include "animate.h"
#include "ui_common.h"
#include "dungeon/dungeon_map.h"

void ui_animate_explosion(struct dm_map *map, coord_t path[], int path_len) {
    if (gbl_game == NULL) return;
    if (gbl_game->player_data.player == NULL) return;

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    chtype chlist[path_len];

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            chlist[i] = mvwinch(map_win->win, path[i].y - scr_y, path[i].x - scr_x);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, chlist[i] | get_colour(TERM_COLOUR_BG_YELLOW) );
        }
    }

    wrefresh(map_win->win);
    usleep(50000);

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, chlist[i] | get_colour(TERM_COLOUR_BG_RED) );
        }
    }

    wrefresh(map_win->win);
    usleep(50000);

    for (int i = 0; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, chlist[i]);
        }
    }
}

void ui_animate_projectile(struct dm_map *map, coord_t path[], int path_len) {
    if (gbl_game == NULL) return;
    if (gbl_game->player_data.player == NULL) return;

    int scr_x = get_viewport(last_ppos.x, map_win->cols, map->size.x);
    int scr_y = get_viewport(last_ppos.y, map_win->lines, map->size.y);

    for (int i = 1; i < path_len; i++) {
        if (dm_get_map_me(&path[i],map)->visible == true) {
            chtype oldch = mvwinch(map_win->win, path[i].y - scr_y, path[i].x - scr_x);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, '*' | get_colour(TERM_COLOUR_RED) );
            wrefresh(map_win->win);
            mvwaddch(map_win->win, path[i].y - scr_y, path[i].x - scr_x, oldch);
            usleep(20000);
        }
    }
}

