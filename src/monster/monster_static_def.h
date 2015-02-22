#define MONSTER(_icon,_sd_name,_ld_name,_gender,maxwounds) \
        {.uid=0, .icon=_icon, .sd_name=cs_MONSTER _sd_name cs_CLOSE, \
        .ld_name=cs_MONSTER _ld_name cs_CLOSE, \
        .gender=_gender, .cur_wounds=maxwounds, .max_wounds=maxwounds,
#define MONSTER_END }

#define CREATION(wght, lvl, l) \
    .weight=wght, .level=lvl, .dungeon_locale=l

#define DESCRIPTION(desc) .description=desc

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,wl,per) \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, \
    .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, \
    .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, \
    .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=per,} 

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_BASIC]=basic_skills, \
    .skills[MSR_SKILL_RATE_ADVANCED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERT]=expert_skills

#define DEF_ITEM(idx, item_id) .def_items[idx]=item_id

#define DWARF( t...) .race=MSR_RACE_DWARF, .size=MSR_SIZE_AVERAGE, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=3, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }, \
    .talents = { t, TLT_NONE, }, .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED

#define ELF( t...) .race=MSR_RACE_ELF, .size=MSR_SIZE_AVERAGE, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=5, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }, \
    .talents = { t, TLT_NONE, }, .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED

#define HALFLING( t...) .race=MSR_RACE_HALFLING, .size=MSR_SIZE_AVERAGE, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=3, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }, \
    .talents = { t, TLT_NONE, }, .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED

#define HUMAN( t...) .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=4, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }, \
    .talents = { t, TLT_NONE, }, .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED

#define GREENSKIN( t...) .race=MSR_RACE_GREENSKIN, .size=MSR_SIZE_AVERAGE, \
    .icon_attr = TERM_COLOUR_GREEN, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=4, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }, \
    .talents = { t, TLT_NONE, }, .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED

#define BEAST( t...) .race=MSR_RACE_BEAST, .size=MSR_SIZE_AVERAGE, \
    .icon_attr = TERM_COLOUR_MUD, \
    .talents = { t , TLT_NONE, }, .creature_traits = bf(CTRTRTS_BESTIAL) | bf(CTRTRTS_QUADRUPED), \
    .crtr_wpn=IID_CREATURE_BITE_TRAINED, \
    .characteristic[MSR_SEC_CHAR_ATTACKS]={ .base_value=1, }, \
    .characteristic[MSR_SEC_CHAR_MOVEMENT]={ .base_value=6, }, \
    .characteristic[MSR_SEC_CHAR_MAGIC]={ .base_value=0, }

struct msr_monster static_monster_list[] = {

    /*----------------- CHARACTER TEMPLATES  --------------------*/
    /*
       Remember to modify the starting wounds table and the fatepoint table
       in player.c when adding new player selectable races.
     */
    [MID_DUMMY]=MONSTER('h',"dummy","a dummy",MSR_GENDER_MALE,1) 
        HUMAN(TLT_NONE), CHARACTERISTICS(0,0,0,0,0,0,0,0), SKILLS(0,0,0),
    MONSTER_END,

    [MID_BASIC_DWARF]=MONSTER('d',"dwarf","a dwarf",MSR_GENDER_MALE,0)
        DWARF(TLT_GRUDGE_BORN_FURY, TLT_NIGHT_VISION, TLT_RESITANCE_TO_MAGIC, TLT_STOUT_HEARTED, TLT_STURDY),
        CHARACTERISTICS(30,20,20,30,10,20,20,10),
        SKILLS(0,0,0), .fate_points=0, .is_player=true,
        DESCRIPTION("description of a dwarf"),
    MONSTER_END,

    [MID_BASIC_ELF]=MONSTER('e',"elf","an elf",MSR_GENDER_MALE,0)
        ELF(TLT_NIGHT_VISION),
        CHARACTERISTICS(15,30,15,20,30,20,20,20),
        SKILLS(0,0,0), .fate_points=0,  .is_player=true,
        DESCRIPTION("description of an elf"),
    MONSTER_END,

