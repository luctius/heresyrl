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

#ifndef AI_UTILS_H
#define AI_UTILS_H

#include "heresyrl_def.h"
#include "pathfinding.h"
#include "coord.h"

struct msr_monster *aiu_get_nearest_enemy(struct msr_monster *monster, int ignore_cnt, struct dm_map *map);
struct msr_monster *aiu_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dm_map *map);

bool aiu_generate_astar(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, coord_t *end, int radius);
bool aiu_generate_dijkstra(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, int radius);

struct itm_item *aiu_next_unused_item(struct msr_monster *monster, int idx);
struct itm_item *aiu_next_thrown_weapon(struct msr_monster *monster, int idx);

#endif /* AI_UTILS_H */
