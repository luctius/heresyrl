#ifndef SPECIALQUALITIES_H
#define SPECIALQUALITIES_H

enum weapon_special_quality {
    WEAPON_SPEC_QUALITY_NONE,
    //WEAPON_SPEC_QUALITY_ACCURATE,   /* Additional +10 to hit when used with the aim action */
    WEAPON_SPEC_QUALITY_BALANCED,   /* +10 Parry */
    WEAPON_SPEC_QUALITY_BLAST_2,    /* All within the weapon's blast radius in squares is hit */
    WEAPON_SPEC_QUALITY_BLAST_3,
    WEAPON_SPEC_QUALITY_BLAST_4,
    WEAPON_SPEC_QUALITY_DEFENSIVE,  /* +15 Parry, -10 to hit  */
    WEAPON_SPEC_QUALITY_FLAME,      /* No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. */
    WEAPON_SPEC_QUALITY_FLEXIBLE,   /* Cannot be Parried. */
    //WEAPON_SPEC_QUALITY_INACCURATE, /* No bonus with aim action */
    WEAPON_SPEC_QUALITY_OVERHEATS,  /* 90+ -> Overheat */
    WEAPON_SPEC_QUALITY_POWER_FIELD, /* When this is parried, 75% chance to destroy parrying weapon */
    WEAPON_SPEC_QUALITY_PRIMITIVE,  /* AP doubled, unless armour is also primitive */
    WEAPON_SPEC_QUALITY_RECHARGE,   /* Fire only every other round */
    WEAPON_SPEC_QUALITY_RELIABLE,   /* If jam, 10 on 1d10 to actually jam. <I> */
    WEAPON_SPEC_QUALITY_SCATTER,    /* Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. */
    WEAPON_SPEC_QUALITY_SHOCKING,   /* If damage, test Toughness or be stunned. */
    WEAPON_SPEC_QUALITY_SMOKE,      /* Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds. */
    WEAPON_SPEC_QUALITY_SNARE,      /* Ag. test or be immobilised */
    WEAPON_SPEC_QUALITY_TEARING,    /* Roll twice, take highest damage */
    WEAPON_SPEC_QUALITY_TOXIC,      /* if damage, teest toughness -5 for every damage taken. if failed, take 1d10 extra impact damage. */
    WEAPON_SPEC_QUALITY_UNARMED,    /* 1d5-3 +Str, Armour doubles */
    WEAPON_SPEC_QUALITY_UNBALANCED, /* -10 when parry */
    WEAPON_SPEC_QUALITY_UNRELIABLE, /* jams on 90+ */
    WEAPON_SPEC_QUALITY_UNSTABLE,   /* on hit, roll 1d10: 1: half dmg, 2-9: normal, 10: double damage */
    WEAPON_SPEC_QUALITY_UNWIELDY,   /* Cannot be used to parry. */
    WEAPON_SPEC_MAX,
};

enum wearable_special_quality {
    WEARABLE_SPEC_QUALITY_NONE,
    WEARABLE_SPEC_QUALITY_PRIMITIVE, /* AP halved unless weapon is also primitive. */
    WEARABLE_SPEC_QUALITY_MAX,
};

enum weapon_upgrades {
    WEAPON_UPGRADE_NONE,
    WEAPON_UPGRADE_COMPACT,
    WEAPON_UPGRADE_EXTERMINATOR,
    WEAPON_UPGRADE_EXTRA_GRIP,
    WEAPON_UPGRADE_FIRE_SELECTOR,
    WEAPON_UPGRADE_MELEE_ATTACHMENT,
    WEAPON_UPGRADE_MONO,
    WEAPON_UPGRADE_OVERCHARGE_PACK,
    WEAPON_UPGRADE_RED_DOT,
    WEAPON_UPGRADE_SILENCER,
    WEAPON_UPGRADE_TELESCOPE_SIGHT,
    WEAPON_UPGRADE_MAX,
};

