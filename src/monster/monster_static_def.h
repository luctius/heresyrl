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

#define MONSTER(_icon,_sd_name,_ld_name,_gender,maxwounds) \
        {.uid=0, .icon=_icon, .sd_name=cs_MONSTER _sd_name cs_CLOSE, \
        .ld_name=cs_MONSTER _ld_name cs_CLOSE, \
        .skills[MSR_SKILL_RATE_UNKNOWN]=~(0), \
        .gender=_gender, .wounds = { .curr=maxwounds, .max=maxwounds, },
#define MONSTER_END }

#define CREATION(wght, lvl, l, grp_ch) \
    .spwn.weight=wght, .spwn.level=lvl, .spwn.dungeon_locale=l, .spwn.group_chance = grp_ch

#define DESCRIPTION(desc) .description=desc

#define CHARACTERISTICS(chr, value) \
    .characteristic[chr]={ .base_value=value,} \

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_KNOWN]=basic_skills, \
    .skills[MSR_SKILL_RATE_TRAINED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERIENCED]=expert_skills

#define TALENTS(t1, t2, t3, tmisc) .talent[MSR_TALENT_TIER_T1]=(t1), .talent[MSR_TALENT_TIER_T2]=(t2),\
                                .talent[MSR_TALENT_TIER_T3]=(t3), .talent[MSR_TALENT_TIER_MISC]=(tmisc)

#define DEF_ITEM(idx, item_id) .def_items[idx]=item_id

#define HUMAN() .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
    .icon_attr = TERM_COLOUR_YELLOW, \
    .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED


struct msr_monster static_monster_list[] = {

    /*----------------- CHARACTER TEMPLATES  --------------------*/
    /*
       Remember to modify the starting wounds table and the fatepoint table
       in player.c when adding new player selectable races.
     */
    [MID_DUMMY]=MONSTER('h',"dummy","a dummy",MSR_GENDER_MALE,1)
        HUMAN(),
    MONSTER_END,

    [MID_PLAYER]=MONSTER('@',"Player","player",MSR_GENDER_MALE,7)
        HUMAN(),
        CHARACTERISTICS(MSR_CHAR_COMBAT,        25),
        CHARACTERISTICS(MSR_CHAR_STRENGTH,      25),
        CHARACTERISTICS(MSR_CHAR_TOUGHNESS,     25),
        CHARACTERISTICS(MSR_CHAR_AGILITY,       25),
        CHARACTERISTICS(MSR_CHAR_INTELLIGENCE,  25),
        CHARACTERISTICS(MSR_CHAR_WILLPOWER,     25),
        CHARACTERISTICS(MSR_CHAR_PERCEPTION,    25),
        SKILLS(0,0,0), .fate_points=0,  .is_player=true,
        DESCRIPTION("description of player"),
    MONSTER_END,
    /*----------------------------------------------------------*/

    MONSTER('s',"scavvie","a scavenger",MSR_GENDER_MALE,1)
        HUMAN(),
        CHARACTERISTICS(MSR_CHAR_COMBAT,        25),
        CHARACTERISTICS(MSR_CHAR_STRENGTH,      25),
        CHARACTERISTICS(MSR_CHAR_TOUGHNESS,     25),
        CHARACTERISTICS(MSR_CHAR_AGILITY,       30),
        CHARACTERISTICS(MSR_CHAR_INTELLIGENCE,  25),
        CHARACTERISTICS(MSR_CHAR_WILLPOWER,     20),
        CHARACTERISTICS(MSR_CHAR_PERCEPTION,    30),
        DEF_ITEM(0, ITEM_GROUP_1H_MELEE),
        SKILLS(bf(MSR_SKILLS_AWARENESS) | bf(MSR_SKILLS_RANGED) | bf(MSR_SKILLS_MELEE) ,0,0),
        DESCRIPTION("description of an scavenger"),
        CREATION(30,1,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN), 70),
    MONSTER_END,
};

