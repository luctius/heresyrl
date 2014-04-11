#ifndef SPECIALQUALITIES_H
#define SPECIALQUALITIES_H

enum weapon_special_quality {
    WPN_SPCQLTY_ACCURATE,    /* Additional +10 to hit when used with the aim action */
    WPN_SPCQLTY_BALANCED,    /* +10 Parry */
    WPN_SPCQLTY_BLAST_1,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_2,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_3,
    WPN_SPCQLTY_BLAST_4,
    WPN_SPCQLTY_DEFENSIVE,   /* +15 Parry, -10 to hit  */
    WPN_SPCQLTY_FLAME,       /* No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. */
    WPN_SPCQLTY_FLEXIBLE,    /* Cannot be Parried. */
    WPN_SPCQLTY_INACCURATE,  /* No bonus with aim action */
    WPN_SPCQLTY_OVERHEATS,   /* 90+ -> Overheat */
    WPN_SPCQLTY_POWER_FIELD, /* When this is parried, 75% chance to destroy parrying weapon */
    WPN_SPCQLTY_PRIMITIVE,   /* AP doubled, unless armour is also primitive */
    WPN_SPCQLTY_RECHARGE,    /* Fire only every other round */
    WPN_SPCQLTY_RELIABLE,    /* If jam, 10 on 1d10 to actually jam. <I> */
    WPN_SPCQLTY_SCATTER,     /* Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. */
    WPN_SPCQLTY_SHOCKING,    /* If damage, test Toughness or be stunned. */
    WPN_SPCQLTY_SMOKE,       /* Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds. */
    WPN_SPCQLTY_SNARE,       /* Ag. test or be immobilised */
    WPN_SPCQLTY_TEARING,     /* Roll twice, take highest damage */
    WPN_SPCQLTY_TOXIC,       /* if damage, teest toughness -5 for every damage taken. if failed, take 1d10 extra impact damage. */
    WPN_SPCQLTY_UNARMED,     /* 1d5-3 +Str, Armour doubles */
    WPN_SPCQLTY_UNBALANCED,  /* -10 when parry */
    WPN_SPCQLTY_UNRELIABLE,  /* jams on 90+ */
    WPN_SPCQLTY_UNSTABLE,    /* on hit, roll 1d10: 1: half dmg, 2-9: normal, 10: double damage */
    WPN_SPCQLTY_UNWIELDY,    /* Cannot be used to parry. */
    WPN_SPCQLTY_MAX,
};

enum wearable_special_quality {
    WBL_SPCQLTY_PRIMITIVE, /* AP halved unless weapon is also primitive. */
    WBL_SPCQLTY_MAX,
};

enum weapon_upgrades {
    WPN_UPG_COMPACT,
    WPN_UPG_EXTERMINATOR,
    WPN_UPG_EXTRA_GRIP,
    WPN_UPG_FIRE_SELECTOR,
    WPN_UPG_MELEE_ATTACHMENT,
    WPN_UPG_MONO,
    WPN_UPG_OVERCHARGE_PACK,
    WPN_UPG_RED_DOT,
    WPN_UPG_SILENCER,
    WPN_UPG_TELESCOPE_SIGHT,
    WPN_UPG_MAX,
};

enum ammo_upgrades {
    AMMO_UPG_DUMDUM,     /* +2 dmg, armour doubles */
    AMMO_UPG_HOTSHOT,    /* +1 dmg, tearing, +pen 4, clip_sz == 1, -reliable */
    AMMO_UPG_INFERNO,    /* Ag test or catch fire */
    AMMO_UPG_MANSTOPPER, /* +3 Penetration */
    AMMO_UPG_MAX,
};

enum msr_skill_rate {
    MSR_SKILL_RATE_NONE,
    MSR_SKILL_RATE_BASIC,
    MSR_SKILL_RATE_ADVANCED,
    MSR_SKILL_RATE_EXPERT,
    MSR_SKILL_RATE_MAX,
};

