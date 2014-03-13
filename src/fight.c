#include <math.h>
#include <sys/param.h>
#include "fight.h"
#include "heresyrl_def.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "items.h"
#include "tiles.h"
#include "inventory.h"
#include "random.h"
#include "ui.h"
#include "game.h"

const char *fght_weapon_hand_name(enum fght_hand hand) {
    switch (hand) {
        case FGHT_MAIN_HAND: return "main"; break;
        case FGHT_OFF_HAND: return "off"; break;
        default: break;
    }
    return "unknown";
}

struct itm_item *fght_get_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (msr_weapons_check(monster) == false) return NULL;
    if (msr_weapon_type_check(monster, type) == false) return NULL;
    struct itm_item *item = NULL;

    if (hand == FGHT_MAIN_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND)  ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
            item = inv_get_item_from_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
        }
    }

    if (hand == FGHT_OFF_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
            item = inv_get_item_from_location(monster->inventory, INV_LOC_OFFHAND_WIELD);
        }
    }

    if (item == NULL) return NULL;
    if (itm_verify_item(item) == false) return NULL;
    if (item->specific.weapon.weapon_type != type) return NULL;

    return item;
}

struct itm_item *fght_get_working_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand) {
    struct itm_item *item = fght_get_weapon(monster, type, hand);
    if (item == NULL) return NULL;
    if (itm_verify_item(item) == false) return NULL;
    if (item->specific.weapon.weapon_type != type) return NULL;

    if (type == WEAPON_TYPE_RANGED) {
        if (item->specific.weapon.jammed == true) {
            Your(monster, "%s-hand weapon is jammed.", fght_weapon_hand_name(hand) );
            return NULL;
        }
        if (item->specific.weapon.magazine_left == 0) {
            Your(monster, "%s-hand weapon is empty.", fght_weapon_hand_name(hand) );
            return NULL;
        }
    }

    return item;
}

bool fght_do_weapon_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (r == NULL) return false;
    if (hits < 1) return false;

    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;

    for (int i = 0; i < WEAPON_TYPE_MAX; i++) {
        item = fght_get_working_weapon(monster, i, hand);
        if (item != NULL) i = WEAPON_TYPE_MAX;
    }
    if (item == NULL) return false;
    if (itm_verify_item(item) == false) return false;
    wpn = &item->specific.weapon;

    for (int h = 0; h < hits; h++) {
        int dmg_die_sz = 10;
        int dmg = random_xd10(r, wpn->nr_dmg_die);
        if (wpn->nr_dmg_die == 0) {
            dmg = random_xd5(r, 1);
            dmg_die_sz = 5;
        }

        int dmg_add = wpn->dmg_addition;
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Doing %d%s+%d damage => %d dmg.", wpn->nr_dmg_die, random_die_name(dmg_die_sz), wpn->dmg_addition, dmg);

        enum msr_hit_location mhl = msr_get_hit_location(target, random_d100(r) );
        int penetration = wpn->penetration;
        int armour = msr_calculate_armour(monster, mhl);
        int toughness = msr_calculate_characteristic_bonus(target, MSR_CHAR_TOUGHNESS);

        {
            /* Modifiers to damage here */
            if ( (wpn->weapon_type == WEAPON_TYPE_MELEE) ||
                 (wpn->weapon_type == WEAPON_TYPE_THROWN) ) {
                dmg_add += msr_calculate_characteristic_bonus(monster, MSR_CHAR_STRENGTH);
            }

            /* Modifiers to penatration here */
            /* Modifiers to toughness here */
            /* Modifiers to armour here */
        }

        You(monster, "Do %d%s+%d => (%d) damage.", wpn->nr_dmg_die, random_die_name(dmg_die_sz), dmg_add, dmg + dmg_add);
        armour = MIN((armour - penetration), 0);
        dmg = (dmg + dmg_add) - (armour  + toughness);
        msr_do_dmg(target, dmg, mhl, gbl_game->current_map);

        if (target->dead) h = hits;
    }

    return true;
}

