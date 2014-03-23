#ifndef FIGHT_H
#define FIGHT_H

#include "heresyrl_def.h"
#include "monster.h"
#include "coord.h"

#define FGHT_MODIFIER_MAX (80)
#define FGHT_MELEE_RANGE (1)
#define FGHT_POINT_BLANK_RANGE (3)

#define FGHT_MELEE_UNARMED  (-20)

#define FGHT_RANGED_JAM (95)
#define FGHT_RANGED_JAM_SEMI (93)
#define FGHT_RANGED_JAM_UNRELIABLE (90)
#define FGHT_RANGED_MODIFIER_ROF_SEMI (20)
#define FGHT_RANGED_MODIFIER_ROF_AUTO (10)
#define FGHT_RANGED_MODIFIER_MELEE    (-20)
#define FGHT_RANGED_MODIFIER_EXTREME_RANGE (-30)
#define FGHT_RANGED_MODIFIER_LONG_RANGE    (-10)
#define FGHT_RANGED_MODIFIER_SHORT_RANGE   (10)
#define FGHT_RANGED_MODIFIER_POINT_BLACK   (30)

#define FGHT_MODIFIER_OFF_HAND (-20)
#define FGHT_MODIFIER_UNTRAINED_WEAPON (-20)

#define FGHT_MODIFIER_QUALITY_TO_HIT_POOR (-10)
#define FGHT_MODIFIER_QUALITY_TO_HIT_GOOD (5)
#define FGHT_MODIFIER_QUALITY_TO_HIT_BEST (10)
#define FGHT_MODIFIER_QUALITY_TO_DMG_BEST (1)

#define FGHT_MODIFIER_VISION_SHADOWS (-10)
#define FGHT_MODIFIER_VISION_DARKNESS (-20)
#define FGHT_MODIFIER_VISION_COMPLETE_DARKNESS (-30)

#define FGHT_MODIFIER_SIZE_MASSIVE   (30)
#define FGHT_MODIFIER_SIZE_ENORMOUS  (20)
#define FGHT_MODIFIER_SIZE_HULKING   (10)
#define FGHT_MODIFIER_SIZE_AVERAGE   (0)
#define FGHT_MODIFIER_SIZE_SCRAWNY   (-10)
#define FGHT_MODIFIER_SIZE_PUNY      (-20)
#define FGHT_MODIFIER_SIZE_MINISCULE (-30)

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
    FGHT_MAX_HAND,
};

int fght_ranged_calc_tohit(struct msr_monster *monster, coord_t *tpos, enum fght_hand hand);

bool fght_do_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand);
int fght_shoot(struct random *r, struct msr_monster *monster, struct dc_map *map, coord_t *e);
bool fght_melee(struct random *r, struct msr_monster *monster, struct msr_monster *target);
struct itm_item *fght_get_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand);
struct itm_item *fght_get_working_weapon(struct msr_monster *monster, enum item_weapon_type type, enum fght_hand hand); /* checks on emptiness and jammedness */
const char *fght_weapon_hand_name(enum fght_hand hand);

#endif /* FIGHT_H */
