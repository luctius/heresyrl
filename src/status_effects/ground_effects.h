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

#ifndef GROUND_EFFECTS_H
#define GROUND_EFFECTS_H

#include <assert.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "ground_effects.h"
#include "ground_effects_static.h"
#include "dungeon/dungeon_map.h"
#include "status_effects_static.h"

enum gr_effects_flags {
    GR_EFFECTS_OPAQUE,
    GR_EFFECTS_REMOVE_ON_EXIT,
    GR_EFFECTS_MAX,
    GR_EFFECTS_NONE,
};

struct ground_effect {
    uint32_t ground_effect_pre;

    uint32_t uid;
    enum ge_ids tid;
    bitfield32_t flags;

    const char *sd_name;
    const char *ld_name;
    const char *description;

    char icon;
    int icon_attr;
    int min_energy;
    int max_energy;
    int current_energy;
    enum se_ids se_id;

    struct dm_map_entity *me;

    uint32_t ground_effect_post;
};

void ge_init(void);
void ge_exit(void);
struct ground_effect *gelst_get_next(struct ground_effect *ge);

struct ground_effect *ge_create(uint32_t tid, struct dm_map_entity *me);
bool ge_destroy(struct dm_map_entity *me);
void ge_process(struct dm_map *map);

#endif /* GROUND_EFFECTS_H */
