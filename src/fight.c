#include <sys/param.h>
#include <assert.h>

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
#include "sight.h"

#define MAX_TO_HIT_MODS (30)
static int tohit_desc_ctr = 0;
static const char *tohit_descr_lst[MAX_TO_HIT_MODS];

#define CALC_TOHIT_INIT() do { tohit_desc_ctr = 0; lg_print("calculating hit (%s)", __func__); } while (0);
/* WARNING: no ';' at the end!*/
#define CALC_TOHIT(expr, mod, msg) if (expr) {to_hit_mod += mod; tohit_descr_lst[tohit_desc_ctr++] = msg; lg_print(msg " (%d)", mod); assert(tohit_desc_ctr < MAX_TO_HIT_MODS); }

int fght_ranged_calc_tohit(struct msr_monster *monster, coord_t *tpos, enum fght_hand hand) {
    struct dc_map_entity *me = sd_get_map_me(tpos, gbl_game->current_map);
    struct msr_monster *target = me->monster;

    struct itm_item *witem = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (witem == NULL) return -1;
    struct item_weapon_specific *wpn = &witem->specific.weapon;

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_BALISTIC_SKILL);
    int to_hit_mod = 0;

    CALC_TOHIT_INIT()

    {/*add to-hit modifiers here*/ 

        /* Offhand Weapon */
        CALC_TOHIT(hand == FGHT_OFF_HAND, FGHT_MODIFIER_OFF_HAND, "using off-hand")

        /* weapon settings */
        CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_SINGLE, FGHT_RANGED_MODIFIER_ROF_SINGLE, "using single shot")
        else CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_SEMI, FGHT_RANGED_MODIFIER_ROF_SEMI, "using semi automatic")
        else CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_AUTO, FGHT_RANGED_MODIFIER_ROF_AUTO, "using full automatic")

        /* Target size modifiers */
        if (target != NULL) {
            CALC_TOHIT(target->size == MSR_SIZE_AVERAGE, FGHT_MODIFIER_SIZE_AVERAGE, "target is average size")
            else CALC_TOHIT(target->size == MSR_SIZE_MASSIVE, FGHT_MODIFIER_SIZE_MASSIVE, "target is of massive size")
            else CALC_TOHIT(target->size == MSR_SIZE_ENORMOUS, FGHT_MODIFIER_SIZE_ENORMOUS, "target is of enormous size")
            else CALC_TOHIT(target->size == MSR_SIZE_HULKING, FGHT_MODIFIER_SIZE_HULKING, "target is of hulking size")
            else CALC_TOHIT(target->size == MSR_SIZE_SCRAWNY, FGHT_MODIFIER_SIZE_SCRAWNY, "target is of scrawny size")
            else CALC_TOHIT(target->size == MSR_SIZE_PUNY, FGHT_MODIFIER_SIZE_PUNY, "target is of puny size")
            else CALC_TOHIT(target->size == MSR_SIZE_MINISCULE, FGHT_MODIFIER_SIZE_MINISCULE, "target is of miniscule size")
        }

        /* Shooting Distances */
        int distance = cd_pyth(&monster->pos, tpos);
        lg_debug("distance == %d", distance);

            /* normally you would not be able to fire with a normal weapon and get this penaly with a pistol, but that is too harsh since you cannot disengage. */
            /* TODO add check for other enemies in melee. */
            CALC_TOHIT( (distance == FGHT_MELEE_RANGE) && (!wpn_is_catergory(witem, WEAPON_CATEGORY_PISTOL) ), FGHT_RANGED_MODIFIER_MELEE, "you are in melee combat")
            else CALC_TOHIT( (distance == FGHT_MELEE_RANGE), 0, "you are in melee combat with a pistol")
            else CALC_TOHIT(distance >= (wpn->range * 3), FGHT_RANGED_MODIFIER_EXTREME_RANGE, "target is at extreme range")
            else CALC_TOHIT(distance >= (wpn->range * 2), FGHT_RANGED_MODIFIER_LONG_RANGE, "target is at long range")
            else CALC_TOHIT(distance <= FGHT_POINT_BLANK_RANGE, FGHT_RANGED_MODIFIER_POINT_BLACK, "target is at point-blank range")
            else CALC_TOHIT(distance <= (wpn->range * 0.5), FGHT_RANGED_MODIFIER_SHORT_RANGE, "target is at short range")

        /* Lighting modifiers */
        CALC_TOHIT(me->in_sight == false, FGHT_MODIFIER_VISION_COMPLETE_DARKNESS, "target is in complete darkness")
        else CALC_TOHIT(me->visible == false, FGHT_MODIFIER_VISION_DARKNESS, "target is in darkness")
        else CALC_TOHIT((me->in_sight) && (me->light_level == 0), FGHT_MODIFIER_VISION_SHADOWS, "target is in shadows")

        /* Quality modifiers */
        CALC_TOHIT(itm_has_quality(witem, ITEM_QUALITY_BEST), FGHT_MODIFIER_QUALITY_TO_HIT_BEST, "your weapon is of best quality")
        else CALC_TOHIT(itm_has_quality(witem, ITEM_QUALITY_GOOD), FGHT_MODIFIER_QUALITY_TO_HIT_GOOD, "your weapon is of good quality")
        else CALC_TOHIT(itm_has_quality(witem, ITEM_QUALITY_POOR), FGHT_MODIFIER_QUALITY_TO_HIT_POOR, "your weapon is of poor quality")

        /* Weapon talent of used weapon */
        CALC_TOHIT(msr_has_talent(monster, wpn->wpn_talent) == false, FGHT_MODIFIER_UNTRAINED_WEAPON, "you are untrained in this weapon")

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        else if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

    }
    return to_hit + to_hit_mod;
}