    [MID_BASIC_HALFLING]=MONSTER('h',"halfling","an halfling",MSR_GENDER_MALE,0)
        HALFLING(TLT_NIGHT_VISION, TLT_RESITANCE_TO_CHAOS, TLT_SPEC_WPN_GRP_SLING),
        CHARACTERISTICS(10,30,10,10,30,20,20,30),
        SKILLS(0,0,0), .fate_points=0,  .is_player=true,
        DESCRIPTION("description of an halfling"),
    MONSTER_END,

    [MID_BASIC_HUMAN]=MONSTER('H',"human","a human",MSR_GENDER_MALE,0)
        HUMAN(TLT_NONE),
        CHARACTERISTICS(20,20,20,20,20,20,20,20),
        SKILLS(0,0,0), .fate_points=0,  .is_player=true,
        DESCRIPTION("description of an human"),
    MONSTER_END,
    /*----------------------------------------------------------*/

    MONSTER('g',"goblin","a goblin warrior",MSR_GENDER_MALE,1)
        GREENSKIN(TLT_NIGHT_VISION),
        CHARACTERISTICS(25,30,30,30,25,25,30,30),
        DEF_ITEM(0, ITEM_GROUP_1H_MELEE),
        SKILLS(MSR_SKILLS_CONCEALMENT|MSR_SKILLS_SURVIVAL|MSR_SKILLS_AWARENESS|MSR_SKILLS_SCALE_SHEER_SURFACE|MSR_SKILLS_SET_TRAP|MSR_SKILLS_SILENT_MOVE,0,0),
        DESCRIPTION("description of an goblin fighter"),
        CREATION(30,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) ),
    MONSTER_END,

    MONSTER('g',"goblin","a goblin archer",MSR_GENDER_MALE,1)
        GREENSKIN(TLT_NIGHT_VISION),
        CHARACTERISTICS(25,30,30,30,25,25,30,30),
        DEF_ITEM(0, ITEM_GROUP_RANGED),
        SKILLS(MSR_SKILLS_CONCEALMENT|MSR_SKILLS_SURVIVAL|MSR_SKILLS_AWARENESS|MSR_SKILLS_SCALE_SHEER_SURFACE|MSR_SKILLS_SET_TRAP|MSR_SKILLS_SILENT_MOVE,0,0),
        DESCRIPTION("description of an goblin archer"),
        CREATION(30,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) ),
    MONSTER_END,

    MONSTER('g',"goblin","a goblin thrower",MSR_GENDER_MALE,1)
        GREENSKIN(TLT_NIGHT_VISION),
        CHARACTERISTICS(25,30,30,30,25,25,30,30),
        DEF_ITEM(0, ITEM_GROUP_THROWING),
        SKILLS(MSR_SKILLS_CONCEALMENT|MSR_SKILLS_SURVIVAL|MSR_SKILLS_AWARENESS|MSR_SKILLS_SCALE_SHEER_SURFACE|MSR_SKILLS_SET_TRAP|MSR_SKILLS_SILENT_MOVE,0,0),
        DESCRIPTION("description of an goblin thrower"),
        CREATION(30,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) ),
    MONSTER_END,

    MONSTER('H',"human","a human fighter",MSR_GENDER_MALE,1)
        HUMAN(TLT_NONE),
        CHARACTERISTICS(29,42,30,31,35,30,28,30),
        DEF_ITEM(0, ITEM_GROUP_1H_MELEE),
        SKILLS(MSR_SKILLS_CHARM_ANIMAL|MSR_SKILLS_CONCEALMENT|MSR_SKILLS_AWARENESS|MSR_SKILLS_SCALE_SHEER_SURFACE|MSR_SKILLS_SET_TRAP|MSR_SKILLS_SILENT_MOVE,0,0),
        DESCRIPTION("description of a human fighter"),
        CREATION(5,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) ),
    MONSTER_END,

    MONSTER('H',"human","a human archer",MSR_GENDER_MALE,1)
        HUMAN(TLT_NONE),
        CHARACTERISTICS(29,42,30,31,35,30,28,30),
        DEF_ITEM(0, ITEM_GROUP_RANGED),
        SKILLS(MSR_SKILLS_CHARM_ANIMAL|MSR_SKILLS_CONCEALMENT|MSR_SKILLS_AWARENESS|MSR_SKILLS_SCALE_SHEER_SURFACE|MSR_SKILLS_SET_TRAP|MSR_SKILLS_SILENT_MOVE,0,0),
        DESCRIPTION("description of a human archer"),
        CREATION(5,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) ),
    MONSTER_END,

    MONSTER('w',"wolf","a vicious wolf",MSR_GENDER_MALE,1)
        BEAST(TLT_KEEN_SENSES),
        CHARACTERISTICS(30,0,30,30,40,14,25,30),
        SKILLS(MSR_SKILLS_AWARENESS|MSR_SKILLS_SILENT_MOVE, 0,0),
        DESCRIPTION("description of a viscious dog"),
        CREATION(20,1,bf(DUNGEON_TYPE_ALL) ),
     MONSTER_END,
};

