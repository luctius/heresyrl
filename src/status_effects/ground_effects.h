#ifndef GROUND_EFFECTS_H
#define GROUND_EFFECTS_H

#include <assert.h>
#include <sys/queue.h>

#include "status_effects.h"
#include "logging.h"
#include "heresyrl_def.h"
#include "random.h"
#include "turn_tick.h"
#include "dungeon/dungeon_map.h"

struct ground_effects {
    uint32_t uid;
    uint32_t tid;
    bitfield_32_t flags;

    char icon;
    int icon_attr;
    int min_energy;
    int max_energy;
    enum se_ids se_id;

    coord_t pos;
    int current_energy;
};

bool ge_create(uint32_t template_id, coord_t pos);
bool ge_process(struct dm_map *map);

#endif /* GROUND_EFFECTS_H */
