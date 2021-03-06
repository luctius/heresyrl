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

#include <sys/param.h>
#include <assert.h>
#include <string.h>


#include "fight.h"
#include "heresyrl_def.h"
#include "inventory.h"
#include "random.h"
#include "game.h"
#include "turn_tick.h"
#include "fov/sight.h"
#include "ui/animate.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "items/items.h"
#include "status_effects/status_effects.h"
#include "status_effects/ground_effects.h"

#define MAX_TO_HIT_MODS (30)
static int tohit_desc_ctr = 0;
static struct tohit_desc tohit_descr_lst[MAX_TO_HIT_MODS];

/* These macro's collect the reasons for the calculated tohit and put them in the tohit_descr_lst, so that the gui can give a list of reasons. */
#define CALC_TOHIT_INIT() do { tohit_desc_ctr = 0; lg_debug("calculating hit (%s)", __func__); } while (0);
/* WARNING: no ';' at the end!*/
#define CALC_TOHIT(expr, mod, msg) \
    if (expr) { \
        to_hit_mod += mod; \
        tohit_descr_lst[tohit_desc_ctr].description = msg; \
        tohit_descr_lst[tohit_desc_ctr].modifier = mod; \
        tohit_desc_ctr++; \
        lg_print(msg " (%d)", mod); \
        assert(tohit_desc_ctr < MAX_TO_HIT_MODS); \
    }

int fght_ranged_calc_tohit(struct msr_monster *monster, coord_t *tpos, struct itm_item *witem, enum fght_hand hand, bool throwing) {
    if (msr_verify_monster(monster) == false) return -1;
    struct dm_map *map = gbl_game->current_map;

    /* check los with a rediculous radius. if true, it means that there is a LoS. */
    if (sgt_has_los(map, &monster->pos, tpos, map->sett.size.x + map->sett.size.y) == false) return -1;

    struct dm_map_entity *me = dm_get_map_me(tpos, gbl_game->current_map);
    struct msr_monster *target = me->monster;
    if (target != NULL) {
        if (msr_verify_monster(target) == false) return -1;
    }

    if ( (throwing == false) && itm_is_type(witem, ITEM_TYPE_WEAPON) && wpn_is_type(witem, WEAPON_TYPE_THROWN) ) return -1;

    int to_hit = msr_calculate_skill(monster, MSR_SKILLS_RANGED);
    int to_hit_mod = 0;

    CALC_TOHIT_INIT()

    {/*add to-hit modifiers here*/

        /* Offhand Weapon */
        CALC_TOHIT(hand == FGHT_OFF_HAND, FGHT_MODIFIER_OFF_HAND, "using off-hand")

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
        int dis_in_meters = cd_pyth(&monster->pos, tpos) * FGHT_RANGE_MULTIPLIER;
        int weapon_range = 1;

        CALC_TOHIT(itm_is_type(witem, ITEM_TYPE_WEAPON) == false || wpn_is_type(witem, WEAPON_TYPE_MELEE), FGHT_MODIFIER_IMPROVISED_WEAPON, "this is an improvised weapon")
        else if (itm_is_type(witem, ITEM_TYPE_WEAPON) && ( (wpn_is_type(witem, WEAPON_TYPE_RANGED) || wpn_is_type(witem, WEAPON_TYPE_THROWN) ) ) ) {
            struct item_weapon_specific *wpn = &witem->specific.weapon;

            CALC_TOHIT(throwing && wpn_is_type(witem, WEAPON_TYPE_RANGED), FGHT_MODIFIER_IMPROVISED_WEAPON, "this is an improvised weapon")
            else CALC_TOHIT(msr_has_talent(monster, wpn->wpn_talent) == false, FGHT_MODIFIER_UNTRAINED_WEAPON, "you are untrained in this weapon") /* Weapon talent of used weapon */

            if (throwing && wpn_is_type(witem, WEAPON_TYPE_THROWN) ) weapon_range = wpn->range;
            else if (throwing == false && wpn_is_type(witem, WEAPON_TYPE_RANGED) ) weapon_range = wpn->range;

            /* Quality modifiers */
            CALC_TOHIT(itm_has_quality(witem, ITEM_QLTY_BEST), FGHT_MODIFIER_QLTY_TO_HIT_BEST, "your weapon is of best quality")
            else CALC_TOHIT(itm_has_quality(witem, ITEM_QLTY_GOOD), FGHT_MODIFIER_QLTY_TO_HIT_GOOD, "your weapon is of good quality")
            else CALC_TOHIT(itm_has_quality(witem, ITEM_QLTY_POOR), FGHT_MODIFIER_QLTY_TO_HIT_POOR, "your weapon is of poor quality")

            /* weapon settings */
            if (wpn->weapon_type == WEAPON_TYPE_RANGED) {
                CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_SINGLE, FGHT_RANGED_MODIFIER_ROF_SINGLE, "using single shot")
                else CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_SEMI, FGHT_RANGED_MODIFIER_ROF_SEMI, "using semi automatic")
                else CALC_TOHIT(wpn->rof_set == WEAPON_ROF_SETTING_AUTO, FGHT_RANGED_MODIFIER_ROF_AUTO, "using full automatic")
            }
        }

        /* Thrown weapon range is calculated by a range number times their strength bonus */
        if (throwing) weapon_range *= msr_calculate_characteristic_bonus(monster, MSR_CHAR_STRENGTH);

        lg_debug("distance: %d, wpn_range: %d", distance, weapon_range);

        {
            /* normally you would not be able to fire with a normal weapon and get this penaly with a pistol, but that is too harsh since you cannot disengage. */

            bool in_melee = false;
            for (int i = 0; i < coord_nhlo_table_sz && in_melee == false; i++) {
                coord_t pos = cd_add(&monster->pos, &coord_nhlo_table[i]);
                struct dm_map_entity *mon_me = dm_get_map_me(&pos, gbl_game->current_map);
                if (mon_me->monster != NULL && mon_me->monster->faction != monster->faction)  {
                    CALC_TOHIT( (distance == FGHT_MELEE_RANGE) && (wpn_is_catergory(witem, WEAPON_CATEGORY_1H_RANGED) ), 0, "you are in melee combat with a pistol")
                    else CALC_TOHIT(true, FGHT_RANGED_MODIFIER_MELEE, "you are in melee combat")
                    in_melee = true;
                }
            }

            if (!in_melee) {
                CALC_TOHIT(dis_in_meters >= (weapon_range * 3), FGHT_RANGED_MODIFIER_EXTREME_RANGE, "target is at extreme range")
                else CALC_TOHIT(dis_in_meters >= (weapon_range * 2), FGHT_RANGED_MODIFIER_LONG_RANGE, "target is at long range")
                else CALC_TOHIT(distance <= FGHT_POINT_BLANK_RANGE, FGHT_RANGED_MODIFIER_POINT_BLACK, "target is at point-blank range")
                else CALC_TOHIT(dis_in_meters <= (weapon_range * 0.5), FGHT_RANGED_MODIFIER_SHORT_RANGE, "target is at short range")
            }
        }


        { /* Lighting modifiers */
            int far_range = msr_get_far_sight_range(monster);
            int medium_range = msr_get_medium_sight_range(monster);
            int near_range = msr_get_near_sight_range(monster);

            /* target is out of sight range... */
            CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, far_range) == false, FGHT_MODIFIER_VISION_COMPLETE_DARKNESS, "target is in complete darkness")
            else if (me->light_level == 0) {
                /* target is within far sight, but there is no light on the tile */
                CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, medium_range) == false, FGHT_MODIFIER_VISION_DARKNESS, "target is in darkness")
                /* target is within medium sight, but there is no light on the tile */
                else CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, near_range) == false, FGHT_MODIFIER_VISION_SHADOWS, "target is in shadows")
            }
        }

        if (target != NULL) {
            /* Conditions */
            CALC_TOHIT(se_has_effect(target, EF_STUNNED), FGHT_MODIFIER_STATUS_EFFECT_STUNNED, "target is stunned")

            struct itm_item *tgt_witem1 = inv_get_item_from_location(target->inventory, INV_LOC_MAINHAND_WIELD);
            struct itm_item *tgt_witem2 = inv_get_item_from_location(target->inventory, INV_LOC_OFFHAND_WIELD);
            CALC_TOHIT( ( (tgt_witem1 != NULL) && itm_is_type(tgt_witem1, ITEM_TYPE_WEAPON) && wpn_has_spc_quality(tgt_witem1, WPN_SPCQLTY_SHIELD) ) ||
                        ( (tgt_witem2 != NULL) && itm_is_type(tgt_witem2, ITEM_TYPE_WEAPON) && wpn_has_spc_quality(tgt_witem2, WPN_SPCQLTY_SHIELD) ),
                                         FGHT_RANGED_MODIFIER_SHIELD, "target is equiped with a shield")
        }

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        else if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;

    }
    return to_hit + to_hit_mod;
}

