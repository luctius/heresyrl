#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <assert.h>


#include "monster_action.h"
#include "monster.h"
#include "items/items.h"
#include "tiles.h"
#include "inventory.h"
#include "coord.h"
#include "game.h"
#include "dowear.h"
#include "turn_tick.h"
#include "ui/ui.h"
#include "dungeon/dungeon_map.h"

bool ma_do_move(struct msr_monster *monster, coord_t *pos) {
    if (msr_verify_monster(monster) == false) return false;
    if (pos == NULL) return false;

    if (msr_move_monster(monster, gbl_game->current_map, pos) == true) {
        struct dm_map_entity *me = dm_get_map_me(&monster->pos, gbl_game->current_map);
        struct itm_item *item = NULL;
        while ( (item = inv_get_next_item(me->inventory, item) ) != NULL) {
            You(monster, "see %s lying here.", item->ld_name);
        }

        int speed = msr_get_movement_rate(monster);
        msr_change_energy(monster, -(MSR_ACTION_MOVE - speed) );
        monster->controller.interruptable = false;
        monster->controller.interrupted = false;
        return true;
    }
    return false;
}

bool ma_do_idle(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    msr_change_energy(monster, -(MSR_ACTION_IDLE) );
    monster->controller.interruptable = true;
    return true;
}

bool ma_do_guard(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    msr_change_energy(monster, -(MSR_ACTION_GUARD) );
    monster->controller.interruptable = true;
    return true;
}

bool ma_do_wear(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == true) return false;
    if (dw_can_wear_item(monster, item) == false) return false;

    if (dw_wear_item(monster, item) == false) return false;
    msr_weapon_next_selection(monster);

    msr_change_energy(monster, -(MSR_ACTION_WEAR * item->use_delay) );
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_remove(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_item_worn(monster->inventory, item) == false) return false;
    if (dw_can_remove_item(monster, item) == false) return false;

    if (dw_remove_item(monster, item) == false) return false;
    msr_weapon_next_selection(monster);

    msr_change_energy(monster, -(MSR_ACTION_REMOVE * item->use_delay) );
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_use(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (dw_use_item(monster, item) == false) return false;

    msr_change_energy(monster, -(MSR_ACTION_USE * item->use_delay) );
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_pickup(struct msr_monster *monster, struct itm_item *items[], int nr_items) {
    if (msr_verify_monster(monster) == false) return false;
    if (items == NULL) return false;
    if (nr_items == 0) return false;

    for (int i = 0; i< nr_items; i++) {
        struct dm_map_entity *me = dm_get_map_me(&monster->pos, gbl_game->current_map);
        if (me != NULL) {
            if (inv_has_item(me->inventory, items[i]) == true) {
                if (inv_remove_item(me->inventory, items[i]) == true) {
                    if (msr_give_item(monster, items[i]) == true) {
                        msr_change_energy(monster, -(MSR_ACTION_PICKUP) );

                        You(monster, "picked up %s.", items[i]->ld_name);
                        Monster(monster, "picked up %s.", items[i]->ld_name);
                    }
                    else itm_destroy(items[i]);
                }
            }
        }
    }

    monster->controller.interruptable = false;
    return true;
}

bool ma_do_drop(struct msr_monster *monster, struct itm_item *items[], int nr_items) {
    if (msr_verify_monster(monster) == false) return false;
    if (items == NULL) return false;
    if (nr_items <= 0) return false;

    for (int i = 0; i< nr_items; i++) {
        if (inv_has_item(monster->inventory, items[i]) == true) {
            if (inv_item_wielded(monster->inventory, items[i]) == true) {
                /*
                   Allow drop of weapons in hand to the ground for free.
                 */
                if (msr_remove_item(monster, items[i]) == true) {
                    itm_insert_item(items[i], gbl_game->current_map, &monster->pos);

                    const char *hand_string = "hand";
                    if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
                        hand_string = "hands";
                    }
                    You(monster, "let %s fall from your %s.", items[i]->ld_name, hand_string);
                    Monster(monster, "dropped %s from %s %s.", items[i]->ld_name, msr_gender_name(monster, true), hand_string);
                }
            }

            if (inv_item_worn(monster->inventory, items[i]) == false) {
                if (msr_remove_item(monster, items[i]) == true) {
                    if (itm_insert_item(items[i], gbl_game->current_map, &monster->pos) == true) {
                        msr_change_energy(monster, -(MSR_ACTION_DROP) );

                        You(monster,"dropped %s.", items[i]->ld_name);
                        Monster(monster, "dropped %s.", items[i]->ld_name);
                    }
                }
            }
        }
    }

    if (msr_get_energy(monster) == TT_ENERGY_FULL) return false;

    monster->controller.interruptable = false;
    return true;
}

