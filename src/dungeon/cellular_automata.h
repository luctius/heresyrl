#ifndef CELLULAR_AUTOMA_H
#define CELLULAR_AUTOMA_H

#include "coord.h"

enum cellular_automata {
    CA_DEAD  = 0,
    CA_ALIVE = 1,
    CA_MASK  = 0x1,

    CA_TOGGLE = 0x80,
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
