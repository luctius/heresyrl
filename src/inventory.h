#ifndef INVENTORY_H
#define INVENTORY_H

#include "heresyrl_def.h"

struct inv_inventory;

enum inv_locations {
    INV_LOC_NONE,
    INV_LOC_INVENTORY,
    INV_LOC_FEET,
    INV_LOC_LEGS,
    INV_LOC_CHEST,
    INV_LOC_ARMS,
    INV_LOC_HANDS,
    INV_LOC_LEFT_RING,
    INV_LOC_RIGHT_RING,
    INV_LOC_OFFHAND_WIELD,
    INV_LOC_MAINHAND_WIELD,
    INV_LOC_HEAD,
    INV_LOC_FACE,
    INV_LOC_BACK,
    INV_LOC_MAX,
};

#define inv_loc(loc) (1<<loc)

struct inv_inventory *inv_init(bitfield_t locations);
void inv_exit(struct inv_inventory *inv);
bool inv_verify_inventory(struct inv_inventory *inv);

bool inv_has_item(struct inv_inventory *inv, struct itm_item *item);
bool inv_add_item(struct inv_inventory *inv, struct itm_item *item); /*Internal use only !!*/
bool inv_remove_item(struct inv_inventory *inv, struct itm_item *item); /*Internal use only!!*/
struct itm_item *inv_get_next_item(struct inv_inventory *inv, struct itm_item *prev);
int inv_inventory_size(struct inv_inventory *inv);

bool inv_support_location(struct inv_inventory *inv, enum inv_locations location);
bool inv_move_item_to_location(struct inv_inventory *inv, struct itm_item *item, enum inv_locations location);
struct itm_item *inv_get_item_from_location(struct inv_inventory *inv, enum inv_locations location);
bool inv_loc_empty(struct inv_inventory *inv, enum inv_locations location);

bool inv_item_worn(struct inv_inventory *inv, struct itm_item *item); /* worn items include wielded items, but not vice versa. */
bool inv_item_wielded(struct inv_inventory *inv, struct itm_item *item);

const char *inv_location_name(enum inv_locations loc);

#define inv_loc_human \
    ( inv_loc(INV_LOC_FEET)            | inv_loc(INV_LOC_LEGS)          | \
      inv_loc(INV_LOC_CHEST)           | inv_loc(INV_LOC_ARMS)          | \
      inv_loc(INV_LOC_HANDS)           | inv_loc(INV_LOC_LEFT_RING)     | \
      inv_loc(INV_LOC_RIGHT_RING)      | inv_loc(INV_LOC_OFFHAND_WIELD) | \
      inv_loc(INV_LOC_MAINHAND_WIELD)  | inv_loc(INV_LOC_HEAD)          | \
      inv_loc(INV_LOC_FACE)            | inv_loc(INV_LOC_BACK)          | \
      inv_loc(INV_LOC_INVENTORY) )

#define inv_loc_tile \
    ( inv_loc(INV_LOC_INVENTORY) )

#endif /* INVENTORY_H */