static const char *msr_char_name[] = {
    [MSR_CHAR_WEAPON_SKILL]   = "Weapon Skill",
    [MSR_CHAR_BALISTIC_SKILL] = "Balistic Skill",
    [MSR_CHAR_STRENGTH]       = "Strength",
    [MSR_CHAR_TOUGHNESS]      = "Toughness",
    [MSR_CHAR_AGILITY]        = "Agility",
    [MSR_CHAR_INTELLIGENCE]   = "Intelligence",
    [MSR_CHAR_WILLPOWER]      = "Willpower",
    [MSR_CHAR_PERCEPTION]     = "Perception",
    [MSR_SEC_CHAR_ATTACKS]    = "Attacks",
    [MSR_SEC_CHAR_MOVEMENT]   = "Movement",
    [MSR_SEC_CHAR_MAGIC]      = "Magic",
};

static const char *msr_char_description[] = {
    [MSR_CHAR_WEAPON_SKILL]   = "Weapon Skill",
    [MSR_CHAR_BALISTIC_SKILL] = "Balistic Skill",
    [MSR_CHAR_STRENGTH]       = "Strength",
    [MSR_CHAR_TOUGHNESS]      = "Toughness",
    [MSR_CHAR_AGILITY]        = "Agility",
    [MSR_CHAR_INTELLIGENCE]   = "Intelligence",
    [MSR_CHAR_WILLPOWER]      = "Willpower",
    [MSR_CHAR_PERCEPTION]     = "Perception",
    [MSR_SEC_CHAR_ATTACKS]    = "Attacks",
    [MSR_SEC_CHAR_MOVEMENT]   = "Movement",
    [MSR_SEC_CHAR_MAGIC]      = "Magic",
};

static const char *msr_skillrate_name[] = {
    [MSR_SKILL_RATE_NONE]       = "untrained", 
    [MSR_SKILL_RATE_BASIC]      = "basic", 
    [MSR_SKILL_RATE_ADVANCED]   = "advanced", 
    [MSR_SKILL_RATE_EXPERT]     = "expert",
};

