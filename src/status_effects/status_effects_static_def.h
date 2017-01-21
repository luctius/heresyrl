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

#define STATUS_EFFECT(_id, _name, _desc, hc) \
                [_id] = { .uid=0, .tid=_id, .name=_name, .description=_desc, .heal_cost=hc
#define STATUS_EFFECT_END }

#define MESSAGES(_of_plr, _of_msr, _oe_plr, _oe_msr) \
                .on_first_plr=_of_plr, .on_first_msr=_of_msr, .on_exit_plr=_oe_plr, .on_exit_msr=_oe_msr

#define EFFECTS_START .effects = {
#define EFFECTS_END }

#define CHECK(_flags, _param, _diff) \
                .check_flags= bf(EF_CHECK_ACTIVE) | _flags, .check_type=_param, .check_difficulty=_diff

#define HEALING(_flags, _diff, _evolve) \
                .heal_flags=bf(EF_HEAL_ACTIVE) | _flags, .heal_difficulty=_diff, .heal_evolve_tid=_evolve

#define SETTINGS(_flags, _dmin, _dmax) .setting_flags=bf(SEF_ACTIVE) | _flags, \
                    .duration_energy_min=(_dmin*TT_ENERGY_TURN), .duration_energy_max=(_dmax*TT_ENERGY_TURN)

#define EFFECT(_effect, _flags, _dmg, _param) \
                {.effect=_effect, .effect_setting_flags=bf(EF_SETT_ACTIVE) | _flags, .strength=_dmg, \
                .tick_interval_energy=0, .tick_energy=0, .param=_param, }

#define TICK_EFFECT(_effect, _flags, _dmg, _param, _interval, _max, _msg_plr, _msg_msr) \
                {.effect=_effect, .effect_setting_flags=bf(EF_SETT_ACTIVE) | bf(EF_SETT_TICK) | _flags, \
                .strength=_dmg, .tick_interval_energy=(_interval*TT_ENERGY_TURN), .tick_energy=0, \
                .ticks_max=_max, .on_tick_plr=_msg_plr, .on_tick_msr=_msg_msr, .param=_param, }

#define TALENT_EFFECT(_effect, _flags, _param) \
                {.effect=_effect, .effect_setting_flags=bf(EF_SETT_ACTIVE) | _flags, .param=_param, \
                .tick_interval_energy=0, .tick_energy=0, }

/*
   Long term and permanent conditions should ideally be offloaded to specific status effects to easy creation of cures.
*/

static const struct status_effect static_status_effect_list[] = {
    STATUS_EFFECT(SEID_NONE, L"", L"", -1),
            SETTINGS(0, 0, 0),
    STATUS_EFFECT_END,

    /* Draughts */
    STATUS_EFFECT(SEID_MINOR_HEALING, L"Healing Stimm", L"", 0),
        MESSAGES(cs_PLAYER "Your" cs_CLOSE " wounds begin to heal.", "%ls's wounds begin to heal.", cs_PLAYER "You" cs_CLOSE " feel the healing wearing off.", NULL),
        EFFECTS_START
            /* Type         Effect      Flags   Strength    Param   Interval    Max Msgs */
            TICK_EFFECT(EF_HEALTH,      0,      1,          0,      2,          0,  NULL, NULL),
        EFFECTS_END,
            /*Settings      Flags   Minimum  -  Maximum Turns*/
            SETTINGS(       0,      5,          15),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_MAD_CAP_AFTER, L"Mad Cap Mushroom After Effect", L"", 0),
        MESSAGES(cs_PLAYER "You" cs_CLOSE " are exhausted from the mushrooms trip.", "%ls looks exhausted.", cs_PLAYER "You" cs_CLOSE " feel the after effects slipping away.", NULL),
        EFFECTS_START
            /* Type     Effect      Flags   Strength    Param*/
            EFFECT(EF_MODIFY_CHAR,  0,         -10,      MSR_CHAR_TOUGHNESS),
            EFFECT(EF_MODIFY_CHAR,  0,         -10,      MSR_CHAR_STRENGTH),
            //EFFECT(EF_DAMAGE,       0,           2,      0),
        EFFECTS_END,
            /*Settings      Flags   Minimum  -  Maximum Turns*/
            SETTINGS(       0,      2,          5),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_MAD_CAP, L"Mad Cap Mushroom Rage", L"", 0),
        MESSAGES(cs_PLAYER "You" cs_CLOSE " slip into a mad and destructive rage.", "%ls slips into a mad and destructive rage.", "The rage starts to wear off.", NULL),
        EFFECTS_START
            /* Type     Effect      Flags   Strength    Param*/
            EFFECT(EF_MODIFY_CHAR,  0,         10,      MSR_CHAR_TOUGHNESS),
            EFFECT(EF_MODIFY_CHAR,  0,         10,      MSR_CHAR_STRENGTH),
            EFFECT(EF_MODIFY_SKILL, 0,        -50,      MSR_SKILLS_DODGE),
            EFFECT(EF_MODIFY_SKILL, 0,        -50,      MSR_SKILLS_PARRY),
            EFFECT(EF_EVOLVES,      0,          0,      SEID_MAD_CAP_AFTER),
        EFFECTS_END,
            /*Settings      Flags   Minimum  -  Maximum Turns*/
            SETTINGS(       0,      2,          10),
    STATUS_EFFECT_END,

    /* Weapon Effects */
    STATUS_EFFECT(SEID_WEAPON_FLAME, L"Flames", L"Flames are engulving you", 0),
        MESSAGES(cs_PLAYER "You" cs_CLOSE " catch fire.", "%ls has catched fire.", cs_PLAYER "You" cs_CLOSE " manage to put out the flames.", "%ls stomps out the flames."),
        EFFECTS_START
            /* Type         Effect           Flags      Strength    param. */
            EFFECT(     EF_ON_FIRE,          0,         0,          0),
            /* Type         Effect           Flags                  Strength            Param.  Interval    Max,    Msgs*/
            TICK_EFFECT(EF_DAMAGE, bf(EF_SETT_DMG_TYPE_ENERGY),     EF_STRENGTH_1D10,     0,    1,          0,      NULL, NULL),
        EFFECTS_END,
        /*Settings  Flags       Minimum  -  Maximum Turns*/
        SETTINGS(       0,      2,          10),
        /*Check         Flags                                           Type                Difficulty */
        CHECK(bf(EF_CHECK_CHARACTERISTIC) | bf(EF_CHECK_EACH_INTERVAL), MSR_CHAR_AGILITY,   0),
        /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
        HEALING(0,      0,              SEID_NONE),
    STATUS_EFFECT_END,




    /* Critical hits */
    /* Note: Critical hits should be in order (blunt_1, then blunt_2, without
               anything interfering.  se_add_critical_hit requires that.) */
    /* Note 2: Althought Left/Right leg and Left/Right arm are almost identical, right now
               I do not see a way to merge them without causing lots headackes. */
    /*-------------------------------------------------------------------------*/

