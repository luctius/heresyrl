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

bool dm_generate_map_plain(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    int no_rooms = (random_int32(r) % 60) +60;

    for (int x = 0; x < sz_x; x++) {
        for (int y = 0; y < sz_y; y++) {
            coord_t abs = cd_create(x + ul->x, y +ul->y);
            dm_get_map_me(&abs,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
        }
    }

    for (int i = 0; i < no_rooms; i++) {
        coord_t r_ul = { .x = random_int32(r) % (sz_x -10), .y = random_int32(r) % (sz_y - 10), };
        coord_t r_dr = { .x = 0, .y = 0, };

        if (random_d100(r) > 5) {
            int r_szx = (random_int32(r) % 10) +5;
            int r_szy = (random_int32(r) % 10) +5;

            if ( (r_ul.x + r_szx) >= sz_x-1) r_szx = sz_x - r_ul.x - 2;
            if ( (r_ul.y + r_szy) >= sz_y-1) r_szy = sz_y - r_ul.y - 2;
            
            r_dr.x = r_ul.x + r_szx;
            r_dr.y = r_ul.y + r_szy;

            dm_generate_map_room(map, r, type, &r_ul, &r_dr);
        }
        else {
            i += 4;
            int r_szx = (random_int32(r) % 30) +30;
            int r_szy = (random_int32(r) % 30) +30;

            if ( (r_ul.x + r_szx) >= sz_x-1) r_szx = sz_x - r_ul.x - 2;
            if ( (r_ul.y + r_szy) >= sz_y-1) r_szy = sz_y - r_ul.y - 2;
            
            r_dr.x = r_ul.x + r_szx;
            r_dr.y = r_ul.y + r_szy;

            cave_generate_map(map, r, type, &r_ul, &r_dr);
        }
    }
}

