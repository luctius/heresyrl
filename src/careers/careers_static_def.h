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

static struct cr_homeworld static_homeworld_list[] = {
    [CR_HWID_NONE] = {
        .name = "None",
        .description = "nothing",
    },

    [CR_HWID_HIVE] = {
        .name = "Hive World",
        .description = "description of a hive world",

        .fate_points = 2,
        .fate_threshold = 6,

        .aptitudes = bf(APTITUDE_PERCEPTION),
        .homeworld_talent = 0,
        .wounds = 8,

        .characteristics = {
            [MSR_CHAR_COMBAT]       =  0,
            [MSR_CHAR_STRENGTH]     =  0,
            [MSR_CHAR_TOUGHNESS]    =  0,
            [MSR_CHAR_AGILITY]      =  5,
            [MSR_CHAR_INTELLIGENCE] =  0,
            [MSR_CHAR_WILLPOWER]    = -5,
            [MSR_CHAR_PERCEPTION]   =  5,
        },
    },

    [CR_HWID_MAX] = {
    },
};

static struct cr_background static_background_list[] = {
    [CR_BCKGRNDID_NONE] = {
        .name = "None",
        .description = "nothing",
    },

    [CR_BCKGRNDID_IMPERIAL_GUARD] = {
        .name = "Imperial Guard",
        .description = "description of imperial guard",

        .skills = bf(MSR_SKILLS_MEDICAE),
        .talents = TLT_1_WEAPON_TRAINING_LAS | TLT_1_WEAPON_TRAINING_LOW_TECH,
        .aptitudes = bf(APTITUDE_FIELDCRAFT),
        .background_talent = 0,

        .trappings = {
            IID_LAS_GUN,
            IID_BASIC_AMMO_LAS,
            IID_FLAK_GUARD_ARMOUR,
        },
    },
    [CR_BCKGRNDID_OUTCAST] = {
        .name = "Outcast",
        .description = "description of outcast",

        .skills = bf(MSR_SKILLS_ACROBATICS) | bf(MSR_SKILLS_DODGE) | bf(MSR_SKILLS_STEALTH),
        .talents = TLT_1_WEAPON_TRAINING_SP | TLT_1_WEAPON_TRAINING_CHAIN,
        .aptitudes = bf(APTITUDE_FIELDCRAFT),
        .background_talent = 0,

        .trappings = {
            IID_STUB_AUTOMATIC,
            IID_PISTOL_AMMO_SP,
            IID_FLAK_VEST,
        },
    },

    [CR_BCKGRNDID_MAX] = {
    },
};

static struct cr_role static_role_list[] = {
    [CR_ROLEID_NONE] = {
        .name = "None",
        .description = "nothing",

        .aptitudes = 0,
        .talents = 0,
        .role_talent = 0,
    },

    [CR_ROLEID_SEEKER] = {
        .name = "Seeker",
        .description = "description of seeker",

        .aptitudes = bf(APTITUDE_INTELLIGENCE) | bf(APTITUDE_PERCEPTION),
        .talents = TLT_1_DISARM,
        .role_talent = 0,
    },
    [CR_ROLEID_WARRIOR] = {
        .name = "Warrior",
        .description = "description of warrior",

        .aptitudes = bf(APTITUDE_RANGED) | bf(APTITUDE_DEFENCE) | bf(APTITUDE_OFFENCE) | bf(APTITUDE_STRENGTH) | bf(APTITUDE_MELEE),
        .talents = TLT_1_IRON_JAW,
        .role_talent = 0,
    },

    [CR_ROLEID_MAX] = {
    },
};

struct aptitudes_per_skill {
    enum aptitude_enum apt1;
    enum aptitude_enum apt2;
};

static const struct aptitudes_per_skill aptitude_skill_list[] = {
    [MSR_SKILLS_NONE]       = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_MELEE]      = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_RANGED]     = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_ACROBATICS] = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_AWARENESS]  = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_DODGE]      = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_PARRY]      = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_STEALTH]    = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_SURVIVAL]   = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    [MSR_SKILLS_MAX]        = { .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
};

struct aptitudes_per_talent {
    bitfield64_t talent;
    enum aptitude_enum apt1;
    enum aptitude_enum apt2;
};

static const struct aptitudes_per_talent aptitude_talent_list[] = {
    { .talent = TLT_NONE,                       .apt1 = APTITUDE_NONE, .apt2 = APTITUDE_NONE, },

    { .talent = TLT_1_AMBIDEXTRIOUS,            .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_BLIND_FIGHTING,           .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_DIE_HARD,                 .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_DISARM,                   .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_FRENZY,                   .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_IRON_JAW,                 .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_JADED,                    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_LEAP_UP,                  .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_QUICK_DRAW,               .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RAPID_RELOAD,             .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_COLD,          .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_HEAT,          .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_FEAR,          .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_POISON,        .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_PSYCHIC,       .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_RADIATION,     .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_RESISTANCE_VACUUM,        .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_TAKEDOWN,                 .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_BOLT,     .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_CHAIN,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_FLAME,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_HEAVY,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_LOW_TECH, .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_LAUNCHER, .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_LAS,      .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_MELTA,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_PLASMA,   .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_POWER,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_SHOCK,    .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_1_WEAPON_TRAINING_SP,       .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },

    { .talent = TLT_2_ARMOUR_MONGER,            .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },
    { .talent = TLT_2_TWO_WEAPON_FIGHTING,      .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },

    { .talent = TLT_3_ASSASSINS_STRIKE,         .apt1 = APTITUDE_GENERAL, .apt2 = APTITUDE_GENERAL, },

    { .talent = TLT_MAX,                        .apt1 = APTITUDE_NONE, .apt2 = APTITUDE_NONE, },
};

static const int talent_cost_list[][MSR_TALENT_TIER_MAX] = {
    [0] = { 600, 900, 1200, -1},
    [1] = { 300, 450, 600, -1},
    [2] = { 100, 200, 300, -1},
};

static const char *aptitude_names[] = {
    [APTITUDE_GENERAL]      = "General",
    [APTITUDE_MELEE]        = "Melee",
    [APTITUDE_RANGED]       = "Ranged",
    [APTITUDE_STRENGTH]     = "Strength",
    [APTITUDE_TOUGHNESS]    = "Toughness",
    [APTITUDE_AGILITY]      = "Agility",
    [APTITUDE_WILLPOWER]    = "Willpower",
    [APTITUDE_INTELLIGENCE] = "Intelligence",
    [APTITUDE_PERCEPTION]   = "Perception",
    [APTITUDE_OFFENCE]      = "Offence",
    [APTITUDE_FINESS]       = "Finess",
    [APTITUDE_DEFENCE]      = "Defence",
    [APTITUDE_FIELDCRAFT]   = "Fieldcraft",

    [APTITUDE_MAX]          = "ERROR",
};

