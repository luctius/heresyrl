#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "heresyrl_def.h"

enum condition_type {
    CDN_NONE,
    CDN_LEFT_LEG,
    CDN_RIGHT_LEG,
    CDN_LEFT_ARM,
    CDN_RIGHT_ARM,
    CDN_CHEST,
    CDN_HEAD,
    CDN_BLIND,
    CDN_DEAF,
    CDN_POISON,
    CDN_STUNNED,
    CDN_BLOOD_LOSS,
    CDN_HALLUCINATING,
    CDN_ON_FIRE,
    CDN_DRUGGED,
    CDN_WITHDRAWL,
    CDN_INVISIBLE,
    CDN_MAX,
};

/* 
   a lower severity condition prolongs an existing higher severity condition,
   but an higher severity condition replaces a lower severity condition.
 */
enum condition_severity {
    CDN_CS_PUNY,
    CDN_CS_SLIGHT,
    CDN_CS_AVERAGE,
    CDN_CS_VERY,
    CDN_CS_TERRIBLE,
    CDN_CS_MAX,
    CDN_CS_NONE,
};

enum condition_flags {
    CDN_CF_NONE         = 0,
    CDN_CF_PERMANENT    = (1<<0),
    CDN_CF_TICK         = (1<<1),
    CDN_CF_VISIBLE      = (1<<2),
    CDN_CF_MAX          = (1<<2)+1,
};

struct cdn_condition_list;

struct cdn_condition;
typedef bool (*condition_callback_t)(struct cdn_condition *con, struct msr_monster *monster);

struct cdn_condition {
    uint32_t condition_pre;

    uint32_t uid;
    uint32_t template_id;

    enum condition_type type;
    enum condition_severity severity;
    bitfield_t flags;

    int duration_energy;
    int duration_energy_max;
    int tick_energy;
    int tick_energy_max;

    condition_callback_t on_apply;
    condition_callback_t on_exit;
    condition_callback_t on_tick;

    const char *description;

    uint32_t post_pre;
};

struct cdn_condition_list *cdn_list_init(void);
void cdn_list_exit(struct cdn_condition_list *cdn_list);
int cdn_condition_list_size(struct cdn_condition_list *cdn_list);
void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster);

struct cdn_condition *cdn_create(uint32_t tid);
void cdn_destroy(struct cdn_condition *cdn);
bool cdn_verify_condition(struct cdn_condition *cdn);

struct cdn_condition *cdn_has_condition_type(struct cdn_condition_list *cdn_list, enum condition_type type);
struct cdn_condition *cdn_has_condition_uid(struct cdn_condition_list *cdn_list, uint32_t uid);
struct cdn_condition *cdn_has_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid);

bool cdn_has_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn);
bool cdn_has_condition_type(struct cdn_condition_list *cdn_list, enum condition_type type);
bool cdn_has_condition_uid(struct cdn_condition_list *cdn_list, uint32_t uid);
bool cdn_has_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid);

bool cdn_add_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn);
bool cdn_remove_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn);

/* Peek at items. if prev is NULL, it gives the first item, otherwise it gives the item after prev. */
struct cdn_condition *cdn_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev);

char *cdn_get_description(struct cdn_condition *cdn);

#endif /* CONDITIONS_H */
