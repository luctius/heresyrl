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

#include "diffusion_limited_aggregation.h"
#include "logging.h"

struct dla_map {
    uint8_t *map;
    coord_t size;
};

static inline uint8_t *get_block(coord_t *p, struct dla_map *map) {
    return &map->map[(p->x * map->size.y) + p->y];
}

const coord_t dla_coord_lo_table[] = {
    {-1,  0}, {0, -1}, { 1, 0}, {0, 1}, /* ORTHOGANAL */
    {-1, -1}, {1, -1}, {-1, 1}, {1, 1}, /* DIAGONAL   */
};

static inline coord_t next_coord(coord_t *last, uint32_t dir, enum dla_directions dir_allowed) {
    int num_dirs = 0;
    if ( (dir_allowed & DLA_ORTHOGONAL) > 0) num_dirs += 4;
    if ( (dir_allowed & DLA_DIAGONAL)   > 0) num_dirs += 4;
    assert(num_dirs != 0);

    int rand_dir = dir % num_dirs;
    if ( ( (dir_allowed & DLA_ORTHOGONAL) == 0) &&
            ( (dir_allowed & DLA_DIAGONAL) > 0) ) {
        rand_dir += 4;
    }

    return cd_add(last, &dla_coord_lo_table[rand_dir]);
}

static inline bool has_neightbour(struct dla_map *map, coord_t *miner, enum dla_directions dir_allowed) {
    if (map == NULL) return false;
    if (cd_within_bound(miner, &map->size) == false) return false;

    for (int i = 0; i < 8; i++) {
        coord_t n = next_coord(miner, i, dir_allowed);
        if (*get_block(&n, map) == DLA_FLOOR) return true;
    }
    return false;
}

bool dla_generate(struct dla_map *map, struct random *r, int perc_floor, enum dla_directions dir_allowed) {
    if (map == NULL) return false;

    coord_t miner;

    /* Start mining in the middle */
    bool new_miner = false;
    miner.x = random_int32(r) % (map->size.x / 2) + (map->size.x / 4);
    miner.y = random_int32(r) % (map->size.y / 2) + (map->size.y / 4);

    int dug = 0; /* Number of blocks dug */
    int dug_target = ( (map->size.x * map->size.y) * perc_floor) / 100; /* Number of floor we have to dig */
    int tunnel = 0;

    while (dug < dug_target) {
        if (new_miner) {
            miner.x = (random_int32(r) % (map->size.x) );
            miner.y = (random_int32(r) % (map->size.y) );

            if (has_neightbour(map, &miner, dir_allowed) == true) {
                new_miner = false;
                tunnel = 0;
            }
        }

        int dir = random_int32(r);
        for (int i = 0; i < 4 && new_miner == false; i++) {
            coord_t next = next_coord(&miner, dir, dir_allowed);
            if (cd_within_bound(&next, &map->size) == true && tunnel <= map->size.x) {

                if (*get_block(&next, map) != DLA_FLOOR) {
                    miner = next;

                    uint8_t *block = get_block(&miner, map);
                    *block = DLA_FLOOR;
                    dug++;
                }
            }
            else new_miner = true;

            tunnel++;
        }
    }

    return true;
}

struct dla_map *dla_init(coord_t *size) {
    struct dla_map *map = malloc(sizeof(struct dla_map) );
    if (map == NULL) return NULL;

    map->size = *size;
    map->map = malloc(map->size.x * map->size.y * sizeof(uint8_t) );
    if (map->map == NULL) {
        free(map);
        return NULL;
    }

    memset(map->map, DLA_WALL, map->size.x * map->size.y * sizeof(uint8_t) );
    return map;
}

void dla_free(struct dla_map *map) {
    free(map->map);
    free(map);
}

bool dla_set_coord(struct dla_map *map, coord_t *point, enum dla_map_types type) {
    if (cd_within_bound(point, &map->size) == false) return false;
   
    uint8_t *block = get_block(point, map);
    *block = type;

    return true;
}

enum dla_map_types dla_get_coord(struct dla_map *map, coord_t *point) {
    if (cd_within_bound(point, &map->size) == false) return 0;

    uint8_t *block = get_block(point, map);
    return *block;
}

/* debugging */
void dla_print_map(struct dla_map *map) {
    coord_t c;
    for (c.y = 0; c.y < map->size.y; c.y++) {
        for (c.x = 0; c.x < map->size.x; c.x++) {
            if (dla_get_coord(map, &c)       == DLA_WALL )  putchar('#');
            else if (dla_get_coord(map, &c)  == DLA_FLOOR ) putchar('.');
            else putchar('?');
        }
        putchar('\n');
    }
    putchar('\n');
}
