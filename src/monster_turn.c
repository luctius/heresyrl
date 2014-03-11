#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "monster_turn.h"
#include "monster.h"
#include "items.h"
#include "dungeon_creator.h"
#include "tiles.h"
#include "inventory.h"
#include "ui.h"
#include "coord.h"
#include "game.h"
#include "dowear.h"

struct mt_event;

/* Intermediate callback at the end of a turn. 
   should return true of nothing else needs to be done, 
   or false if it still needs to wait.

   It can change the event struct in the call.
 */
typedef bool (*event_callback_t)(struct mt_event *e);

struct mt_event {
    struct msr_monster *monster;
    void *controller;
    event_callback_t cb_do;
    bool interruptable;
    bool interrupted;
    mt_callback_t controller_cb;
    struct itm_item *item;
};

struct mt_event_entry {
    struct mt_event event;
    LIST_ENTRY(mt_event_entry) entries;
};

static LIST_HEAD(mt_event_list, mt_event_entry) head;

void mt_init(void) {
    LIST_INIT(&head);
}

void mt_exit(void) {
    struct mt_event_entry  *eve;
    while ( (eve = head.lh_first ) != NULL) {
        LIST_REMOVE(head.lh_first, entries);
        free(eve);
    }
}

bool mt_add_event(struct mt_event *event, uint32_t cost) {
    if (event == NULL) return false;

    struct mt_event_entry *eve = calloc(1, sizeof(struct mt_event_entry));
    if (eve == NULL) return false;

    LIST_INSERT_HEAD(&head, eve, entries);
    memcpy(&eve->event, event, sizeof(struct mt_event));
    eve->event.interrupted = false;
    eve->event.monster->energy = eve->event.monster->energy - cost;
    return true;
}

bool mt_remove_event(struct mt_event *event) {
    if (event == NULL) return false;
    struct mt_event_entry *eve = head.lh_first;
    if (eve == NULL) return false;
    struct mt_event_entry *ev_remove = container_of(event, struct mt_event_entry, event);

    LIST_REMOVE(ev_remove, entries);
    free(ev_remove);
    return true;
}

bool mt_interrupt_event(uint32_t monster_uid) {
    struct mt_event_entry *eve = head.lh_first;
    if (eve == NULL) return false;

    while (eve != NULL) {
        if (eve->event.monster->uid == monster_uid) {
            if (eve->event.interruptable == true) eve->event.interrupted = true;
            return true;
        }
        
        eve = eve->entries.le_next;
    }
    return false;
}

bool mt_process(void) {
    struct mt_event_entry *eve = head.lh_first;

    for (eve = head.lh_first; eve != NULL; eve = eve->entries.le_next) {
        struct mt_event *event = &eve->event;
        bool do_action = false;

        if (event->interrupted == true) do_action = true;

        if (event->monster->energy >= MT_ENERGY_FULL) {
            do_action = true;
            event->interrupted = false;
        }
        else {
            event->monster->energy += MT_ENERGY_TICK;
        }

        if (do_action == true) {
            if (event->cb_do(event) == true) {
                mt_remove_event(event);
            }
        }
    }
    return true;
}

static bool mt_cb_to_controller(struct mt_event *e) {
    if (e != NULL) {
        e->controller_cb(e->monster, e->controller);
    }

    return true;
}

bool mt_do_move(struct msr_monster *monster, coord_t *pos, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (pos == NULL) return false;
    if (cb == NULL) return false;
    coord_t oldpos = monster->pos;

    if (msr_move_monster(monster, gbl_game->current_map, pos) == true) {
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
            .item = NULL,
        };
        if ( (sd_get_map_me(&oldpos, gbl_game->current_map)->visible == true) || 
             (sd_get_map_me(pos, gbl_game->current_map)->visible == true) ) {
            /* TODO interrupt enemies who see me. */
            update_screen();
        }

        int speed = msr_calculate_characteristic_bonus(monster, MSR_CHAR_TOUGHNESS);
        uint32_t cost = MT_ENERGY_TURN - speed;
        if (monster->energy < MT_ENERGY_FULL) {
            cost += MT_ENERGY_FULL - monster->energy;
        }
        return mt_add_event(&e, cost);
    }
    return false;
}

static bool mt_guard_cb(struct mt_event *e) {
    if (e != NULL) {
        /* check surroundings here, and shoot/slash if neccesary */

        if (e->monster->energy >= MT_ENERGY_FULL) {
            /* cancel guard */
            e->controller_cb(e->monster, e->controller);
            return true;
        }

        /* 
           We were probably interrupted to shoot,
           now we wait untill we have enough energy 
           to continue our turn.
         */
        e->cb_do = &mt_cb_to_controller;
    }

    return false;
}

