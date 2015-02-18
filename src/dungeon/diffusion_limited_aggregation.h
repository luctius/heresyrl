#ifndef DIFFUSION_LIMITED_AGGREGATION_H
#define DIFFUSION_LIMITED_AGGREGATION_H

#include "coord.h"
#include "random.h"

enum dla_directions {
    DLA_ORTHOGONAL = (1<<0),
    DLA_DIAGONAL   = (2<<0),
};

enum dla_map_types {
    DLA_WALL  = 0,
    DLA_FLOOR = 1,
};

struct dla_map;

struct dla_map *dla_init(coord_t *size);
void dla_free(struct dla_map *map);
bool dla_set_coord(struct dla_map *map, coord_t *point, enum dla_map_types type);
enum dla_map_types dla_get_coord(struct dla_map *map, coord_t *point);

bool dla_generate(struct dla_map *map, struct random *r, int perc_floor, enum dla_directions dir_allowed);

/* debugging */
void dla_print_map(struct dla_map *map);

#endif /* DIFFUSION_LIMITED_AGGREGATION_H */
