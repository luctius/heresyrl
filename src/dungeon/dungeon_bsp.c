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

#define NEIGHBOUR_MAX 10
#define NEIGHBOUR_LEN_MIN 5
struct bsp_area {
    coord_t size;
    coord_t ul;
    int level;
    struct bsp_area *neighbours[NEIGHBOUR_MAX];
};

enum AXIS {
    X,
    Y,
};

static int area(coord_t *a) {
    return a->x * a->y;
}

bool dm_generate_map_bsp(struct dm_map *map, struct random *r, coord_t *ul, coord_t *dr) {
    int min_area = 20;
    int max_div_levels = 4;
    int alist_sz = 1;
    struct bsp_area area_list[ (max_div_levels * max_div_levels) +1];

    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };

    area_list[0].size = size;
    area_list[0].ul = *ul;
    area_list[0].level = 0;

    for (int l = 0; l < max_div_levels +1; l++) {
        int a =  a = (l*2) +1;
        int incr = 0;

        if (l == 0) a = 0;
        for (; a < alist_sz; a++) {
            struct bsp_area *ba = &area_list[a];
            lg_debug("splitting area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

            /* Choose division axis */
            int axis_r = random_int32(r) % 2;
            int *axis = (axis_r == X) ? &ba->size.x : &ba->size.y;
            if (*axis < NEIGHBOUR_LEN_MIN) continue;

            int range = *axis - (2 * NEIGHBOUR_LEN_MIN);
            int div_range = (random_int32(r) % range) + NEIGHBOUR_LEN_MIN;

            /* Add both areas to area list */
            ba = &area_list[a+1];
            ba->size = cd_create( (axis_r == X) ? div_range : ba->size.x,
                                             (axis_r == Y) ? div_range : ba->size.y);
            ba->ul   = cd_create( (axis_r == X) ? ba->ul.x + div_range : ba->ul.x,
                                             (axis_r == Y) ? ba->ul.y + div_range : ba->ul.y);
            ba->level = l +1;
            lg_debug("area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a + 1, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

            ba = &area_list[a+2];
            ba->size = cd_create( (axis_r == X) ? ba->size.x - div_range : ba->size.x,
                                             (axis_r == Y) ? ba->size.y - div_range : ba->size.y);
            ba->ul   = cd_create( (axis_r == X) ? ba->ul.x + (ba->size.x - div_range) : ba->ul.x,
                                             (axis_r == Y) ? ba->ul.y + (ba->size.y - div_range) : ba->ul.y);
            ba->level = l +1;
            lg_debug("area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a + 2, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

            /* Increase list */
            incr += 2;
        }
        alist_sz += incr;
    }

    for (int a = 0; a < alist_sz; a++) {
        struct bsp_area *ba = &area_list[a];

        coord_t c = cd_create(0,0);
        for (c.x = 0; c.x < ba->size.x; c.x++) {
            c.y = ba->ul.y;
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            c.y = ba->ul.y + ba->size.y;
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
        }

        c = cd_create(0,0);
        for (c.y = 0; c.y < ba->size.y; c.y++) {
            c.x = ba->ul.x;
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            c.x = ba->ul.x + ba->size.x;
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
        }
    }
}
