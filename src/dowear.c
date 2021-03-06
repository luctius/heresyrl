/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>

#include "monster/monster.h"
#include "status_effects/status_effects.h"
#include "items/items.h"
#include "inventory.h"
#include "game.h"
#include "turn_tick.h"
#include "dungeon/dungeon_map.h"

static bool wield_melee_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == true) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_MELEE) == false) return false;
    struct inv_inventory *inv = monster->inventory;

    bitfield32_t location = INV_LOC_NONE;
    if (wpn_is_catergory(item, WEAPON_CATEGORY_2H_MELEE) ) {
        if ( (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) ||
             (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) ) {
            You(monster, "do not have two hands available.");
            return false;
        }
        location = INV_LOC_MAINHAND_WIELD | INV_LOC_OFFHAND_WIELD;
        monster->wpn_sel = MSR_WEAPON_SELECT_BOTH_HAND;
    } else {
        if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
            location = INV_LOC_MAINHAND_WIELD;
            monster->wpn_sel = MSR_WEAPON_SELECT_MAIN_HAND;
        }
        else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
            location = INV_LOC_OFFHAND_WIELD;
            monster->wpn_sel = MSR_WEAPON_SELECT_OFF_HAND;
        }
        else {
            You(monster, "have no hands free.");
            return false;
        }
    }

    assert(inv_move_item_to_location(inv, item, location) == true);
    assert(msr_weapons_check(monster) );
    return true;
}

static bool wield_ranged_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == true) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    struct inv_inventory *inv = monster->inventory;

    bitfield32_t location = INV_LOC_NONE;
    if (wpn_is_catergory(item, WEAPON_CATEGORY_2H_RANGED) ) {
        if ( ( (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) ||
             (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) )  ||
             ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == false) ||
               (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == false) ) ) {
            You(monster, "do not have two hands available.");
            return false;
        }
        location = INV_LOC_MAINHAND_WIELD | INV_LOC_OFFHAND_WIELD;
    } else {
        if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == true) ) {
            location = INV_LOC_MAINHAND_WIELD;
        }
        else if ( (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) &&
                (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
            location = INV_LOC_OFFHAND_WIELD;
        }
        else {
            You(monster, "have no hands free.");
            return false;
        }
    }


    assert(inv_move_item_to_location(inv, item, location) == true);
    return true;
}

static bool wear_wearable(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == true) return false;
    if (item->item_type != ITEM_TYPE_WEARABLE) return false;
    struct inv_inventory *inv = monster->inventory;
    struct item_wearable_specific *wearable = &item->specific.wearable;

    if ( (inv_loc_empty(inv, wearable->locations) == true) &&
            (inv_support_location(inv, wearable->locations) == true) ) {

        assert(inv_move_item_to_location(inv, item, wearable->locations) == true);
        return true;
    }

    return false;
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
        case ITEM_TYPE_WEARABLE: retval = wear_wearable(monster, item); break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) { retval = wield_ranged_weapon(monster, item); break; }
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) )  { retval = wield_melee_weapon(monster, item);  break; }
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) ) { retval = false; break; }
                 break;
        default: break;
    }

    if (retval == true) {
        if (item->item_type == ITEM_TYPE_WEAPON) {
            You(monster,     "wield %s.", item->ld_name);
            Monster(monster, "wields %s.", item->ld_name);
        }
        else {
            You(monster,     "wear %s.", item->ld_name);
            Monster(monster, "wears %s.", item->ld_name);
        }

        /* Do check here */
        if (msr_weapons_check(monster) == false) msr_weapon_next_selection(monster);
        else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) msr_weapon_next_selection(monster);
    }

    return retval;
}

static bool remove_weapon(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_wielded(monster->inventory, item) == false) return false;
    //struct item_weapon_specific *weapon = &item->specific.weapon;
    struct inv_inventory *inv = monster->inventory;

    assert(inv_move_item_to_location(inv, item, INV_LOC_INVENTORY) == true);

    You(monster,     "remove %s.", item->ld_name);
    Monster(monster, "removes %s.", item->ld_name);

    return true;
}

static bool remove_wearable(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;
    struct inv_inventory *inv = monster->inventory;

    assert(inv_move_item_to_location(inv, item, INV_LOC_INVENTORY) == true);

    You(monster,     "remove %s.", item->ld_name);
    Monster(monster, "removes %s.", item->ld_name);

    return true;
}