bool mt_do_guard(struct msr_monster *monster, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (cb == NULL) return false;

    struct mt_event  e = {
        .monster = monster,
        .controller = controller,
        .cb_do = &mt_guard_cb,
        .interruptable = true,
        .interrupted = false,
        .controller_cb = cb,
        .item = NULL,
    };
    uint32_t cost = MT_ENERGY_TURN;
    if (monster->energy < MT_ENERGY_FULL) {
        cost += MT_ENERGY_FULL - monster->energy;
    }
    return mt_add_event(&e, cost);
}

static bool mt_wear_cb(struct mt_event *e) {
    if (e != NULL) {
        if (e->interrupted == true) {
            You("Were interrupted while putting on %s.", e->item->ld_name);
            e->monster->energy = MT_ENERGY_FULL;
            e->controller_cb(e->monster, e->controller);
            return true;
        }
        else if (e->monster->energy >= MT_ENERGY_FULL) {
            if (inv_get_item_location(e->monster->inventory, e->item) == INV_LOC_INVENTORY) {
                dw_wear_item(e->monster, e->item);
            }

            e->controller_cb(e->monster, e->controller);
            return true;
        }
    }

    return false;
}

bool mt_do_wear(struct msr_monster *monster, struct itm_item *item, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    if (dw_can_wear_item(monster, item) == false) return false;

    /* we can do something about wear cost here */

    struct mt_event  e = {
        .monster = monster,
        .controller = controller,
        .cb_do = &mt_wear_cb,
        .interruptable = true,
        .interrupted = false,
        .controller_cb = cb,
        .item = NULL,
    };
    uint32_t cost = MT_ENERGY_TURN;
    if (monster->energy < MT_ENERGY_FULL) {
        cost += MT_ENERGY_FULL - monster->energy;
    }
    return mt_add_event(&e, cost);
}

static bool mt_remove_cb(struct mt_event *e) {
    if (e != NULL) {
        if (e->interrupted == true) {
            You("Were interrupted while removing %s.", e->item->ld_name);
            e->monster->energy = MT_ENERGY_FULL;
            e->controller_cb(e->monster, e->controller);
            return true;
        }
        else if (e->monster->energy >= MT_ENERGY_FULL) {
            if (inv_get_item_location(e->monster->inventory, e->item) != INV_LOC_INVENTORY) {
                dw_remove_item(e->monster, e->item);
            }
            e->controller_cb(e->monster, e->controller);
            return true;
        }
    }

    return false;
}

bool mt_do_remove(struct msr_monster *monster, struct itm_item *item, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;
    if (dw_can_remove_item(monster, item) == false) return false;

    /* we can do something about remove cost here */

    struct mt_event e = {
        .monster = monster,
        .controller = controller,
        .cb_do = &mt_remove_cb,
        .interruptable = true,
        .interrupted = false,
        .controller_cb = cb,
        .item = NULL,
    };
    uint32_t cost = MT_ENERGY_TURN;
    if (monster->energy < MT_ENERGY_FULL) {
        cost += MT_ENERGY_FULL - monster->energy;
    }
    return mt_add_event(&e, cost);
}

bool mt_do_use(struct msr_monster *monster, struct itm_item *item, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;

    if (dw_use_item(monster, item) == true) {
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
            .item = NULL,
        };

        uint32_t cost = MT_ENERGY_TURN * item->use_delay;
        if (monster->energy < MT_ENERGY_FULL) {
            cost += MT_ENERGY_FULL - monster->energy;
        }
        return mt_add_event(&e, cost);
    }
    return false;
}

bool mt_do_pickup(struct msr_monster *monster, struct itm_item *item, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;
    if (inv_has_item(monster->inventory, item) == true) return false;

    struct dc_map_entity *me = sd_get_map_me(&monster->pos, gbl_game->current_map);
    if (inv_has_item(me->inventory, item) == false) return false;

    if (msr_give_item(monster, item) == true) {
        inv_remove_item(me->inventory, item);
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
            .item = NULL,
        };
        uint32_t cost = MT_ENERGY_TURN;
        if (monster->energy < MT_ENERGY_FULL) {
            cost += MT_ENERGY_FULL - monster->energy;
        }
        return mt_add_event(&e, cost);
    }
    return false;
}