int fght_melee_calc_tohit(struct msr_monster *monster, coord_t *tpos, struct itm_item *witem, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return -1;
    struct dm_map *map = gbl_game->current_map;
    if (witem == NULL) return -1;

    /* check los with a rediculous radius. if true, it means that there is a LoS. */
    if (sgt_has_los(map, &monster->pos, tpos, map->sett.size.x + map->sett.size.y) == false) return false;

    struct dm_map_entity *me = dm_get_map_me(tpos, gbl_game->current_map);
    struct msr_monster *target = me->monster;
    if (target != NULL) {
        if (msr_verify_monster(target) == false) return -1;
    }

    int to_hit = msr_calculate_skill(monster, MSR_SKILLS_MELEE);
    int to_hit_mod = 0;

    CALC_TOHIT_INIT()

    {/*add to-hit modifiers here*/

        /* Dual-wielding and Off-hand Weapon */
        if (hand == FGHT_OFF_HAND) {
            CALC_TOHIT(true, FGHT_MODIFIER_OFF_HAND, "using off-hand")
            CALC_TOHIT(wpn_has_spc_quality(witem, WPN_SPCQLTY_LIGHT), FGHT_MODIFIER_DUAL_WIELD_WEAPON_LIGHT,  "your weapon is light")
            CALC_TOHIT( (hand == FGHT_OFF_HAND) && msr_has_talent(monster, TLT_1_AMBIDEXTRIOUS), FGHT_MODIFIER_OFF_HAND_DUAL_WIELD,  "you are ambidextrous")
        }

        if ( (inv_loc_empty(monster->inventory, INV_LOC_MAINHAND_WIELD) == false) &&
             (inv_loc_empty(monster->inventory, INV_LOC_OFFHAND_WIELD) == false) ) {
            CALC_TOHIT(hand == FGHT_MAIN_HAND, FGHT_MODIFIER_MAIN_HAND_DUAL_WIELD, "you are dual wielding, this is your main hand")
            else CALC_TOHIT(hand == FGHT_OFF_HAND,  FGHT_MODIFIER_OFF_HAND_DUAL_WIELD,  "you are dual wielding, this is your off-hand")

            CALC_TOHIT(msr_has_talent(monster, TLT_2_TWO_WEAPON_FIGHTING),  FGHT_MODIFIER_DUAL_WIELD_TWO_WEAPON_FIGHTING,  "you are trained in dual wielding")
            CALC_TOHIT( (hand == FGHT_MAIN_HAND) && wpn_has_spc_quality(witem, WPN_SPCQLTY_LIGHT), FGHT_MODIFIER_DUAL_WIELD_WEAPON_LIGHT,  "your weapon is light")
        }

        /* Unarmed */
        CALC_TOHIT(wpn_has_spc_quality(witem, WPN_SPCQLTY_UNARMED), FGHT_MODIFIER_MELEE_UNARMED, "you are unarmed")

        /* Target size modifiers */
        if (target != NULL) {
            CALC_TOHIT(target->size == MSR_SIZE_AVERAGE, FGHT_MODIFIER_SIZE_AVERAGE, "target is average size")
            else CALC_TOHIT(target->size == MSR_SIZE_MASSIVE, FGHT_MODIFIER_SIZE_MASSIVE, "target is of massive size")
            else CALC_TOHIT(target->size == MSR_SIZE_ENORMOUS, FGHT_MODIFIER_SIZE_ENORMOUS, "target is of enormous size")
            else CALC_TOHIT(target->size == MSR_SIZE_HULKING, FGHT_MODIFIER_SIZE_HULKING, "target is of hulking size")
            else CALC_TOHIT(target->size == MSR_SIZE_SCRAWNY, FGHT_MODIFIER_SIZE_SCRAWNY, "target is of scrawny size")
            else CALC_TOHIT(target->size == MSR_SIZE_PUNY, FGHT_MODIFIER_SIZE_PUNY, "target is of puny size")
            else CALC_TOHIT(target->size == MSR_SIZE_MINISCULE, FGHT_MODIFIER_SIZE_MINISCULE, "target is of miniscule size")

            struct itm_item *ti = fght_get_working_weapon(target, WEAPON_TYPE_MELEE, FGHT_MAIN_HAND);
            if (ti == NULL) ti = fght_get_working_weapon(target, WEAPON_TYPE_MELEE, FGHT_OFF_HAND);
            CALC_TOHIT(ti == NULL, FGHT_MODIFIER_MELEE_TGT_NO_MELEE, "target does not wield a melee weapon")
        }

        { /* Lighting modifiers */
            int far_range = msr_get_far_sight_range(monster);
            int medium_range = msr_get_medium_sight_range(monster);
            //int near_range = msr_get_near_sight_range(monster);

            /* target is out of sight range... */
            CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, far_range) == false, FGHT_MODIFIER_VISION_COMPLETE_DARKNESS, "target is in complete darkness")
            else if (me->light_level == 0) {
                /* target is within far sight, but there is no light on the tile */
                CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, medium_range) == false, FGHT_MODIFIER_VISION_DARKNESS, "target is in darkness")
                /* target is within medium sight, but there is no light on the tile */
                //else CALC_TOHIT(sgt_has_los(map, &monster->pos, tpos, near_range) == false, FGHT_MODIFIER_VISION_SHADOWS, "target is in shadows")
                else CALC_TOHIT(true, FGHT_MODIFIER_VISION_SHADOWS, "target is in shadows")
            }
        }

        /* Weapon talent of used weapon */
        CALC_TOHIT(itm_is_type(witem, ITEM_TYPE_WEAPON) == false || wpn_is_type(witem, WEAPON_TYPE_RANGED || wpn_is_type(witem, WEAPON_TYPE_THROWN) ), FGHT_MODIFIER_IMPROVISED_WEAPON, "this is an improvised weapon")
        else if (itm_is_type(witem, ITEM_TYPE_WEAPON) && wpn_is_type(witem, WEAPON_TYPE_MELEE) ) {
            struct item_weapon_specific *wpn = &witem->specific.weapon;
            CALC_TOHIT(msr_has_talent(monster, wpn->wpn_talent) == false, FGHT_MODIFIER_UNTRAINED_WEAPON, "you are untrained in this weapon")
        }

        /* Conditions */
        if (target != NULL) {
            CALC_TOHIT(se_has_effect(target, EF_STUNNED), FGHT_MODIFIER_STATUS_EFFECT_STUNNED, "target is stunned")
        }

        /* Maximum modifier, keep these at the end! */
        if (to_hit_mod < -FGHT_MODIFIER_MAX) to_hit_mod = -FGHT_MODIFIER_MAX;
        else if (to_hit_mod > FGHT_MODIFIER_MAX)  to_hit_mod = FGHT_MODIFIER_MAX;
    }
    return to_hit + to_hit_mod;
}

