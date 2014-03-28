#include "turn_tick.h"
#include "monster.h"
#include "items.h"
#include "coord.h"
#include "game.h"

bool tt_interrupt_event(uint32_t monster_uid) {
    struct msr_monster *monster = NULL;
    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->uid == monster_uid) {
            if (monster->controller.interruptable) {
                monster->controller.interrupted = true;
            }
        }
    }
    return false;
}

void tt_process_monsters(struct dc_map *map) {
    struct msr_monster *monster = NULL;

    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->dead) {

            /* Clean-up monsters which can be cleaned up. */
            if (monster->controller.controller_cb == NULL) {
                struct msr_monster *dead_monster = monster;
                monster = msrlst_get_next_monster(monster);

                msr_destroy(dead_monster, map);
                continue;
            }
        }

        bool do_action = false;

        if (msr_get_energy(monster) < TT_ENERGY_FULL) msr_change_energy(monster, TT_ENERGY_TICK);

        if (msr_get_energy(monster) >= TT_ENERGY_FULL) do_action = true;
        if (monster->controller.interrupted == true) do_action = true;

        if (do_action || monster->dead) {
            if (monster->controller.controller_cb != NULL) {
                monster->controller.controller_cb(monster, monster->controller.controller_ctx);
            }
            monster->controller.interrupted = false;
        }

    }
}

void tt_process_items(struct dc_map *map) {
    struct itm_item *item = NULL;

    while ( (item = itmlst_get_next_item(item) ) != NULL) {
        if (item->energy_action == true) {
            itm_change_energy(item, -TT_ENERGY_TICK);

            if (itm_get_energy(item) < 0) {
                item->energy_action = false;
                itm_energy_action(item, map);
            }
        }
    }
}

void tt_process(struct dc_map *map) {
    tt_process_items(map);

    tt_process_monsters(map);
}

void tt_init(void) {}
void tt_exit(void) {}