#define MONSTER(tid,_icon,_sd_name,_ld_name,_gender,maxwounds) \
    [tid]={.uid=0, .template_id=tid, .icon=_icon, \
        .sd_name=cs_MONSTER _sd_name cs_MONSTER, .ld_name=cs_MONSTER _ld_name cs_MONSTER, \
        .gender=_gender, .cur_wounds=maxwounds, .max_wounds=maxwounds,
#define MONSTER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,pr,wl,fl) \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, \
    .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, \
    .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, \
    .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=pr,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, \
    .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_FELLOWSHIP]={ .base_value=fl,} 

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_BASIC]=basic_skills, \
    .skills[MSR_SKILL_RATE_ADVANCED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERT]=expert_skills

#define HUMAN( t...) .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
    .talents = { t, TLT_NONE }, .creature_traits = 0, .def_wpns[0]=IID_HUMAN_UNARMED

#define BEAST( t...) .race=MSR_RACE_BEAST, .size=MSR_SIZE_AVERAGE, \
    .talents = { t , TLT_NONE }, .creature_traits = bf(CTRTRTS_BESTIAL) | bf(CTRTRTS_QUADRUPED), .def_wpns[0]=IID_CREATURE_BITE_TRAINED

#define DEF_WPN(wpn_idx, item_id) .def_wpns[wpn_idx+1]=item_id

static const char *msr_descs[] = {
    [MID_DUMMY]       = "unknown",
    [MID_BASIC_FERAL] = "you",
    [MID_BASIC_HIVE]  = "you",
    [MID_HIVE_GANGER] = "description of a hive ganger",
    [MID_VICIOUS_DOG] = "description of a vicious dog",
};

static struct msr_monster static_monster_list[] = {

    /*----------------- CHARACTER TEMPLATES  --------------------*/
    MONSTER(MID_DUMMY,'h',"dummy","a dummy",MSR_GENDER_MALE,1) 
        HUMAN(0), CHARACTERISTICS(0,0,0,0,0,0,0,0,0), SKILLS(0,0,0),
    MONSTER_END,

    MONSTER(MID_BASIC_FERAL,'h',"human","a feral human",MSR_GENDER_MALE,14)
        HUMAN(TLT_BASIC_WPN_TRNG_LAS, TLT_PISTOL_WPN_TRNG_LAS, TLT_BASIC_WPN_TRNG_SP, TLT_PISTOL_WPN_TRNG_SP),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), .fate_points=2, MONSTER_END,

    MONSTER(MID_BASIC_HIVE,'h',"human","a hive human",MSR_GENDER_MALE,14)
        HUMAN(TLT_BASIC_WPN_TRNG_SP, TLT_PISTOL_WPN_TRNG_LAS, TLT_BASIC_WPN_TRNG_LAS, TLT_PISTOL_WPN_TRNG_SP),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), .fate_points=2, MONSTER_END,
    /*----------------------------------------------------------*/

    MONSTER(MID_HIVE_GANGER,'h',"human","a hive human",MSR_GENDER_MALE,1)
        HUMAN(TLT_BASIC_WPN_TRNG_SP, TLT_PISTOL_WPN_TRNG_SP),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        DEF_WPN(0, IID_STUB_AUTOMATIC),
        DEF_WPN(1, IID_KNIFE),
        SKILLS(0,0,0), MONSTER_END,

    MONSTER(MID_VICIOUS_DOG,'d',"dog","a vicious dog",MSR_GENDER_MALE,1)
        BEAST(0),
        CHARACTERISTICS(30,0,30,30,30,15,38,40,30),
        SKILLS(MSR_SKILLS_AWARENESS|MSR_SKILLS_SILENT_MOVE|MSR_SKILLS_TRACKING, MSR_SKILLS_AWARENESS|MSR_SKILLS_SILENT_MOVE|MSR_SKILLS_TRACKING ,0), MONSTER_END,
};

static enum msr_characteristic msr_skill_charac[] = {
    [MSR_SKILLS_AWARENESS]      = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_BARTER]         = MSR_CHAR_FELLOWSHIP,
    [MSR_SKILLS_CHEM_USE]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_COMMON_LORE]    = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_CONCEALMENT]    = MSR_CHAR_AGILITY,
    [MSR_SKILLS_DEMOLITION]     = MSR_CHAR_STRENGTH,
    [MSR_SKILLS_DISGUISE]       = MSR_CHAR_FELLOWSHIP,
    [MSR_SKILLS_DODGE]          = MSR_CHAR_AGILITY,
    [MSR_SKILLS_EVALUATE]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_FORBIDDEN_LORE] = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_INVOCATION]     = MSR_CHAR_WILLPOWER,
    [MSR_SKILLS_LOGIC]          = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_MEDICAE]        = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_PSYSCIENCE]     = MSR_CHAR_WILLPOWER,
    [MSR_SKILLS_SCHOLASTIC_LORE]= MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_SEARCH]         = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_SECURITY]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_SILENT_MOVE]    = MSR_CHAR_AGILITY,
    [MSR_SKILLS_SURVIVAL]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_TECH_USE]       = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_TRACKING]       = MSR_CHAR_INTELLIGENCE,
};

