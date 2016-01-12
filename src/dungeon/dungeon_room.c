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

#include "dungeon_room.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"

static coord_t room_size[] = {
    {4, 5},
    {5, 4},
    {5, 5},
    {5, 3},
    {3, 5},
    {6, 6},
    {7, 8},
    {6, 8},
    {8, 8},
};

bool dm_generate_map_room(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    if (dm_verify_map(map) == false) return false;
    FIX_UNUSED(r);
    FIX_UNUSED(type);

    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    coord_t r_ul = { .x = ul->x, .y = ul->y, };
    coord_t r_dr = { .x = 0, .y = 0, };

    int rand = random_int32(r) % ARRAY_SZ(room_size);
    int r_szx = room_size[rand].x;
    int r_szy = room_size[rand].y;

    if ( (r_ul.x + r_szx) >= sz_x-1) r_szx = sz_x - r_ul.x - 4;
    if ( (r_ul.y + r_szy) >= sz_y-1) r_szy = sz_y - r_ul.y - 4;

    r_dr.x = r_ul.x + r_szx;
    r_dr.y = r_ul.y + r_szy;

    coord_t c;
    for (c.x = r_ul.x; c.x < r_dr.x; c.x++) {
        for (c.y = r_ul.y; c.y < r_dr.y; c.y++) {
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
        }
    }
    return true;
}