/* retreive an description from the description array. when there is no more, it return NULL */
struct tohit_desc *fght_get_tohit_mod_description(int idx) {
    if (idx >= tohit_desc_ctr) return NULL;
    return &tohit_descr_lst[idx];
}

/* Monster can be NULL if the weapon is an indirect damage weapon such as a grenade. */
int fght_calc_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, struct itm_item *witem, enum msr_hit_location mhl) {
    if (msr_verify_monster(target) == false) return -1;
    if (itm_verify_item(witem) == false) return -1;
    if (r == NULL) return -1;
    if (hits < 1) return -1;
    struct item_weapon_specific *wpn = &witem->specific.weapon;

    if (wpn->weapon_category != WEAPON_CATEGORY_THROWN_GRENADE) {
        /* Do additional check if the weapon is a direct instead of an indirect weapon */
        if (msr_verify_monster(monster) == false) return -1;
    }

    /*
       retreive hit location.
        TODO: right now it only gives 1 hit location per
        attempt, even with burst for example. The dark
        heresy rules state that a hit location table
        should be used.

        Not using such a system gives advantage to
        burst and auto fire because they do more
        damage to a certain location. For now I'll
        leave it like this.
     */
    int total_damage = 0;

    int dmg_die_sz = 10;
    if (wpn->nr_dmg_die == 0) dmg_die_sz = 5;
    for (int h = 0; h < hits; h++) {
        int dmg = 0;
        int rnd_dmg = 0;
        int die_dmg;
        do {
            die_dmg = random_xd10(r, wpn->nr_dmg_die);
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_IMPACT) ) {
                int dmg2 = random_xd10(r, wpn->nr_dmg_die);
                if (dmg2 > die_dmg) die_dmg = dmg2;
            }
            dmg += die_dmg;
        }
        while (die_dmg == 10 && monster != NULL && monster->is_player); /* Ulrics Fury!  */

        int dmg_add = wpn->dmg_addition;
        int penetration = wpn->penetration;
        int armour = msr_calculate_armour(target, mhl);
        int toughness = msr_calculate_characteristic_bonus(target, MSR_CHAR_TOUGHNESS);
        struct itm_item *aitem = msr_get_armour_from_hitloc(target, mhl);

        {
            /* Modifiers to damage here */
            {
                /* Add strength bonus */
                if ( wpn_is_type(witem, WEAPON_TYPE_MELEE) || wpn_is_catergory(witem, WEAPON_CATEGORY_THROWN_WEAPON) ) dmg_add += msr_calculate_characteristic_bonus(monster, MSR_CHAR_STRENGTH);

                /* Best Quality gains +1 damage */
                if (itm_has_quality(witem, ITEM_QLTY_BEST) ) dmg_add += FGHT_MODIFIER_QLTY_TO_DMG_BEST;

            }

            /* Modifiers to penetration here */
            {}
            /* Modifiers to toughness here */
            {}
            /* Modifiers to armour here */
            {
                /*  Armour counts double against primitive weapons, primitive armour counts as half against weapons, except against each other. */
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_PRIMITIVE) ) armour *= 2;
                if ( (aitem != NULL) && wbl_has_spc_quality(aitem, WBL_SPCQLTY_PRIMITIVE) > 0) armour /= 2;
            }
        }

        lg_debug("Armour %d, penetration %d, toughness %d, dmg %d, dmg_add %d", armour, penetration, toughness, dmg, dmg_add);
        armour = MAX((armour - penetration), 0); /* penetration only works against armour */
        rnd_dmg = MAX((dmg + dmg_add) - (armour  + toughness), 0);
        total_damage += rnd_dmg;

        if (monster != NULL) {
            You(monster,                    "hit and do " cs_DAMAGE "%d" cs_CLOSE " damage.", MAX((dmg + dmg_add) - (armour  + toughness), 0));
            Monster_tgt_he(monster, target, "hits and does " cs_DAMAGE "%d" cs_CLOSE " damage.", MAX((dmg + dmg_add) - (armour  + toughness), 0));
        }
        else {
            Event_msg(&target->pos, "It does %d to %s.", total_damage, msr_ldname(target) );
        }

        Info("Doing %d%s+%d damage => %d, %d wnds left.", wpn->nr_dmg_die, random_die_name(dmg_die_sz), dmg_add, dmg, target->wounds.curr);
        msr_do_dmg(target, msr_ldname(monster), rnd_dmg, wpn->dmg_type, mhl);
        if (target->dead) h = hits;
    }

    if ( (target->dead == false) && (total_damage > 0) ) {
        if (wpn->convey_status_effect != SEID_NONE) {
            const char *origin  = witem->ld_name;
            if (monster != NULL) origin = msr_ldname(monster);
            assert(se_add_status_effect(target, wpn->convey_status_effect, origin) );
        }
    }
    return total_damage;
}