enum msr_skills {
    MSR_SKILLS_AWARENESS,
    MSR_SKILLS_BARTER,
    MSR_SKILLS_CHEM_USE,
    MSR_SKILLS_COMMON_LORE,
    MSR_SKILLS_CONCEALMENT,
    MSR_SKILLS_DEMOLITION,
    MSR_SKILLS_DISGUISE	,
    MSR_SKILLS_DODGE,
    MSR_SKILLS_EVALUATE,
    MSR_SKILLS_FORBIDDEN_LORE,
    MSR_SKILLS_INVOCATION,
    MSR_SKILLS_LOGIC,
    MSR_SKILLS_MEDICAE,
    MSR_SKILLS_PSYSCIENCE,
    MSR_SKILLS_SCHOLASTIC_LORE,
    MSR_SKILLS_SEARCH,
    MSR_SKILLS_SECURITY,
    MSR_SKILLS_SILENT_MOVE,
    MSR_SKILLS_SURVIVAL,
    MSR_SKILLS_TECH_USE,
    MSR_SKILLS_TRACKING,
    MSR_SKILLS_MAX,
};

enum msr_creature_traits {
    CTRTRTS_ARMORPHOUS,
    CTRTRTS_ARMOUR_PLATING,
    CTRTRTS_AUTO_STABILISED,
    CTRTRTS_BESTIAL,
    CTRTRTS_BLIND,
    CTRTRTS_BRUTAL_CHARGE,
    CTRTRTS_CRAWLER,
    CTRTRTS_DAEMONIC,
    CTRTRTS_DARK_SIGHT,
    CTRTRTS_INCOROREAL,
    CTRTRTS_FEAR,
    CTRTRTS_FLYING,
    CTRTRTS_FROM_BEYOND,
    CTRTRTS_HOVERER,
    CTRTRTS_MACHINE,
    CTRTRTS_MULTIPLE_ARMS,
    CTRTRTS_NATURAL_WEAPONS,
    CTRTRTS_PHASE,
    CTRTRTS_POSESSION,
    CTRTRTS_QUADRUPED,
    CTRTRTS_REGENERATION,
    CTRTRTS_SONAR_SENSE,
    CTRTRTS_SOUL_BOUND,
    CTRTRTS_STAMPEDE,
    CTRTRTS_STRANGE_PHYSIOLOGY,
    CTRTRTS_STUFF_OF_NIGHTMARES,
    CTRTRTS_STURDY,
    CTRTRTS_TOXIC,
    CTRTRTS_NATURAL_ARMOUR_1,
    CTRTRTS_NATURAL_ARMOUR_2,
    CTRTRTS_NATURAL_ARMOUR_3,
    CTRTRTS_NATURAL_ARMOUR_4,
    CTRTRTS_UC_STRENGTH_2,
    CTRTRTS_UC_STRENGTH_3,
    CTRTRTS_UC_STRENGTH_4,
    CTRTRTS_UC_AGILITY_2,
    CTRTRTS_UC_AGILITY_3,
    CTRTRTS_UC_AGILITY_4,
    CTRTRTS_UC_TOUGHNESS_2,
    CTRTRTS_UC_TOUGHNESS_3,
    CTRTRTS_UC_TOUGHNESS_4,
    CTRTRTS_UC_INTELLIGIENCE_2,
    CTRTRTS_UC_INTELLIGIENCE_3,
    CTRTRTS_UC_INTELLIGIENCE_4,
    CTRTRTS_UC_WILLPOWER_2,
    CTRTRTS_UC_WILLPOWER_3,
    CTRTRTS_UC_WILLPOWER_4,
    CTRTRTS_UC_FELLOWSHIP_2,
    CTRTRTS_UC_FELLOWSHIP_3,
    CTRTRTS_UC_FELLOWSHIP_4,
    CTRTRTS_UNNATURAL_SPEED,
    CTRTRTS_WARP_INSTABILITY,
    CTRTRTS_WARP_WEAPONS,
    CTRTRTS_MAX,
};

