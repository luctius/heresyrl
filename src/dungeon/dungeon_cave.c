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

#include "cellular_automata.h"
#include "dungeon_cave.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"

static enum cellular_automata randpick(struct random *r, int fillprob) {
    if(random_d100(r) < fillprob) {
        return CA_ALIVE;
    }
    return CA_DEAD;
}

bool cave_generate_map(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    FIX_UNUSED(type);

    /* initialise cellular automata */
    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };
    struct ca_map *cmap = ca_init(&size);

    /* fill the map randomly with floors */
    coord_t p;
    for (p.y = 0; p.y < size.y; p.y++) {
        for (p.x = 0; p.x < size.x; p.x++) {
            ca_set_coord(cmap, &p, randpick(r, 45) );
        }
    }

    /* Fill the map with large caves */
    for (int  i = 0; i < 4; i++) {
        ca_generation(cmap, 16, 8, 2);
    }

    /* Do a few passes to make them less smooth */
    for (int  i = 0; i < 2; i++) {
        int ri = (random_int32(r) % 4) +2;
        ca_generation(cmap, 5, ri, 1);
    }

    /* translate the ca_map to the real map */
    for(p.y = 0; p.y < size.y; p.y++) {
        for(p.x = 0; p.x < size.x; p.x++) {
            coord_t c = cd_add(ul, &p);

            /* check if the cell is alive or dead */
            if (ca_get_coord(cmap, &p) == CA_ALIVE) {
                /* fill the tile with the specified type */
                if(random_int32(r)%100 < 1) {
                    dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_MAD_CAP_FUNGUS);
                }
                else dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            }
        }
    }

    /* cleanup and return */
    ca_free(cmap);
    return true;
}