static const char *msr_skillrate_name[] = {
    [MSR_SKILL_RATE_NONE]       = "untrained", 
    [MSR_SKILL_RATE_BASIC]      = "basic", 
    [MSR_SKILL_RATE_ADVANCED]   = "advanced", 
    [MSR_SKILL_RATE_EXPERT]     = "expert",
};

static const char *msr_skill_name[] = {
    [MSR_SKILLS_AWARENESS]      = "Awareness",
    [MSR_SKILLS_BARTER]         = "Barter",
    [MSR_SKILLS_CHEM_USE]       = "Chem Use",
    [MSR_SKILLS_COMMON_LORE]    = "Common Lore",
    [MSR_SKILLS_CONCEALMENT]    = "Concealment",
    [MSR_SKILLS_DEMOLITION]     = "Demolition",
    [MSR_SKILLS_DISGUISE]       = "Disguise",
    [MSR_SKILLS_DODGE]          = "Dodge",
    [MSR_SKILLS_EVALUATE]       = "Evaluate",
    [MSR_SKILLS_FORBIDDEN_LORE] = "Forbidden Lore",
    [MSR_SKILLS_INVOCATION]     = "Invocation",
    [MSR_SKILLS_LOGIC]          = "Logic",
    [MSR_SKILLS_MEDICAE]        = "Medicae",
    [MSR_SKILLS_PSYSCIENCE]     = "Psyscience",
    [MSR_SKILLS_SCHOLASTIC_LORE]= "Scholastic Lore",
    [MSR_SKILLS_SEARCH]         = "Search",
    [MSR_SKILLS_SECURITY]       = "Security",
    [MSR_SKILLS_SILENT_MOVE]    = "Silent Move",
    [MSR_SKILLS_SURVIVAL]       = "Survival",
    [MSR_SKILLS_TECH_USE]       = "Tech Use",
    [MSR_SKILLS_TRACKING]       = "Tracking",
};

