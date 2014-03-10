#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "event_sys.h"

#define ENERGY_MAX (UINT32_MAX - 1000)
#define ENERGY_TICK (10)

struct es_event_entry {
    struct es_event event;
    LIST_ENTRY(es_event_entry) entries;
};

static LIST_HEAD(es_event_list, es_event_entry) head;

void es_init(void) {
    LIST_INIT(&sys->head);
}

void es_exit(void) {
    struct es_event_entry  *eve;
    while ( (eve = head.lh_first ) != NULL) {
        LIST_REMOVE(head.lh_first, entries);
        free(eve);
    }
}

bool es_add_event(struct es_event *event, int cost) {
    if (event == NULL) return false;
    if (event->ev_set == NULL) return false;
    if (event->ev_get == NULL) return false;
    if (event->ev_do  == NULL) return false;
    if (event->ev_get(event->dataptr) < ENERGY_MAX) return false;
    if ( (event->ev_get(event->dataptr) - cost) > ENERGY_MAX) {
        event->ev_do(event->dataptr, event->controller, false);
        return true;
    }

    struct es_event_entry *eve = calloc(1, sizeof(struct es_event_entry));
    if (eve == NULL) return false;

    LIST_INSERT_HEAD(&head, eve, entries);
    memcpy(&eve->event, event, sizeof(struct es_event));
    eve->event.interrupted = false;
    eve->event.ev_set(eve->event.dataptr, event->ev_get(event->dataptr) - cost);

    return true;
}

bool es_remove_event(struct es_event *event) {
    if (event == NULL) return false;
    struct es_event_entry *eve = head.lh_first;
    if (eve == NULL) return false;
    struct es_event_entry *ev_remove = container_of(event, struct es_event_entry, event);

    LIST_REMOVE(ev_remove, entries);
    free(ev_remove);
    return true;
}

bool es_interrupt_event(void *dataptr_to_interrupt) {
    struct es_event_entry *eve = head.lh_first;
    if (eve == NULL) return false;

    while (eve != NULL) {
        if (eve->event.dataptr == dataptr_to_interrupt) {
            if (eve->event.interruptable == true) eve->event.interrupted = true;
            return true;
        }
        
        eve = eve->entries.le_next;
    }
    return false;
}

bool es_process() {
    struct es_event_entry *eve = head.lh_first;

    for (eve = head.lh_first; eve != NULL; eve = eve->entries.le_next) {
        struct es_event *event = &eve->event;
        bool do_action = false;

        if (event->interrupted == true) do_action = true;

        if (event->ev_get(event->dataptr) >= ENERGY_MAX) {
            do_action = true;
            event->interrupted = false;
        }
        else {
            event->ev_set(event->dataptr, ENERGY_TICK);
        }

        if (do_action == true) {
            event->ev_do(event->dataptr, event->controller, event->interrupted);
            es_remove_event(event);
        }
    }
    return true;
}

