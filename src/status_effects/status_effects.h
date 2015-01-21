#ifndef STATUS_EFFECTS_H
#define STATUS_EFFECTS_H

#include "heresyrl_def.h"
#include "enums.h"
#include "status_effects/status_effects_static.h"
#include "monster/monster.h"

#define STATUS_EFFECT_MAX_NR_EFFECTS 10

enum status_effect_flags {
    /* The status_effect will be permanent (if it passes the initial checks) */
    SEF_PERMANENT,

    /* Only a single instance of this status_effect id will be permitted inside a list */
    SEF_UNIQUE,

    /* Require an specific check with difficulty as modifier, 
       if the monster succeeds, the status_effect will be removed.
       Every degree of failure adds a turn to the duration.
    */
    SEF_REQ_WILL_CHECK,
    SEF_REQ_TGH_CHECK,
    SEF_REQ_AG_CHECK,
    SEF_REQ_CHEM_USE_CHECK,

    /* Sets if this status_effect can be removed by a detox status_effect */
    SEF_DETOXABLE,

    /* If true, there will be no textual updates not will 
       this be visible in a character sheet */
    SEF_INVISIBLE,

    /* If set, it will remove any existing instances of the template_id 
       which this status_effect will evolve in. 
       For example to temporarily relieve the cravings...
     */
    SEF_REMOVE_CONTINUE,

    /* If set and on the ground will make the tile opague. */
    SEF_BLOCKS_SIGHT,

    SEF_MAX,
};

enum status_effect_type_flags {
    SETF_NONE,

    SETF_SET_WS,
    SETF_SET_BS,
    SETF_SET_STR,
    SETF_SET_AG,
    SETF_SET_TGH,
    SETF_SET_PER,
    SETF_SET_WILL,
    SETF_SET_INT,
    SETF_SET_FEL,

    SETF_DECREASE_FATIQUE,
    SETF_DECREASE_WS,
    SETF_DECREASE_BS,
    SETF_DECREASE_STR,
    SETF_DECREASE_AG,
    SETF_DECREASE_TGH,
    SETF_DECREASE_PER,
    SETF_DECREASE_WILL,
    SETF_DECREASE_INT,
    SETF_DECREASE_FEL,
    SETF_DECREASE_MOVEMENT,
    SETF_DECREASE_MAX_WOUNDS,
    SETF_DECREASE_ALL_SKILLS,

    SETF_INCREASE_FATIQUE,
    SETF_INCREASE_WS,
    SETF_INCREASE_BS,
    SETF_INCREASE_STR,
    SETF_INCREASE_AG,
    SETF_INCREASE_TGH,
    SETF_INCREASE_PER,
    SETF_INCREASE_WILL,
    SETF_INCREASE_INT,
    SETF_INCREASE_FEL,
    SETF_INCREASE_MOVEMENT,
    SETF_INCREASE_MAX_WOUNDS,
    SETF_INCREASE_ALL_SKILLS,

    SETF_DISABLE_LLEG,
    SETF_DISABLE_RLEG,
    SETF_DISABLE_LARM,
    SETF_DISABLE_RARM,

    /*Warning, damage status_effects trigger critical hits, do not use in critical hit tables. */
    SETF_DAMAGE,
    SETF_DAMAGE_TICK,

    SETF_HEALTH,
    SETF_HEALTH_TICK,
    SETF_BLOODLOSS,
    SETF_BLINDNESS,
    SETF_DEAFNESS,
    SETF_STUNNED,
    SETF_STUMBLE,
    SETF_HALUCINATING,
    SETF_CONFUSED,
    SETF_INHIBIT_FATE_POINT,
    SETF_PINNED,
    SETF_ON_FIRE,
    SETF_POISON,
    SETF_PSYCHIC_ENHANCE,
    SETF_DETOX,
    SETF_INSTANT_DEATH,
    SETF_EXPLODE,
    SETF_MAX,
};

enum status_effect_setting_flags {
    /* if set, this effect is used. */
    SESF_ACTIVE,

    /* if true, this effect has a certain tick timer. */
    SESF_TICK,

    /* Changes made to characteristics are permanent. */
    SESF_PERMANENT,

