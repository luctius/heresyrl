#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/param.h>

#include "monster_action.h"
#include "monster.h"
#include "items.h"
#include "dungeon_creator.h"
#include "tiles.h"
#include "inventory.h"
#include "ui.h"
#include "coord.h"
#include "game.h"
#include "dowear.h"

void ma_init(void) {
}

void ma_exit(void) {
}

bool ma_process(void) {
    struct msr_monster *monster = NULL;

    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->dead) {

            /* Clean-up monsters which can be cleaned up. */
            if (monster->controller.controller_cb == NULL) {
                struct msr_monster *dead_monster = monster;
                monster = msrlst_get_next_monster(monster);

                msr_destroy(dead_monster, gbl_game->current_map);
            }
        }

        bool do_action = false;

        if (monster->energy < MSR_ENERGY_FULL) monster->energy += MSR_ENERGY_TICK;

        if (monster->energy >= MSR_ENERGY_FULL) do_action = true;
        if (monster->controller.interrupted == true) do_action = true;

        if (do_action) {
            if (monster->controller.controller_cb != NULL) {
                monster->controller.controller_cb(monster, monster->controller.controller_ctx);
            }
            monster->controller.interrupted = false;
        }

    }
    return true;
}

bool mt_interrupt_event(uint32_t monster_uid) {
    struct msr_monster *monster = NULL;
    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        if (monster->uid == monster_uid) {
            if (monster->controller.interruptable) {
                monster->controller.interrupted = true;
            }
        }
    }
    return false;
}

bool ma_do_move(struct msr_monster *monster, coord_t *pos) {
    if (msr_verify_monster(monster) == false) return false;
    if (pos == NULL) return false;
    coord_t oldpos = monster->pos;

    if (msr_move_monster(monster, gbl_game->current_map, pos) == true) {
        if ( (sd_get_map_me(&oldpos, gbl_game->current_map)->visible == true) || 
             (sd_get_map_me(pos, gbl_game->current_map)->visible == true) ) {
            /* TODO interrupt enemies who see me. */
            update_screen();
        }

        int speed = msr_calculate_characteristic_bonus(monster, MSR_CHAR_TOUGHNESS) * 10;
        monster->energy -= (MSR_ACTION_MOVE - speed);
        monster->controller.interruptable = false;
        monster->controller.interrupted = false;
        return true;
    }
    return false;
}

bool ma_do_idle(struct msr_monster *monster) {
    return ma_do_guard(monster);
}

bool ma_do_guard(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    monster->energy -= MSR_ACTION_GUARD;
    monster->controller.interruptable = true;
    return true;
}

bool ma_do_wear(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    if (dw_can_wear_item(monster, item) == false) return false;

    if (dw_wear_item(monster, item) == false) return false;
    monster->energy -= MSR_ACTION_WEAR * item->use_delay;
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_remove(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) == INV_LOC_INVENTORY) return false;
    if (dw_can_remove_item(monster, item) == false) return false;

    if (dw_remove_item(monster, item) == false) return false;
    monster->energy -= MSR_ACTION_REMOVE * item->use_delay;
    monster->controller.interruptable = false;
    monster->controller.interrupted = false;
    return true;
}

bool ma_do_use(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;
    if (inv_get_item_location(monster->inventory, item) != INV_LOC_INVENTORY) return false;
    if (dw_use_item(monster, item) == false) return false;

    monster->energy -= MSR_ACTION_USE * item->use_delay;
    monster->controller.interruptable = false;
    return true;
}

