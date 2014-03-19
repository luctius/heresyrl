#ifndef AI_H
#define AI_H

#include "heresyrl_def.h"

struct msr_monster *ai_get_nearest_enemy(struct msr_monster *monster, int radius, int ignore_cnt, struct dc_map *map);
struct msr_monster *ai_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dc_map *map);

#endif /* AI_H */
