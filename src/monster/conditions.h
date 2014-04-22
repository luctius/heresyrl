#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "heresyrl_def.h"
#include "conditions_static.h"
#include "enums.h"
#include "monster.h"

#define CONDITION_MAX_NR_EFFECTS 10

enum condition_setting_flags {
    /* The condition will be permanent (if it passes the initial checks) */
    CDN_SF_PERMANENT,

    /* Only a single instance of this condition will be permitted inside a list */
    CDN_SF_UNIQUE,

    /* Require an specific check with difficulty as modifier, 
       if the monster succeeds, the condition will be removed.
       Every degree of failure adds a turn to the duration.
    */
    CDN_SF_REQ_WILL_CHECK,
    CDN_SF_REQ_TGH_CHECK,
    CDN_SF_REQ_AG_CHECK,
    CDN_SF_REQ_CHEM_USE_CHECK,

    /* Sets if this condition can be removed by a detox condition */
    CDN_SF_DETOXABLE,

    /* If true, there will be no textual updates not will 
       this be visible in a character sheet */
    CDN_SF_INVISIBLE,

    /* If true, if any condition is set to non-active which 
       does have an effect set, the whole condition is removed. 
       This is used for conditions which have multiple equal 
       priority effects.
     */
    CDN_SF_ACTIVE_ALL,

    /* If set, it will remove any existing instances of the template_id 
       which this condition will evolve in. 
       For example to temporarily relieve the cravings...
     */
    CDN_SF_REMOVE_CONTINUE,

    CDN_SF_MAX,
};

enum condition_effect_flags {
    CDN_EF_NONE,
    CDN_EF_MODIFY_FATIQUE,
    CDN_EF_MODIFY_WS,
    CDN_EF_MODIFY_BS,
    CDN_EF_MODIFY_STR,
    CDN_EF_MODIFY_AG,
    CDN_EF_MODIFY_TGH,
    CDN_EF_MODIFY_PER,
    CDN_EF_MODIFY_WILL,
    CDN_EF_MODIFY_INT,
    CDN_EF_MODIFY_FEL,
    CDN_EF_MODIFY_MOVEMENT,
    CDN_EF_MODIFY_WOUNDS,
    CDN_EF_MODIFY_ALL_SKILLS,
    CDN_EF_DISABLE_LLEG,
    CDN_EF_DISABLE_RLEG,
    CDN_EF_DISABLE_LARM,
    CDN_EF_DISABLE_RARM,
    CDN_EF_DISABLE_BODY,

    /*Warning, damage conditions trigger critical hits, do not use in critical hit tables. */
    CDN_EF_DAMAGE,
    CDN_EF_DAMAGE_TICK,

    CDN_EF_HEALTH,
    CDN_EF_HEALTH_TICK,
    CDN_EF_BLINDNESS,
    CDN_EF_DEAFNESS,
    CDN_EF_STUNNED,
    CDN_EF_STUMBLE,
    CDN_EF_HALUCINATING,
    CDN_EF_CONFUSED,
    CDN_EF_INHIBIT_FATE_POINT,
    CDN_EF_PINNED,
    CDN_EF_ON_FIRE,
    CDN_EF_POISON,
    CDN_EF_PSYCHIC_ENHANCE,
    CDN_EF_DETOX,
    CDN_EF_DEATH,
    CDN_EF_EXPLODE,
    CDN_EF_MAX,
};

enum condition_effect_setting_flags {
    /* if set, this effect is used. */
    CDN_ESF_ACTIVE,

    /* if true, this effect has a certain tick timer. */
    CDN_ESF_TICK,

    /* Changes made to characteristics are permanent. */
    CDN_ESF_PERMANENT,

    /* if true, require a check everytime this effect is 
       processed. if the monster succeeds, the effect will 
       be set to inactive. */
    CDN_ESF_REQ_WILL_CHECK,
    CDN_ESF_REQ_TGH_CHECK,
    CDN_ESF_REQ_AG_CHECK,
    CDN_ESF_REQ_CHEM_USE_CHECK,

