#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "monster_action.h"
#include "monster.h"
#include "items.h"
#include "dungeon_creator.h"
#include "tiles.h"
#include "inventory.h"
#include "ui.h"
#include "coord.h"
#include "game.h"
#include "dowear.h"

void ma_init(void) {
}

void ma_exit(void) {
}

bool ma_process(void) {
    struct msr_monster *monster = NULL;

    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->energy < MSR_ENERGY_FULL) monster->energy += MSR_ENERGY_TICK;

        /*if ( (monster->energy >= MSR_ENERGY_FULL) || 
             (monster->controller.interrupted == true) ) {*/
        if (monster->energy >= MSR_ENERGY_FULL) {
            if (monster->controller.controller_cb != NULL) {
                monster->controller.controller_cb(monster, monster->controller.controller_ctx);
            }
        }

    }
    return true;
}

bool mt_interrupt_event(uint32_t monster_uid) {
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

bool ma_do_move(struct msr_monster *monster, coord_t *pos) {
    if (monster == NULL) return false;
    if (pos == NULL) return false;
    coord_t oldpos = monster->pos;

    if (msr_move_monster(monster, gbl_game->current_map, pos) == true) {
        if ( (sd_get_map_me(&oldpos, gbl_game->current_map)->visible == true) || 
             (sd_get_map_me(pos, gbl_game->current_map)->visible == true) ) {
            /* TODO interrupt enemies who see me. */
            update_screen();
        }

        int speed = msr_calculate_characteristic_bonus(monster, MSR_CHAR_TOUGHNESS) * 10;
        monster->energy -= (MSR_ACTION_MOVE - speed);
        monster->controller.interruptable = false;
        monster->controller.interrupted = false;
        return true;
    }
    return false;
}

bool ma_do_idle(struct msr_monster *monster) {
    return ma_do_guard(monster);
}

bool ma_do_guard(struct msr_monster *monster) {
    if (monster == NULL) return false;

    monster->energy -= MSR_ACTION_GUARD;
    monster->controller.interruptable = true;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_wear(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    if (dw_can_wear_item(monster, item) == false) return false;

    if (dw_wear_item(monster, item) == false) return false;
    monster->energy -= MSR_ACTION_WEAR * item->use_delay;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_remove(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;
    if (dw_can_remove_item(monster, item) == false) return false;

    if (dw_remove_item(monster, item) == false) return false;
    monster->energy -= MSR_ACTION_REMOVE * item->use_delay;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_use(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    if (dw_use_item(monster, item) == false) return false;

    monster->energy -= MSR_ACTION_USE * item->use_delay;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_pickup(struct msr_monster *monster, struct itm_item *items[], int nr_items) {
    if (monster == NULL) return false;
    if (items == NULL) return false;
    if (nr_items == 0) return false;

    for (int i = 0; i< nr_items; i++) {
        struct dc_map_entity *me = sd_get_map_me(&monster->pos, gbl_game->current_map);
        if (me != NULL) {
            if (inv_has_item(me->inventory, items[i]) == true) {
                if (msr_give_item(monster, items[i]) == true) {
                    inv_remove_item(me->inventory, items[i]);
                    monster->energy -= MSR_ACTION_PICKUP;
                }
            }
        }
    }

    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_drop(struct msr_monster *monster, struct itm_item *items[], int nr_items) {
    if (monster == NULL) return false;
    if (items == NULL) return false;
    if (nr_items <= 0) return false;

    for (int i = 0; i< nr_items; i++) {
        if (inv_has_item(monster->inventory, items[i]) == true) {
            if ( (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_MAINHAND_WIELD) ||
                 (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_OFFHAND_WIELD) ) {
                /*
                   Allow drop of weapons to the ground for free.
                 */
                if (msr_remove_item(monster, items[i]) == true) {
                    itm_insert_item(items[i], gbl_game->current_map, &monster->pos);
                }
            }
            if (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_INVENTORY) {
                if (msr_remove_item(monster, items[i]) == true) {
                    if (itm_insert_item(items[i], gbl_game->current_map, &monster->pos) == true) {
                        monster->energy -= MSR_ACTION_DROP;
                    }
                }
            }
        }
    }

    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_fire(struct msr_monster *monster, coord_t *pos) {
    if (monster == NULL) return false;
    if (pos == NULL) return false;

    if (fght_shoot(gbl_game->game_random, monster, gbl_game->current_map, pos) == false) {
        return false;
    }

    monster->energy -= MSR_ACTION_FIRE;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

static bool ma_has_ammo(struct msr_monster *monster, struct itm_item *item) {
    /* TODO check for ammo in inventory */
    return true;
}

bool ma_do_reload(struct msr_monster *monster) {
    if (monster == NULL) return false;
    struct inv_inventory *inv = monster->inventory;
    if (inv_loc_empty(inv,INV_LOC_MAINHAND_WIELD) && inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    struct itm_item *item = NULL;
    struct item_weapon_specific *wpn = NULL;
    uint32_t cost = 0;

    if ( (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
         (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND)  ||
         (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
        item = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
        if (wpn_is_type(item, WEAPON_TYPE_RANGED) != false) {
            wpn = &item->specific.weapon;
            if (wpn->magazine_left < wpn->magazine_sz) {
                if (ma_has_ammo(monster, item) == true ) {
                    wpn->magazine_left = wpn->magazine_sz;
                    cost += MSR_ACTION_RELOAD * item->use_delay;
                }
            }
        }
    }

    if (cost == 0) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (wpn_is_type(item, WEAPON_TYPE_RANGED) != false) {
                wpn = &item->specific.weapon;
                if (wpn->magazine_left < wpn->magazine_sz) {
                    if (ma_has_ammo(monster, item) == true ) {
                        wpn->magazine_left = wpn->magazine_sz;
                        cost += MSR_ACTION_RELOAD * item->use_delay;
                    }
                }
            }
        }
    }

    if (cost == 0) return false;
    monster->energy -= cost;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