int fght_ranged_calc_tohit(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand, int ammo) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (ammo == 0) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;

    item = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (item == NULL) return -1;

    wpn = &item->specific.weapon;
    You_action(monster, "fire at %s with your %s.", monster->ld_name, item->sd_name);
    Monster_action(monster, "fires at you with his %s.", item->sd_name);

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_BALISTIC_SKILL);
    int to_hit_mod = 0;

    {/*add to-hit modifiers here*/ 
        /* Add lighting modifiers */

        /* Offhand Weapon */
        if (hand == FGHT_OFF_HAND) to_hit_mod += FGHT_MODIFIER_OFF_HAND;

        /* Single Shot */
        if (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) to_hit_mod += 0;

        /* Semi Automatic */
        if (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) to_hit_mod += FGHT_RANGED_MODIFIER_ROF_SEMI;

        /* Fully Automatic */
        if (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) to_hit_mod += FGHT_RANGED_MODIFIER_ROF_AUTO;

        /* Target size modifiers */
        if (target->size == MSR_SIZE_AVERAGE) to_hit_mod += FGHT_MODIFIER_SIZE_AVERAGE;
        else if (target->size == MSR_SIZE_MASSIVE) to_hit_mod += FGHT_MODIFIER_SIZE_MASSIVE;
        else if (target->size == MSR_SIZE_ENORMOUS) to_hit_mod += FGHT_MODIFIER_SIZE_ENORMOUS;
        else if (target->size == MSR_SIZE_HULKING) to_hit_mod += FGHT_MODIFIER_SIZE_HULKING;
        else if (target->size == MSR_SIZE_SCRAWY) to_hit_mod += FGHT_MODIFIER_SIZE_SCRAWNY;
        else if (target->size == MSR_SIZE_PUNY) to_hit_mod += FGHT_MODIFIER_SIZE_PUNY;
        else if (target->size == MSR_SIZE_MINISCULE) to_hit_mod += FGHT_MODIFIER_SIZE_MINISCULE;

        /* Shooting Distances */
        int distance = cd_pyth(&monster->pos, &target->pos);
        if (distance == FGHT_MELEE_RANGE) to_hit_mod += FGHT_RANGED_MODIFIER_MELEE;
        else if (distance >= (wpn->range * 3) ) to_hit_mod += FGHT_RANGED_MODIFIER_EXTREME_RANGE;
        else if (distance >= (wpn->range * 2) ) to_hit_mod += FGHT_RANGED_MODIFIER_LONG_RANGE;
        else if (distance <= FGHT_POINT_BLANK_RANGE) to_hit_mod += FGHT_RANGED_MODIFIER_POINT_BLACK;
        else if (distance <= (wpn->range * 0.5) ) to_hit_mod += FGHT_RANGED_MODIFIER_SHORT_RANGE;

        /* Maximum modifier */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

        to_hit += to_hit_mod;
    }

    int roll = random_d100(r);
    int jammed_threshold = FGHT_RANGED_JAM;
    if ( (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) ||
         (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) ) {
        jammed_threshold = FGHT_RANGED_JAM_SEMI;
    }

    if (roll >= jammed_threshold) {
        wpn->jammed = true;

        Your(monster, "%s-hand weapon jammed.", fght_weapon_hand_name(hand) );
        Monster_action(monster, "'s %s jammed.", item->sd_name);
        return -1;
    }

    if (to_hit <= 0) {
        You(monster, "miss the shot by a huge margin.");
        Monster_action(monster, "missed you with a huge margin.");
        return -1;
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Shot attempt with calcBS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return MIN(ammo, ((to_hit - roll) /10) +1);
    }

    if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
        You(monster, "miss with your %s.", item->sd_name);
    }
    else {
        You(monster, "miss with your %s-hand weapon.", fght_weapon_hand_name(hand) );
    }

    return -1;
}

