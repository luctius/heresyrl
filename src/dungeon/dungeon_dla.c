#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
 
#include "diffusion_limited_aggregation.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"

bool dm_generate_map_dla(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    FIX_UNUSED(type);

    /* initialise cellular automata */
    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };
    struct dla_map *dlamap = dla_init(&size);

    /* Generate the map */
    dla_generate(dlamap, r, 15, DLA_ORTHOGONAL);

    /* translate the dla_map to the real map */
    coord_t p;
    for(p.y = 0; p.y < size.y; p.y++) {
        for(p.x = 0; p.x < size.x; p.x++) {

            /* tranlate point to dm_map coordinations */
            coord_t c = cd_add(ul, &p);

            /* check if the block is a floor or wall */
            if (dla_get_coord(dlamap, &p) == DLA_FLOOR) {
                /* fill the tile with the specified type */
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            }
        }
    }

    /* cleanup and return */
    dla_free(dlamap);
    return true;
}