enum ammo_upgrades {
    AMMO_UPGRADE_NONE,
    AMMO_UPGRADE_DUMDUM,     /* +2 dmg, armour doubles */
    AMMO_UPGRADE_HOTSHOT,    /* +1 dmg, tearing, +pen 4, clip_sz == 1, -reliable */
    AMMO_UPGRADE_INFERNO,    /* Ag test or catch fire */
    AMMO_UPGRADE_MANSTOPPER, /* +3 Penetration */
    AMMO_UPGRADE_MAX,
};

enum skill_rate {
    MSR_SKILL_RATE_BASIC,
    MSR_SKILL_RATE_ADVANCED,
    MSR_SKILL_RATE_EXPERT,
    MSR_SKILL_RATE_MAX,
    MSR_SKILL_RATE_NONE,
};

enum skills {
    SKILLS_AWARENESS,
    SKILLS_BARTER,
    SKILLS_CHEM_USE,
    SKILLS_COMMON_LORE,
    SKILLS_CONCEALMENT,
    SKILLS_DEMOLITION,
    SKILLS_DISGUISE,
    SKILLS_DODGE,
    SKILLS_EVALUATE,
    SKILLS_FORBIDDEN_LORE,
    SKILLS_INVOCATION,
    SKILLS_LOGIC,
    SKILLS_MEDICAE,
    SKILLS_PSYSCIENCE,
    SKILLS_SCHOLASTIC_LORE,
    SKILLS_SEARCH,
    SKILLS_SECURITY,
    SKILLS_SILENT_MOVE,
    SKILLS_SURVIVAL,
    SKILLS_TECH_USE,
    SKILLS_TRACKING,
    SKILLS_MAX,
};

