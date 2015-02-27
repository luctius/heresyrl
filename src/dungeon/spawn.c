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

#include <assert.h>
#include <sys/param.h>

#include "spawn.h"
#include "logging.h"
#include "game.h"
#include "tiles.h"
#include "random.h"
#include "dowear.h"
#include "ai/ai.h"
#include "dungeon/dungeon_map.h"
#include "items/items.h"
#include "monster/monster.h"

bool spwn_add_item_to_monster(struct msr_monster *monster, struct spwn_monster_item *sitem, struct random *r) {
    int nr = sitem->min;
    if (sitem->min != sitem->max) {
        nr = (random_int32(r) % (sitem->max - sitem->min) ) + sitem->min;
    }

    if (nr >= 1) {
        struct itm_item *item = itm_create(sitem->id);
        if (item != NULL) {
            item->stacked_quantity = MIN(nr, item->max_quantity);
            assert(msr_give_item(monster, item) == true);
            if (sitem->wear == true) {
                dw_wear_item(monster, item);
                return true;
            }
        }
    }
    return false;
}
