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

#include <sys/param.h>
#include <string.h>

#include "ai_utils.h"
#include "pathfinding.h"
#include "fov/sight.h"
#include "monster/monster.h"
#include "items/items.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "fight.h"

#include "game.h"

/*
   This file contains helper functions for the ai, and for automated actions for the player.
 */

/* TODO: use sight.c to make sure visibility is the same for players and npcs */
static struct msr_monster *aiu_get_enemy_near(struct msr_monster *monster, struct msr_monster *last, struct dm_map *map) {
    struct msr_monster *target = NULL;

    bool found_last = false;
    if (last == NULL) found_last = true;

    int far_radius = msr_get_far_sight_range(monster);
    while ( (target = msrlst_get_next_monster(target) ) != NULL) {
        if (target == last) {
            found_last = true;
            continue;
        }
        if (found_last == false) continue;

        if (target->faction == monster->faction) continue; /* ignore same faction */
        if (cd_equal(&target->pos, &monster->pos) ) continue; /* ignore current position*/
        if (cd_pyth(&target->pos, &monster->pos) > far_radius) continue; /* ignore out of maximum radius */

        if (fght_can_see(map, monster, target) ) return target;
    }
    return NULL;
}

struct msr_monster *aiu_get_nearest_enemy(struct msr_monster *monster, int ignore_cnt, struct dm_map *map) {
    struct msr_monster *target_best[ignore_cnt+1];
    struct msr_monster *target = NULL;

    memset(target_best, 0x0, sizeof(struct msr_monster *) * (ignore_cnt +1) );

    /*
       get the <ignore_cnt> nearest enemies, then return the last one.
     */
    for (int i = 0; i < ignore_cnt+1; i++) {
        while ( (target = aiu_get_enemy_near(monster, target, map) ) != NULL) {

            /* check if target allready exists in the array */
            bool pass = false;
            for (int j = 0; j < i; j++) if (target_best[j] == target) pass = true;
            if (pass) continue; /* if so, continue */

            if (target_best[i] == NULL) { target_best[i] = target; continue; }
            if (cd_pyth(&target->pos, &monster->pos) > cd_pyth(&target_best[i]->pos, &monster->pos) ) continue; /* ignore out of maximum radius */

            target_best[i] = target;
        }
        if (target_best[i] == NULL)  return NULL;
        target = NULL;
    }
    return target_best[ignore_cnt];
}

struct msr_monster *aiu_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dm_map *map) {
    struct msr_monster *target = NULL;

    while ( (target = msrlst_get_next_monster(target) ) != NULL) {
        if (cd_equal(&target->pos, pos) ) continue; /* ignore current position*/
        if (cd_pyth(&target->pos, pos) > radius) continue; /* ignore out of radius */

        if (sgt_has_los(map, pos, &target->pos, radius) == true) {
            ignore_cnt--;
            if (ignore_cnt < 0) return target;
        }
    }
    return target;
}

static unsigned int aiu_traversable_callback(void *vmap, coord_t *coord) {
    unsigned int cost = PF_BLOCKED;
    if (vmap == NULL) return cost;
    if (coord == NULL) return cost;

    //if (dm_verify_map(map) == false) return PF_BLOCKED;
    struct dm_map *map = (struct dm_map *) vmap;
    struct dm_map_entity *me = dm_get_map_me(coord, map);
    struct tl_tile *te = me->tile;

    if (TILE_HAS_ATTRIBUTE(te,TILE_ATTR_BORDER) == true) return cost;

    if (TILE_HAS_ATTRIBUTE(te,TILE_ATTR_TRAVERSABLE) == true) {
        cost = te->movement_cost;
    }

    if (me->monster != NULL) {
        cost += me->monster->idle_counter;
    }

    if (me->effect != NULL) {
        cost += 100;
    }

    return cost;
}

bool aiu_generate_dijkstra(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, int radius) {
    if (dm_verify_map(map) == false) return false;

    struct pf_settings pf_set = {
        .map_start = {
            .x = 0,
            .y = 0,
        },
        .map_end = {
            .x = map->sett.size.x,
            .y = map->sett.size.y,
        },
        .map = map,
        .pf_traversable_callback = aiu_traversable_callback,
    };

    if (radius > 0) {
        pf_set.map_start.x = MAX(start->x - radius, 0);
        pf_set.map_start.y = MAX(start->y - radius, 0);
        pf_set.map_end.x = MIN(start->x + radius, map->sett.size.x);
        pf_set.map_end.y = MIN(start->y + radius, map->sett.size.y);
    }

    if (*pf_ctx == NULL) {
        *pf_ctx = pf_init(&pf_set);
    }

    if (pf_dijkstra_map(*pf_ctx, start) ) {
        return true;
    }
    return false;
}

bool aiu_generate_astar(struct pf_context **pf_ctx, struct dm_map *map, coord_t *start, coord_t *end, int radius) {
    bool retval = false;
    if (dm_verify_map(map) == false) return retval;

    struct pf_settings pf_set = {
        .map_start = {
            .x = 0,
            .y = 0,
        },
        .map_end = {
            .x = map->sett.size.x,
            .y = map->sett.size.y,
        },
        .map = map,
        .pf_traversable_callback = aiu_traversable_callback,
    };

    if (radius > 0) {
        pf_set.map_start.x = MAX(start->x - radius, 0);
        pf_set.map_start.y = MAX(start->y - radius, 0);
        pf_set.map_end.x = MIN(start->x + radius, map->sett.size.x);
        pf_set.map_end.y = MIN(start->y + radius, map->sett.size.y);
    }

    if (*pf_ctx == NULL) {
        *pf_ctx = pf_init(&pf_set);
    }

    if (pf_astar_map(*pf_ctx, start, end) ) {
        retval = true;
    }
    return retval;
}

struct itm_item *aiu_next_thrown_weapon(struct msr_monster *monster, int idx) {
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(monster->inventory, item) ) != NULL) {
        if (wpn_is_type(item, WEAPON_TYPE_THROWN) ) {
            if (idx == 0) {
                break;
            }
            idx--;
        }
    }
    return item;
}

struct itm_item *aiu_next_unused_item(struct msr_monster *monster, int idx) {
    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(monster->inventory, item) ) != NULL) {
        if (inv_item_worn(monster->inventory, item) == false) {
            if (idx == 0) {
                break;
            }
            idx--;
        }
    }
    return item;
}