static const char *msr_talent_name[] = {
    [TLT_NONE]                           = "none",
    [TLT_BASIC_WPN_TRNG_BOLT]            = "basic weapon training bolt",
    [TLT_BASIC_WPN_TRNG_FLAME]           = "basic weapon training flame",
    [TLT_BASIC_WPN_TRNG_LAS]             = "basic weapon training las",
    [TLT_BASIC_WPN_TRNG_LAUNCHER]        = "basic weapon training launcher",
    [TLT_BASIC_WPN_TRNG_MELTA]           = "basic weapon training melta",
    [TLT_BASIC_WPN_TRNG_PLASMA]          = "basic weapon training plasma",
    [TLT_BASIC_WPN_TRNG_PRIMITIVE]       = "basic weapon training primitive",
    [TLT_BASIC_WPN_TRNG_SP]              = "basic weapon training solid projectile",
    [TLT_HEAVY_WPN_TRNG_BOLT]            = "heavy weapon training bolt",
    [TLT_HEAVY_WPN_TRNG_FLAME]           = "heavy weapon training flame",
    [TLT_HEAVY_WPN_TRNG_LAS]             = "heavy weapon training las",
    [TLT_HEAVY_WPN_TRNG_LAUNCHER]        = "heavy weapon training launcher",
    [TLT_HEAVY_WPN_TRNG_MELTA]           = "heavy weapon training melta",
    [TLT_HEAVY_WPN_TRNG_PLASMA]          = "heavy weapon training plasma",
    [TLT_HEAVY_WPN_TRNG_PRIMITIVE]       = "heavy weapon training primitive",
    [TLT_HEAVY_WPN_TRNG_SP]              = "heavy weapon training solid projectile",
    [TLT_PISTOL_WPN_TRNG_BOLT]           = "pistol weapon training bolt",
    [TLT_PISTOL_WPN_TRNG_FLAME]          = "pistol weapon training flame",
    [TLT_PISTOL_WPN_TRNG_LAS]            = "pistol weapon training las",
    [TLT_PISTOL_WPN_TRNG_LAUNCHER]       = "pistol weapon training launcher",
    [TLT_PISTOL_WPN_TRNG_MELTA]          = "pistol weapon training melta",
    [TLT_PISTOL_WPN_TRNG_PLASMA]         = "pistol weapon trainig plasma",
    [TLT_PISTOL_WPN_TRNG_PRIMITIVE]      = "pistol weapon training primitive",
    [TLT_PISTOL_WPN_TRNG_SP]             = "pistol weapon training solid projectile",
    [TLT_THROWN_WPN_TRNG_PRIMITIVE]      = "thrown weapon training primitive",
    [TLT_THROWN_WPN_TRNG_CHAIN]          = "thrown weapon training chain",
    [TLT_THROWN_WPN_TRNG_SHOCK]          = "thrown weapon training shock",
    [TLT_THROWN_WPN_TRNG_POWER]          = "thrown weapon training power",
    [TLT_MELEE_WPN_TRNG_PRIMITIVE]       = "melee weapon training primitive",
    [TLT_MELEE_WPN_TRNG_CHAIN]           = "melee weapon training chain",
    [TLT_MELEE_WPN_TRNG_SHOCK]           = "melee weapon training shock",
    [TLT_MELEE_WPN_TRNG_POWER]           = "melee weapon training power",
    [TLT_RESISTANCE_COLD]                = "cold resistance",
    [TLT_RESISTANCE_HEAT]                = "heat resistance",
    [TLT_RESISTANCE_FEAR]                = "fear resistance",
    [TLT_RESISTANCE_POISON]              = "poison resistance",
    [TLT_RESISTANCE_WARP]                = "warp resistance",
    [TLT_HATRED_FACTION_CRIMINALS]       = "hatred faction criminals",
    [TLT_HATRED_FACTION_DAEMONS]         = "hatred faction daemons",
    [TLT_HATRED_FACTION_ORKS]            = "hatred faction orks",
    [TLT_HATRED_FACTION_TYRANIDS]        = "hatred faction tyranids",
    [TLT_HATRED_FACTION_PSYKERS]         = "hatred faction psykers",
    [TLT_HATRED_FACTION_HERETICS]        = "hatred faction heretics",
    [TLT_HATRED_FACTION_MUTANTS]         = "hatred faction mutants",
    [TLT_EXOTIC_WPN_TRNG_NEEDLE_PISTOL]  = "exotic weapon training needle pistol",
    [TLT_EXOTIC_WPN_TRNG_WEB_PISTOL]     = "exotic weapon training web pistol",
    [TLT_EXOTIC_WPN_TRNG_NEEDLE_RIFLE]   = "exotic weapon training needle rifle",
    [TLT_EXOTIC_WPN_TRNG_WEBBER]         = "exotic weapon training webber",
    [TLT_PSY_RATING_1]                   = "psy rating 1",
    [TLT_PSY_RATING_2]                   = "psy rating 2",
    [TLT_PSY_RATING_3]                   = "psy rating 3",
    [TLT_PSY_RATING_4]                   = "psy rating 4",
    [TLT_PSY_RATING_5]                   = "psy rating 5",
    [TLT_PSY_RATING_6]                   = "psy rating 6",

    [TLT_TALENT_AWARENESS]		        = "awareness talent",
    [TLT_TALENT_BARTER]		            = "barter talent",
    [TLT_TALENT_CHEM_USE]		        = "chem use talent",
    [TLT_TALENT_COMMON_LORE]		    = "common lore talent",
    [TLT_TALENT_CONCEALMENT]		    = "concealment talent",
    [TLT_TALENT_DEMOLITION]		        = "demolition talent",
    [TLT_TALENT_DISGUISE]		        = "disguise talent",
    [TLT_TALENT_DODGE]		            = "dodge talent",
    [TLT_TALENT_EVALUATE]		        = "evaluate talent",
    [TLT_TALENT_FORBIDDEN_LORE]		    = "forbidden lore talent",
    [TLT_TALENT_INVOCATION]		        = "invocation talent",
    [TLT_TALENT_LOGIC]		            = "logic talent",
    [TLT_TALENT_MEDICAE]		        = "medicae talent",
    [TLT_TALENT_PSYSCIENCE]		        = "psycience talent",
    [TLT_TALENT_SCHOLASTIC_LORE]		= "scholastic lore talent",
    [TLT_TALENT_SEARCH]		            = "search talent",
    [TLT_TALENT_SECURITY]		        = "security talent",
    [TLT_TALENT_SILENT_MOVE]		    = "silent move talent",
    [TLT_TALENT_SURVIVAL]		        = "survival talent",
    [TLT_TALENT_TECH_USE]		        = "tech use talent",
    [TLT_AMBIDEXTROUS]		            = "ambidextrous",
    [TLT_ARMOUR_OF_CONTEMPT]		    = "armour of contempt",
    [TLT_ARMS_MASTER]		            = "arms master",
    [TLT_ASSASSIN_STRIKE]		        = "assasin strike",
    [TLT_AUTOSANGUINE]		            = "autosanguine",
    [TLT_BATTLE_RAGE]		            = "battle rage",
    [TLT_BLADE_MASTER]		            = "blade master",
    [TLT_BLIND_FIGHTING]		        = "blind fighting",
    [TLT_BULGING_BICEPS]		        = "bulging biceps",
    [TLT_CLEANSE_AND_PURIFY]		    = "cleanse and purify",
    [TLT_COMBAT_MASTER]		            = "combat master",
    [TLT_CORPUS_CONVERSION]		        = "corpus conversion",
    [TLT_COUNTER_ATTACK]		        = "counter attack",
    [TLT_CRACK_SHOT]		            = "crack shot",
    [TLT_DARK_SOUL]		                = "dark soul",
    [TLT_DEADEYE_SHOT]		            = "deadeye shot",
    [TLT_DEFLECT_SHOT]		            = "deflect shot",
    [TLT_DIE_HARD]		                = "die hard",
    [TLT_DISARM]		                = "disarm",
    [TLT_DISCIPLINE_FOCUS]		        = "discipline focus",
    [TLT_DOUBLE_TEAM]		            = "double team",
    [TLT_DUAL_SHOT]		                = "dual shot",
    [TLT_DUAL_STRIKE]		            = "dual strike",
    [TLT_ELECTRICAL_SOCCOUR]		    = "",
    [TLT_ELECTRO_GRAFT_USE]		        = "",
    [TLT_FAVOURED_BY_THE_WARP]		    = "",
    [TLT_FEARLESS]		                = "",
    [TLT_FEEDBACK_SCREECH]		        = "",
    [TLT_FLAGELLANT]		            = "",
    [TLT_FORESIGHT]		                = "",
    [TLT_FRENZY]		                = "",
    [TLT_FURIOUS_ASSAULT]		        = "",
    [TLT_GUN_BLESSING]		            = "",
    [TLT_GUNSLINGER]		            = "",
    [TLT_HARDY]		                    = "",
    [TLT_HEIGHTENED_SENSES]		        = "",
    [TLT_INSANELY_FAITHFULL]		    = "",
    [TLT_IRON_JAW]		                = "",
    [TLT_JADED]		                    = "",
    [TLT_LIGHTNING_ATTACK]		        = "",
    [TLT_LOGIS_IMPLANT]		            = "",
    [TLT_LUMINEN_CHARGE]		        = "",
    [TLT_MAGLEV_CHARGE]		            = "",
    [TLT_MARKSMEN]		                = "",
    [TLT_MASTER_CHIRUGEON]		        = "",
    [TLT_MECHANDENDRIUSE]		        = "",
    [TLT_MEDITATION]		            = "",
    [TLT_MENTAL_FORTRESS]		        = "",
    [TLT_MENTAL_RAGE]		            = "",
    [TLT_MIGHSHOT]		                = "",
    [TLT_MINOR_PSYCHIC_POWER]		    = "",
    [TLT_NERVES_OF_STEEL]	        	= "",
    [TLT_POWER_WELL]		            = "",
    [TLT_PROSANGUINE]		            = "",
    [TLT_QUICK_DRAW]		            = "",
    [TLT_RAPID_RELOAD]		            = "",
    [TLT_SOUND_CONSTITUTION]		    = "",
    [TLT_SPRINT]		                = "",
    [TLT_STEP_ASIDE]		            = "",
    [TLT_STREET_FIGHTING]		        = "",
    [TLT_STRONG_MINDED]		            = "",
    [TLT_SWIFT_ATTACK]		            = "",
    [TLT_TAKE_DOWN]		                = "",
    [TLT_TECHNICAL_KNOCK]		        = "",
    [TLT_TRUE_GRIT]		                = "",
    [TLT_TWO_WEAPON_WIELDER_MELEE]		= "",
    [TLT_TWO_WEAPON_WIELDER_BALLISTIC]  = "",
    [TLT_UNSHAKEBLE_FAITH]		        = "",
    [TLT_WALL_OF_STEEL]		            = "",
};

