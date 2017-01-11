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

#ifndef SPECIALQUALITIES_H
#define SPECIALQUALITIES_H

enum dmg_type {
    DMG_TYPE_ENERGY,
    DMG_TYPE_EXPLOSIVE,
    DMG_TYPE_IMPACT,
    DMG_TYPE_RENDING,
    DMG_TYPE_MAX,
};

/* Check Only war for more ideas about weapon spc qualities and upgrades*/

enum weapon_special_quality {
    WPN_SPCQLTY_BLAST_1,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_2,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_3,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_4,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_CREATURE,    /* Non dropable creature weapon. */
    WPN_SPCQLTY_DEFENSIVE,   /* +5 Parry */
    WPN_SPCQLTY_FAST,        /* Easier to circumvent parry*/
    WPN_SPCQLTY_FLEXIBLE,    /* Cannot be Parried. */
    WPN_SPCQLTY_JAMS,        /* Can Jam on 95% */
    WPN_SPCQLTY_LIGHT,       /* This light weapon is easier to handle in your off-hand. */
    WPN_SPCQLTY_PRIMITIVE,   /* AP doubled, unless armour is also primitive */
    WPN_SPCQLTY_RELIABLE,    /* If jam, 10 on 1d10 to actually jam. <I> */
    WPN_SPCQLTY_SHIELD,      /* harder to hit, attacker get an -10 penalty */
    WPN_SPCQLTY_SLOW,        /* harder to crcumvent parry */
    WPN_SPCQLTY_TEARING,     /* Roll twice, take highest damage */
    WPN_SPCQLTY_UNARMED,     /* 1d5-3 +Str, Armour doubles */
    WPN_SPCQLTY_UNBALANCED,  /* Cannot be parried with */
    WPN_SPCQLTY_UNRELIABLE,  /* jams on 91+ */
    WPN_SPCQLTY_UNWIELDY,    /* -5 parry */
#if 0
    WPN_SPCQLTY_ACCURATE,    /* Additional +10 to hit when used with the aim action */
    WPN_SPCQLTY_FLAME,       /* No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. */
    WPN_SPCQLTY_INACCURATE,  /* No bonus with aim action */
    WPN_SPCQLTY_OVERHEATS,   /* 90+ -> Overheat */
    WPN_SPCQLTY_SCATTER,     /* Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. */
    WPN_SPCQLTY_SHOCKING,    /* If damage, test Toughness or be stunned. */
    WPN_SPCQLTY_SMOKE,       /* Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds. */
    WPN_SPCQLTY_TOXIC,       /* if damage, test toughness -5 for every damage taken. if failed, take 1d10 extra impact damage. */
    WPN_SPCQLTY_UNSTABLE,
#endif
    WPN_SPCQLTY_MAX,
};

enum wearable_special_quality {
    WBL_SPCQLTY_PRIMITIVE, /* Half armour bonus. */
    WBL_SPCQLTY_FLAK,      /* Half Explosive Damage */
    WBL_SPCQLTY_MAX,
};

enum weapon_upgrades {
    /*WPN_UPG_COMPACT,
    WPN_UPG_EXTERMINATOR,
    WPN_UPG_EXTRA_GRIP,
    WPN_UPG_FIRE_SELECTOR,
    WPN_UPG_MELEE_ATTACHMENT,
    WPN_UPG_MONO,
    WPN_UPG_OVERCHARGE_PACK,
    WPN_UPG_RED_DOT,
    WPN_UPG_SILENCER,
    WPN_UPG_TELESCOPE_SIGHT,*/
    WPN_UPG_MAX,
};

enum ammo_upgrades {
#if 0
    AMMO_UPG_DUMDUM,     /* +2 dmg, armour doubles */
    AMMO_UPG_HOTSHOT,    /* +1 dmg, tearing, +pen 4, clip_sz == 1, -reliable */
    AMMO_UPG_INFERNO,    /* Ag test or catch fire */
    AMMO_UPG_MANSTOPPER, /* +3 Penetration */
#endif
    AMMO_UPG_MAX,
};

enum msr_skill_rate {
    MSR_SKILL_RATE_UNKNOWN,
    MSR_SKILL_RATE_KNOWN,
    MSR_SKILL_RATE_TRAINED,
    MSR_SKILL_RATE_EXPERIENCED,
    MSR_SKILL_RATE_VETERAN,
    MSR_SKILL_RATE_MAX,
};

enum msr_skills {
    MSR_SKILLS_NONE,
    MSR_SKILLS_AWARENESS,
    MSR_SKILLS_DODGE,
    MSR_SKILLS_PARRY,
    MSR_SKILLS_MEDICAE,
    MSR_SKILLS_MELEE,
    MSR_SKILLS_RANGED,
    MSR_SKILLS_STEALTH,
    MSR_SKILLS_MAX,
};

enum msr_creature_traits {
    CTRTRT_DARK_SIGHT,
#if 0
    CTRTRT_BESTIAL,
    CTRTRT_BLIND,
    CTRTRT_FEAR,
#endif
    CTRTRT_MAX,
};