static const char *msr_char_name[] = {
    [MSR_CHAR_COMBAT]         = "Combat Skill",
    [MSR_CHAR_STRENGTH]       = "Strength",
    [MSR_CHAR_TOUGHNESS]      = "Toughness",
    [MSR_CHAR_AGILITY]        = "Agility",
    [MSR_CHAR_INTELLIGENCE]   = "Intelligence",
    [MSR_CHAR_WILLPOWER]      = "Willpower",
    [MSR_CHAR_PERCEPTION]     = "Perception",
};

static const char *msr_char_description[] = {
    [MSR_CHAR_COMBAT]         = "Combat Skill",
    [MSR_CHAR_STRENGTH]       = "Strength",
    [MSR_CHAR_TOUGHNESS]      = "Toughness",
    [MSR_CHAR_AGILITY]        = "Agility",
    [MSR_CHAR_INTELLIGENCE]   = "Intelligence",
    [MSR_CHAR_WILLPOWER]      = "Willpower",
    [MSR_CHAR_PERCEPTION]     = "Perception",
};

static const char *msr_skillrate_name[] = {
    [MSR_SKILL_RATE_UNKNOWN]        = "unkown",
    [MSR_SKILL_RATE_KNOWN]          = "known",
    [MSR_SKILL_RATE_TRAINED]        = "trained",
    [MSR_SKILL_RATE_EXPERIENCED]    = "experienced",
    [MSR_SKILL_RATE_VETERAN]        = "veteran",
};

static enum msr_characteristic msr_skill_charac[] = {
    [MSR_SKILLS_AWARENESS]          = MSR_CHAR_PERCEPTION,
    [MSR_SKILLS_DODGE]              = MSR_CHAR_AGILITY,
    [MSR_SKILLS_MEDICAE]            = MSR_CHAR_INTELLIGENCE,
    [MSR_SKILLS_MELEE]              = MSR_CHAR_COMBAT,
    [MSR_SKILLS_PARRY]              = MSR_CHAR_COMBAT,
    [MSR_SKILLS_RANGED]             = MSR_CHAR_COMBAT,
    [MSR_SKILLS_STEALTH]            = MSR_CHAR_AGILITY,
    [MSR_SKILLS_MAX]                = MSR_CHAR_MAX,
};

static const char *msr_skill_name[] = {
    [MSR_SKILLS_AWARENESS]          = "Awareness",
    [MSR_SKILLS_DODGE]              = "Dodge",
    [MSR_SKILLS_MEDICAE]            = "Medicae",
    [MSR_SKILLS_MELEE]              = "Melee",
    [MSR_SKILLS_PARRY]              = "Parry",
    [MSR_SKILLS_RANGED]             = "Ranged",
    [MSR_SKILLS_STEALTH]            = "Stealth",
    [MSR_SKILLS_MAX]                = "Max,"
};

static const char *msr_skill_description[] = {
    [MSR_SKILLS_AWARENESS]          = "Awareness",
    [MSR_SKILLS_DODGE]              = "Dodge",
    [MSR_SKILLS_MEDICAE]            = "Medicae",
    [MSR_SKILLS_MELEE]              = "Melee",
    [MSR_SKILLS_PARRY]              = "Parry",
    [MSR_SKILLS_RANGED]             = "Ranged",
    [MSR_SKILLS_STEALTH]            = "Stealth",
    [MSR_SKILLS_MAX]                = "Max,"
};

struct talent_descriptions_struct {
    enum msr_talents talent;
    char *name;
    char *description;
};

struct talent_descriptions_struct talent_descriptions[] = {
    { .talent = TLT_NONE,                       .name = "None",                       .description = "None",           } ,

