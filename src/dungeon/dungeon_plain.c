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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon_plain.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"

#include "dungeon_room.h"
#include "dungeon_cave.h"
#include "dungeon_dla.h"

static inline int tiles_used(struct dm_map *map, coord_t *ul, coord_t *dr) {
    int total = 0;

    coord_t p;
    for(p.y = ul->y; p.y < dr->y; p.y++) {
        for(p.x = ul->x; p.x < dr->x; p.x++) {
            coord_t c = cd_add(ul, &p);

            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) ) {
                total++;
            }
        }
    }
    return total;
}

bool dm_generate_map_plain(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    int dug_target = (40 * (sz_x * sz_y) ) / 100;
    int dug = 0;

    for (int i = 0; i < 300 && dug < dug_target; i ++) {
        coord_t r_ul = { .x = (random_int32(r) % (sz_x -5) ) +ul->x, .y = (random_int32(r) % (sz_y - 5) ) +ul->y, };

        int r_szx = (random_int32(r) % 50) +20;
        int r_szy = (random_int32(r) % 50) +20;

        if ( (r_ul.x + r_szx) >= sz_x-1) r_szx = sz_x - r_ul.x - 4;
        if ( (r_ul.y + r_szy) >= sz_y-1) r_szy = sz_y - r_ul.y - 4;

        coord_t r_dr = { .x = r_ul.x + r_szx, .y = r_ul.y + r_szy, };
        if (r_ul.x >= r_dr.x) continue;
        if (r_ul.y >= r_dr.y) continue;

        dm_generate_map_room(map, r, type, &r_ul, &r_dr);
        dug = tiles_used(map, ul, dr);
    };
    return true;
}

