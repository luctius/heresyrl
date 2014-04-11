#include "monster_static.h"

#include "monster_descriptions.h"

#define MONSTER(tid,_icon,_sd_name,_ld_name,_gender,maxwounds) \
    [tid]={.uid=0, .template_id=tid, .icon=_icon, .sd_name=_sd_name, .ld_name=_ld_name, \
        .gender=_gender, .cur_wounds=maxwounds, .max_wounds=maxwounds, .fate_points=0,
#define MONSTER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,pr,wl,fl) \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=pr,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_FELLOWSHIP]={ .base_value=fl,} 

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_BASIC]=basic_skills, \
    .skills[MSR_SKILL_RATE_ADVANCED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERT]=expert_skills

#define HUMAN(tal0,tal1,tal2) .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
        .talents[0] = bf(WPNTLT_CREATURE_WPN_TALENT) | tal0, .creature_traits = 0

#define BEAST(tal0,tal1,tal2) .race=MSR_RACE_BEAST, .size=MSR_SIZE_AVERAGE, \
        .talents[0] = bf(WPNTLT_CREATURE_WPN_TALENT) | tal0, .creature_traits = bf(CTRTRTS_BESTIAL) | bf(CTRTRTS_QUADRUPED)

const char *msr_descs[] = {
    [MID_DUMMY]       = "unknown",
    [MID_BASIC_FERAL] = "you",
    [MID_BASIC_HIVE]  = "you",
    [MID_HIVE_GANGER] = "description of a hive ganger",
    [MID_VICIOUS_DOG] = "description of a vicious dog",
};

struct msr_monster static_monster_list[] = {
    MONSTER(MID_DUMMY,'h',"dummy","a dummy",MSR_GENDER_MALE,1) HUMAN(0,0,0), CHARACTERISTICS(0,0,0,0,0,0,0,0,0), SKILLS(0,0,0), MONSTER_END,

    MONSTER(MID_BASIC_FERAL,'h',"human","a feral human",MSR_GENDER_MALE,14)
        HUMAN(WPNTLT_BASIC_WPN_TRNG_LAS | WPNTLT_PISTOL_WPN_TRNG_LAS,T1_TALENT_DODGE,0),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), MONSTER_END,

    MONSTER(MID_BASIC_HIVE,'h',"human","a hive human",MSR_GENDER_MALE,14)
        HUMAN(WPNTLT_BASIC_WPN_TRNG_LAS | WPNTLT_PISTOL_WPN_TRNG_LAS,T1_TALENT_DODGE,0),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), MONSTER_END,

    MONSTER(MID_HIVE_GANGER,'h',"human","a hive human",MSR_GENDER_MALE,14)
        HUMAN(WPNTLT_BASIC_WPN_TRNG_LAS | WPNTLT_PISTOL_WPN_TRNG_LAS,T1_TALENT_DODGE,0),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), MONSTER_END,

    MONSTER(MID_VICIOUS_DOG,'d',"dog","a vicious dog",MSR_GENDER_MALE,12)
        BEAST(0,0,T2_SPRINT),
        CHARACTERISTICS(30,0,30,30,30,15,38,40,30),
        SKILLS(MSR_SKILLS_AWARENESS|MSR_SKILLS_SILENT_MOVE|MSR_SKILLS_TRACKING, MSR_SKILLS_AWARENESS|MSR_SKILLS_SILENT_MOVE|MSR_SKILLS_TRACKING ,0), MONSTER_END,
};