    /* if true, require a check everytime this effect is 
       processed. if the monster succeeds, the effect will 
       be set to inactive. */
    SESF_REQ_WILL_CHECK,
    SESF_REQ_TGH_CHECK,
    SESF_REQ_AG_CHECK,
    SESF_REQ_CHEM_USE_CHECK,

    /* Set the damage type of any damage done*/
    SESF_DMG_TYPE_ENERGY,
    SESF_DMG_TYPE_IMPACT,
    SESF_DMG_TYPE_RENDING,
    SESF_DMG_TYPE_EXPLODING,

    /* Set the category of damage. */
    SESF_RES_TYPE_HEAT,
    SESF_RES_TYPE_COLD,
    SESF_RES_TYPE_FEAR,
    SESF_RES_TYPE_POISONS,
    SESF_RES_TYPE_PSYCHIC,

    /* Regardless of the duration of the whole status_effect, 
       this effect stops after the first tick. */
    SESF_ONCE,

    SESF_MAX,
};

enum se_strength {
    SE_STRENGTH_NONE,
    SE_STRENGTH_ONE,
    SE_STRENGTH_TWO,
    SE_STRENGTH_THREE,
    SE_STRENGTH_FIVE,
    SE_STRENGTH_TEN,
    SE_STRENGTH_1D5,
    SE_STRENGTH_1D10,
    SE_STRENGTH_2D10,
    SE_STRENGTH_3D10,
    SE_STRENGTH_4D10,
};

struct se_type_struct {
    enum status_effect_type_flags effect;
    bitfield32_t effect_setting_flags;

    int8_t strength;
    int8_t difficulty;

    /* operational variables. */
    int16_t tick_energy_max;
    int16_t tick_energy;
    int8_t ticks_applied;
};

struct status_effect_list;

struct status_effect {
    uint32_t status_effect_pre;

    int uid;
    enum se_ids template_id;

    bitfield32_t setting_flags;
    int8_t difficulty;

    const char *name;
    const char *description;

    struct se_type_struct effects[STATUS_EFFECT_MAX_NR_EFFECTS];

    bool permissible_on_ground;
    char icon;
    int icon_attr;
    const char *see_description;

    /*duration in turns, will be converted to energy when created. */
    int duration_energy_min;
    int duration_energy_max;
    int duration_energy;

    const char *on_first_plr;
    const char *on_first_msr;
    const char *on_apply_plr;
    const char *on_apply_msr;
    const char *on_exit_plr;
    const char *on_exit_msr;

    enum se_ids continues_to_id;

    uint32_t status_effect_post;
};

void se_init(void);
void se_exit(void);

struct status_effect_list *se_list_init(void);
void se_list_exit(struct status_effect_list *se_list);
bool se_verify_list(struct status_effect_list *se_list);
int se_list_size(struct status_effect_list *se_list);
struct status_effect *se_list_get_next_status_effect(struct status_effect_list *se_list, struct status_effect *prev);
void se_process(struct msr_monster *monster);
void se_remove_all_non_permanent(struct msr_monster *monster);

bool se_verify_status_effect(struct status_effect *se);

struct status_effect *se_get_status_effect_tid(struct status_effect_list *se_list, enum se_ids tid);
bool se_has_flag(struct status_effect *se, enum status_effect_flags flag);
bool se_has_effect(struct status_effect_list *se_list, enum status_effect_type_flags effect);
bool se_has_effect_permanent(struct status_effect_list *se_list, enum status_effect_type_flags effect);
bool se_has_tid(struct status_effect_list *se_list, enum se_ids tid);
int se_status_effect_strength(struct status_effect_list *se_list, enum status_effect_type_flags effect);

struct status_effect *se_create(struct status_effect_list *se_list, enum se_ids tid);
bool se_add_to_list(struct status_effect_list *se_list, struct status_effect *con);

bool se_add_status_effect(struct status_effect_list *se_list, enum se_ids tid);
bool se_remove_status_effect(struct status_effect_list *se_list, struct status_effect *c);

bool se_add_critical_hit(struct status_effect_list *se_list, int critical_dmg, enum msr_hit_location mhl, enum dmg_type type);

#endif /* STATUS_EFFECTS_H */