#include "status_effects_critical_blunt.h"


    /*-------------------------------------------------------------------------*/
    /* Environment */
    STATUS_EFFECT(SEID_SWIMMING, L"Swimming", L"", 0),
        EFFECTS_START
            /* Type     Effect      Flags   Strength   Param */
            EFFECT(EF_SWIMMING,     0,      0,         0),
            EFFECT(EF_MODIFY_CHAR,  0,      -15,       MSR_CHAR_COMBAT),
            EFFECT(EF_MODIFY_CHAR,  0,      -15,       MSR_CHAR_AGILITY),
            EFFECT(EF_MODIFY_SKILL, 0,      -15,       MSR_SKILLS_DODGE),
            EFFECT(EF_MODIFY_SKILL, 0,      -15,       MSR_SKILLS_STEALTH),
        EFFECTS_END,
            /*Settings      Flags */
            SETTINGS(bf(SEF_PERMANENT),      0,          0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_WADE, L"Wading", L"", 0),
        EFFECTS_START
            /* Type    Effect       Flags    Strength    Param */
            EFFECT(EF_MODIFY_CHAR,  0,        -10,       MSR_CHAR_COMBAT),
            EFFECT(EF_MODIFY_CHAR,  0,        -10,       MSR_CHAR_AGILITY),
            EFFECT(EF_MODIFY_SKILL, 0,        -10,       MSR_SKILLS_DODGE),
            EFFECT(EF_MODIFY_SKILL, 0,        -10,       MSR_SKILLS_STEALTH),
        EFFECTS_END,
            /*Settings      Flags */
            SETTINGS(bf(SEF_PERMANENT),      0,          0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_MUD, L"Mud", L"", 0),
        EFFECTS_START
            /* Type    Effect       Flags    Strength    Param */
            EFFECT(EF_MODIFY_CHAR,  0,        -5,       MSR_CHAR_COMBAT),
            EFFECT(EF_MODIFY_CHAR,  0,        -5,       MSR_CHAR_AGILITY),
            EFFECT(EF_MODIFY_SKILL, 0,        -5,       MSR_SKILLS_DODGE),
            EFFECT(EF_MODIFY_SKILL, 0,        -5,       MSR_SKILLS_STEALTH),
        EFFECTS_END,
            /*Settings      Flags */
            SETTINGS(bf(SEF_PERMANENT),      0,          0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_MAD_CAP_CLOUD, L"Mad Cap Cloud", L"", 0),
        MESSAGES(cs_PLAYER "You" cs_CLOSE " slip into a mad and destructive rage.", "%ls slips into a mad and destructive rage.", "The rage starts to wear off.", NULL),
        EFFECTS_START
            /* Type     Effect      Flags   Strength    Param*/
            EFFECT(EF_MODIFY_CHAR,  0,         10,      MSR_CHAR_TOUGHNESS),
            EFFECT(EF_MODIFY_CHAR,  0,         10,      MSR_CHAR_STRENGTH),
            EFFECT(EF_MODIFY_SKILL, 0,        -50,      MSR_SKILLS_DODGE),
            EFFECT(EF_MODIFY_SKILL, 0,        -50,      MSR_SKILLS_PARRY),
            EFFECT(EF_EVOLVES,      0,          0,      SEID_MAD_CAP_AFTER),
        EFFECTS_END,
            /*Settings      Flags       Minimum  -  Maximum Turns*/
            SETTINGS(        0,            3,          3),
    STATUS_EFFECT_END,

    /*-------------------------------------------------------------------------*/

    STATUS_EFFECT(SEID_ENCUMBERED, L"Encumbered", L"", -1),
        MESSAGES(cs_PLAYER "You" cs_CLOSE " are encumbered.", NULL, cs_PLAYER "Your" cs_CLOSE " load seems manageble now.", NULL),
        EFFECTS_START
            /* Type    Effect           Flags   Strength     Param  Interval,   Max, Msgs */
            TICK_EFFECT(EF_ENCUMBERED,  0,      0,           0,     0.01,       0,   NULL, NULL),
        EFFECTS_END,
        /*Settings      Flags          Minimum  -  Maximum Turns*/
        SETTINGS(bf(SEF_PERMANENT),        1,          1),
    STATUS_EFFECT_END,

    /* Healing after a fatepoint */
    STATUS_EFFECT(SEID_FATEHEALTH, L"Fatepoint Health", L"", -1),
        EFFECTS_START
            /* Type         Effect      Flags   Strength            Param   Interval   Max, Msgs*/
            TICK_EFFECT(EF_HEALTH,      0,      EF_STRENGTH_4D10,   0,      1,         0,   NULL, NULL),
        EFFECTS_END,
        /*Settings      Flags     Minimum  -  Maximum Turns*/
        SETTINGS(       0,        2,          2),
    STATUS_EFFECT_END,

    /* Stealth penalty after attacking */
    STATUS_EFFECT(SEID_COMBAT_EXCITEMENT, L"Combat Excitement", L"", -1),
        MESSAGES(NULL, NULL, cs_PLAYER "Your" cs_CLOSE " combat excitement wears down.", NULL),
        EFFECTS_START
            /* Type     Effect      Flags   Strength    Param*/
            EFFECT(EF_MODIFY_SKILL,  0,        -50,      MSR_SKILLS_STEALTH),
        EFFECTS_END,
        /*Settings      Flags      Minimum  -  Maximum Turns*/
        SETTINGS(bf(SEF_PERMANENT),  5,          5),
    STATUS_EFFECT_END,

    /* Death */
    STATUS_EFFECT(SEID_DEATH, L"Death", L"", -1),
        EFFECTS_START
            /* Type         Effect     Flags   Strength Param */
            EFFECT(EF_DEAD,    0,      0,       0),
        EFFECTS_END,
        /*Settings   Flags       Minimum  -  Maximum Turns*/
        SETTINGS(bf(SEF_PERMANENT),  0,          0),
    STATUS_EFFECT_END,

    /* Debug */
    STATUS_EFFECT(SEID_DEATH_STIMM, L"death stimm", L"", -1),
        EFFECTS_START
            /* Type         Effect     Flags   Strength Param */
            EFFECT(EF_DISABLED_RARM,    0,      0,       0),
        EFFECTS_END,
        /*Settings   Flags     Minimum  -  Maximum Turns*/
        SETTINGS(       0,     0,          0),
        CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   +20),
    STATUS_EFFECT_END,
};