#define TALENTS_NONE                                      (0)
#define TALENTS0_BASIC_WEAPON_TRAINING_BOLT               ((0x0UL<<27) | (0x1UL<<0))
#define TALENTS0_BASIC_WEAPON_TRAINING_FLAME              ((0x0UL<<27) | (0x1UL<<1))
#define TALENTS0_BASIC_WEAPON_TRAINING_LAS                ((0x0UL<<27) | (0x1UL<<2))
#define TALENTS0_BASIC_WEAPON_TRAINING_LAUNCHER           ((0x0UL<<27) | (0x1UL<<3))
#define TALENTS0_BASIC_WEAPON_TRAINING_MELTA              ((0x0UL<<27) | (0x1UL<<4))
#define TALENTS0_BASIC_WEAPON_TRAINING_PLASMA             ((0x0UL<<27) | (0x1UL<<5))
#define TALENTS0_BASIC_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<27) | (0x1UL<<6))
#define TALENTS0_BASIC_WEAPON_TRAINING_SOLID_PROJECTILE   ((0x0UL<<27) | (0x1UL<<7))
#define TALENTS0_HEAVY_WEAPON_TRAINING_BOLT               ((0x0UL<<27) | (0x1UL<<8))
#define TALENTS0_HEAVY_WEAPON_TRAINING_FLAME              ((0x0UL<<27) | (0x1UL<<9))
#define TALENTS0_HEAVY_WEAPON_TRAINING_LAS                ((0x0UL<<27) | (0x1UL<<10))
#define TALENTS0_HEAVY_WEAPON_TRAINING_LAUNCHER           ((0x0UL<<27) | (0x1UL<<11))
#define TALENTS0_HEAVY_WEAPON_TRAINING_MELTA              ((0x0UL<<27) | (0x1UL<<12))
#define TALENTS0_HEAVY_WEAPON_TRAINING_PLASMA             ((0x0UL<<27) | (0x1UL<<13))
#define TALENTS0_HEAVY_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<27) | (0x1UL<<14))
#define TALENTS0_HEAVY_WEAPON_TRAINING_SOLID_PROJECTILE   ((0x0UL<<27) | (0x1UL<<15))
#define TALENTS0_PISTOL_WEAPON_TRAINING_BOLT              ((0x0UL<<27) | (0x1UL<<16))
#define TALENTS0_PISTOL_WEAPON_TRAINING_FLAME             ((0x0UL<<27) | (0x1UL<<17))
#define TALENTS0_PISTOL_WEAPON_TRAINING_LAS               ((0x0UL<<27) | (0x1UL<<18))
#define TALENTS0_PISTOL_WEAPON_TRAINING_LAUNCHER          ((0x0UL<<27) | (0x1UL<<19))
#define TALENTS0_PISTOL_WEAPON_TRAINING_MELTA             ((0x0UL<<27) | (0x1UL<<20))
#define TALENTS0_PISTOL_WEAPON_TRAINING_PLASMA            ((0x0UL<<27) | (0x1UL<<21))
#define TALENTS0_PISTOL_WEAPON_TRAINING_PRIMITIVE         ((0x0UL<<27) | (0x1UL<<22))
#define TALENTS0_PISTOL_WEAPON_TRAINING_SOLID_PROJECTILE  ((0x0UL<<27) | (0x1UL<<23))
#define TALENTS0_THROWN_WEAPON_TRAINING_PRIMITIVE         ((0x0UL<<27) | (0x1UL<<24))
#define TALENTS0_THROWN_WEAPON_TRAINING_CHAIN             ((0x0UL<<27) | (0x1UL<<25))
#define TALENTS0_THROWN_WEAPON_TRAINING_SHOCK             ((0x0UL<<27) | (0x1UL<<26))
#define TALENTS0_THROWN_WEAPON_TRAINING_POWER             ((0x0UL<<27) | (0x1UL<<27))
#define TALENTS0_CREATURE_TALENT_HANDS                    ((0x0UL<<27) | (0x1UL<<28))
#define TALENTS0_MELEE_WEAPON_TRAINING_PRIMITIVE          ((0x0UL<<27) | (0x1UL<<29))
#define TALENTS0_MELEE_WEAPON_TRAINING_CHAIN              ((0x0UL<<27) | (0x1UL<<30))
#define TALENTS0_MELEE_WEAPON_TRAINING_SHOCK              ((0x0UL<<27) | (0x1UL<<31))
#define TALENTS0_MELEE_WEAPON_TRAINING_POWER              ((0x0UL<<27) | (0x1UL<<32))
#define TALENTS0_RESISTANCE_COLD                          ((0x0UL<<27) | (0x1UL<<33))
#define TALENTS0_RESISTANCE_HEAT                          ((0x0UL<<27) | (0x1UL<<34))
#define TALENTS0_RESISTANCE_FEAR                          ((0x0UL<<27) | (0x1UL<<35))
#define TALENTS0_RESISTANCE_POISON                        ((0x0UL<<27) | (0x1UL<<36))
#define TALENTS0_RESISTANCE_WARP                          ((0x0UL<<27) | (0x1UL<<37))
#define TALENTS0_HATRED_FACTION_CRIMINALS                 ((0x0UL<<27) | (0x1UL<<38))
#define TALENTS0_HATRED_FACTION_DAEMONS                   ((0x0UL<<27) | (0x1UL<<39))
#define TALENTS0_HATRED_FACTION_ORKS                      ((0x0UL<<27) | (0x1UL<<40))
#define TALENTS0_HATRED_FACTION_TYRANIDS                  ((0x0UL<<27) | (0x1UL<<41))
#define TALENTS0_HATRED_FACTION_PSYKERS                   ((0x0UL<<27) | (0x1UL<<42))
#define TALENTS0_HATRED_FACTION_HERETICS                  ((0x0UL<<27) | (0x1UL<<43))
#define TALENTS0_HATRED_FACTION_MUTANTS                   ((0x0UL<<27) | (0x1UL<<44))
#define TALENTS0_EXOTIC_WEAPON_TRAINING_NEEDLE_PISTOL     ((0x0UL<<27) | (0x1UL<<45))
#define TALENTS0_EXOTIC_WEAPON_TRAINING_WEB_PISTOL        ((0x0UL<<27) | (0x1UL<<46))
#define TALENTS0_EXOTIC_WEAPON_TRAINING_NEEDLE_RIFLE      ((0x0UL<<27) | (0x1UL<<47))
#define TALENTS0_EXOTIC_WEAPON_TRAINING_WEBBER            ((0x0UL<<27) | (0x1UL<<48))
#define TALENTS0_PSY_RATING_1                             ((0x0UL<<27) | (0x1UL<<49))
#define TALENTS0_PSY_RATING_2                             ((0x0UL<<27) | (0x1UL<<50))
#define TALENTS0_PSY_RATING_3                             ((0x0UL<<27) | (0x1UL<<51))
#define TALENTS0_PSY_RATING_4                             ((0x0UL<<27) | (0x1UL<<52))
#define TALENTS0_PSY_RATING_5                             ((0x0UL<<27) | (0x1UL<<53))
#define TALENTS0_PSY_RATING_6                             ((0x0UL<<27) | (0x1UL<<54))

