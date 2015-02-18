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

bool dm_generate_map_plain(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    int sz_x = dr->x - ul->x;
    int sz_y = dr->y - ul->y;

    int no_rooms = (random_int32(r) % 60) +60;

    for (int i = 0; i < no_rooms; i++) {
        coord_t r_ul = { .x = (random_int32(r) % (sz_x -30) ) +ul->x, .y = (random_int32(r) % (sz_y - 30) ) +ul->y, };
        coord_t r_dr = { .x = 0, .y = 0, };

        int r_szx = (random_int32(r) % 50) +10;
        int r_szy = (random_int32(r) % 50) +10;

        if ( (r_ul.x + r_szx) >= sz_x-1) r_szx = sz_x - r_ul.x - 4;
        if ( (r_ul.y + r_szy) >= sz_y-1) r_szy = sz_y - r_ul.y - 4;
        
        r_dr.x = r_ul.x + r_szx;
        r_dr.y = r_ul.y + r_szy;

        bool obstructed = false;
        coord_t c;
        for (c.y = r_ul.y; c.y < r_dr.y && obstructed == false; c.y++) {
            for (c.x = r_ul.x; c.x < r_dr.x && obstructed == false; c.x++) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) ) {
                    obstructed = true;
                }
            }
        }

        if (obstructed) continue;

        int rand = random_d100(r);
        if (rand > 75) {
            dm_generate_map_room(map, r, type, &r_ul, &r_dr);
        }
        else if (random_d100(r) > 40) {
            dm_generate_map_dla(map, r, type, &r_ul, &r_dr);
        }
        else {
            cave_generate_map(map, r, type, &r_ul, &r_dr);
        }
    }
    return true;
}

