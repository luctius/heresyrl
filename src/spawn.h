#ifndef SPAWN_H
#define SPAWN_H

#include <stdint.h>
#include "heresyrl_def.h"

bool spwn_populate_map(struct dc_map *map, struct random *r, uint32_t monster_chance, uint32_t item_chance);

#endif /* SPAWN_H */