#define TALENTS1_TALENT_AWARENESS                         ((0x1UL<<27) | (0x1UL<<1))
#define TALENTS1_TALENT_BARTER                            ((0x1UL<<27) | (0x1UL<<2))
#define TALENTS1_TALENT_CHEM_USE                          ((0x1UL<<27) | (0x1UL<<3))
#define TALENTS1_TALENT_COMMON_LORE                       ((0x1UL<<27) | (0x1UL<<4))
#define TALENTS1_TALENT_CONCEALMENT                       ((0x1UL<<27) | (0x1UL<<5))
#define TALENTS1_TALENT_DEMOLITION                        ((0x1UL<<27) | (0x1UL<<6))
#define TALENTS1_TALENT_DISGUISE                          ((0x1UL<<27) | (0x1UL<<7))
#define TALENTS1_TALENT_DODGE                             ((0x1UL<<27) | (0x1UL<<8))
#define TALENTS1_TALENT_EVALUATE                          ((0x1UL<<27) | (0x1UL<<9))
#define TALENTS1_TALENT_FORBIDDEN_LORE                    ((0x1UL<<27) | (0x1UL<<10))
#define TALENTS1_TALENT_INVOCATION                        ((0x1UL<<27) | (0x1UL<<11))
#define TALENTS1_TALENT_LOGIC                             ((0x1UL<<27) | (0x1UL<<12))
#define TALENTS1_TALENT_MEDICAE                           ((0x1UL<<27) | (0x1UL<<13))
#define TALENTS1_TALENT_PSYSCIENCE                        ((0x1UL<<27) | (0x1UL<<14))
#define TALENTS1_TALENT_SCHOLASTIC_LORE                   ((0x1UL<<27) | (0x1UL<<15))
#define TALENTS1_TALENT_SEARCH                            ((0x1UL<<27) | (0x1UL<<16))
#define TALENTS1_TALENT_SECURITY                          ((0x1UL<<27) | (0x1UL<<17))
#define TALENTS1_TALENT_SILENT_MOVE                       ((0x1UL<<27) | (0x1UL<<18))
#define TALENTS1_TALENT_SURVIVAL                          ((0x1UL<<27) | (0x1UL<<19))
#define TALENTS1_TALENT_TECH_USE                          ((0x1UL<<27) | (0x1UL<<20))
#define TALENTS1_AMBIDEXTROUS                             ((0x1UL<<27) | (0x1UL<<21))
#define TALENTS1_ARMOUR_OF_CONTEMPT                       ((0x1UL<<27) | (0x1UL<<22))
#define TALENTS1_ARMS_MASTER                              ((0x1UL<<27) | (0x1UL<<23))
#define TALENTS1_ASSASSIN_STRIKE                          ((0x1UL<<27) | (0x1UL<<24))
#define TALENTS1_AUTOSANGUINE                             ((0x1UL<<27) | (0x1UL<<25))
#define TALENTS1_BATTLE_RAGE                              ((0x1UL<<27) | (0x1UL<<26))
#define TALENTS1_BLADE_MASTER                             ((0x1UL<<27) | (0x1UL<<27))
#define TALENTS1_BLIND_FIGHTING                           ((0x1UL<<27) | (0x1UL<<28))
#define TALENTS1_BULGING_BICEPS                           ((0x1UL<<27) | (0x1UL<<29))
#define TALENTS1_CLEANSE_AND_PURIFY                       ((0x1UL<<27) | (0x1UL<<30))
#define TALENTS1_COMBAT_MASTER                            ((0x1UL<<27) | (0x1UL<<31))
#define TALENTS1_CORPUS_CONVERSION                        ((0x1UL<<27) | (0x1UL<<32))
#define TALENTS1_COUNTER_ATTACK                           ((0x1UL<<27) | (0x1UL<<33))
#define TALENTS1_CRACK_SHOT                               ((0x1UL<<27) | (0x1UL<<34))
#define TALENTS1_CRIPPLING_STRIKE                         ((0x1UL<<27) | (0x1UL<<35))
#define TALENTS1_CRUSHING_BLOW                            ((0x1UL<<27) | (0x1UL<<36))
#define TALENTS1_DARK_SOUL                                ((0x1UL<<27) | (0x1UL<<37))
#define TALENTS1_DEADEYE_SHOT                             ((0x1UL<<27) | (0x1UL<<38))
#define TALENTS1_DEFLECT_SHOT                             ((0x1UL<<27) | (0x1UL<<39))
#define TALENTS1_DIE_HARD                                 ((0x1UL<<27) | (0x1UL<<40))
#define TALENTS1_DISARM                                   ((0x1UL<<27) | (0x1UL<<41))
#define TALENTS1_DISCIPLINE_FOCUS                         ((0x1UL<<27) | (0x1UL<<42))
#define TALENTS1_DOUBLE_TEAM                              ((0x1UL<<27) | (0x1UL<<43))
#define TALENTS1_DUAL_SHOT                                ((0x1UL<<27) | (0x1UL<<44))
#define TALENTS1_DUAL_STRIKE                              ((0x1UL<<27) | (0x1UL<<45))

