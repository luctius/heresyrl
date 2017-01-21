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

#define CREATION(wght, lvl, cst, l, grp_ch) \
    .spwn = { .weight=wght, .level=lvl, .cost=cst, .dungeon_locale=l, .group_chance = grp_ch, }

#define FACTION(f) \
    .faction = f

#define DESCRIPTION(desc) .description=desc

#define CHARACTERISTICS(chr, value) \
    .characteristic[chr]={ .base_value=value,} \

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_KNOWN]=basic_skills, \
    .skills[MSR_SKILL_RATE_TRAINED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERIENCED]=expert_skills

#define TALENTS(t1, t2, t3, tmisc) .talent[MSR_TALENT_TIER_T1]=(t1), .talent[MSR_TALENT_TIER_T2]=(t2),\
                                .talent[MSR_TALENT_TIER_T3]=(t3), .talent[MSR_TALENT_TIER_MISC]=(tmisc)

#define DEF_ITEM(idx, item_id) .def_items[idx]=item_id

#define HUMAN() .size=MSR_SIZE_AVERAGE, .icon_attr = TERM_COLOUR_YELLOW, \
    .creature_traits = 0, .crtr_wpn=IID_HUMAN_UNARMED


struct msr_monster static_monster_list[] = {

    /*----------------- CHARACTER TEMPLATES  --------------------*/
    /*
       Remember to modify the starting wounds table and the fatepoint table
       in player.c when adding new player selectable races.
     */
    [MID_DUMMY]=MONSTER(L'h',L"dummy",L"a dummy",MSR_GENDER_MALE,1)
        HUMAN(),
    MONSTER_END,

    [MID_PLAYER]=MONSTER(L'@',L"Player",L"player",MSR_GENDER_MALE,7)
        HUMAN(),
        CHARACTERISTICS(MSR_CHAR_COMBAT,        25),
        CHARACTERISTICS(MSR_CHAR_STRENGTH,      25),
        CHARACTERISTICS(MSR_CHAR_TOUGHNESS,     25),
        CHARACTERISTICS(MSR_CHAR_AGILITY,       25),
        CHARACTERISTICS(MSR_CHAR_INTELLIGENCE,  25),
        CHARACTERISTICS(MSR_CHAR_WILLPOWER,     25),
        CHARACTERISTICS(MSR_CHAR_PERCEPTION,    25),
        SKILLS(0,0,0), .fate_points=0,  .is_player=true,
        DESCRIPTION(L"description of player"),
        FACTION(FCT_PLAYER),
    MONSTER_END,
    /*----------------------------------------------------------*/

    MONSTER(L's',L"scavvie",L"a scavenger",MSR_GENDER_MALE,1)
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
        DESCRIPTION(L"description of an scavenger"),
        CREATION(30,1,50,bf(DUNGEON_TYPE_CAVE) | bf(DUNGEON_TYPE_PLAIN) | bf(DUNGEON_TYPE_HIVE), 70),
        FACTION(FCT_SCAVENGERS),
    MONSTER_END,
};

static const wchar_t *msr_char_name[] = {
    [MSR_CHAR_COMBAT]         = L"Combat Skill",
    [MSR_CHAR_STRENGTH]       = L"Strength",
    [MSR_CHAR_TOUGHNESS]      = L"Toughness",
    [MSR_CHAR_AGILITY]        = L"Agility",
    [MSR_CHAR_INTELLIGENCE]   = L"Intelligence",
    [MSR_CHAR_WILLPOWER]      = L"Willpower",
    [MSR_CHAR_PERCEPTION]     = L"Perception",
};

static const wchar_t *msr_char_description[] = {
    [MSR_CHAR_COMBAT]         = L"Combat Skill",
    [MSR_CHAR_STRENGTH]       = L"Strength",
    [MSR_CHAR_TOUGHNESS]      = L"Toughness",
    [MSR_CHAR_AGILITY]        = L"Agility",
    [MSR_CHAR_INTELLIGENCE]   = L"Intelligence",
    [MSR_CHAR_WILLPOWER]      = L"Willpower",
    [MSR_CHAR_PERCEPTION]     = L"Perception",
};