bool fght_do_weapon_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (r == NULL) return false;
    if (hits < 1) return false;

    struct item_weapon_specific *wpn = NULL;
    struct itm_item *witem = NULL;

    /* This function is weapon type agnostic, but since we know
       we did hit with this hand, we don't really care with what */
    for (int i = 0; i < WEAPON_TYPE_MAX; i++) {
        witem = fght_get_working_weapon(monster, i, hand);
        if (witem != NULL) i = WEAPON_TYPE_MAX;
    }
    if (witem == NULL) return false;
    if (itm_verify_item(witem) == false) return false;
    wpn = &witem->specific.weapon;

    /* exclude grenades, because the are not direct damage weapons. */
    if (wpn->weapon_category == WEAPON_CATEGORY_THROWN_GRENADE) return false;

    enum msr_hit_location mhl = msr_get_hit_location(target, random_d100(r));

    You(monster,                 "%s at %s's %s.", itm_you_use_desc(witem), msr_ldname(target), msr_hitloc_name(target, mhl) );

    if (target->is_player) Monster_tgt(monster, target, "%s at " cs_PLAYER "your" cs_CLOSE " %s.", itm_msr_use_desc(witem), msr_hitloc_name(target, mhl) );
    else Monster_tgt(monster, target, "%s at %s %s.", itm_msr_use_desc(witem), msr_ldname(target), msr_hitloc_name(target, mhl) );

    fght_calc_dmg(r, monster, target, hits, witem, mhl);

    return true;
}

