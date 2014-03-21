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
#include "los.h"

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

    if (hand == FGHT_CREATURE_HAND) {
        if ( (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) ) {
            item = inv_get_item_from_location(monster->inventory, INV_LOC_CREATURE_WIELD1);
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
    struct itm_item *weapon_item = NULL;

    for (int i = 0; i < WEAPON_TYPE_MAX; i++) {
        weapon_item = fght_get_working_weapon(monster, i, hand);
        if (weapon_item != NULL) i = WEAPON_TYPE_MAX;
    }
    if (weapon_item == NULL) return false;
    if (itm_verify_item(weapon_item) == false) return false;
    wpn = &weapon_item->specific.weapon;

    int dmg_die_sz = 10;
    if (wpn->nr_dmg_die == 0) dmg_die_sz = 5;
    for (int h = 0; h < hits; h++) {
        int dmg = random_xd10(r, wpn->nr_dmg_die);
        if (wpn->nr_dmg_die == 0) dmg = random_xd5(r, 1); /*TODO Emperors fury */

        int dmg_add = wpn->dmg_addition;
        enum msr_hit_location mhl = msr_get_hit_location(target, random_d100(r) );
        int penetration = wpn->penetration;
        int armour = msr_calculate_armour(target, mhl);
        int toughness = msr_calculate_characteristic_bonus(target, MSR_CHAR_TOUGHNESS);
        struct itm_item *armour_item = msr_get_armour_from_hitloc(target, mhl);
        struct item_wearable_specific *amr = NULL;
        if ( (armour_item != NULL) && (wearable_is_type(armour_item, WEARABLE_TYPE_ARMOUR) ) ) amr = &armour_item->specific.wearable;

        {
            /* Modifiers to damage here */
            {
                /* Add strength bonus */
                if ( (wpn->weapon_type == WEAPON_TYPE_MELEE) || (wpn->weapon_type == WEAPON_TYPE_THROWN) ) dmg_add += msr_calculate_characteristic_bonus(monster, MSR_CHAR_STRENGTH);

                /* Best Quality gains +1 damage */
                if (weapon_item->quality >= ITEM_QUALITY_BEST) dmg_add += FGHT_MODIFIER_QUALITY_TO_DMG_BEST;

            }

            /* Modifiers to penetration here */
            {}
            /* Modifiers to toughness here */
            {}
            /* Modifiers to armour here */
            {
                /* Armour counts double against primitive weapons, primitive armour counts as half against weapons, except against each other. */
                if ( (wpn->special_quality & WEAPON_SPEC_QUALITY_PRIMITIVE) > 0) armour *= 2;
                if ( (amr != NULL) && ( (amr->special_quality & WEARABLE_SPEC_QUALITY_PRIMITIVE) > 0) ) armour /= 2;
            }
        }

        You_action(monster, "do %d damage.", dmg + dmg_add);
        armour = MAX((armour - penetration), 0); /* penetration only works against armour */
        dmg = MAX((dmg + dmg_add) - (armour  + toughness), 0);
        Monster_action_end(monster, "does %d damage.", dmg);
        msr_do_dmg(target, dmg, mhl, gbl_game->current_map);

        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Doing %d%s+%d damage => %d, %d wnds left.", wpn->nr_dmg_die, random_die_name(dmg_die_sz), wpn->dmg_addition, dmg, target->cur_wounds);
        if (target->dead) h = hits;
    }

    return true;
}

int fght_ranged_calc_tohit(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand, int ammo) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (ammo == 0) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;
    struct dc_map_entity *me = sd_get_map_me(&target->pos, gbl_game->current_map);
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *item = NULL;

    item = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (item == NULL) return -1;

    wpn = &item->specific.weapon;
    You_action(monster, "fire at %s with your %s", target->ld_name, item->sd_name);
    Monster_action(monster, "fires at you with his %s", item->sd_name);

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_BALISTIC_SKILL);
    int to_hit_mod = 0;

    {/*add to-hit modifiers here*/ 

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

        /* Add lighting modifiers */
        if (me->in_sight == false) to_hit_mod += FGHT_MODIFIER_VISION_COMPLETE_DARKNESS;
        if (me->visible == false)  to_hit_mod += FGHT_MODIFIER_VISION_DARKNESS;
        if ( (me->in_sight == true) && (me->light_level == 0) ) to_hit_mod += FGHT_MODIFIER_VISION_SHADOWS;

        /* Quality modifiers */
        if (item->quality == ITEM_QUALITY_BEST) to_hit_mod += FGHT_MODIFIER_QUALITY_TO_HIT_BEST;
        else if (item->quality == ITEM_QUALITY_GOOD) to_hit_mod += FGHT_MODIFIER_QUALITY_TO_HIT_GOOD;
        else if (item->quality == ITEM_QUALITY_POOR) to_hit_mod += FGHT_MODIFIER_QUALITY_TO_HIT_POOR;

        /* Weapon talent of used weapon */
        if (msr_check_talent(monster, wpn->wpn_talent) == false) to_hit_mod -= FGHT_MODIFIER_UNTRAINED_WEAPON;

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

        to_hit += to_hit_mod;
    }

    int roll = random_d100(r);

    /* Calculate jamming threshold*/
    int jammed_threshold = FGHT_RANGED_JAM;
    if ( (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) || (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) ) jammed_threshold = MIN(FGHT_RANGED_JAM_SEMI,jammed_threshold);
    if ( (wpn->special_quality & WEAPON_SPEC_QUALITY_UNRELIABLE) > 0) jammed_threshold = MIN(FGHT_RANGED_JAM_UNRELIABLE, jammed_threshold);
    if (item->quality == ITEM_QUALITY_POOR) jammed_threshold = MIN(to_hit, jammed_threshold);

    /* Do jamming test */
    if (roll >= jammed_threshold) {
        int reltest = -1;
        wpn->jammed = true;
        if ( (wpn->special_quality & WEAPON_SPEC_QUALITY_RELIABLE) > 0) {
            /* If the weapon is reliable, it only jamms on a 10 on a d10, when there is a chance to jam */
            if ( (reltest = random_xd10(r,1) ) <= 9) {
                wpn->jammed = false;
            }
        }

        if (wpn->jammed) {
            You_action_end(monster, "and your weapon jams.", fght_weapon_hand_name(hand) );
            Monster_action_end(monster, "and his weapon jams", item->sd_name); /*TODO gender*/
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Weapon jamm with roll %d, theshold %d, 2nd roll %d", roll, jammed_threshold, reltest);
            return -1;
        }
    }

    if (to_hit <= 0) {
        You_action_end(monster, "and miss the shot by a huge margin.");
        Monster_action_end(monster, "and missed you with a huge margin.");
        return -1;
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Shot attempt with calcBS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        int dos = (to_hit - roll) / 10;

        /* Single Shot: max 1 hit */
        if (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) return MIN(ammo, 1);
        /* Automatic 1 hit + 1 for ever DoS */
        else if (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) return MIN(ammo, 1 + dos);
        /* Semi automatic: 1 hit + 1 per 2 DoS*/
        else if (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) return MIN(ammo, 1 + (dos/2) );
        return -1;
    }

    You_action_end(monster, "and miss.");
    Monster_action_end(monster, "and he misses."); /*TODO gender*/

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
    You_action(monster, "slash at %s with your %s", target->ld_name, item->sd_name);
    Monster_action(monster, "slashes at you with his %s", item->sd_name);

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_WEAPON_SKILL);
    int to_hit_mod = 0;

    {/*add to-hit modifiers here*/ 
        /* Add lighting modifiers */

        /* Offhand Weapon */
        if (hand == FGHT_OFF_HAND) to_hit_mod += FGHT_MODIFIER_OFF_HAND;
        if ( (wpn->special_quality & WEAPON_SPEC_QUALITY_UNARMED) > 0) to_hit_mod += FGHT_MELEE_UNARMED;

        if (target->size == MSR_SIZE_AVERAGE) to_hit_mod += FGHT_MODIFIER_SIZE_AVERAGE;
        else if (target->size == MSR_SIZE_MASSIVE) to_hit_mod += FGHT_MODIFIER_SIZE_MASSIVE;
        else if (target->size == MSR_SIZE_ENORMOUS) to_hit_mod += FGHT_MODIFIER_SIZE_ENORMOUS;
        else if (target->size == MSR_SIZE_HULKING) to_hit_mod += FGHT_MODIFIER_SIZE_HULKING;
        else if (target->size == MSR_SIZE_SCRAWY) to_hit_mod += FGHT_MODIFIER_SIZE_SCRAWNY;
        else if (target->size == MSR_SIZE_PUNY) to_hit_mod += FGHT_MODIFIER_SIZE_PUNY;
        else if (target->size == MSR_SIZE_MINISCULE) to_hit_mod += FGHT_MODIFIER_SIZE_MINISCULE;

        /* Weapon talent of used weapon */
        if (msr_check_talent(monster, wpn->wpn_talent) == false) to_hit_mod -= FGHT_MODIFIER_UNTRAINED_WEAPON;

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

        to_hit += to_hit_mod;
    }

    int roll = random_d100(r);
    if (to_hit <= 0) {
        You_action_end(monster, "and miss by a huge margin.");
        Monster_action_end(monster, "and missed you with a huge margin.");
        return -1;
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Melee attempt with calcWS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return 1;
    }

    You_action_end(monster, "and miss.");
    Monster_action_end(monster, "and he misses.");

    return -1;
}

bool fght_melee(struct random *r, struct msr_monster *monster, struct msr_monster *target) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (monster->faction == target->faction) return false; /* do not attack members of same faction */
    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) return false;
    if (cd_pyth(&monster->pos, &target->pos) > 1) return false;
    int hits = 0;

    /* Do damage */
    hits = fght_melee_calc_tohit(r, monster, target, FGHT_MAIN_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
    hits = fght_melee_calc_tohit(r, monster, target, FGHT_OFF_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_OFF_HAND);
    hits = fght_melee_calc_tohit(r, monster, target, FGHT_CREATURE_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_CREATURE_HAND);

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
    int path_len = lof_calc_path(&monster->pos, e, path, ARRAY_SZ(path));
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

