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
                assert(dw_wear_item(monster, item) == true);
                return true;
            }
        }
    }
    return false;
}