int fght_melee_calc_tohit(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) return -1;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;

    item = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, hand);
    if (item == NULL) return -1;

    wpn = &item->specific.weapon;
    You_action(monster, "slash at %s with your %s.", monster->ld_name, item->sd_name);
    Monster_action(monster, "slashes at you with his %s.", item->sd_name);

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_WEAPON_SKILL);
    int to_hit_mod = 0;

    {/*add to-hit modifiers here*/ 
        /* Add lighting modifiers */

        /* Offhand Weapon */
        if (hand == FGHT_OFF_HAND) to_hit_mod += FGHT_MODIFIER_OFF_HAND;
        if (wpn->is_unarmed == true) to_hit_mod += FGHT_MELEE_UNARMED;

        if (target->size == MSR_SIZE_AVERAGE) to_hit_mod += FGHT_MODIFIER_SIZE_AVERAGE;
        else if (target->size == MSR_SIZE_MASSIVE) to_hit_mod += FGHT_MODIFIER_SIZE_MASSIVE;
        else if (target->size == MSR_SIZE_ENORMOUS) to_hit_mod += FGHT_MODIFIER_SIZE_ENORMOUS;
        else if (target->size == MSR_SIZE_HULKING) to_hit_mod += FGHT_MODIFIER_SIZE_HULKING;
        else if (target->size == MSR_SIZE_SCRAWY) to_hit_mod += FGHT_MODIFIER_SIZE_SCRAWNY;
        else if (target->size == MSR_SIZE_PUNY) to_hit_mod += FGHT_MODIFIER_SIZE_PUNY;
        else if (target->size == MSR_SIZE_MINISCULE) to_hit_mod += FGHT_MODIFIER_SIZE_MINISCULE;

        /* Maximum modifier */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

        to_hit += to_hit_mod;
    }

    int roll = random_d100(r);
    if (to_hit <= 0) {
        You(monster, "miss by a huge margin.");
        Monster_action(monster, "missed you with a huge margin.");
        return -1;
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Melee attempt with calcWS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return 1;
    }

    if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
        You(monster, "miss with your %s.", item->sd_name);
    }
    else {
        You(monster, "miss with your %s-hand weapon.", fght_weapon_hand_name(hand) );
    }

    return -1;
}

bool fght_melee(struct random *r, struct msr_monster *monster, struct msr_monster *target) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (cd_pyth(&monster->pos, &target->pos) > 1) return false;
    if (monster->faction == target->faction) return false; /* do not attack members of same faction */
    struct itm_item *item1 = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, FGHT_MAIN_HAND);
    struct itm_item *item2 = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, FGHT_OFF_HAND);
    enum msr_weapon_selection wpn_sel = monster->wpn_sel;
    bool unarmed = false;
    int hits = 0;
    bool retval = false;

    if ( (item1 == NULL) || (item2 == NULL) ) {
        /* 
           Lame hack:
           create unarmed dmg item 
         */
        if (inv_loc_empty(monster->inventory, INV_LOC_MAINHAND_WIELD) == true) {
            unarmed = true;
            item1 = msr_unarmed_weapon(monster);
            msr_give_item(monster, item1);
            inv_move_item_to_location(monster->inventory, item1, INV_LOC_MAINHAND_WIELD);
            monster->wpn_sel = MSR_WEAPON_SELECT_MAIN_HAND;
        }
        else if (inv_loc_empty(monster->inventory, INV_LOC_OFFHAND_WIELD) == true) {
            unarmed = true;
            item2 = msr_unarmed_weapon(monster);
            msr_give_item(monster, item2);
            inv_move_item_to_location(monster->inventory, item2, INV_LOC_MAINHAND_WIELD);
            monster->wpn_sel = MSR_WEAPON_SELECT_OFF_HAND;
        }
    }

    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == true) {
        /* Do damage */
        hits = fght_melee_calc_tohit(r, monster, target, FGHT_MAIN_HAND);
        fght_do_weapon_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
        hits = fght_melee_calc_tohit(r, monster, target, FGHT_OFF_HAND);
        fght_do_weapon_dmg(r, monster, target, hits, FGHT_OFF_HAND);
    }

    if (unarmed == true) {
        /* remove unarmed dmg items */
        if (item1 != NULL) {
            if (inv_remove_item(monster->inventory, item1) == true) {
                itm_destroy(item1);
            }
        }
        if (item2 != NULL) {
            if (inv_remove_item(monster->inventory, item2) == true) {
                itm_destroy(item2);
            }
        }
        monster->wpn_sel = wpn_sel;
    }
    return true;
}