int fght_ranged_roll(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand, int ammo) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (ammo == 0) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return 0;
    struct item_weapon_specific *wpn = NULL;
    struct itm_item *witem = NULL;

    witem = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (witem == NULL) return -1;

    wpn = &witem->specific.weapon;

    int to_hit = fght_ranged_calc_tohit(monster, &target->pos, witem, hand, false);
    int roll = random_d100(r);

    /* Calculate jamming threshold*/
    int jammed_threshold = 101;
    if (wpn_has_spc_quality(witem, WPN_SPCQLTY_JAMS) ) {
        jammed_threshold = FGHT_RANGED_JAM;

        if ( (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) || (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) ) jammed_threshold = MIN(FGHT_RANGED_JAM_SEMI,jammed_threshold);
        if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNRELIABLE) ) jammed_threshold = MIN(FGHT_RANGED_JAM_UNRELIABLE, jammed_threshold);
        if (msr_has_talent(monster, wpn->wpn_talent) == false) jammed_threshold = MIN(FGHT_RANGED_JAM_UNRELIABLE, jammed_threshold);
        if (itm_has_quality(witem, ITEM_QLTY_POOR) ) jammed_threshold = MIN(to_hit, jammed_threshold);
    }

    lg_debug("roll %d vs to hit %d, jamm_threshold %d", roll, to_hit, jammed_threshold);

    /*
    You(monster,                 "%s at %s.",  itm_you_use_desc(witem), msr_ldname(target) );
    Monster_tgt(monster, target, "%s at %s.", itm_msr_use_desc(witem), msr_ldname(target) );
    */

    /* Do jamming test */
    if (roll >= jammed_threshold) {
        int reltest = -1;
        wpn->jammed = true;
        if (wpn_has_spc_quality(witem, WPN_SPCQLTY_RELIABLE) ) {
            /* If the weapon is reliable, it only jamms on a 10 on a d10, when there is a chance to jam */
            if ( (reltest = random_xd10(r,1) ) <= 9) {
                wpn->jammed = false;
            }
        }

        if (wpn->jammed) {
            Your(monster,                    "%s weapon jams.",  fght_weapon_hand_name(hand) );
            Monster_tgt_his(monster, target, "weapon jams.");
            lg_debug("Weapon jamm with roll %d, theshold %d, 2nd roll %d", roll, jammed_threshold, reltest);
            return -1;
        }
    }

    Info("Ranged to-hit: roll %d, target: %d", roll, to_hit);

    if (to_hit <= 0) {
        You(monster,                    "miss the shot by a huge margin.");
        Monster_tgt_he(monster, target, "misses the shot by a huge margin.");
        return 0;
    }

    if (roll < to_hit) {
        int dos = (to_hit - roll) / 10;
        if (fght_can_see(gbl_game->current_map, target, monster) ) {
            if (msr_can_use_evasion(monster, MSR_EVASION_DODGE) == true) {
                if (msr_use_evasion(target, monster, witem, MSR_EVASION_DODGE, dos, 0) == true) {
                    return 0;
                }
            }
        }

        /* Single Shot: max 1 hit */
        if (wpn->rof_set == WEAPON_ROF_SETTING_SINGLE) return MIN(ammo, 1);
        /* Automatic 1 hit + 1 for ever DoS */
        else if (wpn->rof_set == WEAPON_ROF_SETTING_AUTO) return MIN(ammo, 1 + dos);
        /* Semi automatic: 1 hit + 1 per 2 DoS*/
        else if (wpn->rof_set == WEAPON_ROF_SETTING_SEMI) return MIN(ammo, 1 + (dos/2) );
        return -1;
    }

    You(monster,                    "miss.");
    Monster_tgt_he(monster, target, "misses.");
    return 0;
}