/*
   This lookup table retrieves the starting entry of the
   critical hit table. The damage is then added to that
   to get to the correct entry.
*/
static const enum se_ids dmg_type_to_id_lot[MSR_HITLOC_MAX][DMG_TYPE_MAX] = {
    [MSR_HITLOC_LEFT_LEG] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_RIGHT_LEG] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_LEFT_ARM] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_RIGHT_ARM] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_BODY] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_HEAD] = {
        [DMG_TYPE_ENERGY]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_EXPLOSIVE] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_IMPACT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_RENDING]   = SEID_BLUNT_LARM_1,
    },
};

static const wchar_t *se_names[] = {
    [EF_NONE]               = L"error",

    [EF_SET_CHAR]           = L"error",            /* param     =            = enum msr_characteristic */

    [EF_MODIFY_FATIQUE]     = L"error",
    [EF_MODIFY_CHAR]        = L"error",       /* param          =            = enum msr_characteristic */
    [EF_MODIFY_SKILL]       = L"error",      /* param           =            = enum msr_skills */
    [EF_MODIFY_MAX_WOUNDS]  = L"error",

    [EF_DEAD]               = L"error",

    [EF_DAMAGE]             = L"DoT",
    [EF_HEALTH]             = L"Healing",

    [EF_ALLY]               = L"error",                /* param =            = ally_faction */
    [EF_ENEMY]              = L"error",               /* param  =            = ally_faction */
    [EF_BLEEDING]           = L"Bleeding",
    [EF_BLINDED]            = L"Blinded",
    [EF_CONFUSED]           = L"Confused",
    [EF_DEAFENED]           = L"Deaf",
    [EF_DISABLED_LLEG]      = L"L Leg",
    [EF_DISABLED_RLEG]      = L"R Leg",
    [EF_DISABLED_LARM]      = L"L Arm",
    [EF_DISABLED_RARM]      = L"R Arm",
    [EF_DISABLED_EYE]       = L"Eye",
    [EF_ENCUMBERED]         = L"Encumbered",
    [EF_EXHAUSTED]          = L"Exhausted",
    [EF_GRAPPLED]           = L"Grappled",
    [EF_HELPLESS]           = L"Helpless",
    [EF_INHIBIT_FATE_POINT] = L"No Fate",
    [EF_INVISIBLE]          = L"Invisible",
    [EF_PARALYZED]          = L"Paralyzed",
    [EF_ON_FIRE]            = L"Fire",
    [EF_PINNED]             = L"Pinned",
    [EF_SWIMMING]           = L"Swimming",
    [EF_STAGGERED]          = L"Staggered",
    [EF_STUNNED]            = L"Stunned",
    [EF_SUMMONED]           = L"Summoned",
    [EF_UNCONSCIOUS]        = L"Unconscious",

    [EF_INSTANT_DEATH]      = L"error",
    [EF_EXPLODE]            = L"error",

    [EF_TALENT]             = L"error",
    [EF_SKILL]              = L"error",
    [EF_TRAIT]              = L"error",

    [EF_EVOLVES]            = L"error",

    [EF_MAX]                = L"error",
};