#define TALENTS2_ELECTRICAL_SOCCOUR                       ((0x2UL<<27) | (0x1UL<<1))
#define TALENTS2_ELECTRO_GRAFT_USE                        ((0x2UL<<27) | (0x1UL<<2))
#define TALENTS2_FAVOURED_BY_THE_WARP                     ((0x2UL<<27) | (0x1UL<<3))
#define TALENTS2_FEARLESS                                 ((0x2UL<<27) | (0x1UL<<4))
#define TALENTS2_FEEDBACK_SCREECH                         ((0x2UL<<27) | (0x1UL<<5))
#define TALENTS2_FLAGELLANT                               ((0x2UL<<27) | (0x1UL<<6))
#define TALENTS2_FORESIGHT                                ((0x2UL<<27) | (0x1UL<<7))
#define TALENTS2_FRENZY                                   ((0x2UL<<27) | (0x1UL<<8))
#define TALENTS2_FURIOUS_ASSAULT                          ((0x2UL<<27) | (0x1UL<<9))
#define TALENTS2_GUN_BLESSING                             ((0x2UL<<27) | (0x1UL<<10))
#define TALENTS2_GUNSLINGER                               ((0x2UL<<27) | (0x1UL<<11))
#define TALENTS2_HARDY                                    ((0x2UL<<27) | (0x1UL<<12))
#define TALENTS2_HEIGHTENED_SENSES                        ((0x2UL<<27) | (0x1UL<<13))
#define TALENTS2_INSANELY_FAITHFULL                       ((0x2UL<<27) | (0x1UL<<14))
#define TALENTS2_IRON_JAW                                 ((0x2UL<<27) | (0x1UL<<15))
#define TALENTS2_JADED                                    ((0x2UL<<27) | (0x1UL<<16))
#define TALENTS2_LIGHTNING_ATTACK                         ((0x2UL<<27) | (0x1UL<<17))
#define TALENTS2_LOGIS_IMPLANT                            ((0x2UL<<27) | (0x1UL<<18))
#define TALENTS2_LUMINEN_CHARGE                           ((0x2UL<<27) | (0x1UL<<19))
#define TALENTS2_MAGLEV_CHARGE                            ((0x2UL<<27) | (0x1UL<<20))
#define TALENTS2_MARKSMEN                                 ((0x2UL<<27) | (0x1UL<<21))
#define TALENTS2_MASTER_CHIRUGEON                         ((0x2UL<<27) | (0x1UL<<22))
#define TALENTS2_MECHANDENDRITE_USE                       ((0x2UL<<27) | (0x1UL<<23))
#define TALENTS2_MEDITATION                               ((0x2UL<<27) | (0x1UL<<24))
#define TALENTS2_MENTAL_FORTRESS                          ((0x2UL<<27) | (0x1UL<<25))
#define TALENTS2_MENTAL_RAGE                              ((0x2UL<<27) | (0x1UL<<26))
#define TALENTS2_MIGHTY_SHOT                              ((0x2UL<<27) | (0x1UL<<27))
#define TALENTS2_MINOR_PSYCHIC_POWER                      ((0x2UL<<27) | (0x1UL<<28))
#define TALENTS2_NERVES_OF_STEEL                          ((0x2UL<<27) | (0x1UL<<29))
#define TALENTS2_POWER_WELL                               ((0x2UL<<27) | (0x1UL<<30))
#define TALENTS2_PROSANGUINE                              ((0x2UL<<27) | (0x1UL<<31))
#define TALENTS2_QUICK_DRAW                               ((0x2UL<<27) | (0x1UL<<32))
#define TALENTS2_RAPID_RELOAD                             ((0x2UL<<27) | (0x1UL<<33))
#define TALENTS2_SOUND_CONSTITUTIOIN                      ((0x2UL<<27) | (0x1UL<<34))
#define TALENTS2_SPRINT                                   ((0x2UL<<27) | (0x1UL<<35))
#define TALENTS2_STEP_ASIDE                               ((0x2UL<<27) | (0x1UL<<36))
#define TALENTS2_STREET_FIGHTING                          ((0x2UL<<27) | (0x1UL<<37))
#define TALENTS2_STRONG_MINDED                            ((0x2UL<<27) | (0x1UL<<38))
#define TALENTS2_SWIFT_ATTACK                             ((0x2UL<<27) | (0x1UL<<39))
#define TALENTS2_TAKE_DOWN                                ((0x2UL<<27) | (0x1UL<<40))
#define TALENTS2_TECHNICAL_KNOCK                          ((0x2UL<<27) | (0x1UL<<41))
#define TALENTS2_TRUE_GRIT                                ((0x2UL<<27) | (0x1UL<<42))
#define TALENTS2_TWO_WEAPON_WIELDER_MELEE                 ((0x2UL<<27) | (0x1UL<<43))
#define TALENTS2_TWO_WEAPON_WIELDER_BALLISTIC             ((0x2UL<<27) | (0x1UL<<44))
#define TALENTS2_UNSHAKEBLE_FAITH                         ((0x2UL<<27) | (0x1UL<<45))
#define TALENTS2_WALL_OF_STEEL                            ((0x2UL<<27) | (0x1UL<<46))


#define TALENTS_MAX                                       ( ((0x2UL<<27) | (0x1UL<<46)) +1)

#endif /* SPECIALQUALITIES_H */
