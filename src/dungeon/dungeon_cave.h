#ifndef DUNGEON_CAVE_H_
#define DUNGEON_CAVE_H_

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

bool cave_generate_map(struct dc_map *map, struct random *r, enum dc_dungeon_type type, int level);

#endif /*DUNGEON_CAVE_H_*/