bool ma_do_pickup(struct msr_monster *monster, struct itm_item *items[], int nr_items) {
    if (msr_verify_monster(monster) == false) return false;
    if (items == NULL) return false;
    if (nr_items == 0) return false;

    for (int i = 0; i< nr_items; i++) {
        struct dc_map_entity *me = sd_get_map_me(&monster->pos, gbl_game->current_map);
        if (me != NULL) {
            if (inv_has_item(me->inventory, items[i]) == true) {
                if (inv_remove_item(me->inventory, items[i]) == true) {
                    if (msr_give_item(monster, items[i]) == true) {
                        monster->energy -= MSR_ACTION_PICKUP;

                        You_action(monster, "picked up %s.", items[i]->ld_name);
                        Monster_action(monster, "picked up %s.", items[i]->ld_name);
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
            if ( (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_MAINHAND_WIELD) ||
                 (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_OFFHAND_WIELD) ) {
                /*
                   Allow drop of weapons in hand to the ground for free.
                 */
                if (msr_remove_item(monster, items[i]) == true) {
                    itm_insert_item(items[i], gbl_game->current_map, &monster->pos);

                    const char *hand_string = "hand";
                    if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
                        hand_string = "hands";
                    }
                    You_action(monster, "let %s fall from your %s.", items[i]->ld_name, hand_string);
                    Monster_action(monster, "dropped %s from %s %s.", items[i]->ld_name, msr_gender_string(monster), hand_string);
                }
            }

            if (inv_get_item_location(monster->inventory, items[i]) == INV_LOC_INVENTORY) {
                if (msr_remove_item(monster, items[i]) == true) {
                    if (itm_insert_item(items[i], gbl_game->current_map, &monster->pos) == true) {
                        monster->energy -= MSR_ACTION_DROP;

                        You_action(monster,"dropped %s.", items[i]->ld_name);
                        Monster_action(monster, "dropped %s.", items[i]->ld_name);
                    }
                }
            }
        }
    }

    monster->controller.interruptable = false;
    return true;
}

bool ma_do_melee(struct msr_monster *monster, coord_t *target_pos) {
    if (gbl_game == NULL) return false;
    if (dc_verify_map(gbl_game->current_map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;
    if (target_pos == NULL) return false;
    struct msr_monster *target = sd_get_map_me(target_pos, gbl_game->current_map)->monster;
    if (target == NULL) return false;
    if (msr_verify_monster(target) == false) return false;

    struct itm_item *item = NULL;
    struct item_weapon_specific *wpn = NULL;
    int cost = MSR_ACTION_MELEE;
    int hand_lst[] = {FGHT_MAIN_HAND, FGHT_OFF_HAND,};
    int hits = 0;

    for (unsigned int i = 0; i < ARRAY_SZ(hand_lst); i++) {
        item = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand_lst[i]);
        if (item != NULL) {
            hits++;
        }
    }

    if (fght_melee(gbl_game->game_random, monster, target) == false) {
        return false;
    }

    if (hits == 1) cost = MSR_ACTION_SINGLE_MELEE;

    monster->energy -= cost;
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

    monster->energy -= cost;
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
        if (a_item->item_type == ITEM_TYPE_AMMO) {
            ammo = &a_item->specific.ammo;
            if (ammo->ammo_type == wpn->ammo_type) {

                /*
                   Transfer ammo from stack to the magazine of the item.
                 */
                int to_fill = wpn->magazine_sz - wpn->magazine_left;
                int sz = MIN(to_fill, a_item->stacked_quantity);
                wpn->magazine_left += sz;
                a_item->stacked_quantity -= sz;

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

                    You_action(monster, "reload %s.", item->ld_name);
                    Monster_action(monster, "reloads %s.", item->ld_name);
                }
                else {
                    You(monster, "do not have any ammo left for %s.", item->ld_name);
                }

                /* Do skill check here.. */
                if (wpn->jammed == true) {
                    wpn->jammed = false;
                    You_action(monster, "unjam %s.", item->ld_name);
                    Monster_action(monster, "unjams %s.", item->ld_name);
                    
                    if (cost == 0) {
                        cost += MSR_ACTION_RELOAD * item->use_delay;
                    }
                }
            }
        }
    }

    if (cost == 0) return false;
    monster->energy -= cost;
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

    struct item_ammo_specific *ammo = &ammo_item->specific.ammo;
    ammo_item->stacked_quantity = wpn->magazine_left;
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
        struct dc_map_entity *me = sd_get_map_me(&monster->pos, gbl_game->current_map);
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
    monster->energy -= cost;
    monster->controller.interruptable = false;
    return true;
}

