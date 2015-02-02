#include <assert.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <string.h>

#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"

#define inv_loc(loc) (1<<loc)

struct inv_entry {
    bitfield32_t location;
    struct itm_item *item;
    TAILQ_ENTRY(inv_entry) entries;
};

struct inv_inventory {
    uint32_t inv_pre;

    bitfield32_t available_locations;
    TAILQ_HEAD(invhead, inv_entry) head;

    uint32_t inv_post;
};

/* Garanteed to be random, rolled it myself ;)  */
#define INVENTORY_PRE_CHECK (16524)
#define INVENTORY_POST_CHECK (411)

struct inv_inventory *inv_init(bitfield32_t locations) {
    struct inv_inventory *i= calloc(1, sizeof(struct inv_inventory) );
    if (i != NULL) {
        TAILQ_INIT(&i->head);
        i->available_locations = locations;

        i->inv_pre = INVENTORY_PRE_CHECK;
        i->inv_post = INVENTORY_POST_CHECK;
    }
    return i;
}

void inv_exit(struct inv_inventory *inv) {
    if (inv_verify_inventory(inv) == false) return;

    struct inv_entry *ie;
    while ( (ie = inv->head.tqh_first ) != NULL) {
        itm_destroy(ie->item);
        TAILQ_REMOVE(&inv->head, inv->head.tqh_first, entries);
        free(ie);
    }
    free(inv);
}

bool inv_verify_inventory(struct inv_inventory *inv) {
    assert(inv != NULL);
    assert(inv->inv_pre == INVENTORY_PRE_CHECK);
    assert(inv->inv_post == INVENTORY_POST_CHECK);

    return true;
}

struct itm_item *inv_get_next_item(struct inv_inventory *inv, struct itm_item *prev) {
    if (inv_verify_inventory(inv) == false) return NULL;
    if (inv->head.tqh_first == NULL) return NULL;
    struct inv_entry *ie = inv->head.tqh_first;

    struct itm_item *item = NULL;
    bitfield32_t item_location = 0;

    if (prev == NULL) { 
        item = ie->item; /* if this is the first attempt take the first item */ 
        item_location =ie->location;
    }

    while ( (ie != NULL) && (item == NULL) ) {
        if (ie->item == prev) { /*return next item */
            if (ie->entries.tqe_next != NULL) {
                item = ie->entries.tqe_next->item;
                item_location = ie->entries.tqe_next->location;
                break;
            }
        }

        ie = ie->entries.tqe_next;
    }

    if (item != NULL) {
        if (item->template_id == IID_HUMAN_UNARMED) {
            if ( (item_location & INV_LOC_CREATURE_WIELD1) > 0) { 
                /* HACK: ignore creature wields (atleast for humans) */
                item = inv_get_next_item(inv, item);
            }
        }
    }

    return item;
}

void inv_disable_location(struct inv_inventory *inv, bitfield32_t loc) {
    if (inv_verify_inventory(inv) == false) return;
    if (inv_support_location(inv, loc) == false) return;

    inv->available_locations &= (~loc);
}

void inv_enable_location(struct inv_inventory *inv, bitfield32_t loc) {
    if (inv_verify_inventory(inv) == false) return;
    if (inv_support_location(inv, loc) == true) return;

    inv->available_locations |= (loc);
}

bool inv_has_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        if  (ie->item == item) {
            return true;
        }
        ie = ie->entries.tqe_next;
    }

    return false;
}

bool inv_add_stack(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (item->max_quantity == 0) return false;
    if (item->stacked_quantity == 0) return false;

    /*
       We want to check if we can add some of this item to one allready in the inventory.
     */

    struct itm_item *i = NULL;
    while ( (i = inv_get_next_item(inv, i) ) != NULL ) {
        if (i->template_id == item->template_id) {
            if (i->stacked_quantity < i->max_quantity) {
                if (itm_stack_compatible(i, item) == true) {
                    int space_left = (i->max_quantity - i->stacked_quantity);
                    int min = MIN(space_left, item->stacked_quantity);
                    i->stacked_quantity += min;
                    item->stacked_quantity -= min;
                }
            }
        }
        if (item->stacked_quantity == 0) return true;
    }

    return false;
}

bool inv_add_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(inv, item) == true ) return false;
    if (inv_add_stack(inv, item) == true) return true;

    struct inv_entry *ie = malloc(sizeof(struct inv_entry) );
    if (ie == NULL) return false;

    ie->location = INV_LOC_INVENTORY;
    ie->item = item;
    TAILQ_INSERT_TAIL(&inv->head, ie, entries);

    return true;
}

bool inv_remove_item(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(inv, item) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;

    while (ie != NULL) {
        if (ie->item == item) {
            TAILQ_REMOVE(&inv->head, ie, entries);
            free(ie);
            return true;
        }
        ie = ie->entries.tqe_next;
    }

    return false;
}

int inv_inventory_size(struct inv_inventory *inv) {
    if (inv_verify_inventory(inv) == false) return -1;
    int sz = 0;

    struct itm_item *i = NULL;
    while ( (i = inv_get_next_item(inv, i) ) != NULL ) { sz++; }
    return sz;
}

