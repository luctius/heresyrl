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

#ifndef DUNGEON_HELPERS_H
#define DUNGEON_HELPERS_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "logging.h"
#include "coord.h"
#include "random.h"
#include "dungeon/dungeon_map.h"
#include "dungeon/tiles.h"
#include "ai/pathfinding.h"
#include "ai/ai_utils.h"

struct dungeon_features_done {
    bool loops;
    bool lights;
    bool features;
    bool reachability;
};

enum dm_feature_type {
    DM_FT_POOL,
};

struct generator {
    struct dungeon_features_done (*generator)(struct dm_map *map, struct random *r, coord_t *ul, coord_t *dr);
    enum dm_dungeon_type type;
    int weight;
};

bool dm_add_generator(struct generator *g);

bool dm_tunnel(struct dm_map *map, struct random *r, coord_t *start, coord_t *end, struct tl_tile *tl);
bool dm_get_tunnel_path(struct dm_map *map, struct pf_context *pf_ctx, struct random *r, coord_t *tnl_start, coord_t *tnl_end);
void dm_add_loops(struct dm_map *map, struct pf_context *pf_ctx, struct random *r);
void dm_add_lights(struct dm_map *map, struct random *r);
bool dm_generate_feature(struct dm_map *map, struct random *r, coord_t *point, int min_radius, int max_radius, enum dm_feature_type ft);

#endif /*DUNGEON_HELPERS_H*/