static const wchar_t *msr_skillrate_name[] = {
    [MSR_SKILL_RATE_UNKNOWN]        = L"unkown",
    [MSR_SKILL_RATE_KNOWN]          = L"known",
    [MSR_SKILL_RATE_TRAINED]        = L"trained",
    [MSR_SKILL_RATE_EXPERIENCED]    = L"experienced",
    [MSR_SKILL_RATE_VETERAN]        = L"veteran",
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

static const wchar_t *msr_skill_name[] = {
    [MSR_SKILLS_AWARENESS]          = L"Awareness",
    [MSR_SKILLS_DODGE]              = L"Dodge",
    [MSR_SKILLS_MEDICAE]            = L"Medicae",
    [MSR_SKILLS_MELEE]              = L"Melee",
    [MSR_SKILLS_PARRY]              = L"Parry",
    [MSR_SKILLS_RANGED]             = L"Ranged",
    [MSR_SKILLS_STEALTH]            = L"Stealth",
    [MSR_SKILLS_MAX]                = L"Max,"
};

static const wchar_t *msr_skill_description[] = {
    [MSR_SKILLS_AWARENESS]          = L"Awareness",
    [MSR_SKILLS_DODGE]              = L"Dodge",
    [MSR_SKILLS_MEDICAE]            = L"Medicae",
    [MSR_SKILLS_MELEE]              = L"Melee",
    [MSR_SKILLS_PARRY]              = L"Parry",
    [MSR_SKILLS_RANGED]             = L"Ranged",
    [MSR_SKILLS_STEALTH]            = L"Stealth",
    [MSR_SKILLS_MAX]                = L"Max,"
};

struct talent_descriptions_struct {
    enum msr_talents talent;
    const wchar_t *name;
    const wchar_t *description;
};

struct talent_descriptions_struct talent_descriptions[] = {
    { .talent = TLT_NONE,                       .name = L"None",                       .description = L"None",           } ,

    { .talent = TLT_1_AMBIDEXTRIOUS,            .name = L"Ambidextrious",              .description = L"Ambidextrious",  } ,
    { .talent = TLT_1_BLIND_FIGHTING,           .name = L"Blind Fighting",             .description = L"",               } ,
    { .talent = TLT_1_DIE_HARD,                 .name = L"Die Hard",                   .description = L"",               } ,
    { .talent = TLT_1_DISARM,                   .name = L"Disarm",                     .description = L"",               } ,
    { .talent = TLT_1_FRENZY,                   .name = L"Frenzy",                     .description = L"",               } ,
    { .talent = TLT_1_IRON_JAW,                 .name = L"Iron Jaw",                   .description = L"",               } ,
    { .talent = TLT_1_JADED,                    .name = L"Jaded",                      .description = L"",               } ,
    { .talent = TLT_1_LEAP_UP,                  .name = L"Leap Up",                    .description = L"",               } ,
    { .talent = TLT_1_QUICK_DRAW,               .name = L"Quick Draw",                 .description = L"",               } ,
    { .talent = TLT_1_RAPID_RELOAD,             .name = L"Rapid Reload",               .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_COLD,          .name = L"Resistance (Cold)",          .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_HEAT,          .name = L"Resistance (Heat)",          .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_FEAR,          .name = L"Resistance (Fear)",          .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_POISON,        .name = L"Resistance (Poison)",        .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_PSYCHIC,       .name = L"Resistance (Psychic)",       .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_RADIATION,     .name = L"Resistance (Radiation)",     .description = L"",               } ,
    { .talent = TLT_1_RESISTANCE_VACUUM,        .name = L"Resistance (Vacuum)",        .description = L"",               } ,
    { .talent = TLT_1_TAKEDOWN,                 .name = L"Takedown",                   .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_BOLT,     .name = L"Weapon Training (Bolt)",     .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_CHAIN,    .name = L"Weapon Training (Chain)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_FLAME,    .name = L"Weapon Training (Flame)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_HEAVY,    .name = L"Weapon Training (Heavy)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LOW_TECH, .name = L"Weapon Training (Low-Tech)", .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LAUNCHER, .name = L"Weapon Training (Launcher)", .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_LAS,      .name = L"Weapon Training (Las)",      .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_MELTA,    .name = L"Weapon Training (Melta)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_PLASMA,   .name = L"Weapon Training (Plasma)",   .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_POWER,    .name = L"Weapon Training (Power)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_SHOCK,    .name = L"Weapon Training (Shock)",    .description = L"",               } ,
    { .talent = TLT_1_WEAPON_TRAINING_SP,       .name = L"Weapon Training (SP)",       .description = L"",               } ,

    /*{ .talent = TLT_2_ARMOUR_MONGER,            .name = L"Armour Monger",              .description = L"",               } ,*/
    { .talent = TLT_2_TWO_WEAPON_FIGHTING,      .name = L"Two-weapon Fighting",        .description = L"",               } ,

    /*{ .talent = TLT_3_ASSASSINS_STRIKE,         .name = L"Assassins Strike",           .description = L"",               } ,*/

    { .talent = TLT_MAX,                        .name = L"Max",                        .description = L"Max",            }
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

static const wchar_t *hitloc_names[] = {
    [MSR_HITLOC_HEAD]       = L"head",
    [MSR_HITLOC_RIGHT_ARM]  = L"right arm",
    [MSR_HITLOC_LEFT_ARM]   = L"left arm",
    [MSR_HITLOC_BODY]       = L"thorax",
    [MSR_HITLOC_RIGHT_LEG]  = L"right leg",
    [MSR_HITLOC_LEFT_LEG]   = L"left leg",
};

static const wchar_t *gender_names[][2] = {
    [MSR_GENDER_MALE]   = {cs_MONSTER L"his" cs_CLOSE, cs_MONSTER L"he"  cs_CLOSE},
    [MSR_GENDER_FEMALE] = {cs_MONSTER L"her" cs_CLOSE, cs_MONSTER L"she" cs_CLOSE},
    [MSR_GENDER_IT]     = {cs_MONSTER L"its" cs_CLOSE, cs_MONSTER L"it"  cs_CLOSE},
};

