#include <stdlib.h>
#include <sys/queue.h>

#include "inventory.h"
#include "items.h"

struct inv_entry {
    enum inv_locations location;
    struct itm_item *item;
    LIST_ENTRY(inv_entry) entries;
};

struct inv_inventory {
    uint16_t available_locations;
    LIST_HEAD(invhead, inv_entry) head;
};

struct inv_inventory *inv_init(uint32_t locations) {
    struct inv_inventory *i= calloc(1, sizeof(struct inv_inventory) );
    if (i != NULL) {
        i->available_locations = locations;
    }
    return i;
}

void inv_exit(struct inv_inventory *inv) {
    if (inv == NULL) return;

    struct inv_entry *ie;
    while ( (ie = inv->head.lh_first ) != NULL) {
        itm_destroy(ie->item);
        LIST_REMOVE(inv->head.lh_first, entries);
        free(ie);
    }
    free(inv);
}

struct itm_item *inv_get_next_item(struct inv_inventory *inv, struct itm_item *prev) {
    if (inv == NULL) return NULL;
    if (prev == NULL) {
        if (inv->head.lh_first != NULL) return inv->head.lh_first->item;
        return NULL;
    }
    struct inv_entry *ie = inv->head.lh_first;

    while (ie != NULL) {
        if (ie->item == prev) return ie->entries.le_next->item;
        ie = ie->entries.le_next;
    }
    return NULL;
}

bool inv_has_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv == NULL) return NULL;
    if (item == NULL) return NULL;

    struct itm_item *i = NULL;
    while ( (i = inv_get_next_item(inv, i) ) != NULL ) {
        if (i == item) return true;
    }

    return false;
}

bool inv_add_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv == NULL) return NULL;
    if (item == NULL) return NULL;
    if (inv_has_item(inv, item) == true ) return false;

    struct inv_entry *ie = malloc(sizeof(struct inv_entry) );
    if (ie == NULL) return false;

    ie->location = INV_LOC_INVENTORY;
    ie->item = item;
    LIST_INSERT_HEAD(&inv->head, ie, entries);

    return true;
}

bool inv_remove_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv == NULL) return NULL;
    if (item == NULL) return NULL;
    if (inv_has_item(inv, item) == false) return false;

    struct inv_entry *ie = inv->head.lh_first;

    while (ie != NULL) {
        if (ie->item == item) {
            LIST_REMOVE(inv->head.lh_first, entries);
            free(ie);
            return true;
        }
        ie = ie->entries.le_next;
    }

    return false;
}

bool inv_support_location(struct inv_inventory *inv, enum inv_locations location) {
    if (inv == NULL) return false;
    if (location > INV_LOC_MAX) return false;
    if ( (inv->available_locations & inv_loc(location) ) > 0) return true;
    return false;
}

bool inv_move_item_to_location(struct inv_inventory *inv, struct itm_item *item, enum inv_locations location) {
    if (inv == NULL) return false;
    if (item == NULL) return false;
    if (inv_support_location(inv, location) == false) return false;
    if (inv_has_item(inv, item) == false) return false;

    struct inv_entry *ie = inv->head.lh_first;

    while (ie != NULL) {
        if (ie->item == item) {
            ie->location = location;
            return true;
        }
        ie = ie->entries.le_next;
    }
    return false;
}

struct itm_item *inv_get_item_from_location(struct inv_inventory *inv, enum inv_locations location) {
    if (inv == NULL) return NULL;
    if (inv_support_location(inv, location) == false) return NULL;

    struct inv_entry *ie = inv->head.lh_first;

    while (ie != NULL) {
        if (ie->location == location) {
            return ie->item;
        }
        ie = ie->entries.le_next;
    }
    return NULL;
}


