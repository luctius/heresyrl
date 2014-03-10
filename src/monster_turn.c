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

bool mt_process() {
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
    if (monster->energy < MT_ENERGY_FULL) return false;
    coord_t oldpos = monster->pos;

    if (msr_move_monster(monster, gbl_game->current_map, pos) == true) {
        struct mt_event  e = {
            .monster = monster,
            .controller = controller,
            .cb_do = &mt_cb_to_controller,
            .interruptable = false,
            .interrupted = false,
            .controller_cb = cb,
        };
        if ( (sd_get_map_me(&oldpos, gbl_game->current_map)->visible == true) || 
             (sd_get_map_me(pos, gbl_game->current_map)->visible == true) ) {
            update_screen();
        }

        int speed = msr_calculate_characteristic_bonus(monster, MSR_CHAR_TOUGHNESS);
        return mt_add_event(&e, MT_ENERGY_TURN - speed);
    }
    return false;
}

static bool mt_guard_cb(struct mt_event *e) {
    if (e != NULL) {
        /* check surroundings here, and shoot/slash if neccesary */

        if (e->monster->energy >= 0) {
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
    if (monster->energy < MT_ENERGY_FULL) return false;

    struct mt_event  e = {
        .monster = monster,
        .controller = controller,
        .cb_do = &mt_guard_cb,
        .interruptable = true,
        .interrupted = false,
        .controller_cb = cb,
    };
    return mt_add_event(&e, MT_ENERGY_TURN);
}

