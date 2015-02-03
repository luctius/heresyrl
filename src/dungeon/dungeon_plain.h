#ifndef DUNGEON_PLAIN_H
#define DUNGEON_PLAIN_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

bool dm_generate_map_plain(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr);

#endif /* DUNGEON_PLAIN_H */
