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

const char *fght_weapon_hand_name(enum fght_hand hand) {
    switch (hand) {
        case FGHT_MAIN_HAND: return "main"; break;
        case FGHT_OFF_HAND: return "off"; break;
        default: break;
    }
    return "unknown";
}

bool fght_do_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand) {
    if (monster == NULL) return false;
    if (target == NULL) return false;
    if (hits < 1) return false;
    if ( (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false)   &&
         (msr_weapon_type_check(monster, WEAPON_TYPE_CREATURE) == false) &&
         (msr_weapon_type_check(monster, WEAPON_TYPE_THROWN) == false)   &&
         (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) ) return false;
    if (msr_weapons_check(monster) == false) return false;

    struct inv_inventory *inv = monster->inventory;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;

    if (hand == FGHT_MAIN_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND)  ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
        }
    }

    if (hand == FGHT_OFF_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
        }
    }
    if (item == NULL) return false;

    for (int h = 0; h < hits; h++) {
        wpn = &item->specific.weapon;

        int dmg = 0;
        for (int i = 0; i < wpn->nr_dmg_die; i++) {
            dmg += (random_int32(r)%10);
        }
        dmg += wpn->dmg_addition;
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Doing %dD10+%d damage => %d dmg.", wpn->nr_dmg_die, wpn->dmg_addition, dmg);

        int hitloc = random_int32(r) % 100;
        msr_do_dmg(target, dmg, wpn->penetration, hitloc);
    }
    return true;
}

int fght_ranged_calc_tohit(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand) {
    if (monster == NULL) return -1;
    if (target == NULL) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;
    struct inv_inventory *inv = monster->inventory;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;
    int ammo = 0;

    /*
       Check monster for weapon.
     */
    if (hand == FGHT_MAIN_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND)  ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
        }
    }

    if (hand == FGHT_OFF_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) ||
             (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) ) {
            item = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
        }
    }

    if (item != NULL) {
        wpn = &item->specific.weapon;
        if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
            ammo = MIN(wpn->magazine_left, wpn->rof[wpn->rof_set]);
            if (ammo > 0) {
                wpn->magazine_left -= ammo;
            }
            else {
                wpn = NULL;
                Your("%s-hand weapon is empty.", fght_weapon_hand_name(hand) );
            }
        }
        else wpn = NULL;
    }
    if (wpn == NULL) return -1;

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_BALISTIC_SKILL);
    /*add to-hit modifiers here*/
    if (hand == FGHT_OFF_HAND) to_hit += FGHT_RANGED_MODIFIER_OFF_HAND;
    if (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) to_hit += 0;
    if (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) to_hit += FGHT_RANGED_MODIFIER_ROF_SEMI;
    if (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) to_hit += FGHT_RANGED_MODIFIER_ROF_AUTO;

    if (to_hit <= 0) {
        You("miss the shot by a huge margin.");
        return -1;
    }
    int roll = random_int32(r) % 100;
    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Shot attempt with calcBS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return MIN(ammo, ((to_hit - roll) /10) +1);
    }

    You("miss with your %s-hand weapon.", fght_weapon_hand_name(hand) );
    return -1;
}

int fght_shoot(struct random *r, struct msr_monster *monster, struct dc_map *map, coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    if (monster == NULL) return -1;
    if (map == NULL) return -1;
    if (s == NULL) return -1;
    if (e == NULL) return -1;
    if (path_list == NULL) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;

    coord_t path[MAX(map->size.x, map->size.y)];
    int path_len = fght_calc_lof_path(s, e, path, ARRAY_SZ(path));
    bool blocked = false;
    int unblocked_length = 0;

    for (int i = 1; (i < path_len) && (blocked == false); i++) {
        if (sd_get_map_me(&path[i], map)->monster != NULL) {
            struct msr_monster *target = sd_get_map_me(&path[i], map)->monster;
            int hits;

            /* Do damage */
            hits = fght_ranged_calc_tohit(r, monster, target, FGHT_MAIN_HAND);
            fght_do_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
            hits = fght_ranged_calc_tohit(r, monster, target, FGHT_OFF_HAND);
            fght_do_dmg(r, monster, target, hits, FGHT_OFF_HAND);

            /* For now, always stop at the first monster. 
               later on we can continue but then we have 
               to keep track of the ammo once...*/
            blocked = true;
        }
        if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&path[i], map), TILE_ATTR_TRAVERSABLE) == false) {
            blocked = true;
        }
        if (blocked == false) {
            if (sd_get_map_me(&path[i], map)->visible == true) {
                if (i-1 < path_list_sz) {
                    path_list[i-1] = path[i];
                    unblocked_length++;
                }
            }
        }
    }
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