int fght_melee_roll(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return -1;
    if (msr_verify_monster(target) == false) return -1;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) return -1;
    struct itm_item *witem = NULL;

    witem = fght_get_working_weapon(monster, WEAPON_TYPE_MELEE, hand);
    if (witem == NULL) return -1;

    /*
    You(monster,                 "%s at %s.", itm_you_use_desc(witem), msr_ldname(target));
    Monster_tgt(monster, target, "%s at %s.", itm_msr_use_desc(witem), msr_ldname(target) );
    */

    /* TODO add Melee attack options */

    int to_hit = fght_melee_calc_tohit(monster, &target->pos, witem, hand);
    int roll = random_d100(r);
    Info("Melee to-hit: roll %d, target: %d", roll, to_hit);

    if (to_hit <= 0) {
        You(monster,                    "miss by a huge margin.");
        Monster_tgt_he(monster, target, "misses by a huge margin.");
        return -1;
    }

    if (roll < to_hit) {
        if (fght_can_see(gbl_game->current_map, target, monster) ) {
            for (int i = 0; i < MSR_EVASION_MAX; i++) {
                if (msr_can_use_evasion(target, i) == true) {
                    int to_hit_DoS = (to_hit - roll) / 10;
                    if (msr_use_evasion(target, monster, witem, i, to_hit_DoS, 0) == true) {
                        return 0;
                    }
                    break;
                }
            }
        }
        return 1;
    }

    You(monster,                    "miss.");
    Monster_tgt_he(monster, target, "misses.");
    return -1;
}

int fght_thrown_roll(struct random *r, struct msr_monster *monster, coord_t *pos, struct itm_item *witem, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return -1;
    if (itm_verify_item(witem) == false) return -1;

    struct msr_monster *target = dm_get_map_me(pos, gbl_game->current_map)->monster;

    int to_hit = fght_ranged_calc_tohit(monster, pos, witem, hand, true);
    int roll = random_d100(r);
    if (to_hit <= 0) {
        You_msg(monster,                "miss by a huge margin.");
        Monster_tgt_he(monster, target, "misses by a huge margin.");
        return -1;
    }

    lg_debug("Throw attempt with calcBS: %d => %d", roll, to_hit);
    if (roll < to_hit) {
        return 1;
    }

    You_msg(monster,                "miss.");
    Monster_tgt_he(monster, target, "misses.");
    return ( (to_hit - roll) / 10) +1;
}

static enum fght_hand wpn_hand_list[] = { FGHT_MAIN_HAND, FGHT_OFF_HAND, FGHT_CREATURE_HAND, };

bool fght_melee(struct random *r, struct msr_monster *monster, struct msr_monster *target) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(target) == false) return false;
    if (monster->faction == target->faction) return false; /* do not attack members of same faction */
    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) == false) return false;
    if (cd_pyth(&monster->pos, &target->pos) > 1) return false;
    int hits = 0;

    for (int w = 0; w < (int) ARRAY_SZ(wpn_hand_list); w++) {
        enum fght_hand hand = wpn_hand_list[w];

        /* check of we can hit the target */
        hits = fght_melee_roll(r, monster, target, hand);

        /* Do the actual damage if we did score a hit. */
        if (hits > 0) {
            fght_do_weapon_dmg(r, monster, target, hits, hand);
        }

        /* Disable parry for this hand. */
        if (hand == FGHT_MAIN_HAND) {
            msr_disable_evasion(monster, MSR_EVASION_MAIN_HAND);
        }
        else if (hand == FGHT_OFF_HAND) {
            msr_disable_evasion(monster, MSR_EVASION_OFF_HAND);
        }
    }

    return true;
}

bool fght_explosion(struct random *r, struct itm_item *bomb, struct dm_map *map) {
    if (itm_verify_item(bomb) == false) return false;
    if (dm_verify_map(map) == false) return false;

    coord_t c = itm_get_pos(bomb);
    int radius = 0;
    if (wpn_has_spc_quality(bomb, WPN_SPCQLTY_BLAST_1) == true) radius = 1;
    if (wpn_has_spc_quality(bomb, WPN_SPCQLTY_BLAST_2) == true) radius = 2;
    if (wpn_has_spc_quality(bomb, WPN_SPCQLTY_BLAST_3) == true) radius = 3;
    if (wpn_has_spc_quality(bomb, WPN_SPCQLTY_BLAST_4) == true) radius = 4;

    lg_debug("Exploding bomb on %d,%d() with radius %d.", c.x, c.y, radius);

    Event_msg(&c, "%s explodes.", bomb->ld_name);

    coord_t *gridlist = NULL;
    int gridlist_sz = sgt_explosion(map, &c, radius, &gridlist);
    struct item_weapon_specific *wpn = &bomb->specific.weapon;

    for (int i = 0; i < gridlist_sz; i++) {
        struct dm_map_entity *me = dm_get_map_me(&gridlist[i], map);
        struct msr_monster *target = me->monster;
        if (target != NULL) {
            enum msr_hit_location mhl = msr_get_hit_location(target, random_d100(r));
            fght_calc_dmg(r, NULL, target, 1, bomb, mhl);

            if (wpn->convey_status_effect != SEID_NONE) {
                se_add_status_effect(target, wpn->convey_status_effect, bomb->sd_name);
            }
        }
        if (wpn->convey_ground_effect != GEID_NONE) {
            ge_create(wpn->convey_ground_effect, me);
        }
    }

    ui_animate_explosion(map, gridlist, gridlist_sz);
    return true;
}

/*
   We assume that the throwing weapon has been equiped
   before use, ma_do_throw should do that for the player
   with a slight cost.

TODO: let us throw things which are not weapons.
 */
