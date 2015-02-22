#include "turn_tick.h"
#include "monster/monster.h"
#include "status_effects/status_effects.h"
#include "status_effects/ground_effects.h"
#include "items/items.h"
#include "coord.h"
#include "game.h"
#include "dungeon/dungeon_map.h"

#include "ui/ui.h"

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

void tt_process_monsters(struct dm_map *map) {
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

        /* A stunned monster can do nothing. */
        if (se_has_effect(monster, EF_STUNNED) ) {
            do_action = false;

            /* Player is a bit special and would like to see something... */
            if (monster->is_player) update_screen();
        }

        if (do_action || monster->dead) {
            if (monster->controller.controller_cb != NULL) {
                monster->controller.controller_cb(monster);
            }
            monster->controller.interrupted = false;

            /* TODO, HACK: performance drain, improve this!! */
            //update_screen();
        }

        if (gbl_game->running == false) return;
    }
}

void tt_process_items(struct dm_map *map) {
    struct itm_item *item = NULL;
    if (gbl_game->running == false) return;

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

void tt_process_status_effects(void) {
    struct msr_monster *monster = NULL;
    if (gbl_game->running == false) return;

    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->dead == false) {
            if (se_list_size(monster->status_effects) > 0 ) {
                se_process(monster);
            }
        }
    }
}

void tt_process(struct dm_map *map) {
    tt_process_monsters(map);

    tt_process_items(map);

    ge_process(map);
    dm_process_tiles(map);

    tt_process_status_effects();
}

void tt_init(void) {}
void tt_exit(void) {}
