#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
 
#include "dungeon_room.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"


bool dm_generate_map_room(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    if (dm_verify_map(map) == false) return false;
    FIX_UNUSED(r);
    FIX_UNUSED(type);

    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    coord_t c;
    for (c.x = 1; c.x < sz_x-1; c.x++) {
        for (c.y = 1; c.y < sz_y-1; c.y++) {
            coord_t abs = cd_create(c.x + ul->x, c.y +ul->y);
            dm_get_map_me(&abs,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
        }
    }
    return true;
}

