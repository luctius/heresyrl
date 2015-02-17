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
    if(random_int32(r)%100 < fillprob) {
        return CA_ALIVE;
    }
    return CA_DEAD;
}
 /*
static void generation(void) {
    int xi, yi, ii, jj;

    for(yi=1; yi<size_y-1; yi++) {
        for(xi=1; xi<size_x-1; xi++) {
            int adjcount_r1 = 0,
            adjcount_r2 = 0;

            for(ii=-1; ii<=1; ii++) {
                for(jj=-1; jj<=1; jj++) {
                    if(grid[yi+ii][xi+jj] != TILE_ID_CONCRETE_FLOOR)
                        adjcount_r1++;
                }
            }

            for(ii=yi-2; ii<=yi+2; ii++) {
                for(jj=xi-2; jj<=xi+2; jj++) {
                    if(abs(ii-yi)==2 && abs(jj-xi)==2)
                        continue;
                    if(ii<0 || jj<0 || ii>=size_y || jj>=size_x)
                        continue;
                    if(grid[ii][jj] != TILE_ID_CONCRETE_FLOOR)
                        adjcount_r2++;
                }
            }
            if(adjcount_r1 >= params->r1_cutoff || adjcount_r2 <= params->r2_cutoff) {
                grid2[yi][xi] = TILE_ID_CONCRETE_WALL;
            }
            else {
                grid2[yi][xi] = TILE_ID_CONCRETE_FLOOR;
            }
        }
    }

    for(yi=1; yi<size_y-1; yi++) {
        for(xi=1; xi<size_x-1; xi++) {
            grid[yi][xi] = grid2[yi][xi];
        }
    }
}
*/

bool cave_generate_map(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    FIX_UNUSED(type);

    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };
    struct ca_map *cmap = ca_init(&size);

    coord_t p;
    for (p.y = 0; p.y < size.y; p.y++) {
        for (p.x = 0; p.x < size.x; p.x++) {
            ca_set_coord(cmap, &p, randpick(r, 45) );
        }
    }
    ca_print_map(cmap);

    for (int  i = 0; i < 4; i++) {
        ca_round(cmap, 8, 7, 2);
    }
    ca_print_map(cmap);

    for (int  i = 0; i < 2; i++) {
        ca_round(cmap, 4, 3, 1);
    }
    ca_print_map(cmap);


    for(p.y = 0; p.y < size.y; p.y++) {
        for(p.x = 0; p.x < size.x; p.x++) {
            coord_t c = cd_add(ul, &p);

            enum tile_ids tid;
            if (ca_get_coord(cmap, &c) == CA_ALIVE) tid = TILE_ID_CONCRETE_FLOOR;
            else tid = TILE_ID_CONCRETE_WALL;

            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(tid);
        }
    }

    ca_free(cmap);
    return true;
}
