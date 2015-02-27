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
