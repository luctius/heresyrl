#include <assert.h>

#include "monster.h"
#include "items.h"
#include "inventory.h"
#include "game.h"
#include "dungeon_creator.h"

static bool wield_melee_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == true) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_MELEE) == false) return false;
    struct inv_inventory *inv = monster->inventory;
    
    bitfield_t location = inv_loc(INV_LOC_NONE);
    if (wpn_is_catergory(item, WEAPON_CATEGORY_2H_MELEE) ) {
        if ( (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) ||
             (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) ) {
            You(monster, "do not have two hands.");
            return false;
        }
        location = inv_loc(INV_LOC_MAINHAND_WIELD) | inv_loc(INV_LOC_OFFHAND_WIELD);
    } else {
        if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
            location = inv_loc(INV_LOC_MAINHAND_WIELD);
        }
        else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
            location = inv_loc(INV_LOC_OFFHAND_WIELD);
        }
        else {
            You(monster, "have no hands free.");
            return false;
        }
    }

    if (inv_move_item_to_location(inv, item, location) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "dw", "Could not move %s to the correct location, bailing.", item->ld_name);
        You(monster, "seem to be unable to wield this weapon.");
        return false;
    }

    return true;
}

static bool wield_ranged_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == true) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    struct inv_inventory *inv = monster->inventory;

    bitfield_t location = INV_LOC_NONE;
    if (wpn_is_catergory(item, WEAPON_CATEGORY_BASIC) || wpn_is_catergory(item, WEAPON_CATEGORY_HEAVY) ) {
        if ( (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) ||
             (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) ) {
            You(monster, "do not have two hands.");
            return false;
        }
        location = inv_loc(INV_LOC_MAINHAND_WIELD) | inv_loc(INV_LOC_OFFHAND_WIELD);
    } else {
        if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
            location = inv_loc(INV_LOC_MAINHAND_WIELD);
        }
        else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
            location = inv_loc(INV_LOC_OFFHAND_WIELD);
        }
        else {
            You(monster, "have no hands free.");
            return false;
        }
    }

    if (inv_move_item_to_location(inv, item, location) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "dw", "Could not move %s to the correct location, bailing.", item->ld_name);
        You(monster, "seem to be unable to wield this weapon.");
        return false;
    }

    return true;
}

bool dw_wear_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == true) return false;
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
                 break;
        default: break;
    }

    if (item->item_type == ITEM_TYPE_WEAPON) {
        You_action(monster, "wield %s.", item->ld_name);
        Monster_action(monster, "wields %s.", item->ld_name);
    }
    else {
        You_action(monster, "wear %s.", item->ld_name);
        Monster_action(monster, "wears %s.", item->ld_name);
    }

    /* Do check here */
    if (msr_weapons_check(monster) == false) msr_weapon_next_selection(monster);

    return retval;
}

static bool dw_remove_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == false) return false;
    //struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;

    if (inv_move_item_to_location(inv, item, inv_loc(INV_LOC_INVENTORY) ) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_WARNING, "dw", "Could not move %s to the correct location, bailing.", item->ld_name);
        You(monster, "are to be unable to remove this weapon.");
        return false;
    }

    You_action(monster, "remove %s.", item->ld_name);
    Monster_action(monster, "removes %s.", item->ld_name);

    return true;
}

bool dw_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;
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
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (item->item_type == ITEM_TYPE_TOOL && item->specific.tool.tool_type == TOOL_TYPE_LIGHT) {
        if (item->specific.tool.lit == false) {
            item->specific.tool.lit = true;

            You_action(monster, "light %s.", item->ld_name);
            Monster_action(monster, "lights %s.", item->ld_name);
        }
        else {
            item->specific.tool.lit = false;

            You(monster, "douse %s.", item->ld_name);
            Monster_action(monster, "douses %s.", item->ld_name);
        }
    }
    return true;
}

bool dw_can_wear_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == true) return false;
    bool retval = false;

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: retval = false; break;
        case ITEM_TYPE_AMMO: retval = false; break;
        case ITEM_TYPE_TOOL: retval = false; break;
        case ITEM_TYPE_WEARABLE: retval = true; break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) )   retval = true; break;
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) )    retval = true; break;
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) )   retval = false; break;
        default: break;
    }

    return retval;
}

bool dw_can_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;

    /* TODO add remove checks here*/

    return true;
}

