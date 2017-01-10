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

#include "turn_tick.h"
#include "monster/monster.h"
#include "status_effects/status_effects.h"
#include "status_effects/ground_effects.h"
#include "items/items.h"
#include "coord.h"
#include "game.h"
#include "dungeon/dungeon_map.h"
#include "random.h"
#include "fov/sight.h"
#include "coord.h"

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

    if ( (gbl_game->turn % TT_ENERGY_TURN_MINI) != 0) return;

    monster = NULL;
    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (se_has_effect(monster, EF_DEAD) ) {
            /* Clean-up monsters which can be cleaned up. */
            if (monster->controller.controller_cb == NULL) {
                struct msr_monster *dead_monster = monster;
                monster = msrlst_get_next_monster(monster);

                msr_destroy(dead_monster, map);
                continue;
            }
        }

        if (se_list_size(monster->status_effects) > 0 ) {
            se_process(monster);
        }

        bool do_action = false;

        msr_change_energy(monster, TT_ENERGY_TICK);

        if (msr_get_energy(monster) >= TT_ENERGY_FULL) do_action = true;
        if (monster->controller.interrupted == true) do_action = true;

        /* A stunned monster can do nothing. */
        if (se_has_effect(monster, EF_STUNNED) ) {
            do_action = false;

            /* Player is a bit special and would like to see something... */
            if (monster->is_player) update_screen();
        }

        if (do_action || (se_has_effect(monster, EF_DEAD) ) ) {
            if (monster->controller.controller_cb != NULL) {
                coord_t zero = cd_create(0,0);
                dm_clear_map_visibility(map, &zero, &map->sett.size);
                sgt_calculate_all_light_sources(map);
                sgt_calculate_player_sight(map, gbl_game->player_data.player);
                monster->controller.controller_cb(monster);
            }
            monster->controller.interrupted = false;

            if (gbl_game->player_data.exit_map == true) return;

            if (dm_get_map_me(&monster->pos, map)->visible) {
                update_screen();
            }
        }

        if (gbl_game->running == false) return;
    }
}

void tt_process_items(struct dm_map *map) {
    struct itm_item *item = NULL;
    struct itm_item *item_prev = NULL;
    if (gbl_game->running == false) return;

    if ( (gbl_game->turn % TT_ENERGY_TURN_MINI) != 0) return;

    while ( (item = itmlst_get_next_item(item_prev) ) != NULL) {
        if (item->energy_action == true) {
            itm_change_energy(item, -TT_ENERGY_TICK);

            if (itm_get_energy(item) < 0) {
                item->energy_action = false;
                if (itm_energy_action(item, map) == false) {
                    item = item_prev;
                }
            }
        }
        item_prev = item;
    }
}

void tt_process_status_effects(void) {
    return;

    struct msr_monster *monster = NULL;
    if (gbl_game->running == false) return;

    if ( (gbl_game->turn % TT_ENERGY_TURN_MINI) != 0) return;

    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (se_has_effect(monster, EF_DEAD) == false) {
            if (se_list_size(monster->status_effects) > 0 ) {
                se_process(monster);
            }
        }
    }
}

void tt_process(struct dm_map *map) {
    tt_process_monsters(map);

    tt_process_items(map);

    dm_process_tiles(map);
    ge_process(map);

    tt_process_status_effects();
}

void tt_init(void) {}
void tt_exit(void) {}
