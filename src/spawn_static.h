#ifndef SPAWN_STATIC_H
#define SPAWN_STATIC_H

#include "items_static.h"
#include "monster_static.h"

struct spwn_item {
    enum item_ids id;
    int weight;
};

struct spwn_monster {
    enum msr_ids id;
    int weight;
};

#endif /* SPAWN_STATIC_H */
