#ifndef SPAWN_STATIC_H
#define SPAWN_STATIC_H

#include "items/items_static.h"
#include "spawn.h"

struct spwn_item {
    enum item_ids id;
    int weight;
};

struct spwn_monster {
    enum msr_ids id;
    int weight;
    struct spwn_monster_item items[6];
};

#endif /* SPAWN_STATIC_H */