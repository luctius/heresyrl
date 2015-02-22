#ifndef AI_UTILS_H
#define AI_UTILS_H

#include "heresyrl_def.h"
#include "pathfinding.h"
#include "coord.h"

struct msr_monster *aiu_get_nearest_enemy(struct msr_monster *monster, int ignore_cnt, struct dm_map *map);
struct msr_monster *aiu_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dm_map *map);

bool aiu_generate_astar(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, coord_t *end, int radius);
bool aiu_generate_dijkstra(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, int radius);

struct itm_item *aiu_next_thrown_weapon(struct msr_monster *monster, int idx);

#endif /* AI_UTILS_H */
