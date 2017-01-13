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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "dungeon_bsp.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "logging.h"
#include "coord.h"
#include "random.h"

enum AXIS {
    X,
    Y,
    AXIS_MAX,
};

struct fallen_wall {
    coord_t a;
    coord_t b;
};

#define NEIGHBOUR_MAX 12
#define NEIGHBOUR_LEN_MIN 10
struct bsp_area {
    coord_t size;
    coord_t ul;
    int level;
    bool split;
    struct bsp_area *neighbours[NEIGHBOUR_MAX];
    int nr_nghbrs;
    struct fallen_wall walls[NEIGHBOUR_MAX];
    int nr_walls;
    int extra_cost;
};

static int area(coord_t *a) {
    return a->x * a->y;
}
// at least 2 cells shared by both areas
bool testLines(int p1, int p2, int s1, int s2) {
    if ( ((s1 >= p1) && (p2-s1>2)) || ((s1 <= p1) && (s2-p1>2))) {
        return true;
    }
    return false;
}

static void add_nghbr(struct bsp_area *a, struct bsp_area *b) {
    lg_debug("\t\tneighbours: (%d,%d,%d,%d) & (%d,%d,%d,%d)", a->ul.x, a->ul.y, a->ul.x + a->size.x, a->ul.y + a->size.y, b->ul.x, b->ul.y, b->ul.x + b->size.x, b->ul.y + b->size.y);
    if (a->nr_nghbrs < NEIGHBOUR_MAX) {
        a->neighbours[a->nr_nghbrs++] = b;
    }
    if (b->nr_nghbrs < NEIGHBOUR_MAX) {
        b->neighbours[b->nr_nghbrs++] = a;
    }
}