bool ma_do_melee(struct msr_monster *monster, coord_t *target_pos) {
    if (gbl_game == NULL) return false;
    if (dm_verify_map(gbl_game->current_map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;
    if (target_pos == NULL) return false;
    struct msr_monster *target = dm_get_map_me(target_pos, gbl_game->current_map)->monster;
    if (target == NULL) return false;
    if (msr_verify_monster(target) == false) return false;

    struct itm_item *item = NULL;
    int cost = MSR_ACTION_MELEE;

    /* 
       Here we check with how many weapons we try to hit.
       If it is more than one, the cost of the action increases.
       In addition, we nerf creatures by having them have the higher cost
       anyway.

       we should include weapon talents here too.
     */
    int hand_lst[] = {FGHT_MAIN_HAND, FGHT_OFF_HAND,};
    int hits = 0;
    for (unsigned int i = 0; i < ARRAY_SZ(hand_lst); i++) {
        item = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, hand_lst[i]);
        if (item != NULL) {
            hits++;
            if (hand_lst[i] == MSR_WEAPON_SELECT_CREATURE1) hits++;
        }
    }

    if (fght_melee(gbl_game->game_random, monster, target) == false) {
        return false;
    }

    /* if we do only one attack, we have a lower cost */
    if (hits == 1) cost = MSR_ACTION_SINGLE_MELEE;

    msr_change_energy(monster, -(cost) );
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_throw(struct msr_monster *monster, coord_t *pos, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (wpn_is_type(item, WEAPON_TYPE_THROWN) == false)  return false; /*only allow thrown weapons for now*/
    if (pos == NULL) return false;
    struct itm_item *item_bkp = NULL;
    enum fght_hand hand = FGHT_MAIN_HAND;
    int cost = MSR_ACTION_THROW;
    bool change = false;
    bool thrown = false;
    bitfield_t locs;

    /* Check in which hand the item is, or, if it is not, 
       exchange the main hand weapon temporarily with this item*/
    item_bkp = inv_get_item_from_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
    if (item_bkp != item) {

        /* check off hand*/
        if (inv_get_item_from_location(monster->inventory, INV_LOC_OFFHAND_WIELD) == item) {
            hand = FGHT_OFF_HAND;
        }
        else {
            /*item is not in any hand, we will switch it with the main hand weapon */
            change = true;

            /* save item locations */
            locs = inv_get_item_locations(monster->inventory, item_bkp);

            /* move current weapon to inventory */
            inv_move_item_to_location(monster->inventory, item_bkp, INV_LOC_INVENTORY);

            /* move thrown weapon to main hand*/
            inv_move_item_to_location(monster->inventory, item, INV_LOC_MAINHAND_WIELD);
        }
    }

    /* do the action*/
    thrown = fght_throw_weapon(gbl_game->game_random, monster, gbl_game->current_map, pos, hand);

    if (change) {
        /* check if the item still exists (mainly if there is anythin left in the stack. )*/
        if (inv_has_item(monster->inventory, item) == true) {
            /* return weapon back to inventory */
            inv_move_item_to_location(monster->inventory, item, INV_LOC_INVENTORY);
        }

        /* move the backup weapon back to its location*/
        inv_move_item_to_location(monster->inventory, item_bkp, locs);
    }

    /* if the action failed, return failure */
    if (thrown == false) return false;

    /* else we are done*/
    msr_change_energy(monster, -(cost) );
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_fire(struct msr_monster *monster, coord_t *pos) {
    if (msr_verify_monster(monster) == false) return false;
    if (pos == NULL) return false;
    struct itm_item *item = NULL;
    struct item_weapon_specific *wpn = NULL;
    int shots = 0;
    int cost = MSR_ACTION_FIRE;
    int hand_lst[] = {FGHT_MAIN_HAND, FGHT_OFF_HAND,};

    for (unsigned int i = 0; i < ARRAY_SZ(hand_lst); i++) {
        item = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand_lst[i]);
        if (item != NULL) {
            wpn = &item->specific.weapon;
            shots += wpn->rof[wpn->rof_set];
        }
    }

    if (fght_shoot(gbl_game->game_random, monster, gbl_game->current_map, pos) == false) {
        return false;
    }

    if (shots == 1) cost = MSR_ACTION_SINGLE_SHOT;

    msr_change_energy(monster, -(cost) );
    monster->controller.interruptable = false;
    return true;
}

static bool ma_has_ammo(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;

    struct item_weapon_specific *wpn = &item->specific.weapon;
    struct item_ammo_specific *ammo = NULL;
    struct itm_item *a_item = NULL;

    while ( (a_item = inv_get_next_item(monster->inventory, a_item) ) != NULL) {
        if (ammo_is_type(a_item, wpn->ammo_type) ) {
            ammo = &a_item->specific.ammo;

            int to_fill = wpn->magazine_sz - wpn->magazine_left;
            if (ammo->energy > 0) {
                float mod = ammo->energy / (float) wpn->magazine_sz;
                int ammo_in_pack = round(ammo->energy_left / mod);
                int sz = MIN(to_fill, ammo_in_pack);
                wpn->magazine_left += sz;
                ammo->energy_left -= (sz * mod);
                if (ammo->energy_left < 2) a_item->stacked_quantity = 0;
            }
            else {
                /*
                   Transfer ammo from stack to the magazine of the item.
                 */
                int sz = MIN(to_fill, a_item->stacked_quantity);
                wpn->magazine_left += sz;
                a_item->stacked_quantity -= sz;
            }

            /* TODO: destroy all items in inventory of stack_qnty == 0 */
            if (a_item->stacked_quantity == 0) {
                /*
                   Destroy stacked ammo item,
                   and start over, looking for more ammo.
                 */
                if (inv_remove_item(monster->inventory, a_item) == true) {
                    itm_destroy(a_item);
                    a_item = NULL;
                }
            }

            if (wpn->magazine_left == wpn->magazine_sz) {
                /*Until the weapon is full. */
                return true;
            }
        }
    }

    return false;
}

bool ma_do_reload_carried(struct msr_monster *monster, struct itm_item *ammo_item) {
    if (msr_verify_monster(monster) == false) return false;
    struct itm_item *item = NULL;
    struct item_weapon_specific *wpn = NULL;
    uint32_t cost = 0;
    int hand_lst[] = {FGHT_MAIN_HAND, FGHT_OFF_HAND,};

    for (unsigned int i = 0; i < ARRAY_SZ(hand_lst); i++) {
        item = fght_get_weapon(monster, WEAPON_TYPE_RANGED, hand_lst[i]);
        if (item != NULL) {
            wpn = &item->specific.weapon;
            if (wpn->magazine_left < wpn->magazine_sz) {
                if (ma_has_ammo(monster, item) == true ) {
                    cost += MSR_ACTION_RELOAD * item->use_delay;

                    You(monster, "reload %s.", item->ld_name);
                    Monster(monster, "reloads %s.", item->ld_name);
                } else {
                    You(monster, "do not have any ammo left for %s.", item->ld_name);
                }

                /* Do skill check here.. */
                if (wpn->jammed == true) {
                    wpn->jammed = false;
                    You(monster, "unjam %s.", item->ld_name);
                    Monster(monster, "unjams %s.", item->ld_name);
                    
                    if (cost == 0) {
                        cost += MSR_ACTION_RELOAD * item->use_delay;
                    }
                }
            }
        }
    }

    if (cost == 0) return false;
    msr_change_energy(monster, -(cost) );
    monster->controller.interruptable = false;
    return true;
}

static bool unload(struct msr_monster *monster, struct itm_item *weapon_item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(weapon_item) == false) return false;
    if (wpn_is_type(weapon_item, WEAPON_TYPE_RANGED) == false) return false;
    struct item_weapon_specific *wpn = &weapon_item->specific.weapon;
    if (wpn->magazine_left == 0) return false;

    struct itm_item *ammo_item = itm_create(wpn->ammo_used_template_id);
    if (itm_verify_item(ammo_item) == false) return false;
    assert(ammo_is_type(ammo_item, wpn->ammo_type) == true);

    struct item_ammo_specific *ammo = &ammo_item->specific.ammo;

    if (ammo->energy > 0) {
        /* We count charge packs with energy */
        ammo_item->stacked_quantity = 1;
        float mod = ammo->energy / (float) wpn->magazine_sz;
        ammo->energy_left = wpn->magazine_left * mod;
    } else {
        /* We count individual bullets */
        ammo_item->stacked_quantity = wpn->magazine_left;
    }
    wpn->magazine_left = 0;

    You(monster, "have unloaded %s.", weapon_item->ld_name);
    return msr_give_item(monster, ammo_item);
}

bool ma_do_unload(struct msr_monster *monster, struct itm_item *weapon_item) {
    if (msr_verify_monster(monster) == false) return false;
    struct itm_item *item = weapon_item;
    struct item_weapon_specific *wpn = NULL;
    uint32_t cost = 0;

    /* handle item given */
    if (item != NULL) {
        if (itm_verify_item(item)) {
            wpn = &item->specific.weapon;
            if (wpn->magazine_left > 0) {
                if (unload(monster, item) == true) {
                    cost += MSR_ACTION_UNLOAD;
                }
            }
        }
    }

    /* or handle items on the ground*/
    if (item == NULL) {
        struct dm_map_entity *me = dm_get_map_me(&monster->pos, gbl_game->current_map);
        while ( (item = inv_get_next_item(me->inventory, item) ) != NULL) {
            if (itm_verify_item(item) == true) {
                wpn = &item->specific.weapon;
                if (wpn->magazine_left > 0) {
                    if (unload(monster, item) ) {
                        cost += MSR_ACTION_UNLOAD;
                        break;
                    }
                }
            }
        }
    }

    /* .. or handle carried weapons. */
    if (item == NULL) {
        int hand_lst[] = {FGHT_MAIN_HAND, FGHT_OFF_HAND,};
        for (unsigned int i = 0; i < ARRAY_SZ(hand_lst); i++) {
            item = fght_get_weapon(monster, WEAPON_TYPE_RANGED, hand_lst[i]);
            if (item != NULL) {
                wpn = &item->specific.weapon;
                if (wpn->magazine_left > 0) {
                    if (unload(monster, item) ) {
                        cost += MSR_ACTION_UNLOAD;
                    }
                }
            }
        }
    }

    if (cost == 0) return false;
    msr_change_energy(monster, -(cost) );
    monster->controller.interruptable = false;
    return true;
}