    /* Set the damage type of any damage done*/
    CDN_ESF_DMG_TYPE_ENERGY,
    CDN_ESF_DMG_TYPE_IMPACT,
    CDN_ESF_DMG_TYPE_RENDING,
    CDN_ESF_DMG_TYPE_EXPLODING,

    /* Set the category of damage. */
    CDN_ESF_RES_TYPE_HEAT,
    CDN_ESF_RES_TYPE_COLD,
    CDN_ESF_RES_TYPE_FEAR,
    CDN_ESF_RES_TYPE_POISONS,
    CDN_ESF_RES_TYPE_PSYCHIC,

    /* if set, it directly modifies the base value 
       of an characteristic, instead of damaging it. */
    CDN_ESF_MODIFY_BASE,

    /* Regardless of the duration of the whole condition, 
       this effect stops after the first tick. */
    CDN_ESF_ONCE,

    CDN_ESF_MAX,
};

enum cdn_strength {
    CDN_DAMAGE_NONE,
    CDN_DAMAGE_ONE,
    CDN_DAMAGE_TWO,
    CDN_DAMAGE_THREE,
    CDN_DAMAGE_FIVE,
    CDN_DAMAGE_TEN,
    CDN_DAMAGE_1D5,
    CDN_DAMAGE_1D10,
    CDN_DAMAGE_2D10,
    CDN_DAMAGE_3D10,
    CDN_DAMAGE_4D10,
    CDN_HEALTH_ONE,
    CDN_HEALTH_TWO,
    CDN_HEALTH_THREE,
    CDN_HEALTH_FIVE,
    CDN_HEALTH_TEN,
    CDN_HEALTH_1D5,
    CDN_HEALTH_1D10,
    CDN_HEALTH_2D10,
    CDN_HEALTH_3D10,
    CDN_HEALTH_4D10,
};

struct condition_effect_struct {
    enum condition_effect_flags effect;
    uint32_t effect_setting_flags;

    int8_t strength;
    int8_t difficulty;

    /* duration in turns, will be converted to energy when created */
    uint16_t tick_energy_max;

    uint16_t tick_energy;
};

struct cdn_condition_list;

struct cdn_condition {
    uint32_t condition_pre;

    int uid;
    enum cdn_ids template_id;

    uint16_t setting_flags;
    int8_t difficulty;

    const char *name;
    const char *description;

    struct condition_effect_struct effects[CONDITION_MAX_NR_EFFECTS];

    /*duration in turns, will be converted to energy when created. */
    int duration_energy_min;
    int duration_energy_max;

    int duration_energy;

    const char *on_apply_plr;
    const char *on_apply_msr;
    const char *on_exit_plr;
    const char *on_exit_msr;

    enum cdn_ids continues_to_id;

    uint32_t condition_post;
};

void cdn_init(void);
void cdn_exit(void);

struct cdn_condition_list *cdn_list_init(void);
void cdn_list_exit(struct cdn_condition_list *cdn_list);
bool cdn_verify_list(struct cdn_condition_list *cdn_list);
int cdn_list_size(struct cdn_condition_list *cdn_list);
struct cdn_condition *cdn_list_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev);
void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster);

bool cdn_verify_condition(struct cdn_condition *cdn);

struct cdn_condition *cdn_get_condition_tid(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
bool cdn_has_effect(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);
bool cdn_has_tid(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
int cdn_condition_effect_strength(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);

struct cdn_condition *cdn_create(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
bool cdn_add_to_list(struct cdn_condition_list *cdn_list, struct cdn_condition *con);

bool cdn_add_condition(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
bool cdn_remove_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *c);

bool cdn_add_critical_hit(struct cdn_condition_list *cdn_list, int critical_dmg, enum msr_hit_location mhl, enum dmg_type type);

#endif /* CONDITIONS_H */
