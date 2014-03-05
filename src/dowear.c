#include "monster.h"
#include "items.h"
#include "inventory.h"

static bool wield_melee_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (wpn_is_type(item, WEAPON_TYPE_MELEE) == false) return false;
    struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;
    
    enum inv_locations location = INV_LOC_RIGHT_WIELD;
    if (weapon->weapon_category == WEAPON_CATEGORY_2H_MELEE) {
        location = INV_LOC_BOTH_WIELD;
        if (inv_support_location(inv, INV_LOC_RIGHT_WIELD) == false) {
            You("do not have two hands.");
        }
    }

    if ( (inv_loc_empty(inv, INV_LOC_RIGHT_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_RIGHT_WIELD) == true) ) {
        location = INV_LOC_RIGHT_WIELD;
    }
    else if ( (inv_loc_empty(inv, INV_LOC_LEFT_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_LEFT_WIELD) == true) ) {
        location = INV_LOC_LEFT_WIELD;
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

static bool wield_ranged_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;
    
    enum inv_locations location = INV_LOC_RIGHT_WIELD;
    if (wpn_is_catergory(item, WEAPON_CATEGORY_BASIC) || wpn_is_catergory(item, WEAPON_CATEGORY_HEAVY) ) {
        location = INV_LOC_BOTH_WIELD;
        if (inv_support_location(inv, INV_LOC_RIGHT_WIELD) == false) {
            You("do not have two hands.");
        }
    }

    if ( (inv_loc_empty(inv, INV_LOC_RIGHT_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_RIGHT_WIELD) == true) ) {
        location = INV_LOC_RIGHT_WIELD;
    }
    else if ( (inv_loc_empty(inv, INV_LOC_LEFT_WIELD) == true) &&
            (inv_support_location(inv, INV_LOC_LEFT_WIELD) == true) ) {
        location = INV_LOC_LEFT_WIELD;
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

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: return false; break;
        case ITEM_TYPE_AMMO: return false; break;
        case ITEM_TYPE_TOOL: return false; break;
        case ITEM_TYPE_WEARABLE:
                 break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) return wield_ranged_weapon(monster, item);
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) ) return wield_melee_weapon(monster, item);
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) ) return false;
                 if (wpn_is_type(item, WEAPON_TYPE_CREATURE) ) return false; /*TODO add wield_creature_weapon*/
                 break;
        default: break;
    }
    return false;
}