bool fght_throw_item(struct random *r, struct msr_monster *monster, struct dm_map *map, coord_t *e, struct itm_item *witem, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(witem) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(e, &map->sett.size) == false) return false;
    if (sgt_has_los(map, &monster->pos, e, 1000) == false) return false;
    coord_t end = *e;

    lg_debug("Throwing item to (%d,%d)", e->x, e->y);

    /* Generate a path our projectile will take. Start at
       the shooter position, and continue the same path
       untill an obstacle is found.*/
    coord_t *path;
    int path_len = sgt_los_path(map, &monster->pos, &end, &path, false);
    ui_animate_projectile(map, path, path_len);

    /* if the path was succesfully created, free it here */
    if (path_len > 0) free(path);

    /* check of we can hit the target */
    int hits = fght_thrown_roll(r, monster, e, witem, hand);

    /* Do the actual damage if we did score a hit. */
    if (hits > 0) {
        /* and if there actually is an enemy there... */
        struct msr_monster *target = dm_get_map_me(e, map)->monster;
        if (target != NULL) {
            fght_do_weapon_dmg(r, monster, target, hits, hand);
        }
    }
    else {
        /* if we miss, scatter the object */

        int dis = random_xd5(r, 1);
        /* Maximum scatter is DoF. */
        if (dis > (hits * -1) ) dis = hits * -1;
        /* Maximum scatter is equal to the distance. */
        if (dis > cd_pyth(&monster->pos, e) ) dis = cd_pyth(&monster->pos, e);

        end = sgt_scatter(map, r, e, dis);
        lg_debug("%s is scattered towards (%d,%d)", witem->ld_name, end.x, end.y);

        /* I first wanted to do the animation in one go, scatter them animate the whole path
           But it is very possible that the scattered target is out of LoS of the origin.  */
        path = NULL;
        path_len = sgt_los_path(map, e, &end, &path, false);
        ui_animate_projectile(map, path, path_len);

        /* if the path was succesfully created, free it here */
        if (path_len > 0) free(path);
    }

    /* create a copy of the item to place it on the map. */
    struct itm_item *witem_copy = itm_create(witem->tid);
    if (witem_copy != NULL) {
        /* copy any changes from the item to its copy */
        witem_copy->energy = witem->energy;
        /* set the number of items in the stack to 1*/
        witem_copy->stacked_quantity = 1;

        /* decrease that number from the item in the monsters hand */
        witem->stacked_quantity -= 1;
        if (witem->stacked_quantity == 0) {
            /* remove the item from monsters inventory if that was the last one */
            if (msr_remove_item(monster, witem) ) itm_destroy(witem);
            lg_debug("No more copied in inventory, destroying last one.");
        }

        /* if the item is an grenade */
        if (itm_is_type(witem_copy, ITEM_TYPE_WEAPON) && wpn_is_catergory(witem_copy, WEAPON_CATEGORY_THROWN_GRENADE) ) {
            /* set the fuse. the item processing loop will then handle
               the explosion.  This does mean that the throwing code has
               has to set the energy. If that is not done, we set it here
               to a defualt value. This does mean that you cannot just throw
               a grenade and expect it not to explode, but that is a use
               case I doubt many will use.  */
            if (witem_copy->energy <= 0)  witem_copy->energy = TT_ENERGY_TURN;
            witem_copy->energy_action = true;

            lg_debug("Setting the fuse to %d.", witem_copy->energy);
        }

        /* and place it on the target/scatter position */
        itm_insert_item(witem_copy, gbl_game->current_map, &end);
    }

    return true;
}

bool fght_shoot(struct random *r, struct msr_monster *monster, struct dm_map *map, coord_t *e, enum fght_hand hand) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(&monster->pos, &map->sett.size) == false) return false;
    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) == false) return false;
    if (sgt_has_los(map, &monster->pos, e, 1000) == false) return false;
    struct itm_item *item = fght_get_working_weapon(monster, WEAPON_TYPE_RANGED, hand);
    if (item == NULL) return false;
    int ammo = 0;

    /* Check if we can actually shoot and substract ammo from our attempt. */
    struct item_weapon_specific *wpn = &item->specific.weapon;
    ammo = MIN(wpn->magazine_left, wpn->rof[wpn->rof_set]);
    if (wpn_uses_ammo(item) && ammo == 0) return false;

    /* Genereate a path our projectile will take. Start at
       the shooter position, and continue the same path
       untill an obstacle is found.*/
    coord_t *path;
    int path_len = sgt_los_path(map, &monster->pos, e, &path, true);

    /*
        Here we loop over the
     */
    int i = 1;
    int blocked_i = 0;
    bool has_hit = false;
    bool blocked = false;
    int unblocked_length = 0;
    int max_continue_length = 4; //maximum nr of tiles we continue after hitting a monster
    while ((i < path_len) && ( (blocked == false) || (blocked_i > i - (max_continue_length) ) ) ) {

        /* check if there is a monster on the current tile */
        if (blocked == false) {

            /* check for a valid target */
            if (dm_get_map_me(&path[i], map)->monster != NULL) {

                /* get the monster on the tile as our target */
                struct msr_monster *target = dm_get_map_me(&path[i], map)->monster;

                /* do weapon checks and roll tohit */
                int hits = fght_ranged_roll(r, monster, target, hand, ammo);

                /* do damage */
                if (hits > 0) {
                    if (fght_do_weapon_dmg(r, monster, target, hits, hand) ) has_hit = true;
                    //we can also splatter some blood on the target's tile
                }

                if (has_hit) {
                    /* continue our path if nothing hit */
                    blocked = true;
                    blocked_i = i;
                }
            }
        }

        if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&path[i], map), TILE_ATTR_TRAVERSABLE) == false) {
            blocked = true;

            if (has_hit) {
                /* here we can splatter blood against a wall behind the victim */
            }
        }
        if (blocked == false) {
            unblocked_length++;
        }
        i++;
    }

    /* if the path was succesfully created, free it here */
    if (path_len > 0) {
        ui_animate_projectile(map, path, unblocked_length +1);
        free(path);
    }

    if (wpn_uses_ammo(item) ) wpn->magazine_left -= ammo;
    if (unblocked_length >= 0) return true;
    return false;
}

