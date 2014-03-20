#ifndef INVENTORY_H
#define INVENTORY_H

#include "heresyrl_def.h"

struct inv_inventory;

enum inv_locations {
    INV_LOC_NONE            = (0),
    INV_LOC_INVENTORY       = (1<<1),
    INV_LOC_FEET            = (1<<2),
    INV_LOC_LEGS            = (1<<3),
    INV_LOC_CHEST           = (1<<4),
    INV_LOC_ARMS            = (1<<5),
    INV_LOC_HANDS           = (1<<6),
    INV_LOC_LEFT_RING       = (1<<7),
    INV_LOC_RIGHT_RING      = (1<<8),
    INV_LOC_OFFHAND_WIELD   = (1<<9),
    INV_LOC_MAINHAND_WIELD  = (1<<10),
    INV_LOC_HEAD            = (1<<11),
    INV_LOC_FACE            = (1<<12),
    INV_LOC_BACK            = (1<<13),
    INV_LOC_CREATURE_WIELD1 = (1<<14),
    INV_LOC_MAX             = (1<<14)+1,
};

struct inv_inventory *inv_init(bitfield_t locations);
void inv_exit(struct inv_inventory *inv);
bool inv_verify_inventory(struct inv_inventory *inv);

bool inv_has_item(struct inv_inventory *inv, struct itm_item *item);
bool inv_add_item(struct inv_inventory *inv, struct itm_item *item); /*Internal use only !!*/
bool inv_remove_item(struct inv_inventory *inv, struct itm_item *item); /*Internal use only!!*/
struct itm_item *inv_get_next_item(struct inv_inventory *inv, struct itm_item *prev);
int inv_inventory_size(struct inv_inventory *inv);

bool inv_support_location(struct inv_inventory *inv, bitfield_t location);
bool inv_move_item_to_location(struct inv_inventory *inv, struct itm_item *item, bitfield_t location);
struct itm_item *inv_get_item_from_location(struct inv_inventory *inv, bitfield_t location);
bool inv_loc_empty(struct inv_inventory *inv, bitfield_t location);
bitfield_t inv_get_item_locations(struct inv_inventory *inv, struct itm_item *item);

bool inv_item_worn(struct inv_inventory *inv, struct itm_item *item); /* worn items include wielded items, but not vice versa. */
bool inv_item_wielded(struct inv_inventory *inv, struct itm_item *item);

const char *inv_location_name(bitfield_t loc);

#define inv_loc_human \
    ( INV_LOC_FEET            | INV_LOC_LEGS          | \
      INV_LOC_CHEST           | INV_LOC_ARMS          | \
      INV_LOC_HANDS           | INV_LOC_LEFT_RING     | \
      INV_LOC_RIGHT_RING      | INV_LOC_OFFHAND_WIELD | \
      INV_LOC_MAINHAND_WIELD  | INV_LOC_HEAD          | \
      INV_LOC_FACE            | INV_LOC_BACK          | \
      INV_LOC_INVENTORY       | INV_LOC_CREATURE_WIELD1 )

#define inv_loc_animal \
    ( INV_LOC_INVENTORY       | INV_LOC_CREATURE_WIELD1 )

#define inv_loc_tile \
    ( INV_LOC_INVENTORY)

#endif /* INVENTORY_H */
