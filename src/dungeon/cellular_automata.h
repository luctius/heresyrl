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

#ifndef CELLULAR_AUTOMA_H
#define CELLULAR_AUTOMA_H

#include "coord.h"

enum cellular_automata {
    CA_DEAD     = 0,
    CA_ALIVE    = 1,
    CA_OBSTACLE = 2,

    CA_MASK     = 0x3,
    CA_TOGGLE   = 0x80,
};

struct ca_map;

struct ca_map *ca_init(coord_t *size);
void ca_free(struct ca_map *map);
bool ca_set_coord(struct ca_map *map, coord_t *point, enum cellular_automata val);
enum cellular_automata ca_get_coord(struct ca_map *map, coord_t *point);

/* Get the sum of all the cell in the radius around point, *excluding* point itself. */
int ca_get_coord_sum(struct ca_map *map, coord_t *point, int radius);

bool ca_generation(struct ca_map *map, uint8_t birth_sum, uint8_t surv_sum, int radius);

/* debugging */
void ca_print_map(struct ca_map *map);

#endif /* CELLULAR_AUTOMA_H */
