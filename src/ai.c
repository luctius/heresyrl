#include "monster.h"
#include "dungeon_creator.h"
#include "ai.h"
#include "los.h"

struct msr_monster *ai_get_nearest_enemy(struct msr_monster *monster, int radius, int ignore_cnt, struct dc_map *map) {
    struct msr_monster *target = NULL;

    while ( (target = msrlst_get_next_monster(target) ) != NULL) {
        if (cd_equal(&target->pos, &monster->pos) ) continue; /* ignore current position*/
        if (cd_pyth(&target->pos, &monster->pos) > radius) continue; /* ignore out of radius */
        if (target->faction == monster->faction) continue; /* ignore same faction */

        if (los_has_sight(&monster->pos, &target->pos, map) == true) {
            ignore_cnt--;
            if (ignore_cnt < 0) return target;
        }
    }
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
