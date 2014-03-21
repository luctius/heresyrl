#ifndef SPAWN_H
#define SPAWN_H

#include <stdint.h>
#include "heresyrl_def.h"
#include "items_static.h"

struct spwn_monster_item {
    enum item_ids id;
    int min;
    int max;
    bool wear;
};

bool spwn_populate_map(struct dc_map *map, struct random *r, uint32_t monster_chance, uint32_t item_chance);
bool spwn_add_item_to_monster(struct msr_monster *monster, struct spwn_monster_item *sitem, struct random *r);

#endif /* SPAWN_H */