int fght_melee_calc_tohit(struct msr_monster *monster, coord_t *tpos, enum fght_hand hand) {
    struct dc_map_entity *me = sd_get_map_me(tpos, gbl_game->current_map);
    struct msr_monster *target = me->monster;

    struct itm_item *witem = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, hand);
    if (witem == NULL) return -1;
    struct item_weapon_specific *wpn = &witem->specific.weapon;

    int to_hit = msr_calculate_characteristic(monster, MSR_CHAR_WEAPON_SKILL);
    int to_hit_mod = 0;

    CALC_TOHIT_INIT()

    {/*add to-hit modifiers here*/ 

        /* Offhand Weapon */
        CALC_TOHIT(hand == FGHT_OFF_HAND, FGHT_MODIFIER_OFF_HAND, "using off-hand")

        /* Unarmed */
        CALC_TOHIT(wpn_has_spc_quality(witem, WEAPON_SPEC_QUALITY_UNARMED), FGHT_MODIFIER_MELEE_UNARMED, "you are unarmed")

        /* Target size modifiers */
        if (target != NULL) {
            CALC_TOHIT(target->size == MSR_SIZE_AVERAGE, FGHT_MODIFIER_SIZE_AVERAGE, "target is average size")
            else CALC_TOHIT(target->size == MSR_SIZE_MASSIVE, FGHT_MODIFIER_SIZE_MASSIVE, "target is of massive size")
            else CALC_TOHIT(target->size == MSR_SIZE_ENORMOUS, FGHT_MODIFIER_SIZE_ENORMOUS, "target is of enormous size")
            else CALC_TOHIT(target->size == MSR_SIZE_HULKING, FGHT_MODIFIER_SIZE_HULKING, "target is of hulking size")
            else CALC_TOHIT(target->size == MSR_SIZE_SCRAWNY, FGHT_MODIFIER_SIZE_SCRAWNY, "target is of scrawny size")
            else CALC_TOHIT(target->size == MSR_SIZE_PUNY, FGHT_MODIFIER_SIZE_PUNY, "target is of puny size")
            else CALC_TOHIT(target->size == MSR_SIZE_MINISCULE, FGHT_MODIFIER_SIZE_MINISCULE, "target is of miniscule size")
        }

        /* Add lighting modifiers */
        CALC_TOHIT(me->in_sight == false, FGHT_MODIFIER_VISION_COMPLETE_DARKNESS, "target is in complete darkness")
        else CALC_TOHIT(me->visible == false, FGHT_MODIFIER_VISION_DARKNESS, "target is in darkness")
        else CALC_TOHIT((me->in_sight) && (me->light_level == 0), FGHT_MODIFIER_VISION_SHADOWS, "target is in shadows")

        /* Weapon talent of used weapon */
        CALC_TOHIT(msr_has_talent(monster, wpn->wpn_talent) == false, FGHT_MODIFIER_UNTRAINED_WEAPON, "you are untrained in this weapon")

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        else if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;
    }
    return to_hit + to_hit_mod;
}