bool inv_support_location(struct inv_inventory *inv, bitfield32_t location) {
    if (inv_verify_inventory(inv) == false) return false;
    if (location > INV_LOC_MAX) return false;
    if ( (inv->available_locations & location) > 0) return true;
    return false;
}

bool inv_move_item_to_location(struct inv_inventory *inv, struct itm_item *item, bitfield32_t location) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(inv, item) == false) return false;

    for (int i = 0; i < INV_LOC_MAX; i++) {
        if ( (location & inv_loc(i) ) > 0) {
            if (inv_support_location(inv, inv_loc(i) ) == false) return false;
            if (inv_loc_empty(inv, inv_loc(i) ) == false) return false;
        }
    }

    struct inv_entry *ie = inv->head.tqh_first;

    while (ie != NULL) {
        if (ie->item == item) {
            ie->location = location;
            return true;
        }
        ie = ie->entries.tqe_next;
    }
    return false;
}

struct itm_item *inv_get_item_from_location(struct inv_inventory *inv, bitfield32_t location) {
    if (inv_verify_inventory(inv) == false) return NULL;
    if (inv_support_location(inv, location) == false) return NULL;

    struct inv_entry *ie = inv->head.tqh_first;

    while (ie != NULL) {
        if ( (ie->location & location) > 0) {
            return ie->item;
        }
        ie = ie->entries.tqe_next;
    }
    return NULL;
}

bool inv_loc_empty(struct inv_inventory *inv, bitfield32_t location) {
    if (inv_verify_inventory(inv) == false) return false;
    if (inv_support_location(inv, location) == false) return false;
    if (location == INV_LOC_INVENTORY) return true; /*Inventory is the base where everything can go */

    return (inv_get_item_from_location(inv, location) == NULL);
}

bitfield32_t inv_get_item_locations(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return INV_LOC_NONE;
    if (itm_verify_item(item) == false) return INV_LOC_NONE;
    if (inv_has_item(inv, item) == false) return INV_LOC_NONE;
    
    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        if (ie->item == item) {
            return ie->location;
        }
        ie = ie->entries.tqe_next;
    }
    return INV_LOC_NONE;
}

/* return true if this item is in another location than INV_LOC_INVENTORY */
bool inv_item_worn(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(inv, item) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        if (ie->item == item) {
            if ( (ie->location & (~INV_LOC_INVENTORY) ) > 0) {
                return true;
            }
        }
        ie = ie->entries.tqe_next;
    }
    return false;
}

/* return true if the weapon is in either the main or the off hand */
bool inv_item_wielded(struct inv_inventory *inv, struct itm_item *item) {
    if (inv_verify_inventory(inv) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_worn(inv, item) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        if (ie->item == item) {
            if ( (ie->location & (INV_LOC_OFFHAND_WIELD | INV_LOC_MAINHAND_WIELD) ) > 0) return true;
        }
        ie = ie->entries.tqe_next;
    }
    return false;
}

bool inv_wields_weapon_with_spcqlty(struct inv_inventory *inv, enum weapon_special_quality spcqlty) {
    if (inv_verify_inventory(inv) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        struct itm_item *i = ie->item;
        if (inv_item_wielded(inv, i) ) {
            if (itm_is_type(i, ITEM_TYPE_WEAPON) ) {
                if (wpn_has_spc_quality(i, spcqlty) ) return true;
            }
        }
        ie = ie->entries.tqe_next;
    }
    return false;
}

bool inv_wears_wearable_with_spcqlty(struct inv_inventory *inv, enum wearable_special_quality spcqlty) {
    if (inv_verify_inventory(inv) == false) return false;

    struct inv_entry *ie = inv->head.tqh_first;
    while (ie != NULL) {
        struct itm_item *i = ie->item;
        if (inv_item_worn(inv, i) ) {
            if (itm_is_type(i, ITEM_TYPE_WEARABLE) ) {
                if (wbl_has_spc_quality(i, spcqlty) ) return true;
            }
        }
        ie = ie->entries.tqe_next;
    }
    return false;
}

static const char *location_name_lst[] = {
    [INV_LOC_NONE] = "",
    [INV_LOC_INVENTORY] = "",
    [INV_LOC_FEET] = "feet",
    [INV_LOC_LEGS] = "legs",
    [INV_LOC_BODY] = "body",
    [INV_LOC_ARMS] = "arms",
    [INV_LOC_HANDS] = "hands",
    [INV_LOC_LEFT_RING] = "lring",
    [INV_LOC_RIGHT_RING] = "rring",
    [INV_LOC_OFFHAND_WIELD] = "lhand",
    [INV_LOC_MAINHAND_WIELD] = "rhand",
    [INV_LOC_HEAD] = "head",
    [INV_LOC_FACE] = "face",
    [INV_LOC_BACK] = "back",
    [INV_LOC_CREATURE_WIELD1] = "",
    [INV_LOC_MAX] = "",
};

const char *inv_location_name(bitfield32_t loc) {
    if (loc >= INV_LOC_MAX) return NULL;
    for (int i = 0; inv_loc(i) < INV_LOC_MAX; i++) {
        if ( (loc & inv_loc(i) ) > 0) {
            return location_name_lst[loc];
        }
    }
    return NULL;
}