bool dw_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;
    bool retval = false;

    switch(item->item_type) {
        case ITEM_TYPE_FOOD:     retval = false; break;
        case ITEM_TYPE_AMMO:     retval = false; break;
        case ITEM_TYPE_TOOL:     retval = false; break;
        case ITEM_TYPE_WEARABLE: retval = remove_wearable(monster, item); break;
        case ITEM_TYPE_WEAPON:   retval = remove_weapon(monster, item); break;
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

    /* do status checks on monster*/
    if (se_has_effect(monster, EF_SWIMMING) ) {
        You(monster, "cannot change your equipment while swimming.");
        return false;
    }

    if (item->item_type == ITEM_TYPE_TOOL && item->specific.tool.tool_type == TOOL_TYPE_LIGHT) {
        if (item->specific.tool.lit == false) {
            if (item->specific.tool.energy == 0) {
                msg("The %s is out of juice.", item->sd_name);
                return false;
            }

            item->specific.tool.lit = true;
            item->energy = item->specific.tool.energy * TT_ENERGY_TURN;
            item->energy_action = true;

            You(monster,     "light %s.", item->ld_name);
            Monster(monster, "lights %s.", item->ld_name);
        }
        else {
            item->specific.tool.lit = false;
            item->energy_action = false;
            item->specific.tool.energy = 0;

            if (item->energy > 0) {
                item->specific.tool.energy = item->energy / TT_ENERGY_TURN;

                You(monster,     "douse %s.", item->ld_name);
                Monster(monster, "douses %s.", item->ld_name);
            }
        }
    }
    if (item->item_type == ITEM_TYPE_FOOD) {
        bool destroy = false;
        bool identified = item->identified;

        struct item_food_specific *food = &item->specific.food;

        if (food->food_type == FOOD_TYPE_LIQUID) {
            You(monster,     "quaff %s.", item->ld_name);
            Monster(monster, "quaffs %s.", item->ld_name);

            food->nutrition_left -= 1;
            if (food->nutrition_left <= 0) {
                You_msg(monster, "That was the last drop.");
                destroy = true;
            }
        }
        else if (food->food_type == FOOD_TYPE_SOLID) {
            You(monster,     "eat from %s.", item->ld_name);
            Monster(monster, "eats from %s.", item->ld_name);

            food->nutrition_left -= 1;
            if (food->nutrition_left <= 0) {
                You_msg(monster, "That was the last bit.");
                destroy = true;
            }
        }
        else if (food->food_type == FOOD_TYPE_INJECTION) {
            You(monster,     "inject a %s.", item->ld_name);
            Monster(monster, "injects a %s.", item->ld_name);

            food->nutrition_left -= 1;
            if (food->nutrition_left <= 0) {
                You_msg(monster, "That was the last drop.");
                destroy = true;
            }
        }

        /* We just identified the item by trying it. */
        itm_identify(item);

        /* Get the normal benefit. */
        if (food->convey_status_effect != SEID_NONE) se_add_status_effect(monster, food->convey_status_effect, item->ld_name);

        /* Get the side efect. */
        if (food->side_effect != SEID_NONE) {
            se_add_status_effect(monster, food->side_effect, item->ld_name);

            if (item->identified && !identified) {
                You(monster, "identified the side effects of %s.\n", item->ld_name);
            }
        }

        if (destroy) {
            if (inv_remove_item(monster->inventory, item) == true) {
                itm_destroy(item);
            }
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

    /* do status checks on monster*/
    if (se_has_effect(monster, EF_SWIMMING) ) {
        You(monster, "cannot change your equipment while swimming.");
        return false;
    }

    switch(item->item_type) {
        case ITEM_TYPE_FOOD: retval = false; break;
        case ITEM_TYPE_AMMO: retval = false; break;
        case ITEM_TYPE_TOOL: retval = false; break;
        case ITEM_TYPE_WEARABLE: retval = true; break;
        case ITEM_TYPE_WEAPON:
                 if (wpn_is_type(item, WEAPON_TYPE_RANGED) )   { retval = true; break;  }
                 if (wpn_is_type(item, WEAPON_TYPE_MELEE) )    { retval = true; break;  }
                 if (wpn_is_type(item, WEAPON_TYPE_THROWN) )   { retval = false; break; }
        default: break;
    }

    return retval;
}

bool dw_can_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;

    /* do status checks on monster*/
    if (se_has_effect(monster, EF_SWIMMING) ) {
        You(monster, "cannot change your equipment while swimming.");
        return false;
    }

    /* TODO add remove checks here*/

    return true;
}