static enum msr_characteristic msr_skill_charac[] = {
    [MSR_SKILLS_AWARENESS]          = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_CHARM_ANIMAL]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_CHANNELING]         = MSR_CHAR_WILLPOWER,
    [MSR_SKILLS_CHEM_USE]           = MSR_CHAR_TOUGHNESS,
    [MSR_SKILLS_COMMAND]            = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_CONCEALMENT]        = MSR_CHAR_AGILITY,
    [MSR_SKILLS_DODGE]              = MSR_CHAR_AGILITY,
    [MSR_SKILLS_FOLLOW_TRAIL]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_HAGGLE]             = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_HEAL]               = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_MAGICAL_SENSE]      = MSR_CHAR_WILLPOWER,
    [MSR_SKILLS_PICK_LOCK]          = MSR_CHAR_AGILITY,
    [MSR_SKILLS_PREPARE_POISON]     = MSR_CHAR_TOUGHNESS,
    [MSR_SKILLS_READ_WRITE]         = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_SCALE_SHEER_SURFACE]= MSR_CHAR_AGILITY,
    [MSR_SKILLS_SEARCH]             = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_SET_TRAP]           = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_SILENT_MOVE]        = MSR_CHAR_AGILITY,
    [MSR_SKILLS_SURVIVAL]           = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_TORTURE]            = MSR_CHAR_STRENGTH,
    [MSR_SKILLS_VENTRILOQUISM]      = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_MAX]                = MSR_CHAR_MAX,
};

static const char *msr_skill_name[] = {
    [MSR_SKILLS_AWARENESS]          = "Awareness",
    [MSR_SKILLS_CHARM_ANIMAL]       = "Charm Animal",
    [MSR_SKILLS_CHANNELING]         = "Channeling",
    [MSR_SKILLS_CHEM_USE]           = "Chem Use",
    [MSR_SKILLS_COMMAND]            = "Command",
    [MSR_SKILLS_CONCEALMENT]        = "Concealment",
    [MSR_SKILLS_DODGE]              = "Dodge",
    [MSR_SKILLS_FOLLOW_TRAIL]       = "Follow Trail",
    [MSR_SKILLS_HAGGLE]             = "Haggle",
    [MSR_SKILLS_HEAL]               = "Heal",
    [MSR_SKILLS_MAGICAL_SENSE]      = "Magical Sense",
    [MSR_SKILLS_PICK_LOCK]          = "Pick Lock",
    [MSR_SKILLS_PREPARE_POISON]     = "Prepare Poison",
    [MSR_SKILLS_READ_WRITE]         = "Read/Write",
    [MSR_SKILLS_SCALE_SHEER_SURFACE]= "Scale Sheer Surface",
    [MSR_SKILLS_SEARCH]             = "Search",
    [MSR_SKILLS_SET_TRAP]           = "Set Trap",
    [MSR_SKILLS_SILENT_MOVE]        = "Silent Move",
    [MSR_SKILLS_SURVIVAL]           = "Survival",
    [MSR_SKILLS_TORTURE]            = "Torture",
    [MSR_SKILLS_VENTRILOQUISM]      = "Ventriloquism",
    [MSR_SKILLS_MAX]                = "Max,"
};

static const char *msr_skill_description[] = {
    [MSR_SKILLS_AWARENESS]          = "Awareness",
    [MSR_SKILLS_CHARM_ANIMAL]       = "Charm Animal",
    [MSR_SKILLS_CHANNELING]         = "Channeling",
    [MSR_SKILLS_CHEM_USE]           = "Chem Use",
    [MSR_SKILLS_COMMAND]            = "Command",
    [MSR_SKILLS_CONCEALMENT]        = "Concealment",
    [MSR_SKILLS_DODGE]              = "Dodge",
    [MSR_SKILLS_FOLLOW_TRAIL]       = "Follow Trail",
    [MSR_SKILLS_HAGGLE]             = "Haggle",
    [MSR_SKILLS_HEAL]               = "Heal",
    [MSR_SKILLS_MAGICAL_SENSE]      = "Magical Sense",
    [MSR_SKILLS_PICK_LOCK]          = "Pick Lock",
    [MSR_SKILLS_PREPARE_POISON]     = "Prepare Poison",
    [MSR_SKILLS_READ_WRITE]         = "Read/Write",
    [MSR_SKILLS_SCALE_SHEER_SURFACE]= "Scale Sheer Surface",
    [MSR_SKILLS_SEARCH]             = "Search",
    [MSR_SKILLS_SET_TRAP]           = "Set Trap",
    [MSR_SKILLS_SILENT_MOVE]        = "Silent Move",
    [MSR_SKILLS_SURVIVAL]           = "Survival",
    [MSR_SKILLS_TORTURE]            = "Torture",
    [MSR_SKILLS_VENTRILOQUISM]      = "Ventriloquism",
    [MSR_SKILLS_MAX]                = "Max,"
};


