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
    {9, 9},
    {10, 10},
    {11, 11},
    {12, 12},
    {12, 11},
    {12, 10},
    {12, 9},
    {12, 8},
    {8, 9},
    {8, 10},
    {8, 11},
    {8, 12},
};

static struct dungeon_features_done features = {
    .loops          = false,
    .lights         = false,
    .features       = false,
    .reachability   = false,
};

struct dungeon_features_done *dm_generate_map_room(struct dm_map *map, struct random *r, coord_t *ul, coord_t *dr) {
    if (dm_verify_map(map) == false) return false;
    if (ul->x >= dr->x) return false;
    if (ul->y >= dr->y) return false;
    FIX_UNUSED(r);

    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    coord_t r_ul = { .x = ul->x, .y = ul->y, };
    coord_t r_dr = { .x = 0, .y = 0, };

    int rand = random_int32(r) % ARRAY_SZ(room_size);
    int r_szx = room_size[rand].x;
    int r_szy = room_size[rand].y;

    if ( (r_ul.x + r_szx) > sz_x) r_szx = sz_x - r_ul.x;
    if ( (r_ul.y + r_szy) > sz_y) r_szy = sz_y - r_ul.y;

    assert(sz_x > 0);
    assert(sz_y > 0);

    r_dr.x = r_ul.x + r_szx;
    r_dr.y = r_ul.y + r_szy;

    coord_t c;
    coord_t d;
    for (c.x = r_ul.x+1; c.x < r_dr.x-1; c.x++) {
        for (c.y = r_ul.y+1; c.y < r_dr.y-1; c.y++) {
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
        }
    }

    /* Create an non-destructable border wall around the 4 sides of the map */
    c.y = r_ul.y;
    d.y = r_dr.y;
    for (c.x = r_ul.x; c.x < r_dr.x; c.x++) {
        d.x = c.x;
        dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
        dm_get_map_me(&d,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
    }

    c.x = r_ul.x;
    d.x = r_dr.x;
    for (c.y = r_ul.y; c.y < r_dr.y; c.y++) {
        d.y = c.y;
        dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
        dm_get_map_me(&d,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
    }

    return &features;
}

