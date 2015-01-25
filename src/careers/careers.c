#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include <float.h>

#include "careers.h"
#include "careers_static.h"
#include "random.h"
#include "tiles.h"
#include "inventory.h"
#include "fight.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "items/items.h"

#include "careers_static.c"

enum career_ids cr_spawn(double roll, enum msr_race race) {
    int sz = ARRAY_SZ(static_career_list);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0;

    int idx = CRID_NONE;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = CRID_NONE; i < sz; i++) {
        if (static_career_list[i].available[race] == true) {
            sum += static_career_list[i].weight[race];
        }
        else cumm_prob_arr[i] = DBL_MAX;
    }

    double cumm = 0;
    for (int i = CRID_NONE; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = static_career_list[i].weight[race] / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    for (int i = sz-1; i > CRID_NONE; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    return idx;
}

struct cr_career *cr_get_career_by_id(enum career_ids template_id) {
    if (template_id < CRID_NONE) return NULL;
    if (template_id >= CRID_MAX) return NULL;

    return &static_career_list[template_id];
}

enum career_ids cr_get_next_career_id_for_race(enum msr_race race, enum career_ids prev_tid) {
    if (prev_tid < CRID_NONE) return CRID_NONE;

    for (int i = prev_tid; i < ARRAY_SZ(static_career_list); i++) {
        if (static_career_list[i].available[race] == true) return i;
    }
    return CRID_NONE;
}

bool cr_give_trappings_to_player(struct msr_monster *monster, struct cr_career *car) {
    if (msr_verify_monster(monster) == false) return false;

    for (int i = 0; i < (int) ARRAY_SZ(car->trappings); i++) {
        if (car->trappings[i] != IID_NONE) {
            struct itm_item *item = itm_create(car->trappings[i]);
            assert(item != NULL);
            assert( (msr_give_item(monster, item) ) );
        }
    }
    return true;
}

