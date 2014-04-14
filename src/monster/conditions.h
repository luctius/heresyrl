#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "heresyrl_def.h"

enum condition_setting_flags {
    CDN_SF_ACTIVE,
    CDN_SF_PERMANENT,
    CDN_SF_TICK,
    CDN_SF_VISIBLE,
    CND_SF_REQ_WILL_CHECK,
    CND_SF_REQ_TGH_CHECK,
    CND_SF_INACTIVE_IF_LESS_SEVERITY,
    CDN_SF_MAX,
};

enum condition_effect_flags {
    CND_EF_FATIQUE_INCREASE,
    CND_EF_FATIQUE_DECREASE,
    CDN_EF_MOVEMENT_REDUCE,
    CDN_EF_MOVEMENT_INCREASE,
    CDN_EF_REDUCE_WS,
    CDN_EF_REDUCE_BS,
    CDN_EF_REDUCE_STR,
    CDN_EF_REDUCE_AG,
    CDN_EF_REDUCE_TGH,
    CDN_EF_REDUCE_PER,
    CDN_EF_REDUCE_WILL,
    CDN_EF_REDUCE_INT,
    CDN_EF_REDUCE_FEL,
    CDN_EF_INCREASE_WS,
    CDN_EF_INCREASE_BS,
    CDN_EF_INCREASE_STR,
    CDN_EF_INCREASE_AG,
    CDN_EF_INCREASE_TGH,
    CDN_EF_INCREASE_PER,
    CDN_EF_INCREASE_WILL,
    CDN_EF_INCREASE_INT,
    CDN_EF_INCREASE_FEL,
    CDN_EF_DISABLE_LLEG,
    CDN_EF_DISABLE_RLEG,
    CDN_EF_DISABLE_LARM,
    CDN_EF_DISABLE_RARM,
    CDN_EF_DISABLE_CHEST,
    CDN_EF_DISABLE_HEAD,
    CDN_EF_DAMAGE_TICK,
    CDN_EF_HEALTH_TICK,
    CDN_EF_BLINDNESS,
    CDN_EF_DEAFNESS,
    CDN_EF_STUNNED,
    CDN_EF_HALUCINATING,
    CDN_EF_CONFUSED,
    CDN_EF_INHIBIT_FATE_POINT,
    CDN_EF_DEATH,
    CDN_EF_MAX,
};

struct condition_effect_struct {
    uint16_t setting_flags;
    uint16_t severity;
    uint16_t duration_energy_min;
    uint16_t duration_energy_max;
    uint16_t tick_energy_max;

    uint16_t duration_energy;
    uint16_t tick_energy;
};

struct cdn_condition_list;

struct cdn_condition {
    uint32_t condition_pre;

    uint32_t uid;
    uint32_t template_id;

    struct condition_effect_struct effects[CDN_EF_MAX];

    const char *description;
    const char *on_apply_plr;
    const char *on_apply_msr;
    const char *on_tick_plr;
    const char *on_tick_msr;
    const char *on_exit_plr;
    const char *on_exit_msr;

    uint32_t post_pre;
};

struct cdn_condition_list *cdn_list_init(void);
void cdn_list_exit(struct cdn_condition_list *cdn_list);
int cdn_list_size(struct cdn_condition_list *cdn_list);
void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster);

struct cdn_condition *cdn_create(uint32_t tid);
void cdn_destroy(struct cdn_condition *cdn);
bool cdn_verify_condition(struct cdn_condition *cdn);

struct cdn_condition *cdn_has_condition_uid(struct cdn_condition_list *cdn_list, uint32_t uid);
struct cdn_condition *cdn_has_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid);
int cdn_condition_effect_severity(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect);

bool cdn_add_condition(struct cdn_condition_list *cdn_list, uint32_t tid);
bool cdn_remove_condition(struct cdn_condition_list *cdn_list, uint32_t uid);

/* Peek at items. if prev is NULL, it gives the first item, otherwise it gives the item after prev. */
struct cdn_condition *cdn_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev);

char *cdn_get_description(struct cdn_condition *cdn);

#endif /* CONDITIONS_H */