enum msr_talents {
    WPNTLT_BASIC_WPN_TRNG_BOLT,
    WPNTLT_BASIC_WPN_TRNG_FLAME,
    WPNTLT_BASIC_WPN_TRNG_LAS,
    WPNTLT_BASIC_WPN_TRNG_LAUNCHER,
    WPNTLT_BASIC_WPN_TRNG_MELTA,
    WPNTLT_BASIC_WPN_TRNG_PLASMA,
    WPNTLT_BASIC_WPN_TRNG_PRIMITIVE,
    WPNTLT_BASIC_WPN_TRNG_SP,
    WPNTLT_HEAVY_WPN_TRNG_BOLT,
    WPNTLT_HEAVY_WPN_TRNG_FLAME,
    WPNTLT_HEAVY_WPN_TRNG_LAS,
    WPNTLT_HEAVY_WPN_TRNG_LAUNCHER,
    WPNTLT_HEAVY_WPN_TRNG_MELTA,
    WPNTLT_HEAVY_WPN_TRNG_PLASMA,
    WPNTLT_HEAVY_WPN_TRNG_PRIMITIVE,
    WPNTLT_HEAVY_WPN_TRNG_SP,
    WPNTLT_PISTOL_WPN_TRNG_BOLT,
    WPNTLT_PISTOL_WPN_TRNG_FLAME,
    WPNTLT_PISTOL_WPN_TRNG_LAS,
    WPNTLT_PISTOL_WPN_TRNG_LAUNCHER,
    WPNTLT_PISTOL_WPN_TRNG_MELTA,
    WPNTLT_PISTOL_WPN_TRNG_PLASMA,
    WPNTLT_PISTOL_WPN_TRNG_PRIMITIVE,
    WPNTLT_PISTOL_WPN_TRNG_SP,
    WPNTLT_THROWN_WPN_TRNG_PRIMITIVE,
    WPNTLT_THROWN_WPN_TRNG_CHAIN,
    WPNTLT_THROWN_WPN_TRNG_SHOCK,
    WPNTLT_THROWN_WPN_TRNG_POWER,
    WPNTLT_CREATURE_WPN_TALENT,
    WPNTLT_MELEE_WPN_TRNG_PRIMITIVE,
    WPNTLT_MELEE_WPN_TRNG_CHAIN,
    WPNTLT_MELEE_WPN_TRNG_SHOCK,
    WPNTLT_MELEE_WPN_TRNG_POWER,
    WPNTLT_RESISTANCE_COLD,
    WPNTLT_RESISTANCE_HEAT,
    WPNTLT_RESISTANCE_FEAR,
    WPNTLT_RESISTANCE_POISON,
    WPNTLT_RESISTANCE_WARP,
    WPNTLT_HATRED_FACTION_CRIMINALS,
    WPNTLT_HATRED_FACTION_DAEMONS,
    WPNTLT_HATRED_FACTION_ORKS,
    WPNTLT_HATRED_FACTION_TYRANIDS,
    WPNTLT_HATRED_FACTION_PSYKERS,
    WPNTLT_HATRED_FACTION_HERETICS,
    WPNTLT_HATRED_FACTION_MUTANTS,
    WPNTLT_EXOTIC_WPN_TRNG_NEEDLE_PISTOL,
    WPNTLT_EXOTIC_WPN_TRNG_WEB_PISTOL,
    WPNTLT_EXOTIC_WPN_TRNG_NEEDLE_RIFLE,
    WPNTLT_EXOTIC_WPN_TRNG_WEBBER,
    WPNTLT_PSY_RATING_1,
    WPNTLT_PSY_RATING_2,
    WPNTLT_PSY_RATING_3,
    WPNTLT_PSY_RATING_4,
    WPNTLT_PSY_RATING_5,
    WPNTLT_PSY_RATING_6,
    TALENTS_MAX,
    TALENTS_NONE,
};

