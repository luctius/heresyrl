#ifndef SPECIALQUALITIES_H
#define SPECIALQUALITIES_H

enum dmg_type {
    DMG_TYPE_ARROW,
    DMG_TYPE_BLUNT,
    DMG_TYPE_BULLET,
    DMG_TYPE_CLAW,
    DMG_TYPE_CUTTING,
    DMG_TYPE_ENERGY,
    DMG_TYPE_PIERCING,
    DMG_TYPE_SHRAPNEL,
    DMG_TYPE_UNARMED,
    DMG_TYPE_MAX,
};

/* Check Only war for more ideas about weapon spc qualities and upgrades*/

enum weapon_special_quality {
    WPN_SPCQLTY_ACCURATE,    /* Additional +10 to hit when used with the aim action */
    WPN_SPCQLTY_BALANCED,    /* Ignore the -20% Dual Wield Penalty */
    WPN_SPCQLTY_BLAST_1,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_2,     /* All within the weapon's blast radius in squares is hit */
    WPN_SPCQLTY_BLAST_3,
    WPN_SPCQLTY_BLAST_4,
    WPN_SPCQLTY_CREATURE,    /* Non dropable creature weapon. */
    WPN_SPCQLTY_DEFENSIVE,   /* +10 Parry, -10 WS to attack */
    WPN_SPCQLTY_EXPERIMENTAL,/* Unstable, 96-98, jam, 99-00 -> explodes.  */
    WPN_SPCQLTY_FAST,        /* -10% to parry or dodge against */
    WPN_SPCQLTY_FLAME,       /* No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. */
    WPN_SPCQLTY_FLEXIBLE,    /* Cannot be Parried. */
    WPN_SPCQLTY_IMPACT,      /* 2x 1d10, pick highest */
    WPN_SPCQLTY_INACCURATE,  /* No bonus with aim action */
    WPN_SPCQLTY_JAMS,        /* Can Jam on 95% */
    WPN_SPCQLTY_LIGHT,       /* This light weapon is easier to handle in your off-hand. */
    WPN_SPCQLTY_OVERHEATS,   /* 90+ -> Overheat */
    WPN_SPCQLTY_PRIMITIVE,   /* AP doubled, unless armour is also primitive */
    WPN_SPCQLTY_PRECISE,     /* +2 Critical hit value */
    WPN_SPCQLTY_PUMMELING,   /* +10% on Strength for Strike to Stun */
    WPN_SPCQLTY_RELIABLE,    /* If jam, 10 on 1d10 to actually jam. <I> */
    WPN_SPCQLTY_SCATTER,     /* Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. */
    WPN_SPCQLTY_SHIELD,      /* Gives -10% against ranged attackers. */
    WPN_SPCQLTY_SHRAPNEL,    /* Fires in a Cone, requires an Agility Check */
    WPN_SPCQLTY_SHOCKING,    /* If damage, test Toughness or be stunned. */
    WPN_SPCQLTY_SLOW,        /* +10% to parry or dodge against. */
    WPN_SPCQLTY_SMOKE,       /* Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds. */
    WPN_SPCQLTY_SNARE,       /* Ag. test or be immobilised */
    WPN_SPCQLTY_TEARING,     /* Roll twice, take highest damage */
    WPN_SPCQLTY_TOXIC,       /* if damage, test toughness -5 for every damage taken. if failed, take 1d10 extra impact damage. */
    WPN_SPCQLTY_UNARMED,     /* 1d5-3 +Str, Armour doubles */
    WPN_SPCQLTY_UNBALANCED,  /* -10 when parry */
    WPN_SPCQLTY_UNRELIABLE,  /* jams on 90+ */
    WPN_SPCQLTY_UNSTABLE,    /* on hit, roll 1d10: 1: half dmg, 2-9: normal, 10: double damage */
    WPN_SPCQLTY_UNWIELDY,    /* Cannot be used to parry. */
    WPN_SPCQLTY_MAX,
};

enum wearable_special_quality {
    WBL_SPCQLTY_PRIMITIVE, /* Half armour bonus. */
    WBL_SPCQLTY_LEATHER,   /* No Penalties */
    WBL_SPCQLTY_MAIL,      /* -10% Agility */
    WBL_SPCQLTY_SCALE,     /* -10% Agility -1 Movement */
    WBL_SPCQLTY_PLATE,     /* -20% Agility -1 Movement */
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
    MSR_SKILLS_CHARM_ANIMAL,
    MSR_SKILLS_CHANNELING,
    MSR_SKILLS_CHEM_USE,
    MSR_SKILLS_COMMAND,
    MSR_SKILLS_CONCEALMENT,
    MSR_SKILLS_DODGE,
    MSR_SKILLS_FOLLOW_TRAIL,
    MSR_SKILLS_HAGGLE,
    MSR_SKILLS_HEAL,
    MSR_SKILLS_MAGICAL_SENSE,
    MSR_SKILLS_PICK_LOCK,
    MSR_SKILLS_PREPARE_POISON,
    MSR_SKILLS_READ_WRITE,
    MSR_SKILLS_SCALE_SHEER_SURFACE,
    MSR_SKILLS_SEARCH,
    MSR_SKILLS_SET_TRAP,
    MSR_SKILLS_SILENT_MOVE,
    MSR_SKILLS_SURVIVAL,
    MSR_SKILLS_TORTURE,
    MSR_SKILLS_VENTRILOQUISM,
    MSR_SKILLS_MAX,
};