const char *fght_get_tohit_mod_description(int idx) {
    if (idx >= tohit_desc_ctr) return NULL;
    return tohit_descr_lst[idx];
}

bool fght_do_weapon_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (r == NULL) return false;
    if (hits < 1) return false;

    struct item_weapon_specific *wpn = NULL;
    struct itm_item *witem = NULL;

    for (int i = 0; i < WEAPON_TYPE_MAX; i++) {
        witem = fght_get_working_weapon(monster, i, hand);
        if (witem != NULL) i = WEAPON_TYPE_MAX;
    }
    if (witem == NULL) return false;
    if (itm_verify_item(witem) == false) return false;
    wpn = &witem->specific.weapon;

    int total_damage = 0;
    enum msr_hit_location mhl = msr_get_hit_location(target, random_d100(r) );

    int dmg_die_sz = 10;
    if (wpn->nr_dmg_die == 0) dmg_die_sz = 5;
    for (int h = 0; h < hits; h++) {
        int dmg = random_xd10(r, wpn->nr_dmg_die);
        if (wpn->nr_dmg_die == 0) dmg = random_xd5(r, 1); /*TODO Emperors fury */

        int dmg_add = wpn->dmg_addition;
        int penetration = wpn->penetration;
        int armour = msr_calculate_armour(target, mhl);
        int toughness = msr_calculate_characteristic_bonus(target, MSR_CHAR_TOUGHNESS);
        struct itm_item *aitem = msr_get_armour_from_hitloc(target, mhl);

        {
            /* Modifiers to damage here */
            {
                /* Add strength bonus */
                
                if ( wpn_is_type(witem, WEAPON_TYPE_MELEE) || wpn_is_type(witem, WEAPON_TYPE_THROWN) ) dmg_add += msr_calculate_characteristic_bonus(monster, MSR_CHAR_STRENGTH);

                /* Best Quality gains +1 damage */
                if (itm_has_quality(witem, ITEM_QUALITY_BEST) ) dmg_add += FGHT_MODIFIER_QUALITY_TO_DMG_BEST;

            }

            /* Modifiers to penetration here */
            {}
            /* Modifiers to toughness here */
            {}
            /* Modifiers to armour here */
            {
                /* Armour counts double against primitive weapons, primitive armour counts as half against weapons, except against each other. */
                if (wpn_has_spc_quality(witem, WEAPON_SPEC_QUALITY_PRIMITIVE) > 0) armour *= 2;
                if ( (aitem != NULL) && wbl_has_spc_quality(aitem, WEARABLE_SPEC_QUALITY_PRIMITIVE) > 0) armour /= 2;
            }
        }

        lg_print("armour %d, penetration %d, toughness %d, dmg %d, dmg_add %d", armour, penetration, toughness, dmg, dmg_add);
        armour = MAX((armour - penetration), 0); /* penetration only works against armour */
        total_damage += MAX((dmg + dmg_add) - (armour  + toughness), 0);

        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Doing %d%s+%d damage => %d, %d wnds left.", wpn->nr_dmg_die, random_die_name(dmg_die_sz), wpn->dmg_addition, dmg, target->cur_wounds);
        if (target->dead) h = hits;
    }

    msg_plr(" and score");  msg_plr_number(" %d", hits); msg_plr(" hits and a total of"); msg_plr_number(" %d", total_damage); msg_plr(" damage.");
    msg_msr(" and scores"); msg_msr_number(" %d", hits); msg_msr(" hits and a total of"); msg_msr_number(" %d", total_damage); msg_msr(" damage.");

    msr_do_dmg(target, total_damage, mhl, gbl_game->current_map);
    return true;
}

