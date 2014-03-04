#ifndef INVENTORY_H
#define INVENTORY_H

#include "heresyrl_def.h"

struct inv_inventory;

enum inv_locations {
    INV_LOC_INVENTORY,
    INV_LOC_FEET,
    INV_LOC_LEGS,
    INV_LOC_CHEST,
    INV_LOC_SHOULDERS,
    INV_LOC_ARMS,
    INV_LOC_HANDS,
    INV_LOC_LEFT_RING,
    INV_LOC_RIGHT_RING,
    INV_LOC_LEFT_WIELD,
    INV_LOC_RIGHT_WIELD,
    INV_LOC_HEAD,
    INV_LOC_FACE,
    INV_LOC_BACK,
    INV_LOC_ARMOUR_CHEST,
    INV_LOC_MAX,
};

#define inv_loc(loc) (1<<loc)

struct inv_inventory *inv_init(uint32_t locations);
void inv_exit(struct inv_inventory *inv);
bool inv_has_item(struct inv_inventory *inv, struct itm_items *item);
bool inv_add_item(struct inv_inventory *inv, struct itm_items *item);
bool inv_remove_item(struct inv_inventory *inv, struct itm_items *item);
struct itm_items *inv_get_next_item(struct inv_inventory *inv, struct itm_items *prev);

bool inv_support_location(struct inv_inventory *inv, enum inv_locations location);
bool inv_move_item_to_location(struct inv_inventory *inv, struct itm_items *item, enum inv_locations location);
struct itm_items *inv_get_item_from_location(struct inv_inventory *inv, enum inv_locations location);

#define inv_loc_human \
    ( inv_loc(INV_LOC_FEET)       | inv_loc(INV_LOC_LEGS)         | \
      inv_loc(INV_LOC_CHEST)      | inv_loc(INV_LOC_SHOULDERS)    | \
      inv_loc(INV_LOC_ARMS)       | inv_loc(INV_LOC_HANDS)        | \
      inv_loc(INV_LOC_LEFT_RING)  | inv_loc(INV_LOC_RIGHT_RING)   | \
      inv_loc(INV_LOC_LEFT_WIELD) | inv_loc(INV_LOC_RIGHT_WIELD)  | \
      inv_loc(INV_LOC_HEAD)       | inv_loc(INV_LOC_FACE)         | \
      inv_loc(INV_LOC_BACK)       | inv_loc(INV_LOC_ARMOUR_CHEST) | \
      inv_loc(INV_LOC_INVENTORY) )

#define inv_loc_tile \
    ( inv_loc(INV_LOC_INVENTORY) )

#endif /* INVENTORY_H */
