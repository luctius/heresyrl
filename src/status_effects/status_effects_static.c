#define STATUS_EFFECT(_id, _name, _desc) \
                [_id] = { .uid=0, .template_id=_id, .name=_name, .description=_desc
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
    STATUS_EFFECT(SEID_NONE, "", ""),
            SETTINGS(0, 0, 0),
    STATUS_EFFECT_END,

    /* Draughts */
    STATUS_EFFECT(SEID_MINOR_HEALING, "healing draught", ""),
        MESSAGES("Your wounds begin to heal.", "%s's wounds begin to heal.", "You feel the healing wearing off.", NULL),
        EFFECTS_START
            /* Type         Effect      Flags   Strength    Param   Interval    Max Msgs */
            TICK_EFFECT(EF_HEALTH,      0,      0,          0,      2,          0,  NULL, NULL),
        EFFECTS_END,
            /*Settings      Flags   Minimum  -  Maximum Turns*/
            SETTINGS(       0,      2,          10),
    STATUS_EFFECT_END,

    /* Weapon Effects */
    STATUS_EFFECT(SEID_WEAPON_FLAME, "flames", "flames are engulving you"),
        MESSAGES("You catch fire.", "%s has catched fire.", "You manage to put out the flames.", "%s stomps out the flames."),
        EFFECTS_START
            /* Type         Effect           Flags      Strength    param. */
            EFFECT(     EF_ON_FIRE,          0,         0,          0),
            /* Type         Effect           Flags                  Strength            Param.  Interval    Max,    Msgs*/
            TICK_EFFECT(EF_DAMAGE, bf(EF_SETT_DMG_TYPE_ENERGY),     EF_STRENGTH_1D10,     0,    1,          0,      NULL, NULL),
        EFFECTS_END,
        /*Settings  Flags       Minimum  -  Maximum Turns*/
        SETTINGS(       0,      1,          10),
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

    /* Healing after a fatepoint */
    STATUS_EFFECT(SEID_FATEHEALTH, "fatepoint_health", ""),
        EFFECTS_START
            /* Type         Effect      Flags   Strength            Param   Interval   Max, Msgs*/
            TICK_EFFECT(EF_HEALTH,      0,      EF_STRENGTH_4D10,   0,      1,         0,   NULL, NULL),
        EFFECTS_END,
        /*Settings      Flags     Minimum  -  Maximum Turns*/
        SETTINGS(       0,        2,          2),
    STATUS_EFFECT_END,

    /* Debug */
    STATUS_EFFECT(SEID_DEATH_STIMM, "death stimm", ""),
        EFFECTS_START
            /* Type         Effect     Flags   Strength Param */
            EFFECT(EF_DISABLED_RARM,    0,      0,       0),
        EFFECTS_END,
        /*Settings   Flags     Minimum  -  Maximum Turns*/
        SETTINGS(       0,     0,          0),
        CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   +20),
    STATUS_EFFECT_END,
};