int fght_ranged_roll(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand, int ammo) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (ammo == 0) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return -1;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *witem = NULL;

    witem = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (witem == NULL) return -1;

    wpn = &witem->specific.weapon;

    msg_plr("You fire at %s with your %s", msr_ldname(target), witem->sd_name);
    msg_msr("%s fires at %s with %s %s", msr_ldname(monster), msr_ldname(target), msr_gender_name(target, true), witem->sd_name);

    int to_hit = fght_ranged_calc_tohit(monster, &target->pos, hand);
    int roll = random_d100(r);

    /* Calculate jamming threshold*/
    int jammed_threshold = FGHT_RANGED_JAM;
    if ( (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) || (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) ) jammed_threshold = MIN(FGHT_RANGED_JAM_SEMI,jammed_threshold);
    if (wpn_has_spc_quality(witem, WEAPON_SPEC_QUALITY_UNRELIABLE) ) jammed_threshold = MIN(FGHT_RANGED_JAM_UNRELIABLE, jammed_threshold);
    if (itm_has_quality(witem, ITEM_QUALITY_POOR) ) jammed_threshold = MIN(to_hit, jammed_threshold);

    lg_print("roll %d vs to hit %d, jamm_threshold %d", roll, to_hit, jammed_threshold);

    /* Do jamming test */
    if (roll >= jammed_threshold) {
        int reltest = -1;
        wpn->jammed = true;
        if (wpn_has_spc_quality(witem, WEAPON_SPEC_QUALITY_RELIABLE) ) {
            /* If the weapon is reliable, it only jamms on a 10 on a d10, when there is a chance to jam */
            if ( (reltest = random_xd10(r,1) ) <= 9) {
                wpn->jammed = false;
            }
        }

        if (wpn->jammed) {
            msg_plr(" and your weapon jams.", fght_weapon_hand_name(hand) );
            msg_msr(" and %s weapon jams.", msr_gender_name(monster, true) );
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Weapon jamm with roll %d, theshold %d, 2nd roll %d", roll, jammed_threshold, reltest);
            return -1;
        }
    }

    if (to_hit <= 0) {
        msg_plr(" and miss the shot by a huge margin.");
        msg_msr(" and misses %s by a huge margin.", msr_ldname(target) );
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

    msg_plr(" and miss.");
    msg_msr(" and %s misses.", msr_gender_name(monster, false) );

    return 0;
}

int fght_melee_roll(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) return -1;
    struct itm_item *witem = NULL;

    witem = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, hand);
    if (witem == NULL) return -1;

    msg_plr("You slash at %s with your %s", msr_ldname(target), witem->sd_name);
    msg_msr("%s slashes at you with his %s", msr_ldname(monster), witem->sd_name);

    int to_hit = fght_melee_calc_tohit(monster, &target->pos, hand);
    int roll = random_d100(r);
    if (to_hit <= 0) {
        msg_plr(" and miss by a huge margin.");
        msg_msr(" and missed %s with a huge margin.", msr_ldname(target) );
        return -1;
    }

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "Melee attempt with calcWS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return 1;
    }

    msg_plr(" and miss.");
    msg_msr(" and %s misses.", msr_gender_name(monster, false) );
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
    msg_init(monster, target);
    hits = fght_melee_roll(r, monster, target, FGHT_MAIN_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
    msg_exit();

    msg_init(monster, target);
    hits = fght_melee_roll(r, monster, target, FGHT_OFF_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_OFF_HAND);
    msg_exit();

    msg_init(monster, target);
    hits = fght_melee_roll(r, monster, target, FGHT_CREATURE_HAND);
    fght_do_weapon_dmg(r, monster, target, hits, FGHT_CREATURE_HAND);
    msg_exit();

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

    coord_t *path;
    int path_len = sgt_los_path(gbl_game->sight, gbl_game->current_map, &monster->pos, e, &path, false);
    bool blocked = false;
    int unblocked_length = 0;

    int i = 1;
    bool animate = false;
    while ((i < path_len) && (blocked == false)) {
        if (sd_get_map_me(&path[i], map)->monster != NULL) {
            struct msr_monster *target = sd_get_map_me(&path[i], map)->monster;
            int hits = 0;


            /* Do damage */
            msg_init(monster, target);
            hits = fght_ranged_roll(r, monster, target, FGHT_MAIN_HAND, ammo1);
            fght_do_weapon_dmg(r, monster, target, hits, FGHT_MAIN_HAND);
            if (hits >= 0) animate = true;
            msg_exit();

            msg_init(monster, target);
            hits = fght_ranged_roll(r, monster, target, FGHT_OFF_HAND, ammo2);
            fght_do_weapon_dmg(r, monster, target, hits, FGHT_OFF_HAND);
            if (hits >= 0) animate = true;
            msg_exit();

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

    if (animate) ui_animate_projectile(map, path, unblocked_length +1, '*');
    if (path_len > 0) free(path);

    return unblocked_length;
}

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
            msg_plr("Your %s-hand weapon is jammed.", fght_weapon_hand_name(hand) );
            return NULL;
        }
        if (item->specific.weapon.magazine_left == 0) {
            msg_plr("Your %s-hand weapon is empty.", fght_weapon_hand_name(hand) );
            return NULL;
        }
    }

    return item;
}