enum msr_talent_tiers {
    MSR_TALENT_TIER_T1,
    MSR_TALENT_TIER_T2,
    MSR_TALENT_TIER_T3,
    MSR_TALENT_TIER_MISC,
    MSR_TALENT_TIER_MAX,
};


#define MSR_TALENT_HEADER_SHIFT 60
#define MSR_TALENT_HEADER_MASK  (0x0FUL<<MSR_TALENT_HEADER_SHIFT)
#define MSR_TALENT_ID_MASK      (~MSR_TALENT_HEADER_MASK)
#define MSR_TALENT_HEADER(a)    ( (1UL<<a) <<MSR_TALENT_HEADER_SHIFT)
#define MSR_TALENT(h,id)        ( MSR_TALENT_HEADER(h) | (1UL<<id) )
#define MSR_TALENTS_PER_TIER    MSR_TALENT_HEADER_SHIFT
enum msr_talents {
    TLT_NONE                       = MSR_TALENT(0, 0),   /* TLT_NONE is a talent every one has, used to denote talentless weapons */

    TLT_1_AMBIDEXTRIOUS            = MSR_TALENT(0, 1),
    TLT_1_BLIND_FIGHTING           = MSR_TALENT(0, 2),
    TLT_1_DIE_HARD                 = MSR_TALENT(0, 3),
    TLT_1_DISARM                   = MSR_TALENT(0, 4),
    TLT_1_FRENZY                   = MSR_TALENT(0, 5),
    TLT_1_IRON_JAW                 = MSR_TALENT(0, 6),
    TLT_1_JADED                    = MSR_TALENT(0, 7),
    TLT_1_LEAP_UP                  = MSR_TALENT(0, 8),
    TLT_1_QUICK_DRAW               = MSR_TALENT(0, 9),
    TLT_1_RAPID_RELOAD             = MSR_TALENT(0, 10),
    TLT_1_RESISTANCE_COLD          = MSR_TALENT(0, 11),
    TLT_1_RESISTANCE_HEAT          = MSR_TALENT(0, 12),
    TLT_1_RESISTANCE_FEAR          = MSR_TALENT(0, 13),
    TLT_1_RESISTANCE_POISON        = MSR_TALENT(0, 14),
    TLT_1_RESISTANCE_PSYCHIC       = MSR_TALENT(0, 15),
    TLT_1_RESISTANCE_RADIATION     = MSR_TALENT(0, 16),
    TLT_1_RESISTANCE_VACUUM        = MSR_TALENT(0, 17),
    TLT_1_TAKEDOWN                 = MSR_TALENT(0, 18),
    TLT_1_WEAPON_TRAINING_BOLT     = MSR_TALENT(0, 19),
    TLT_1_WEAPON_TRAINING_CHAIN    = MSR_TALENT(0, 20),
    TLT_1_WEAPON_TRAINING_FLAME    = MSR_TALENT(0, 21),
    TLT_1_WEAPON_TRAINING_HEAVY    = MSR_TALENT(0, 22),
    TLT_1_WEAPON_TRAINING_LOW_TECH = MSR_TALENT(0, 23),
    TLT_1_WEAPON_TRAINING_LAUNCHER = MSR_TALENT(0, 24),
    TLT_1_WEAPON_TRAINING_LAS      = MSR_TALENT(0, 25),
    TLT_1_WEAPON_TRAINING_MELTA    = MSR_TALENT(0, 26),
    TLT_1_WEAPON_TRAINING_PLASMA   = MSR_TALENT(0, 27),
    TLT_1_WEAPON_TRAINING_POWER    = MSR_TALENT(0, 28),
    TLT_1_WEAPON_TRAINING_SHOCK    = MSR_TALENT(0, 29),
    TLT_1_WEAPON_TRAINING_SP       = MSR_TALENT(0, 30),

    /*TLT_2_ARMOUR_MONGER            = MSR_TALENT(1, 0),*/
    TLT_2_TWO_WEAPON_FIGHTING      = MSR_TALENT(1, 1),

    /*TLT_3_ASSASSINS_STRIKE         = MSR_TALENT(2, 0),*/

    TLT_MAX,        /*TLT_MAX is also used as an eof marker*/
};

enum aptitude_enum {
    APTITUDE_GENERAL,
    APTITUDE_MELEE,
    APTITUDE_RANGED,
    APTITUDE_STRENGTH,
    APTITUDE_TOUGHNESS,
    APTITUDE_AGILITY,
    APTITUDE_WILLPOWER,
    APTITUDE_INTELLIGENCE,
    APTITUDE_PERCEPTION,
    APTITUDE_OFFENCE,
    APTITUDE_FINESS,
    APTITUDE_DEFENCE,
    APTITUDE_FIELDCRAFT,

    APTITUDE_MAX,
    APTITUDE_NONE,
};

#endif /* SPECIALQUALITIES_H */