/*
#define TALENTS_IDX_OFFSET                          (60UL)
#define TALENTS_NONE                                (0)
#define T0_BASIC_WEAPON_TRAINING_BOLT               ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<0))
#define T0_BASIC_WEAPON_TRAINING_FLAME              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<1))
#define T0_BASIC_WEAPON_TRAINING_LAS                ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<2))
#define T0_BASIC_WEAPON_TRAINING_LAUNCHER           ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<3))
#define T0_BASIC_WEAPON_TRAINING_MELTA              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<4))
#define T0_BASIC_WEAPON_TRAINING_PLASMA             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<5))
#define T0_BASIC_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<6))
#define T0_BASIC_WEAPON_TRAINING_SP                 ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<7))
#define T0_HEAVY_WEAPON_TRAINING_BOLT               ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<8))
#define T0_HEAVY_WEAPON_TRAINING_FLAME              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<9))
#define T0_HEAVY_WEAPON_TRAINING_LAS                ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<10))
#define T0_HEAVY_WEAPON_TRAINING_LAUNCHER           ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<11))
#define T0_HEAVY_WEAPON_TRAINING_MELTA              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<12))
#define T0_HEAVY_WEAPON_TRAINING_PLASMA             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<13))
#define T0_HEAVY_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<14))
#define T0_HEAVY_WEAPON_TRAINING_SP                 ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<15))
#define T0_PISTOL_WEAPON_TRAINING_BOLT              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<16))
#define T0_PISTOL_WEAPON_TRAINING_FLAME             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<17))
#define T0_PISTOL_WEAPON_TRAINING_LAS               ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<18))
#define T0_PISTOL_WEAPON_TRAINING_LAUNCHER          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<19))
#define T0_PISTOL_WEAPON_TRAINING_MELTA             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<20))
#define T0_PISTOL_WEAPON_TRAINING_PLASMA            ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<21))
#define T0_PISTOL_WEAPON_TRAINING_PRIMITIVE         ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<22))
#define T0_PISTOL_WEAPON_TRAINING_SP                ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<23))
#define T0_THROWN_WEAPON_TRAINING_PRIMITIVE         ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<24))
#define T0_THROWN_WEAPON_TRAINING_CHAIN             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<25))
#define T0_THROWN_WEAPON_TRAINING_SHOCK             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<26))
#define T0_THROWN_WEAPON_TRAINING_POWER             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<27))
#define T0_CREATURE_WEAPON_TALENT                   ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<28))
#define T0_MELEE_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<29))
#define T0_MELEE_WEAPON_TRAINING_CHAIN              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<30))
#define T0_MELEE_WEAPON_TRAINING_SHOCK              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<31))
#define T0_MELEE_WEAPON_TRAINING_POWER              ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<32))
#define T0_RESISTANCE_COLD                          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<33))
#define T0_RESISTANCE_HEAT                          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<34))
#define T0_RESISTANCE_FEAR                          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<35))
#define T0_RESISTANCE_POISON                        ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<36))
#define T0_RESISTANCE_WARP                          ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<37))
#define T0_HATRED_FACTION_CRIMINALS                 ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<38))
#define T0_HATRED_FACTION_DAEMONS                   ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<39))
#define T0_HATRED_FACTION_ORKS                      ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<40))
#define T0_HATRED_FACTION_TYRANIDS                  ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<41))
#define T0_HATRED_FACTION_PSYKERS                   ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<42))
#define T0_HATRED_FACTION_HERETICS                  ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<43))
#define T0_HATRED_FACTION_MUTANTS                   ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<44))
#define T0_EXOTIC_WEAPON_TRAINING_NEEDLE_PISTOL     ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<45))
#define T0_EXOTIC_WEAPON_TRAINING_WEB_PISTOL        ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<46))
#define T0_EXOTIC_WEAPON_TRAINING_NEEDLE_RIFLE      ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<47))
#define T0_EXOTIC_WEAPON_TRAINING_WEBBER            ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<48))
#define T0_PSY_RATING_1                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<49))
#define T0_PSY_RATING_2                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<50))
#define T0_PSY_RATING_3                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<51))
#define T0_PSY_RATING_4                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<52))
#define T0_PSY_RATING_5                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<53))
#define T0_PSY_RATING_6                             ((0x0UL<<TALENTS_IDX_OFFSET) | (0x1UL<<54))

#define T1_TALENT_AWARENESS                         ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<1))
#define T1_TALENT_BARTER                            ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<2))
#define T1_TALENT_CHEM_USE                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<3))
#define T1_TALENT_COMMON_LORE                       ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<4))
#define T1_TALENT_CONCEALMENT                       ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<5))
#define T1_TALENT_DEMOLITION                        ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<6))
#define T1_TALENT_DISGUISE                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<7))
#define T1_TALENT_DODGE                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<8))
#define T1_TALENT_EVALUATE                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<9))
#define T1_TALENT_FORBIDDEN_LORE                    ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<10))
#define T1_TALENT_INVOCATION                        ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<11))
#define T1_TALENT_LOGIC                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<12))
#define T1_TALENT_MEDICAE                           ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<13))
#define T1_TALENT_PSYSCIENCE                        ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<14))
#define T1_TALENT_SCHOLASTIC_LORE                   ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<15))
#define T1_TALENT_SEARCH                            ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<16))
#define T1_TALENT_SECURITY                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<17))
#define T1_TALENT_SILENT_MOVE                       ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<18))
#define T1_TALENT_SURVIVAL                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<19))
#define T1_TALENT_TECH_USE                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<20))
#define T1_AMBIDEXTROUS                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<21))
#define T1_ARMOUR_OF_CONTEMPT                       ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<22))
#define T1_ARMS_MASTER                              ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<23))
#define T1_ASSASSIN_STRIKE                          ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<24))
#define T1_AUTOSANGUINE                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<25))
#define T1_BATTLE_RAGE                              ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<26))
#define T1_BLADE_MASTER                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<27))
#define T1_BLIND_FIGHTING                           ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<28))
#define T1_BULGING_BICEPS                           ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<29))
#define T1_CLEANSE_AND_PURIFY                       ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<30))
#define T1_COMBAT_MASTER                            ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<31))
#define T1_CORPUS_CONVERSION                        ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<32))
#define T1_COUNTER_ATTACK                           ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<33))
#define T1_CRACK_SHOT                               ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<34))

#define T1_DARK_SOUL                                ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<37))
#define T1_DEADEYE_SHOT                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<38))
#define T1_DEFLECT_SHOT                             ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<39))
#define T1_DIE_HARD                                 ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<40))
#define T1_DISARM                                   ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<41))
#define T1_DISCIPLINE_FOCUS                         ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<42))
#define T1_DOUBLE_TEAM                              ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<43))
#define T1_DUAL_SHOT                                ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<44))
#define T1_DUAL_STRIKE                              ((0x1UL<<TALENTS_IDX_OFFSET) | (0x1UL<<45))

#define T2_ELECTRICAL_SOCCOUR                       ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<1))
#define T2_ELECTRO_GRAFT_USE                        ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<2))
#define T2_FAVOURED_BY_THE_WARP                     ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<3))
#define T2_FEARLESS                                 ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<4))
#define T2_FEEDBACK_SCREECH                         ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<5))
#define T2_FLAGELLANT                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<6))
#define T2_FORESIGHT                                ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<7))
#define T2_FRENZY                                   ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<8))
#define T2_FURIOUS_ASSAULT                          ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<9))
#define T2_GUN_BLESSING                             ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<10))
#define T2_GUNSLINGER                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<11))
#define T2_HARDY                                    ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<12))
#define T2_HEIGHTENED_SENSES                        ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<13))
#define T2_INSANELY_FAITHFULL                       ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<14))
#define T2_IRON_JAW                                 ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<15))
#define T2_JADED                                    ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<16))
#define T2_LIGHTNING_ATTACK                         ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<17))
#define T2_LOGIS_IMPLANT                            ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<18))
#define T2_LUMINEN_CHARGE                           ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<19))
#define T2_MAGLEV_CHARGE                            ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<20))
#define T2_MARKSMEN                                 ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<21))
#define T2_MASTER_CHIRUGEON                         ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<22))
#define T2_MECHANDENDRITE_USE                       ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<23))
#define T2_MEDITATION                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<24))
#define T2_MENTAL_FORTRESS                          ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<25))
#define T2_MENTAL_RAGE                              ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<26))
#define T2_MIGHTY_SHOT                              ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<27))
#define T2_MINOR_PSYCHIC_POWER                      ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<28))
#define T2_NERVES_OF_STEEL                          ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<29))
#define T2_POWER_WELL                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<30))
#define T2_PROSANGUINE                              ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<31))
#define T2_QUICK_DRAW                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<32))
#define T2_RAPID_RELOAD                             ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<33))
#define T2_SOUND_CONSTITUTIOIN                      ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<34))
#define T2_SPRINT                                   ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<35))
#define T2_STEP_ASIDE                               ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<36))
#define T2_STREET_FIGHTING                          ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<37))
#define T2_STRONG_MINDED                            ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<38))
#define T2_SWIFT_ATTACK                             ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<39))
#define T2_TAKE_DOWN                                ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<40))
#define T2_TECHNICAL_KNOCK                          ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<41))
#define T2_TRUE_GRIT                                ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<42))
#define T2_TWO_WEAPON_WIELDER_MELEE                 ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<43))
#define T2_TWO_WEAPON_WIELDER_BALLISTIC             ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<44))
#define T2_UNSHAKEBLE_FAITH                         ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<45))
#define T2_WALL_OF_STEEL                            ((0x2UL<<TALENTS_IDX_OFFSET) | (0x1UL<<46))
*/

#endif /* SPECIALQUALITIES_H */
