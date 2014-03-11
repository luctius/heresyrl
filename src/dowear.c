#include "monster.h"
#include "items.h"
#include "inventory.h"

static bool wield_melee_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_MELEE) == false) return false;
    struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;
    
    enum inv_locations location = INV_LOC_MAINHAND_WIELD;
    if (weapon->weapon_category == WEAPON_CATEGORY_2H_MELEE) {
        location = INV_LOC_BOTH_WIELD;
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) {
            You("do not have two hands.");
        }
    }

    if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
        location = INV_LOC_MAINHAND_WIELD;
    }
    else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
        location = INV_LOC_OFFHAND_WIELD;
    }
    else {
        You("have no hands free.");
        return false;
    }

    if (inv_move_item_to_location(inv, item, location) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "Could not move %s to the correct location, bailing.", item->ld_name);
        You("seem to be unable to wield this weapon.");
        return false;
    }

    return true;
}

static bool wield_ranged_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;
    
    enum inv_locations location = INV_LOC_MAINHAND_WIELD;
    if (wpn_is_catergory(item, WEAPON_CATEGORY_BASIC) || wpn_is_catergory(item, WEAPON_CATEGORY_HEAVY) ) {
        location = INV_LOC_BOTH_WIELD;
        if (inv_support_location(inv, INV_LOC_BOTH_WIELD) == false) {
            You("do not have two hands.");
        }
    }

    if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
        location = INV_LOC_MAINHAND_WIELD;
    }
    else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
        location = INV_LOC_OFFHAND_WIELD;
    }
    else {
        You("have no hands free.");
        return false;
    }

    if (inv_move_item_to_location(inv, item, location) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "Could not move %s to the correct location, bailing.", item->ld_name);
        You("seem to be unable to wield this weapon.");
    }

    return true;
}

bool dw_wear_item(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    bool retval = false;

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: retval = false; break;
        case ITEM_TYPE_AMMO: retval = false; break;
        case ITEM_TYPE_TOOL: retval = false; break;
        case ITEM_TYPE_WEARABLE:
                 break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) )   retval = wield_ranged_weapon(monster, item); break;
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) )    retval = wield_melee_weapon(monster, item);  break;
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) )   retval = false;
                 if (wpn_is_type(item, WEAPON_TYPE_CREATURE) ) retval = false; /*TODO add wield_creature_weapon*/
                 break;
        default: break;
    }

    /* Do check here */
    if (msr_weapons_check(monster) == false) msr_weapon_next_selection(monster);

    return retval;
}

static bool dw_remove_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;
    //struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;

    if (inv_move_item_to_location(inv, item, INV_LOC_INVENTORY) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "Could not move %s to the correct location, bailing.", item->ld_name);
        You("are to be unable to remove this weapon.");
    }
    return false;
}

bool dw_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;
    bool retval = false;

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: retval = false; break;
        case ITEM_TYPE_AMMO: retval = false; break;
        case ITEM_TYPE_TOOL: retval = false; break;
        case ITEM_TYPE_WEARABLE:
                 break;
        case ITEM_TYPE_WEAPON:
                 retval= dw_remove_weapon(monster, item);
                 break;
        default: break;
    }
    /* Do checks here */
    if (msr_weapons_check(monster) == false) msr_weapon_next_selection(monster);

    return retval;
}

bool dw_use_item(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (monster->inventory == NULL) {
        Your("inventory is empty.");
        return false;
    }
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (item->item_type == ITEM_TYPE_TOOL && item->specific.tool.tool_type == TOOL_TYPE_LIGHT) {
        if (item->specific.tool.lit == false) {
            item->specific.tool.lit = true;
            You("light %s.", item->ld_name);
        }
        else {
            item->specific.tool.lit = false;
            You("douse %s.", item->ld_name);
        }
    }
    return true;
}

bool dw_can_wear_item(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    bool retval = false;

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: retval = false; break;
        case ITEM_TYPE_AMMO: retval = false; break;
        case ITEM_TYPE_TOOL: retval = false; break;
        case ITEM_TYPE_WEARABLE: retval = true; break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) )   retval = true; break;
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) )    retval = true; break;
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) )   retval = false;
                 if (wpn_is_type(item, WEAPON_TYPE_CREATURE) ) 
                    /*TODO add checks here*/
                    retval = false; break;
        default: break;
    }

    return retval;
}

bool dw_can_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;

    /* TODO add remove checks here*/

    return true;
}

