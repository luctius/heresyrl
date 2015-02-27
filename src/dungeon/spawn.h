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

#ifndef SPAWN_H
#define SPAWN_H

#include <stdint.h>

#include "heresyrl_def.h"
#include "items/items_static.h"

struct spwn_monster_item {
    enum item_ids id;
    int min;
    int max;
    bool wear;
};

bool spwn_populate_map(struct dm_map *map, struct random *r, uint32_t monster_chance, uint32_t item_chance);
bool spwn_add_item_to_monster(struct msr_monster *monster, struct spwn_monster_item *sitem, struct random *r);

#endif /* SPAWN_H */