int fght_shoot(struct random *r, struct msr_monster *monster, struct dc_map *map, coord_t *e) {
    if (msr_verify_monster(monster) == false) return -1;
    if (dc_verify_map(map) == false) return -1;
    if (e == NULL) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;
    struct itm_item *item1 = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, FGHT_MAIN_HAND);
    struct itm_item *item2 = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, FGHT_OFF_HAND);
    if ( (item1 == NULL) && (item2 == NULL) ) return -1;
    int ammo1 = 0;
    int ammo2 = 0;

    if (item1 != NULL) {
        struct item_weapon_specific *wpn = &item1->specific.weapon;
        ammo1 = MIN(wpn->magazine_left, wpn->rof[wpn->rof_set]);
        wpn->magazine_left -= ammo1;
    }
    if (item2 != NULL) {
        struct item_weapon_specific *wpn = &item2->specific.weapon;
        ammo2 = MIN(wpn->magazine_left, wpn->rof[wpn->rof_set]);
        wpn->magazine_left -= ammo2;
    }

    charwin_refresh();

    coord_t path[MAX(map->size.x, map->size.y)];
    int path_len = fght_calc_lof_path(&monster->pos, e, path, ARRAY_SZ(path));
    bool blocked = false;
    int unblocked_length = 0;

    int i = 1;
    while ((i < path_len) && (blocked == false)) {
        if (sd_get_map_me(&path[i], map)->monster != NULL) {
            struct msr_monster *target = sd_get_map_me(&path[i], map)->monster;
            int hits;

            /* Do damage */
            hits = fght_ranged_calc_tohit(r, monster, target, FGHT_MAIN_HAND, ammo1);
            fght_do_weapon_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
            hits = fght_ranged_calc_tohit(r, monster, target, FGHT_OFF_HAND, ammo2);
            fght_do_weapon_dmg(r, monster, target, hits, FGHT_OFF_HAND);

            /* For now, always stop at the first monster. 
               later on we can continue but then we have 
               to keep track of the ammo once...*/
            blocked = true;
        }
        if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&path[i], map), TILE_ATTR_TRAVERSABLE) == false) {
            blocked = true;
        }
        if (blocked == false) {
            unblocked_length++;
        }
        i++;
    }

    ui_animate_projectile(map, path, unblocked_length, '*');

    return unblocked_length;
}

/* Adapted from the code displayed at RogueBasin's "Bresenham's Line
 * Algorithm" article, this function checks for an unobstructed line
 * of sight between two locations using Bresenham's line algorithm to
 * draw a line from one point to the other. Returns true if there is
 * line of sight, false if there is no line of sight. */
/*int los (int los_x_1, int los_y_1, int los_x_2, int
         los_y_2, int level) {*/
int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    int los_x_1 = s->x;
    int los_y_1 = s->y;
    int los_x_2 = e->x;
    int los_y_2 = e->y;
    int delta_x, delta_y, move_x, move_y, error;
    int pl_counter = 0;

    /* Calculate deltas. */
    delta_x = abs (los_x_2 - los_x_1) << 1;
    delta_y = abs (los_y_2 - los_y_1) << 1;

    /* Calculate signs. */
    move_x = los_x_2 >= los_x_1 ? 1 : -1;
    move_y = los_y_2 >= los_y_1 ? 1 : -1;

    /* There is an automatic line of sight, of course, between a
    * location and the same location or directly adjacent
    * locations. */
    if (abs (los_x_2 - los_x_1) < 2 && abs (los_y_2 - los_y_1) < 2) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return 0;
    }

    /* Ensure that the line will not extend too long. */
    if (((los_x_2 - los_x_1) * (los_x_2 - los_x_1))
    + ((los_y_2 - los_y_1) * (los_y_2 -
                             los_y_1)) > path_list_sz) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return -1;
    }

    /* "Draw" the line, checking for obstructions. */
    if (delta_x >= delta_y) {
        /* Calculate the error factor, which may go below zero. */
        error = delta_y - (delta_x >> 1);

        /* Search the line. */
        //while (los_x_1 != los_x_2) {
        while (pl_counter < path_list_sz) {
             /* Check for an obstruction. If the obstruction can be "moved
              * around", it isn't really an obstruction. */
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            //if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_x > 0)) {
                    los_y_1 += move_y;
                    error -= delta_x;
                }
            }
            los_x_1 += move_x;
            error += delta_y;
        }
    }
    else {
        /* Calculate the error factor, which may go below zero. */
        error = delta_x - (delta_y >> 1);

        /* Search the line. */
        //while (los_y_1 != los_y_2) {
        while (pl_counter < path_list_sz) {
            /* Check for an obstruction. If the obstruction can be "moved
            * around", it isn't really an obstruction. */
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_y > 0)) {
                    los_x_1 += move_x;
                    error -= delta_y;
                }
            }
            los_y_1 += move_y;
            error += delta_x;
        }
    }

    path_list[pl_counter].x = los_x_2;
    path_list[pl_counter].y = los_y_2;
    pl_counter++;

    /* Return. */
    return pl_counter;
}
