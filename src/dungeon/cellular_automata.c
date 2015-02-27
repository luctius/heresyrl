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

#include "cellular_automata.h"
#include "logging.h"

struct ca_map {
    uint8_t *map;
    coord_t size;
};

static inline uint8_t *get_cell(coord_t *p, struct ca_map *map) {
    return &map->map[(p->x * map->size.y) + p->y];
}

bool ca_generation(struct ca_map *map, uint8_t birth_sum, uint8_t surv_sum, int radius) {
    /* 
       We do a two pass over the grid.
       The first pass check if a cell should life or 
       die and if so, we toggle the last bit.

       The second pass performs the action.
       This is to ensure that we do not interfere 
       with the calculations of the first pass.
    */


    coord_t p;
    for (p.y = 0; p.y < map->size.y; p.y++) {
        for (p.x = 0; p.x < map->size.x; p.x++) {
            int sum = ca_get_coord_sum(map, &p, radius);

            uint8_t *cell = get_cell(&p, map);
            if ( (*cell & CA_MASK) == CA_ALIVE && sum < surv_sum) {
                *cell |= CA_TOGGLE; /* Die */
            }
            else if ( (*cell & CA_MASK) == CA_DEAD && sum >= birth_sum) {
                *cell |= CA_TOGGLE; /* Born */
            }
        }
    }

    for (p.y = 0; p.y < map->size.y; p.y++) {
        for (p.x = 0; p.x < map->size.x; p.x++) {
            uint8_t *cell = get_cell(&p, map);
            if (*cell >= CA_TOGGLE) {
                if ( (*cell & CA_MASK) == CA_ALIVE) {
                    *cell = CA_DEAD;
                }
                else if ( (*cell & CA_MASK) == CA_DEAD) {
                    *cell = CA_ALIVE;
                }
                else if ( (*cell & CA_MASK) != CA_OBSTACLE) assert(false);
            }
        }
    }

    return true;
}


struct ca_map *ca_init(coord_t *size) {
    struct ca_map *map = malloc(sizeof(struct ca_map) );
    if (map == NULL) return NULL;

    map->size = *size;
    map->map = malloc(map->size.x * map->size.y * sizeof(uint8_t) );
    if (map->map == NULL) {
        free(map);
        return NULL;
    }

    memset(map->map, CA_DEAD, map->size.x * map->size.y * sizeof(uint8_t) );
    return map;
}

void ca_free(struct ca_map *map) {
    free(map->map);
    free(map);
}

bool ca_set_coord(struct ca_map *map, coord_t *point, enum cellular_automata val) {
    if (cd_within_bound(point, &map->size) == false) return false;
    
    uint8_t *cell = get_cell(point, map);
    *cell = val;

    return true;
}

enum cellular_automata ca_get_coord(struct ca_map *map, coord_t *point) {
    if (cd_within_bound(point, &map->size) == false) return 0;

    uint8_t *cell = get_cell(point, map);
    return *cell & CA_MASK;
}

int ca_get_coord_sum(struct ca_map *map, coord_t *point, int radius) {
    if (cd_within_bound(point, &map->size) == false) return -1;

    int sum = 0;

    coord_t p;
    for (p.y = -radius; p.y <= radius; p.y++) {
        for (p.x = -radius; p.x <= radius; p.x++) {
            if (p.x == 0 && p.y == 0) continue;

            coord_t test_p = cd_add(point, &p);
            if (cd_within_bound(&test_p, &map->size) ) {
                uint8_t *cell = get_cell(&test_p, map);
                if ( (*cell & CA_MASK) == CA_ALIVE) sum++;
            }
        }
    }

    return sum;
}

void ca_print_map(struct ca_map *map) {
    coord_t c;
    for (c.y = 0; c.y < map->size.y; c.y++) {
        for (c.x = 0; c.x < map->size.x; c.x++) {
            if (ca_get_coord(map, &c)       == CA_ALIVE )   putchar('.');
            else if (ca_get_coord(map, &c)  == CA_DEAD )    putchar(' ');
            else if (ca_get_coord(map, &c)  == CA_OBSTACLE) putchar('#');
            else putchar('?');
        }
        putchar('\n');
    }
    putchar('\n');
}