static const char *msr_talent_name[] = {
    [TLT_NONE]                  = "none",
	[TLT_ACUTE_SIGHT]			= "Acute Sight",
	[TLT_AETHYRIC_ATTUNEMENT]	= "Aethyric Attunement",
	[TLT_ALLEY_CAT]			    = "Alley Cat",
	[TLT_ANIMAL_HANDLER]		= "Animal Handler",
	[TLT_AMBIDEXTROUS]			= "Ambidextrous",
	[TLT_ARCANE_LORE_BEAST]		= "Arcane Lore Beasts",
	[TLT_ARCANE_LORE_DEATH]		= "Arcane Lore Death",
	[TLT_ARCANE_LORE_FIRE]		= "Arcane Lore Fire",
	[TLT_ARCANE_LORE_HEAVENS]	= "Arcane Lore Heavens",
	[TLT_ARCANE_LORE_LIFE]		= "Arcane Lore Life",
	[TLT_ARCANE_LORE_LIGHT]		= "Arcane Lore Light",
	[TLT_ARCANE_LORE_METAL]		= "Arcane Lore Metal",
	[TLT_ARCANE_LORE_SHADOW]	= "Arcane Lore Shadow",
	[TLT_ARMOURED_CASTING]		= "Armoured Casting",
	[TLT_BLIND_FIGHTING]		= "Blind Fighting",
	[TLT_CRACK_SHOT]			= "Crack Shot",
	[TLT_COOL_HEADED]			= "Cool Headed",
	[TLT_CRY_OF_WAR]			= "Cry of War",
	[TLT_CRIPPLING_STRIKE]		= "Crippling Strike",
	[TLT_DARK_LORE_CHAOS]		= "Dark Lore Chaos",
	[TLT_DARK_LORE_NECROMANCY]	= "Dark Lore Necromancy",
	[TLT_DEAD_AIM]			    = "Dead Aim",
	[TLT_DIVINE_LORE_MANANN]    = "Divine Lore Manann",
	[TLT_DIVINE_LORE_MORR]		= "Divine Lore Morr",
	[TLT_DIVINE_LORE_MYRMIDIA]	= "Divine Lore Myrmidia",
	[TLT_DIVINE_LORE_RANALD]	= "Divine Lore Ranald",
	[TLT_DIVINE_LORE_SHALLYA]	= "Divine Lore Shallya",
	[TLT_DIVINE_LORE_SIGMAR]	= "Divine Lore Sigmar",
	[TLT_DIVINE_LORE_TAAL_AND_RHYA]	= "Divine Lore Taal and Rhya",
	[TLT_DIVINE_LORE_VERENA]	= "Divine Lore Verena",
    [TLT_EXCELENT_VISION]       = "Excelent Vision",
	[TLT_FAST_HANDS]			= "Fast Hands",
	[TLT_FEARLESS]			    = "Fearless",
	[TLT_FLEE]			        = "Flee",
	[TLT_FLEET_FOOTED]			= "Fleet Footed",
	[TLT_FOCUSSED_STRIKE]		= "Focussed Strike",
	[TLT_FRENZY]			    = "Frenzy",
	[TLT_GRUDGE_BORN_FURY]	    = "Grudge Born Fury",
	[TLT_HAMMER_BLOW]			= "Hammer Blow",
	[TLT_HARDY]			        = "Hardy",
	[TLT_HEDGE_MAGIC]			= "Hedge Magic",
	[TLT_HOVERER]			    = "Hoverer",
	[TLT_KEEN_SENSES]			= "Keen Senses",
	[TLT_LESSER_MAGIC_DISPEL]   = "Lesser Magic Dispel",
	[TLT_LESSER_MAGIC_BLESSED_WEAPON] = "Lesser Magic Blessed Weapon",
	[TLT_LIGHTNING_ATTACK]		= "Lightning Attack",
	[TLT_LIGHTNING_REFLEXES]	= "Lighting Reflexes",
	[TLT_LUCKY]			        = "Lucky",
	[TLT_MARKSMAN]			    = "Marksman",
	[TLT_MASTER_GUNNER]			= "Master Gunner",
	[TLT_MEDITATION]			= "Meditation",
	[TLT_MENACING]			    = "Menacing",
	[TLT_MIGHTY_MISSILE]		= "Mighty Missile",
	[TLT_MIGHTY_SHOT]			= "Mighty Shot",
	[TLT_NIGHT_VISION]			= "Night Vision",
	[TLT_PETTY_MAGIC_ARCANE]	= "Petty Magic Arcane",
	[TLT_PETTY_MAGIC_DIVINE]	= "Petty Magic Divine",
	[TLT_PETTY_MAGIC_HEDGE]		= "Petty Magic Hedge",
	[TLT_QUICK_DRAW]			= "Quick Draw",
	[TLT_RAPID_RELOAD]			= "Rapid Reload",
	[TLT_RESITANCE_TO_CHAOS]	= "Resistance to Chaos",
	[TLT_RESITANCE_TO_DISEASE]	= "Resistance to Disease",
	[TLT_RESITANCE_TO_MAGIC]	= "Resistance to Magic",
	[TLT_RESITANCE_TO_POISON]	= "Resistance to Poison",
	[TLT_ROVER]			        = "Rover",
	[TLT_SAVVY]			        = "Savvy",
	[TLT_SHARPSHOOTER]			= "Sharpshooter",
	[TLT_SIXTH_SENSE]			= "Sixth Sense",
	[TLT_SPEC_WPN_GRP_CROSSBOW] = "Specialist Weapon Group (Crossbow)",
	[TLT_SPEC_WPN_GRP_ENTANGLING] = "Specialist Weapon Group (Entangling)",
	[TLT_SPEC_WPN_GRP_FENCING]	= "Specialist Weapon Group (Fencing)",
	[TLT_SPEC_WPN_GRP_FLAIL]	= "Specialist Weapon Group (Flail)",
	[TLT_SPEC_WPN_GRP_GUNPOWDER]= "Specialist Weapon Group (Gunpowder)",
	[TLT_SPEC_WPN_GRP_LONGBOW]	= "Specialist Weapon Group (Longbow)",
	[TLT_SPEC_WPN_GRP_PARRYING]	= "Specialist Weapon Group (Parrying)",
	[TLT_SPEC_WPN_GRP_SLING]	= "Specialist Weapon Group (Sling)",
	[TLT_SPEC_WPN_GRP_THROWING]	= "Specialist Weapon Group (Throwing)",
	[TLT_SPEC_WPN_GRP_TWO_HANDED]= "Specialist Weapon Group (Two-Handed)",
	[TLT_STOUT_HEARTED]			= "Strong Hearted",
	[TLT_STREET_FIGHTING]		= "Street Fighting",
	[TLT_STRIKE_MIGHTY_BLOW]	= "Strike Mighty Blow",
	[TLT_STRIKE_TO_INJURE]		= "Strike to Injure",
	[TLT_STRIKE_TO_STUN]		= "Strike to Stun",
	[TLT_STRONG_MINDED]			= "Strong Minded",
	[TLT_STURDY]			    = "Sturdy",
	[TLT_SURE_SHOT]			    = "Sure Shot",
	[TLT_SURGERY]			    = "Surgery",
	[TLT_TRAPFINDING]			= "Trapfindig",
	[TLT_TUNNEL_RAT]			= "Tunnel Rat",
	[TLT_VERY_RESILIENT]		= "Very Resilient",
	[TLT_VERY_STRONG]			= "Very Strong",
	[TLT_WARRIOR_BORN]			= "Warrior Born",
	[TLT_WRESTLING]			    = "Wrestling",
    [TLT_MAX]                   = "max",
};

