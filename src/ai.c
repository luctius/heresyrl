#include <sys/param.h>

#include "ai.h"
#include "pathfinding.h"
#include "monster.h"
#include "dungeon_creator.h"
#include "los.h"
#include "tiles.h"

#include "game.h"
#include "monster_action.h"

struct msr_monster *ai_get_nearest_enemy(struct msr_monster *monster, int ignore_cnt, struct dc_map *map) {
    struct msr_monster *target = NULL;

    int far_radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster);
    while ( (target = msrlst_get_next_monster(target) ) != NULL) {
        if (target->faction == monster->faction) continue; /* ignore same faction */
        if (cd_equal(&target->pos, &monster->pos) ) continue; /* ignore current position*/
        if (cd_pyth(&target->pos, &monster->pos) > far_radius) continue; /* ignore out of maximum radius */

        /* if the target tile is not lit, ignore it if it is further than near_sight_range*/
        if (sd_get_map_me(&target->pos, map)->light_level == 0) {
            if (cd_pyth(&target->pos, &monster->pos) > msr_get_near_sight_range(monster) ) continue;
        }

        if (los_has_sight(&monster->pos, &target->pos, map) == true) {
            ignore_cnt--;
            if (ignore_cnt < 0) return target;
        }
    }
    if (ignore_cnt > 0) return NULL;
    return target;
}

struct msr_monster *ai_get_nearest_monster(coord_t *pos, int radius, int ignore_cnt, struct dc_map *map) {
    struct msr_monster *target = NULL;

    while ( (target = msrlst_get_next_monster(target) ) != NULL) {
        if (cd_equal(&target->pos, pos) ) continue; /* ignore current position*/
        if (cd_pyth(&target->pos, pos) > radius) continue; /* ignore out of radius */

        if (los_has_sight(pos, &target->pos, map) == true) {
            ignore_cnt--;
            if (ignore_cnt < 0) return target;
        }
    }
    return target;
}

static unsigned int ai_traversable_callback(void *vmap, coord_t *coord) {
    if (vmap == NULL) return PF_BLOCKED;
    if (coord == NULL) return PF_BLOCKED;
    struct dc_map *map = (struct dc_map *) vmap;

    unsigned int cost = PF_BLOCKED;
    if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(coord, map),TILE_ATTR_TRAVERSABLE) == true) {
        cost = sd_get_map_tile(coord, map)->movement_cost;
    }
    if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(coord, map),TILE_ATTR_BORDER) == true) cost = PF_BLOCKED;

    return cost;
}


bool ai_generate_dijkstra(struct pf_context **pf_ctx, struct dc_map *map, coord_t *start, int radius) {
    if (dc_verify_map(map) == false) return false;

    struct pf_settings pf_set = { 
        .map_start = { 
            .x = 0, 
            .y = 0, 
        }, 
        .map_end = {
            .x = map->size.x,
            .y = map->size.y,
        },
        .map = map,
        .pf_traversable_callback = ai_traversable_callback,
    };

    if (radius > 0) {
        pf_set.map_start.x = MAX(start->x - radius, 0);
        pf_set.map_start.y = MAX(start->y - radius, 0);
        pf_set.map_end.x = MIN(start->x + radius, map->size.x);
        pf_set.map_end.y = MIN(start->y + radius, map->size.y);
    }

    if (*pf_ctx == NULL) {
        *pf_ctx = pf_init(&pf_set);
    }

    if (pf_dijkstra_map(*pf_ctx, start) ) {
        return true;
    }
    return false;
}

bool ai_generate_astar(struct pf_context **pf_ctx, struct dc_map *map, coord_t *start, coord_t *end, int radius) {
    if (dc_verify_map(map) == false) return false;

    struct pf_settings pf_set = { 
        .map_start = { 
            .x = 0, 
            .y = 0, 
        }, 
        .map_end = {
            .x = map->size.x,
            .y = map->size.y,
        },
        .map = map,
        .pf_traversable_callback = ai_traversable_callback,
    };

    if (radius > 0) {
        pf_set.map_start.x = MAX(start->x - radius, 0);
        pf_set.map_start.y = MAX(start->y - radius, 0);
        pf_set.map_end.x = MIN(start->x + radius, map->size.x);
        pf_set.map_end.y = MIN(start->y + radius, map->size.y);
    }

    if (*pf_ctx == NULL) {
        *pf_ctx = pf_init(&pf_set);
    }

    if (pf_astar_map(*pf_ctx, start, end) ) {
        return true;
    }
    return false;
}

struct beast_ai_struct {
    int something;
    struct pf_context *pf_ctx;
};

static bool ai_beast_loop(struct msr_monster *monster, void *controller) {
    if (monster->dead == true) {
        msr_clear_controller(monster);
    }
    struct dc_map *map = gbl_game->current_map;
    struct beast_ai_struct *ai = controller;
    bool has_action = false;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "ai_beast_loop for [uid: %d, tid: %d]", monster->uid, monster->template_id);

    struct msr_monster *enemy = NULL;
    if ( (enemy = ai_get_nearest_enemy(monster, 0, map) ) != NULL) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] sees an enemy", monster->uid, monster->template_id);
        if (cd_pyth(&monster->pos, &enemy->pos) > 1) {
            int radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster) +1;
            if (ai_generate_astar(&ai->pf_ctx, map, &monster->pos, &enemy->pos, 0) == true) {
                coord_t *coord_lst;
                int coord_lst_sz =  pf_calculate_path(ai->pf_ctx, &monster->pos, &enemy->pos, &coord_lst);
                if (coord_lst_sz > 1) {
                    if (ma_do_move(monster, &coord_lst[1]) == true) {
                        has_action = true;
                    }
                    free(coord_lst);
                }
            }
        }
        else {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] attacks an enemy", monster->uid, monster->template_id);
            monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
            if (ma_do_melee(monster, &enemy->pos) == true) {
                has_action = true;
            }
        }
    }

    if (has_action == false) {
        return ma_do_guard(monster);
    }

    return false;
}

void ai_monster_init(struct msr_monster *monster) {
    struct beast_ai_struct *ai = calloc(1, sizeof(struct beast_ai_struct) );
    if (ai != NULL) {
        struct monster_controller mc = {
            .controller_ctx = ai,
            .controller_cb = ai_beast_loop,
        };

        msr_assign_controller(monster, &mc);
    }
}