enum msr_creature_traits {
    CTRTRTS_ARMORPHOUS,
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
    CTRTRTS_POSESSION,
    CTRTRTS_QUADRUPED,
    CTRTRTS_REGENERATION,
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
    CTRTRTS_CHAOR_INSTABILITY,
    CTRTRTS_CHAOR_WEAPONS,
    CTRTRTS_MAX,
};

enum msr_talents {
    TLT_NONE,                   /* TLT_NONE is a talent every one has, used to denote talentless weapons */
    TLT_ACUTE_SIGHT,
    TLT_AETHYRIC_ATTUNEMENT,
    TLT_ALLEY_CAT,
    TLT_ANIMAL_HANDLER,
    TLT_AMBIDEXTROUS,
    TLT_ARCANE_LORE_BEAST,
    TLT_ARCANE_LORE_DEATH,
    TLT_ARCANE_LORE_FIRE,
    TLT_ARCANE_LORE_HEAVENS,
    TLT_ARCANE_LORE_LIFE,
    TLT_ARCANE_LORE_LIGHT,
    TLT_ARCANE_LORE_METAL,
    TLT_ARCANE_LORE_SHADOW,
    TLT_ARMOURED_CASTING,
    TLT_BLIND_FIGHTING,
    TLT_CRACK_SHOT,
    TLT_COOL_HEADED,
    TLT_CRY_OF_WAR,
    TLT_CRIPPLING_STRIKE,
    TLT_DARK_LORE_CHAOS,
    TLT_DARK_LORE_NECROMANCY,
    TLT_DEAD_AIM,
    TLT_DIVINE_LORE_MANANN,
    TLT_DIVINE_LORE_MORR,
    TLT_DIVINE_LORE_MYRMIDIA,
    TLT_DIVINE_LORE_RANALD,
    TLT_DIVINE_LORE_SHALLYA,
    TLT_DIVINE_LORE_SIGMAR,
    TLT_DIVINE_LORE_TAAL_AND_RHYA,
    TLT_DIVINE_LORE_VERENA,
    TLT_EXCELENT_VISION,
    TLT_FAST_HANDS,
    TLT_FEARLESS,
    TLT_FLEE,
    TLT_FLEET_FOOTED,
    TLT_FOCUSSED_STRIKE,
    TLT_FRENZY,
    TLT_GRUDGE_BORN_FURY,
    TLT_HAMMER_BLOW,
    TLT_HARDY,
    TLT_HEDGE_MAGIC,
    TLT_HOVERER,
    TLT_KEEN_SENSES,
    TLT_LESSER_MAGIC_DISPEL,
    TLT_LESSER_MAGIC_BLESSED_WEAPON,
    TLT_LIGHTNING_ATTACK,
    TLT_LIGHTNING_REFLEXES,
    TLT_LUCKY,
    TLT_MARKSMAN,
    TLT_MASTER_GUNNER,
    TLT_MEDITATION,
    TLT_MENACING,
    TLT_MIGHTY_MISSILE,
    TLT_MIGHTY_SHOT,
    TLT_NIGHT_VISION,
    TLT_PETTY_MAGIC_ARCANE,
    TLT_PETTY_MAGIC_DIVINE,
    TLT_PETTY_MAGIC_HEDGE,
    TLT_QUICK_DRAW,
    TLT_RAPID_RELOAD,
    TLT_RESITANCE_TO_CHAOS,
    TLT_RESITANCE_TO_DISEASE,
    TLT_RESITANCE_TO_MAGIC,
    TLT_RESITANCE_TO_POISON,
    TLT_ROVER,
    TLT_SAVVY,
    TLT_SHARPSHOOTER,
    TLT_SIXTH_SENSE,
    TLT_SPEC_WPN_GRP_CROSSBOW,
    TLT_SPEC_WPN_GRP_ENTANGLING,
    TLT_SPEC_WPN_GRP_FENCING,
    TLT_SPEC_WPN_GRP_FLAIL,
    TLT_SPEC_WPN_GRP_GUNPOWDER,
    TLT_SPEC_WPN_GRP_LONGBOW,
    TLT_SPEC_WPN_GRP_PARRYING,
    TLT_SPEC_WPN_GRP_SLING,
    TLT_SPEC_WPN_GRP_THROWING,
    TLT_SPEC_WPN_GRP_TWO_HANDED,
    TLT_STOUT_HEARTED,
    TLT_STREET_FIGHTING,
    TLT_STRIKE_MIGHTY_BLOW,
    TLT_STRIKE_TO_INJURE,
    TLT_STRIKE_TO_STUN,
    TLT_STRONG_MINDED,
    TLT_STURDY,
    TLT_SURE_SHOT,
    TLT_SURGERY,
    TLT_TRAPFINDING,
    TLT_TWO_WEAPON_FIGHTING,
    TLT_TUNNEL_RAT,
    TLT_VERY_RESILIENT,
    TLT_VERY_STRONG,
    TLT_WARRIOR_BORN,
    TLT_WRESTLING,
    TLT_MAX,        /*MSR_TALENTS_MAX is also used as an eof marker*/
};


#endif /* SPECIALQUALITIES_H */
