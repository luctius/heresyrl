#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "heresyrl_def.h"
#include "conditions_static.h"

#define CONDITION_MAX_NR_EFFECTS 10

enum condition_setting_flags {
    CDN_SF_PERMANENT,
    CDN_SF_REQ_WILL_CHECK,
    CDN_SF_REQ_TGH_CHECK,
    CDN_SF_REQ_AG_CHECK,
    CDN_SF_REQ_CHEM_USE_CHECK,
    CDN_SF_DETOXABLE,
    CDN_SF_INVISIBLE,
    CDN_SF_MAX,
};

enum condition_effect_flags {
    CDN_EF_NONE,
    CDN_EF_MODIFY_WS,
    CDN_EF_MODIFY_BS,
    CDN_EF_MODIFY_STR,
    CDN_EF_MODIFY_AG,
    CDN_EF_MODIFY_TGH,
    CDN_EF_MODIFY_PER,
    CDN_EF_MODIFY_WILL,
    CDN_EF_MODIFY_INT,
    CDN_EF_MODIFY_FEL,
    CDN_EF_MODIFY_FATIQUE,
    CDN_EF_MODIFY_MOVEMENT,
    CDN_EF_MODIFY_ALL_SKILLS,
    CDN_EF_DISABLE_LLEG,
    CDN_EF_DISABLE_RLEG,
    CDN_EF_DISABLE_LARM,
    CDN_EF_DISABLE_RARM,
    CDN_EF_DISABLE_CHEST,
    CDN_EF_DAMAGE_TICK,
    CDN_EF_HEALTH_TICK,
    CDN_EF_BLINDNESS,
    CDN_EF_DEAFNESS,
    CDN_EF_STUNNED,
    CDN_EF_HALUCINATING,
    CDN_EF_CONFUSED,
    CDN_EF_INHIBIT_FATE_POINT,
    CDN_EF_PINNED,
    CDN_EF_ON_FIRE,
    CDN_EF_POISON,
    CDN_EF_PSYCHIC_ENHANCE,
    CDN_EF_DETOX,
    CDN_EF_DEATH,
    CDN_EF_MAX,
};

enum condition_effect_setting_flags {
    CDN_ESF_ACTIVE,
    CDN_ESF_TICK,
    CDN_ESF_REQ_WILL_CHECK,
    CDN_ESF_REQ_TGH_CHECK,
    CDN_ESF_REQ_AG_CHECK,
    CDN_ESF_REQ_CHEM_USE_CHECK,
    CDN_ESF_INACTIVE_IF_LESS_PRIORITY,
    CDN_ESF_DMG_TYPE_ENERGY,
    CDN_ESF_DMG_TYPE_IMPACT,
    CDN_ESF_DMG_TYPE_RENDING,
    CDN_ESF_DMG_TYPE_EXPLODING,
    CDN_ESF_RES_TYPE_HEAT,
    CDN_ESF_RES_TYPE_COLD,
    CDN_ESF_RES_TYPE_FEAR,
    CDN_ESF_RES_TYPE_POISONS,
    CDN_ESF_RES_TYPE_PSYCHIC,
    CDN_ESF_MODIFY_BASE,
    CDN_ESF_MAX,
};

enum cdn_priority {
    CDN_PRIORITY_NONE,
    CDN_PRIORITY_VERY_LOW,
    CDN_PRIORITY_LOW,
    CDN_PRIORITY_AVERAGE,
    CDN_PRIORITY_MEDIUM,
    CDN_PRIORITY_HIGH,
    CDN_PRIORITY_VERY_HIGH,
    CDN_PRIORITY_PERMANENT,
};

enum cdn_damage {
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

    enum cdn_priority priority;
    int8_t damage;
    int8_t difficulty;

    /* duration in turns, will be converted to energy when created */
    uint16_t tick_energy_max;

    uint16_t tick_energy;
};

struct cdn_condition_list;

struct cdn_condition {
    uint32_t condition_pre;

    uint32_t uid;
    uint32_t template_id;

    uint16_t setting_flags;
    int8_t difficulty;

    const char *name;
    const char *description;

    struct condition_effect_struct effects[CONDITION_MAX_NR_EFFECTS];

    /*duration in turns, will be converted to energy when created. */
    uint16_t duration_energy_min;
    uint16_t duration_energy_max;

    uint16_t duration_energy;

    const char *on_apply_plr;
    const char *on_apply_msr;
    const char *on_exit_plr;
    const char *on_exit_msr;

    enum cdn_ids continues_to_id;

    uint32_t condition_post;
};

struct cdn_condition_list *cdn_list_init(void);
void cdn_list_exit(struct cdn_condition_list *cdn_list);
bool cdn_verify_list(struct cdn_condition_list *cdn_list);
int cdn_list_size(struct cdn_condition_list *cdn_list);
struct cdn_condition *cdn_list_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev);
void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster);

bool cdn_verify_condition(struct cdn_condition *cdn);

struct cdn_condition *cdn_get_condition_tid(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
bool cdn_condition_has_effect(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);
enum cdn_priority cdn_condition_effect_priority(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);
int cdn_condition_effect_damage(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);

bool cdn_add_condition(struct cdn_condition_list *cdn_list, enum cdn_ids tid);
bool cdn_remove_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *c);

#endif /* CONDITIONS_H */
