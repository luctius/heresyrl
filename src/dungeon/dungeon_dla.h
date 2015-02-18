#ifndef DUNGEON_DLA_H
#define DUNGEON_DLA_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

bool dm_generate_map_dla(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr);

#endif /* DUNGEON_DLA_H */