static enum msr_characteristic msr_skill_charac[] = {
    [MSR_SKILLS_AWARENESS]      = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_BARTER]         = MSR_CHAR_INTELLIGENCE,
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
    [WPNTLT_BASIC_WPN_TRNG_BOLT]            = "basic weapon training bolt",
    [WPNTLT_BASIC_WPN_TRNG_FLAME]           = "basic weapon training flame",
    [WPNTLT_BASIC_WPN_TRNG_LAS]             = "basic weapon training las",
    [WPNTLT_BASIC_WPN_TRNG_LAUNCHER]        = "basic weapon training launcher",
    [WPNTLT_BASIC_WPN_TRNG_MELTA]           = "basic weapon training melta",
    [WPNTLT_BASIC_WPN_TRNG_PLASMA]          = "basic weapon training plasma",
    [WPNTLT_BASIC_WPN_TRNG_PRIMITIVE]       = "basic weapon training primitive",
    [WPNTLT_BASIC_WPN_TRNG_SP]              = "basic weapon training solid projectile",
    [WPNTLT_HEAVY_WPN_TRNG_BOLT]            = "heavy weapon training bolt",
    [WPNTLT_HEAVY_WPN_TRNG_FLAME]           = "heavy weapon training flame",
    [WPNTLT_HEAVY_WPN_TRNG_LAS]             = "heavy weapon training las",
    [WPNTLT_HEAVY_WPN_TRNG_LAUNCHER]        = "heavy weapon training launcher",
    [WPNTLT_HEAVY_WPN_TRNG_MELTA]           = "heavy weapon training melta",
    [WPNTLT_HEAVY_WPN_TRNG_PLASMA]          = "heavy weapon training plasma",
    [WPNTLT_HEAVY_WPN_TRNG_PRIMITIVE]       = "heavy weapon training primitive",
    [WPNTLT_HEAVY_WPN_TRNG_SP]              = "heavy weapon training solid projectile",
    [WPNTLT_PISTOL_WPN_TRNG_BOLT]           = "pistol weapon training bolt",
    [WPNTLT_PISTOL_WPN_TRNG_FLAME]          = "pistol weapon training flame",
    [WPNTLT_PISTOL_WPN_TRNG_LAS]            = "pistol weapon training las",
    [WPNTLT_PISTOL_WPN_TRNG_LAUNCHER]       = "pistol weapon training launcher",
    [WPNTLT_PISTOL_WPN_TRNG_MELTA]          = "pistol weapon training melta",
    [WPNTLT_PISTOL_WPN_TRNG_PLASMA]         = "pistol weapon trainig plasma",
    [WPNTLT_PISTOL_WPN_TRNG_PRIMITIVE]      = "pistol weapon training primitive",
    [WPNTLT_PISTOL_WPN_TRNG_SP]             = "pistol weapon training solid projectile",
    [WPNTLT_THROWN_WPN_TRNG_PRIMITIVE]      = "thrown weapon training primitive",
    [WPNTLT_THROWN_WPN_TRNG_CHAIN]          = "thrown weapon training chain",
    [WPNTLT_THROWN_WPN_TRNG_SHOCK]          = "thrown weapon training shock",
    [WPNTLT_THROWN_WPN_TRNG_POWER]          = "thrown weapon training power",
    [WPNTLT_CREATURE_WPN_TALENT]            = "creature weapon training",
    [WPNTLT_MELEE_WPN_TRNG_PRIMITIVE]       = "melee weapon training primitive",
    [WPNTLT_MELEE_WPN_TRNG_CHAIN]           = "melee weapon training chain",
    [WPNTLT_MELEE_WPN_TRNG_SHOCK]           = "melee weapon training shock",
    [WPNTLT_MELEE_WPN_TRNG_POWER]           = "melee weapon training power",
    [WPNTLT_RESISTANCE_COLD]                = "cold resistance",
    [WPNTLT_RESISTANCE_HEAT]                = "heat resistance",
    [WPNTLT_RESISTANCE_FEAR]                = "fear resistance",
    [WPNTLT_RESISTANCE_POISON]              = "poison resistance",
    [WPNTLT_RESISTANCE_WARP]                = "warp resistance",
    [WPNTLT_HATRED_FACTION_CRIMINALS]       = "hatred faction criminals",
    [WPNTLT_HATRED_FACTION_DAEMONS]         = "hatred faction daemons",
    [WPNTLT_HATRED_FACTION_ORKS]            = "hatred faction orks",
    [WPNTLT_HATRED_FACTION_TYRANIDS]        = "hatred faction tyranids",
    [WPNTLT_HATRED_FACTION_PSYKERS]         = "hatred faction psykers",
    [WPNTLT_HATRED_FACTION_HERETICS]        = "hatred faction heretics",
    [WPNTLT_HATRED_FACTION_MUTANTS]         = "hatred faction mutants",
    [WPNTLT_EXOTIC_WPN_TRNG_NEEDLE_PISTOL]  = "exotic weapon training needle pistol",
    [WPNTLT_EXOTIC_WPN_TRNG_WEB_PISTOL]     = "exotic weapon training web pistol",
    [WPNTLT_EXOTIC_WPN_TRNG_NEEDLE_RIFLE]   = "exotic weapon training needle rifle",
    [WPNTLT_EXOTIC_WPN_TRNG_WEBBER]         = "exotic weapon training webber",
    [WPNTLT_PSY_RATING_1]                   = "psy rating 1",
    [WPNTLT_PSY_RATING_2]                   = "psy rating 2",
    [WPNTLT_PSY_RATING_3]                   = "psy rating 3",
    [WPNTLT_PSY_RATING_4]                   = "psy rating 4",
    [WPNTLT_PSY_RATING_5]                   = "psy rating 5",
    [WPNTLT_PSY_RATING_6]                   = "psy rating 6",
};

