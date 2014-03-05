#include <math.h>
#include <sys/param.h>
#include "fight.h"
#include "heresyrl_def.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "items.h"
#include "tiles.h"

int fght_shoot(struct msr_monster *monster, struct dc_map *map, enum fght_weapon_selection sel, enum fght_weapon_setting set, coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    if (monster == NULL) return -1;
    if (map == NULL) return -1;
    if (s == NULL) return -1;
    if (e == NULL) return -1;
    if (path_list == NULL) return -1;
    if (sel >= FGHT_WEAPON_SELECT_MAX) return -1;
    if (set >= FGHT_WEAPON_SETTING_MAX) return -1;

    coord_t path[MAX(map->size.x, map->size.y)];
    int path_len = fght_calc_lof_path(s, e, path, ARRAY_SZ(path));
    bool blocked = false;
    int unblocked_length = 0;

    for (int i = 1; (i < path_len) && (blocked == false); i++) {
        if ( (sd_get_map_me(&path[i], map)->monster != NULL) || 
                (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&path[i], map), TILE_ATTR_TRAVERSABLE) == false) ) {
            blocked = true;
            /* Do damage */
        } else if (sd_get_map_me(&path[i], map)->visible == true) {
            if (i-1 < path_list_sz) {
                path_list[i-1] = path[i];
                unblocked_length++;
            }
        }
    }
    return unblocked_length;
}

/* Adapted from the code displayed at RogueBasin's "Bresenham's Line
 * Algorithm" article, this function checks for an unobstructed line
 * of sight between two locations using Bresenham's line algorithm to
 * draw a line from one point to the other. Returns true if there is
 * line of sight, false if there is no line of sight. */
/*int los (int los_x_1, int los_y_1, int los_x_2, int
         los_y_2, int level) {*/
int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    int los_x_1 = s->x;
    int los_y_1 = s->y;
    int los_x_2 = e->x;
    int los_y_2 = e->y;
    int delta_x, delta_y, move_x, move_y, error;
    int pl_counter = 0;

    /* Calculate deltas. */
    delta_x = abs (los_x_2 - los_x_1) << 1;
    delta_y = abs (los_y_2 - los_y_1) << 1;

    /* Calculate signs. */
    move_x = los_x_2 >= los_x_1 ? 1 : -1;
    move_y = los_y_2 >= los_y_1 ? 1 : -1;

    /* There is an automatic line of sight, of course, between a
    * location and the same location or directly adjacent
    * locations. */
    if (abs (los_x_2 - los_x_1) < 2 && abs (los_y_2 - los_y_1) < 2) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return 0;
    }

    /* Ensure that the line will not extend too long. */
    if (((los_x_2 - los_x_1) * (los_x_2 - los_x_1))
    + ((los_y_2 - los_y_1) * (los_y_2 -
                             los_y_1)) > path_list_sz) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return -1;
    }

    /* "Draw" the line, checking for obstructions. */
    if (delta_x >= delta_y) {
        /* Calculate the error factor, which may go below zero. */
        error = delta_y - (delta_x >> 1);

        /* Search the line. */
        //while (los_x_1 != los_x_2) {
        while (pl_counter < path_list_sz) {
             /* Check for an obstruction. If the obstruction can be "moved
              * around", it isn't really an obstruction. */
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "%d:%d => %d:%d", los_x_1, los_y_1, los_x_2, los_y_2);
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            //if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_x > 0)) {
                    los_y_1 += move_y;
                    error -= delta_x;
                }
            }
            los_x_1 += move_x;
            error += delta_y;
        }
    }
    else {
        /* Calculate the error factor, which may go below zero. */
        error = delta_x - (delta_y >> 1);

        /* Search the line. */
        //while (los_y_1 != los_y_2) {
        while (pl_counter < path_list_sz) {
            /* Check for an obstruction. If the obstruction can be "moved
            * around", it isn't really an obstruction. */
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "%d:%d => %d:%d", los_x_1, los_y_1, los_x_2, los_y_2);
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_y > 0)) {
                    los_x_1 += move_x;
                    error -= delta_y;
                }
            }
            los_y_1 += move_y;
            error += delta_x;
        }
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "%d:%d => %d:%d", los_x_2, los_y_2, los_x_2, los_y_2);
    path_list[pl_counter].x = los_x_2;
    path_list[pl_counter].y = los_y_2;
    pl_counter++;

    /* Return. */
    return pl_counter;
}
