#ifndef AI_H
#define AI_H

#include "heresyrl_def.h"
#include "pathfinding.h"
#include "coord.h"

struct msr_monster *ai_get_nearest_enemy(struct msr_monster *monster, int ignore_cnt, struct dc_map *map);
struct msr_monster *ai_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dc_map *map);

bool ai_generate_astar(struct pf_context **pf_ctx, struct dc_map *map, coord_t *start, coord_t *end, int radius);
bool ai_generate_dijkstra(struct pf_context **pf_ctx, struct dc_map *map, coord_t *start, int radius);
void ai_monster_init(struct msr_monster *monster);

#endif /* AI_H */
