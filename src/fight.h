#ifndef FIGHT_H
#define FIGHT_H

#include "heresyrl_def.h"
#include "monster/monster.h"
#include "coord.h"

/* ranged modifiers */
#define FGHT_RANGE_MULTIPLIER RANGE_MULTIPLIER
#define FGHT_MODIFIER_MAX (80)
#define FGHT_MELEE_RANGE (1)
#define FGHT_POINT_BLANK_RANGE (3)

/* melee specific */
#define FGHT_MODIFIER_MELEE_UNARMED  (-20)

/* ranged specific */
#define FGHT_RANGED_JAM (95)
#define FGHT_RANGED_JAM_SEMI (93)
#define FGHT_RANGED_JAM_UNRELIABLE (90)
#define FGHT_RANGED_MODIFIER_ROF_SINGLE (0)
#define FGHT_RANGED_MODIFIER_ROF_SEMI (20)
#define FGHT_RANGED_MODIFIER_ROF_AUTO (10)
#define FGHT_RANGED_MODIFIER_MELEE    (-20)
#define FGHT_RANGED_MODIFIER_EXTREME_RANGE (-30)
#define FGHT_RANGED_MODIFIER_LONG_RANGE    (-10)
#define FGHT_RANGED_MODIFIER_SHORT_RANGE   (10)
#define FGHT_RANGED_MODIFIER_POINT_BLACK   (30)
#define FGHT_RANGED_MODIFIER_SHIELD       (-10)

#define FGHT_MODIFIER_SURPRISED (30)
#define FGHT_MODIFIER_OFF_HAND (-20)
#define FGHT_MODIFIER_MAIN_HAND_DUAL_WIELD (-20)
#define FGHT_MODIFIER_OFF_HAND_DUAL_WIELD  (-20)
#define FGHT_MODIFIER_OFF_HAND_DUAL_WIELD_AMBIDEXTROUS (10)
#define FGHT_MODIFIER_DUAL_WIELD_TWO_WEAPON_FIGHTING   (10)
#define FGHT_MODIFIER_DUAL_WIELD_WEAPON_LIGHT          (10)
#define FGHT_MODIFIER_IMPROVISED_WEAPON (-20)
#define FGHT_MODIFIER_UNTRAINED_WEAPON (-20)

/* quality */
#define FGHT_MODIFIER_QLTY_TO_HIT_POOR (-10)
#define FGHT_MODIFIER_QLTY_TO_HIT_GOOD (5)
#define FGHT_MODIFIER_QLTY_TO_HIT_BEST (10)
#define FGHT_MODIFIER_QLTY_TO_DMG_BEST (1)

/* lighting */
#define FGHT_MODIFIER_VISION_SHADOWS (-10)
#define FGHT_MODIFIER_VISION_DARKNESS (-20)
#define FGHT_MODIFIER_VISION_COMPLETE_DARKNESS (-30)

/* size */
#define FGHT_MODIFIER_SIZE_MASSIVE   (30)
#define FGHT_MODIFIER_SIZE_ENORMOUS  (20)
#define FGHT_MODIFIER_SIZE_HULKING   (10)
#define FGHT_MODIFIER_SIZE_AVERAGE   (0)
#define FGHT_MODIFIER_SIZE_SCRAWNY   (-10)
#define FGHT_MODIFIER_SIZE_PUNY      (-20)
#define FGHT_MODIFIER_SIZE_MINISCULE (-30)

/* conditions */
#define FGHT_MODIFIER_STATUS_EFFECT_STUNNED (20)

/*
    +30
        Attacking Surprised
        Shooting Massive Target
        Shooting Point Blank Range (3 meters (2 spaces))
    +20
        Close combat with foe who is outnumbered 3 to one or more
        Attack Stunned opponent
        Shooting with Semi Auto fire
        Shooting an enourmous target
    +10
        Close combat with foe who is outnumbered 2 to one
        Attacking a prone opponent
        Shooting with Full Auto fire
        Shooting an Hulking target
        Shooting Short Range (half wpn range)
    +0
        Standard attack
    -10
        Attacking while knocked down
        Fatigued
        Attacking or dodging while in the mud or heavy rain
        Shooting at a target at Long Range (two times wpn range)
        Shooting a scrawny target
        Shooting at a target with a shield
    -20
        A called shot at a specific location
        Shooting in melee combat
        dodging while prone
        makeing an unarmed attack against an armed opponent
        secondary hand attack
        attacking with a weapon in either hand
        close combat attack in darkness
        shooting at a target in mist, fog or shadow
        shooting at a puny target
        using a weapon without the correct talent
    -30
        Attacking or dodging in deep snow
        Shooting a miniscule target
        Shooting at Extreme Range (three times wpn range)
        Shooting at a completely concealed target
        Shooting at a target in darkness
 */

enum fght_hand {
    FGHT_MAIN_HAND = MSR_WEAPON_SELECT_MAIN_HAND,
    FGHT_OFF_HAND = MSR_WEAPON_SELECT_OFF_HAND,
    FGHT_CREATURE_HAND = MSR_WEAPON_SELECT_CREATURE1,
};

struct tohit_desc {
    const char *description;
    int modifier;
};

struct tohit_desc *fght_get_tohit_mod_description(int idx);
int fght_ranged_calc_tohit(struct msr_monster *monster, coord_t *tpos, struct itm_item *witem, enum fght_hand hand, bool throwing);
int fght_melee_calc_tohit(struct msr_monster *monster, coord_t *tpos, struct itm_item *item, enum fght_hand hand);

bool fght_shoot(struct random *r, struct msr_monster *monster, struct dm_map *map, coord_t *e, enum fght_hand hand);
bool fght_melee(struct random *r, struct msr_monster *monster, struct msr_monster *target);
bool fght_explosion(struct random *r, struct itm_item *bomb, struct dm_map *map);
bool fght_throw_item(struct random *r, struct msr_monster *monster, struct dm_map *map, coord_t *e, struct itm_item *witem, enum fght_hand hand);

struct itm_item *fght_get_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand);
struct itm_item *fght_get_working_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand); /* checks on emptiness and jammedness */
const char *fght_weapon_hand_name(enum fght_hand hand);

#endif /* FIGHT_H */