bool dm_generate_map_bsp(struct dm_map *map, struct random *random, coord_t *ul, coord_t *dr) {
    int max_div_levels = 7 + (random_int32(random) %5);
    int alist_sz = 1;
    struct bsp_area area_list[ (max_div_levels * max_div_levels) +1];
    memset(area_list, 0x0, sizeof(area_list));

    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };

    area_list[0].size = size;
    area_list[0].ul = *ul;
    area_list[0].level = 0;
    area_list[0].split = false;
    area_list[0].nr_nghbrs = 0;
    area_list[0].nr_walls = 0;
    area_list[0].extra_cost = 0;

    for (int l = 0; l < max_div_levels; l++) {
        int incr = 0;
        for (int a = 0; a < alist_sz && a < ARRAY_SZ(area_list); a++) {
            if (alist_sz +incr +2 >= ARRAY_SZ(area_list) ) break;

            struct bsp_area *ba = &area_list[a];
            if (ba->split == true) continue;
            //if (ba->size.x == 0) continue;
            lg_debug("splitting area[%d: l %d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, l, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

            /* Choose division axis */
            int axis_r = random_int32(random) % AXIS_MAX;
            int *axis = (axis_r == X) ? &ba->size.x : &ba->size.y;
            lg_debug("\taxis check area[%d, a %d]: (sz %d)", a, axis_r, *axis);
            if (*axis <= (2 * NEIGHBOUR_LEN_MIN) ) {
                axis_r = (axis_r == X) ? Y : X;
                axis = (axis_r == X) ? &ba->size.x : &ba->size.y;
                lg_debug("\taxis check area[%d, a %d]: (sz %d)", a, axis_r, *axis);

                if (*axis <= (2 * NEIGHBOUR_LEN_MIN) ) {
                    lg_debug("\t--> error: too small");
                    continue;
                }
            }
            int range = *axis - (2 * NEIGHBOUR_LEN_MIN);
            int div_range = (random_int32(random) % range) + NEIGHBOUR_LEN_MIN;
            lg_debug("\t\taxis[%d -> %d] div_range %d", axis_r, *axis, div_range);

            /* Add both areas to area list */
            struct bsp_area *ba1 = &area_list[alist_sz +incr];
            ba1->size = cd_create( (axis_r == X) ? div_range : ba->size.x,
                                   (axis_r == Y) ? div_range : ba->size.y);
            ba1->ul   = cd_create(ba->ul.x, ba->ul.y);
            ba1->level = l +1;
            ba1->split = false;
            ba1->nr_nghbrs = 0;
            ba1->nr_walls = 0;
            ba1->extra_cost = 0;
            lg_debug("\t\tarea[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", alist_sz +incr, ba1->size.x, ba1->size.y, ba1->ul.x, ba1->ul.y, ba1->level);

            struct bsp_area *ba2 = &area_list[alist_sz +incr +1];
            ba2->size = cd_create( (axis_r == X) ? ba->size.x - div_range : ba->size.x,
                                   (axis_r == Y) ? ba->size.y - div_range : ba->size.y);
            ba2->ul   = cd_create( (axis_r == X) ? (ba->ul.x + div_range) : ba->ul.x,
                                   (axis_r == Y) ? (ba->ul.y + div_range) : ba->ul.y );
            ba2->level = l +1;
            ba2->split = false;
            ba2->nr_nghbrs = 0;
            ba2->nr_walls = 0;
            ba2->extra_cost = 0;
            lg_debug("\t\tarea[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", alist_sz +incr +1, ba2->size.x, ba2->size.y, ba2->ul.x, ba2->ul.y, ba2->level);

            /* Increase list */
            ba->split = true;
            incr += 2;
        }
        alist_sz += incr;
    }

    /* create neighbour lists */
    for (int a = 1; a < alist_sz; a++) {
        for (int b = 1; b < alist_sz; b++) {
            if (a == b) continue;
            struct bsp_area *bs_a = &area_list[a];
            struct bsp_area *bs_b = &area_list[b];
            if (bs_a->split == true) continue;
            if (bs_b->split == true) continue;

            bool found = false;
            if (bs_a->ul.x == bs_b->ul.x) {
                found |= testLines(bs_a->ul.y, bs_a->ul.y + bs_a->size.y, bs_b->ul.y, bs_b->ul.y + bs_b->size.y);
            }
            else if (bs_a->ul.y == bs_b->ul.y) {
                found |= testLines(bs_a->ul.y, bs_a->ul.y + bs_a->size.y, bs_b->ul.y, bs_b->ul.y + bs_b->size.y);
            }
            else if (bs_a->ul.x +bs_a->size.x == bs_b->ul.x +bs_b->size.x) {
                found |= testLines(bs_a->ul.x, bs_a->ul.x + bs_a->size.x, bs_b->ul.x, bs_b->ul.x + bs_b->size.y);
            }
            else if (bs_a->ul.y +bs_a->size.y == bs_b->ul.y +bs_b->size.y) {
                found |= testLines(bs_a->ul.x, bs_a->ul.x + bs_a->size.x, bs_b->ul.x, bs_b->ul.x + bs_b->size.x);
            }

            if (found) {
                add_nghbr(bs_a, bs_b);
            }
        }
    }

    /* place influence points */
    int nr_influence_points = random_int32(random) % 5;
    coord_t points[nr_influence_points];
    for (int i = 0; i < nr_influence_points; i++) {
        points[i] = cd_create(random_int32(random) % size.x, 
                              random_int32(random) % size.y);
    }

    /* calculate influence */
    /*
     * find area which contains the influence,
     *  for every neighbour, add cost
     *    for ever neighbour neighbour add cost -1
     *      ...
     */
    for (int a = 1; a < alist_sz; a++) {
        for (int b = 1; b < alist_sz; b++) {
            if (a == b) continue;
            struct bsp_area *bs_a = &area_list[a];
            struct bsp_area *bs_b = &area_list[b];
            if (bs_a->split == true) continue;
            if (bs_b->split == true) continue;
        }
    }

    /* pathfinder */
    /*
     * - Passing through walls cost extra
     * - take influence of room into accout
     * - play route, and note which walls not to place.
     */


    /* paint areas */
    for (int a = 1; a < alist_sz; a++) {
        struct bsp_area *ba = &area_list[a];
        if (ba->split == true) continue;
        lg_debug("painting area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

        coord_t c = cd_create(0,0);
        for (c.x = ba->ul.x +1; c.x < ba->size.x + ba->ul.x; c.x++) {
            for (c.y = ba->ul.y +1; c.y < ba->size.y + ba->ul.y; c.y++) {
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            }
        }
    }

    return true;
}