bool mt_do_drop(struct msr_monster *monster, struct itm_item *item, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if ( (inv_get_item_location(monster->inventory, item) == INV_LOC_MAINHAND_WIELD) ||
         (inv_get_item_location(monster->inventory, item) == INV_LOC_OFFHAND_WIELD) ) {
        /*
           Allow drop of weapons to the ground for free.
         */
        if (msr_remove_item(monster, item) == true) {
            return itm_insert_item(item, gbl_game->current_map, &monster->pos);
        }
        return false;
    }
    else if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;

    if (msr_remove_item(monster, item) == true) {
        itm_insert_item(item, gbl_game->current_map, &monster->pos);
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
            .item = NULL,
        };

        uint32_t cost = MT_ENERGY_TURN;
        if (monster->energy < MT_ENERGY_FULL) {
            cost += MT_ENERGY_FULL - monster->energy;
        }
        return mt_add_event(&e, cost);
    }
    return false;
}

bool mt_do_fire(struct msr_monster *monster, coord_t *pos, mt_callback_t cb, void *controller) {
    if (monster == NULL) return false;
    if (pos == NULL) return false;
    if (cb == NULL) return false;
    if (monster->energy < MT_ENERGY_FULL) return false;

    if (fght_shoot(gbl_game->game_random, monster, gbl_game->current_map, pos) == true) {
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
            .item = NULL,
        };

        uint32_t cost = MT_ENERGY_TURN;
        if (monster->energy < MT_ENERGY_FULL) {
            cost += MT_ENERGY_FULL - monster->energy;
        }
        return mt_add_event(&e, cost);
    }
    return false;
}

static bool mt_has_ammo(struct msr_monster *monster, struct itm_item *item) {
    /* TODO check for ammo in inventory */
    return true;
}

static struct itm_item *mt_get_weapon_to_reload(struct msr_monster *monster) {
    if (monster == NULL) return NULL;
    struct inv_inventory *inv = monster->inventory;
    if (inv_loc_empty(inv,INV_LOC_MAINHAND_WIELD) && inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) ) return NULL;
    struct itm_item *item = NULL;
    struct item_weapon_specific *wpn = NULL;
    bool found = false;

    if ( (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
         (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND)  ||
         (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
        item = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
        if (wpn_is_type(item, WEAPON_TYPE_RANGED) != false) {
            wpn = &item->specific.weapon;
            if (wpn->magazine_left < wpn->magazine_sz) {
                if (mt_has_ammo(monster, item) == true ) {
                    found = true;
                }
            }
        }
    }

    if (found == false) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (wpn_is_type(item, WEAPON_TYPE_RANGED) != false) {
                wpn = &item->specific.weapon;
                if (wpn->magazine_left < wpn->magazine_sz) {
                    if (mt_has_ammo(monster, item) == true ) {
                        found = true;
                    }
                }
            }
        }
    }
    if (found == false) return NULL;
    return item;
}

static bool mt_reload_cb(struct mt_event *e) {
    if (e != NULL) {
        if (e->interrupted == true) {
            You("Were interrupted while reloading %s.", e->item->ld_name);
            e->monster->energy = MT_ENERGY_FULL;
            e->controller_cb(e->monster, e->controller);
            return true;
        }
        else if (e->monster->energy >= MT_ENERGY_FULL) {
            /* Try to continue with the next one */
            e->item->specific.weapon.magazine_left = e->item->specific.weapon.magazine_sz;

            struct itm_item *item = mt_get_weapon_to_reload(e->monster);
            if (item != NULL) {
                uint32_t cost = MT_ENERGY_TURN * item->use_delay;
                e->monster->energy -= cost;
                e->item = item;
                return false; 
            }

            /* if not, return to controller */
            e->controller_cb(e->monster, e->controller);
            return true;
        }
    }

    return false;
}

bool mt_do_reload(struct msr_monster *monster, mt_callback_t cb, void *controller) {
    struct itm_item *item = mt_get_weapon_to_reload(monster);
    if (item == NULL) return false;

    struct mt_event  e = {
        .monster = monster,
        .controller = controller,
        .cb_do = &mt_reload_cb,
        .interruptable = false,
        .interrupted = false,
        .controller_cb = cb,
        .item = item,
    };

    uint32_t cost = MT_ENERGY_TURN * item->use_delay;
    if (monster->energy < MT_ENERGY_FULL) {
        cost += MT_ENERGY_FULL - monster->energy;
    }
    return mt_add_event(&e, cost);
}