    { .talent = TLT_1_AMBIDEXTRIOUS,            .name = "Ambidextrious",              .description = "Ambidextrious",  } ,
    { .talent = TLT_1_BLIND_FIGHTING,           .name = "Blind Fighting",             .description = "",               } ,
    { .talent = TLT_1_DIE_HARD,                 .name = "Die Hard",                   .description = "",               } ,
    { .talent = TLT_1_DISARM,                   .name = "Disarm",                     .description = "",               } ,
    { .talent = TLT_1_FRENZY,                   .name = "Frenzy",                     .description = "",               } ,
    { .talent = TLT_1_IRON_JAW,                 .name = "Iron Jaw",                   .description = "",               } ,
    { .talent = TLT_1_JADED,                    .name = "Jaded",                      .description = "",               } ,
    { .talent = TLT_1_LEAP_UP,                  .name = "Leap Up",                    .description = "",               } ,
    { .talent = TLT_1_QUICK_DRAW,               .name = "Quick Draw",                 .description = "",               } ,
    { .talent = TLT_1_RAPID_RELOAD,             .name = "Rapid Reload",               .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_COLD,          .name = "Resistance (Cold)",          .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_HEAT,          .name = "Resistance (Heat)",          .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_FEAR,          .name = "Resistance (Fear)",          .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_POISON,        .name = "Resistance (Poison)",        .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_PSYCHIC,       .name = "Resistance (Psychic)",       .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_RADIATION,     .name = "Resistance (Radiation)",     .description = "",               } ,
    { .talent = TLT_1_RESISTANCE_VACUUM,        .name = "Resistance (Vacuum)",        .description = "",               } ,
    { .talent = TLT_1_TAKEDOWN,                 .name = "Takedown",                   .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_BOLT,     .name = "Weapon Training (Bolt)",     .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_CHAIN,    .name = "Weapon Training (Chain)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_FLAME,    .name = "Weapon Training (Flame)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_HEAVY,    .name = "Weapon Training (Heavy)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LOW_TECH, .name = "Weapon Training (Low-Tech)", .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LAUNCHER, .name = "Weapon Training (Launcher)", .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LAS,      .name = "Weapon Training (Las)",      .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_MELTA,    .name = "Weapon Training (Melta)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_PLASMA,   .name = "Weapon Training (Plasma)",   .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_POWER,    .name = "Weapon Training (Power)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_SHOCK,    .name = "Weapon Training (Shock)",    .description = "",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_SP,       .name = "Weapon Training (SP)",       .description = "",               } ,

    /*{ .talent = TLT_2_ARMOUR_MONGER,            .name = "Armour Monger",              .description = "",               } ,*/
    { .talent = TLT_2_TWO_WEAPON_FIGHTING,      .name = "Two-weapon Fighting",        .description = "",               } ,

    /*{ .talent = TLT_3_ASSASSINS_STRIKE,         .name = "Assassins Strike",           .description = "",               } ,*/

    { .talent = TLT_MAX,                        .name = "Max",                        .description = "Max",            } 
};

static const int human_hitloc_lotable[] = {
    [MSR_HITLOC_HEAD]       = 0,
    [MSR_HITLOC_RIGHT_ARM]  = 10,
    [MSR_HITLOC_LEFT_ARM]   = 20,
    [MSR_HITLOC_BODY]       = 30,
    [MSR_HITLOC_RIGHT_LEG]  = 70,
    [MSR_HITLOC_LEFT_LEG]   = 85,
};

static const int beast_hitloc_lotable[] = {
    [MSR_HITLOC_HEAD]       = 0,
    [MSR_HITLOC_BODY]       = 10,
    [MSR_HITLOC_RIGHT_LEG]  = 50,
    [MSR_HITLOC_LEFT_LEG]   = 75,
};

static const char *hitloc_names[] = {
    [MSR_HITLOC_HEAD]       = "head",
    [MSR_HITLOC_RIGHT_ARM]  = "right arm",
    [MSR_HITLOC_LEFT_ARM]   = "left arm",
    [MSR_HITLOC_BODY]       = "thorax",
    [MSR_HITLOC_RIGHT_LEG]  = "right leg",
    [MSR_HITLOC_LEFT_LEG]   = "left leg",
};

static const char *gender_names[][2] = {
    [MSR_GENDER_MALE]   = {cs_MONSTER "his" cs_CLOSE, cs_MONSTER "he"  cs_CLOSE},
    [MSR_GENDER_FEMALE] = {cs_MONSTER "her" cs_CLOSE, cs_MONSTER "she" cs_CLOSE},
    [MSR_GENDER_IT]     = {cs_MONSTER "its" cs_CLOSE, cs_MONSTER "it"  cs_CLOSE},
};