const char *fght_weapon_hand_name(enum fght_hand hand) {
    switch (hand) {
        case FGHT_MAIN_HAND: return "main";
        case FGHT_OFF_HAND: return "off";
        default: return "unknown";
    }
}

bool fght_can_see(struct dm_map *map, struct msr_monster *monster, struct msr_monster *tgt) {
    if (dm_verify_map(map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;
    if (msr_verify_monster(tgt) == false) return false;
    if (monster == tgt) return true;

    /* Same faction can always see each other */
    if (monster->faction == tgt->faction) return true;

    lg_ai_debug(monster, "testing visual to %s", msr_ldname(tgt));

    struct dm_map_entity *me = dm_get_map_me(&tgt->pos, map);
    int near    = msr_get_near_sight_range(monster);
    int medium  = msr_get_medium_sight_range(monster);
    int far     = msr_get_far_sight_range(monster);

    if ( (monster->is_player) && (me->in_sight == false) ) return false;
    else if (sgt_has_los(map, &monster->pos, &tgt->pos, far) == false) return false;

    /* base modifiers are zero */
    int awareness_mod   = 0;
    int stealth_mod     = 0;

    /* near is easier to see, medium bit harder and far is again harder. */
    if (sgt_in_radius(map, &monster->pos, &tgt->pos, near) )   awareness_mod =  10;
    else if (sgt_in_radius(map, &monster->pos, &tgt->pos, medium) ) awareness_mod =  -1;
    else if (sgt_in_radius(map, &monster->pos, &tgt->pos, far) )    awareness_mod =  -2;

    /* cascading light levels, bright is triggered twice and thus +20 */
    awareness_mod -= cd_pyth(&monster->pos, &tgt->pos);
    if (me->light_level >= 5) awareness_mod += 10;
    if (me->light_level >  0) awareness_mod += 10;
    if (me->light_level <  0) stealth_mod   += 5;

    /* modify stealth by relative carrying weight */
    int tgt_cc = msr_calculate_carrying_capacity(tgt);
    int tgt_invweight = inv_get_weight(tgt->inventory);

    if (tgt_invweight > ( (tgt_cc * 70) / 100) ) stealth_mod -= 2;
    stealth_mod -= 5 * (tgt_invweight / tgt_cc);

    /* for each tile which is not a sight blocking this, it is harder to be stealthy.
       so try not to be in the open. */
    for (int i = 0; i < coord_nhlo_table_sz; i++) {
        coord_t c = cd_add(&tgt->pos, &coord_nhlo_table[i]);
        if (cd_within_bound(&c, &map->sett.size) == true) {
            struct dm_map_entity *cme = dm_get_map_me(&c, map);
            if (TILE_HAS_ATTRIBUTE(cme->tile, TILE_ATTR_TRANSPARENT) ||
                ( (cme->effect != NULL) && ( (cme->effect->flags & GR_EFFECTS_OPAQUE) == 0) ) ) {
                stealth_mod -= 1;
            }
        }
    }

    /* The actual check and Degree of Success */
    int awareness = msr_calculate_skill(monster, MSR_SKILLS_AWARENESS);
    int stealth   = msr_calculate_skill(tgt, MSR_SKILLS_STEALTH);
    int awareness_DoS = (awareness + awareness_mod) / 10;
    int stealth_DoS   = (stealth + stealth_mod)  / 10;

    lg_ai_debug(monster, "test see: (%d(+%d) vs %s %d(+%d) )", awareness, awareness_mod, msr_ldname(tgt), stealth, stealth_mod);
    if (awareness_DoS > stealth_DoS) {
        /* You(monster, "notice %s", msr_ldname(tgt) );*/
        /* Monster(monster, "notices %s", msr_ldname(tgt) );*/
        lg_ai_debug(monster, "sees %s", msr_ldname(tgt));
        return true;
    }

    lg_ai_debug(monster, "does not see %s", msr_ldname(tgt));
    return false;
}


/*
   Given a monster, a type of weapon and a hand,
   check if that weapon type exists in our monsters
   hand and return it.
 */
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
        if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
            item = inv_get_item_from_location(monster->inventory, INV_LOC_CREATURE_WIELD1);
        }
    }

    if (item == NULL) return NULL;
    if (itm_verify_item(item) == false) return NULL;
    if (item->specific.weapon.weapon_type != type) return NULL;

    return item;
}

/*
    same as fght_get_weapon except we also check
    if the weapon is in working status_effect.
 */
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

    if (type == WEAPON_TYPE_MELEE) {
        if (hand == FGHT_OFF_HAND) {
            /* Off hand can only be used once per round, and only if it is not used for an evasion that round. */
            if (msr_can_use_evasion(monster, MSR_EVASION_OFF_HAND) == false) {
                return NULL;
            }
        }
    }

    return item;
}