static const char *msr_talent_description[] = {
    [TLT_NONE]                  = "none",
	[TLT_ACUTE_SIGHT]			= "Acute Sight",
	[TLT_AETHYRIC_ATTUNEMENT]	= "Aethyric Attunement",
	[TLT_ALLEY_CAT]			    = "Alley Cat",
	[TLT_ANIMAL_HANDLER]		= "Animal Handler",
	[TLT_AMBIDEXTROUS]			= "Ambidextrous",
	[TLT_ARCANE_LORE_BEAST]		= "Arcane Lore Beasts",
	[TLT_ARCANE_LORE_DEATH]		= "Arcane Lore Death",
	[TLT_ARCANE_LORE_FIRE]		= "Arcane Lore Fire",
	[TLT_ARCANE_LORE_HEAVENS]	= "Arcane Lore Heavens",
	[TLT_ARCANE_LORE_LIFE]		= "Arcane Lore Life",
	[TLT_ARCANE_LORE_LIGHT]		= "Arcane Lore Light",
	[TLT_ARCANE_LORE_METAL]		= "Arcane Lore Metal",
	[TLT_ARCANE_LORE_SHADOW]	= "Arcane Lore Shadow",
	[TLT_ARMOURED_CASTING]		= "Armoured Casting",
	[TLT_BLIND_FIGHTING]		= "Blind Fighting",
	[TLT_CRACK_SHOT]			= "Crack Shot",
	[TLT_COOL_HEADED]			= "Cool Headed",
	[TLT_CRY_OF_WAR]			= "Cry of War",
	[TLT_CRIPPLING_STRIKE]		= "Crippling Strike",
	[TLT_DARK_LORE_CHAOS]		= "Dark Lore Chaos",
	[TLT_DARK_LORE_NECROMANCY]	= "Dark Lore Necromancy",
	[TLT_DEAD_AIM]			    = "Dead Aim",
	[TLT_DIVINE_LORE_MANANN]    = "Divine Lore Manann",
	[TLT_DIVINE_LORE_MORR]		= "Divine Lore Morr",
	[TLT_DIVINE_LORE_MYRMIDIA]	= "Divine Lore Myrmidia",
	[TLT_DIVINE_LORE_RANALD]	= "Divine Lore Ranald",
	[TLT_DIVINE_LORE_SHALLYA]	= "Divine Lore Shallya",
	[TLT_DIVINE_LORE_SIGMAR]	= "Divine Lore Sigmar",
	[TLT_DIVINE_LORE_TAAL_AND_RHYA]	= "Divine Lore Taal and Rhya",
	[TLT_DIVINE_LORE_VERENA]	= "Divine Lore Verena",
    [TLT_EXCELENT_VISION]       = "Excelent Vision",
	[TLT_FAST_HANDS]			= "Fast Hands",
	[TLT_FEARLESS]			    = "Fearless",
	[TLT_FLEE]			        = "Flee",
	[TLT_FLEET_FOOTED]			= "Fleet Footed",
	[TLT_FOCUSSED_STRIKE]		= "Focussed Strike",
	[TLT_FRENZY]			    = "Frenzy",
	[TLT_GRUDGE_BORN_FURY]	    = "Grudge Born Fury",
	[TLT_HAMMER_BLOW]			= "Hammer Blow",
	[TLT_HARDY]			        = "Hardy",
	[TLT_HEDGE_MAGIC]			= "Hedge Magic",
	[TLT_HOVERER]			    = "Hoverer",
	[TLT_KEEN_SENSES]			= "Keen Senses",
	[TLT_LESSER_MAGIC_DISPEL]   = "Lesser Magic Dispel",
	[TLT_LESSER_MAGIC_BLESSED_WEAPON] = "Lesser Magic Blessed Weapon",
	[TLT_LIGHTNING_ATTACK]		= "Lightning Attack",
	[TLT_LIGHTNING_REFLEXES]	= "Lighting Reflexes",
	[TLT_LUCKY]			        = "Lucky",
	[TLT_MARKSMAN]			    = "Marksman",
	[TLT_MASTER_GUNNER]			= "Master Gunner",
	[TLT_MEDITATION]			= "Meditation",
	[TLT_MENACING]			    = "Menacing",
	[TLT_MIGHTY_MISSILE]		= "Mighty Missile",
	[TLT_MIGHTY_SHOT]			= "Mighty Shot",
	[TLT_NIGHT_VISION]			= "Night Vision",
	[TLT_PETTY_MAGIC_ARCANE]	= "Petty Magic Arcane",
	[TLT_PETTY_MAGIC_DIVINE]	= "Petty Magic Divine",
	[TLT_PETTY_MAGIC_HEDGE]		= "Petty Magic Hedge",
	[TLT_QUICK_DRAW]			= "Quick Draw",
	[TLT_RAPID_RELOAD]			= "Rapid Reload",
	[TLT_RESITANCE_TO_CHAOS]	= "Resistance to Chaos",
	[TLT_RESITANCE_TO_DISEASE]	= "Resistance to Disease",
	[TLT_RESITANCE_TO_MAGIC]	= "Resistance to Magic",
	[TLT_RESITANCE_TO_POISON]	= "Resistance to Poison",
	[TLT_ROVER]			        = "Rover",
	[TLT_SAVVY]			        = "Savvy",
	[TLT_SHARPSHOOTER]			= "Sharpshooter",
	[TLT_SIXTH_SENSE]			= "Sixth Sense",
	[TLT_SPEC_WPN_GRP_CROSSBOW] = "Specialist Weapon Group (Crossbow)",
	[TLT_SPEC_WPN_GRP_ENTANGLING] = "Specialist Weapon Group (Entangling)",
	[TLT_SPEC_WPN_GRP_FENCING]	= "Specialist Weapon Group (Fencing)",
	[TLT_SPEC_WPN_GRP_FLAIL]	= "Specialist Weapon Group (Flail)",
	[TLT_SPEC_WPN_GRP_GUNPOWDER]= "Specialist Weapon Group (Gunpowder)",
	[TLT_SPEC_WPN_GRP_LONGBOW]	= "Specialist Weapon Group (Longbow)",
	[TLT_SPEC_WPN_GRP_PARRYING]	= "Specialist Weapon Group (Parrying)",
	[TLT_SPEC_WPN_GRP_SLING]	= "Specialist Weapon Group (Sling)",
	[TLT_SPEC_WPN_GRP_THROWING]	= "Specialist Weapon Group (Throwing)",
	[TLT_SPEC_WPN_GRP_TWO_HANDED]= "Specialist Weapon Group (Two-Handed)",
	[TLT_STOUT_HEARTED]			= "Strong Hearted",
	[TLT_STREET_FIGHTING]		= "Street Fighting",
	[TLT_STRIKE_MIGHTY_BLOW]	= "Strike Mighty Blow",
	[TLT_STRIKE_TO_INJURE]		= "Strike to Injure",
	[TLT_STRIKE_TO_STUN]		= "Strike to Stun",
	[TLT_STRONG_MINDED]			= "Strong Minded",
	[TLT_STURDY]			    = "Sturdy",
	[TLT_SURE_SHOT]			    = "Sure Shot",
	[TLT_SURGERY]			    = "Surgery",
	[TLT_TRAPFINDING]			= "Trapfindig",
	[TLT_TUNNEL_RAT]			= "Tunnel Rat",
	[TLT_VERY_RESILIENT]		= "Very Resilient",
	[TLT_VERY_STRONG]			= "Very Strong",
	[TLT_WARRIOR_BORN]			= "Warrior Born",
	[TLT_WRESTLING]			    = "Wrestling",
    [TLT_MAX]                   = "max",
};
