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

#ifndef STATUS_EFFECTS_H
#define STATUS_EFFECTS_H

#include "heresyrl_def.h"
#include "enums.h"
#include "status_effects/status_effects_static.h"
#include "monster/monster.h"

#define STATUS_EFFECT_MAX_NR_EFFECTS    5
#define STATUS_EFFECT_CRITICAL_MAX      10
#define STATUS_EFFECT_CRITICAL_RATIO    0.5f
hrl_static_assert((STATUS_EFFECT_CRITICAL_MAX * STATUS_EFFECT_CRITICAL_RATIO) == STATUS_EFFECT_MAX_NR_EFFECTS, "Status Effect Maximums do not make sense.");

enum status_effect_flags {
    /* The status_effect is active and should be processed. */
    SEF_ACTIVE,

    /* The status_effect will be permanent (if it passes the initial checks) */
    SEF_PERMANENT,

    /* Only one  instance of this status_effect id will be permitted inside a list */
    SEF_UNIQUE,

    /* If true, there will be no textual updates not will
       this be visible in a character sheet */
    SEF_INVISIBLE,

    SEF_MAX,
};

enum status_effect_effect_flags {
    EF_NONE,

    EF_SET_CHAR,            /* param == enum msr_characteristic */

    EF_MODIFY_FATIQUE,
    EF_MODIFY_CHAR,       /* param == enum msr_characteristic */
    EF_MODIFY_SKILL,      /* param == enum msr_skills */
    EF_MODIFY_MAX_WOUNDS,

    EF_DAMAGE,
    EF_HEALTH,

    EF_ALLY,                /* param == ally_faction */
    EF_ENEMY,               /* param == ally_faction */
    EF_BLEEDING,
    EF_BLINDED,
    EF_BROKEN,
    EF_CONFUSED,
    EF_COWERING,
    EF_DAZED,
    EF_DAZZLED,
    EF_DEAD,
    EF_DEAFENED,
    EF_DISABLED_LLEG,
    EF_DISABLED_RLEG,
    EF_DISABLED_LARM,
    EF_DISABLED_RARM,
    EF_DISABLED_EYE,
    EF_ENCUMBERED,
    EF_ENTANGLED,
    EF_EXHAUSTED,
    EF_FLAT_FOOTED,
    EF_FRIGHTENED,
    EF_GRAPPLED,
    EF_HELPLESS,
    EF_INHIBIT_FATE_POINT,
    EF_INVISIBLE,
    EF_NAUSEATED,
    EF_ON_FIRE,
    EF_PANICKED,
    EF_PARALYZED,
    EF_PETRIFIED,
    EF_POISON,
    EF_PINNED,
    EF_PRONE,
    EF_SHAKEN,
    EF_SICKENED,
    EF_SWIMMING,
    EF_STABLE,
    EF_STAGGERED,
    EF_STUNNED,
    EF_SUMMONED,
    EF_UNCONSCIOUS,

    EF_INSTANT_DEATH,
    EF_EXPLODE,

    EF_TALENT,  /* param == TLT_... */
    EF_SKILL,   /* param == MSR_SKILLS_... */
    EF_TRAIT,   /* param == CTRTRTS_... */

    EF_EVOLVES  /* param == SEID_... , always happend at the end. */,

    EF_MAX,
};

enum status_effect_setting_flags {
    /* if set, this effect is used. */
    EF_SETT_ACTIVE,

    /* if true, this effect has a certain tick timer. */
    EF_SETT_TICK,

    /* When a Beneficial effect requires an skill/char check, failing it will
       set the effect to inactive.  */
    EF_SETT_BENEFICIAL,

    /* Mutual Exclusive Damage types */
    EF_SETT_DMG_TYPE_ARROW,
    EF_SETT_DMG_TYPE_BLUNT,
    EF_SETT_DMG_TYPE_BULLET,
    EF_SETT_DMG_TYPE_CLAW,
    EF_SETT_DMG_TYPE_CUTTING,
    EF_SETT_DMG_TYPE_ENERGY,
    EF_SETT_DMG_TYPE_PIERCING,
    EF_SETT_DMG_TYPE_SHRAPNEL,
    EF_SETT_DMG_TYPE_UNARMED,

    EF_SETT_HITLOC_RANDOM,

    EF_SETT_MAX,
};

enum se_strength {
    EF_STRENGTH_1D5  = 100,
    EF_STRENGTH_1D10 = 101,
    EF_STRENGTH_2D10 = 102,
    EF_STRENGTH_3D10 = 103,
    EF_STRENGTH_4D10 = 104,
};

enum se_check_flags {
    EF_CHECK_ACTIVE,

    /* Mutual exclusive check type. */
    EF_CHECK_CHARACTERISTIC,
    EF_CHECK_SKILL,

    EF_CHECK_EACH_INTERVAL,
    EF_CHECK_BENEFICIAL, /* Effect is active on success instead of failure. */

    EF_CHECK_MAX,
};

enum se_heal_flags {
    EF_HEAL_ACTIVE,
    EF_HEAL_MAGIC_ONLY,
};

struct se_type_struct {
    enum status_effect_effect_flags effect;
    bitfield32_t effect_setting_flags;

    int8_t strength;
    int param;

    const char *on_tick_plr;
    const char *on_tick_msr;

    /* operational variables. */
    int16_t tick_interval_energy;
    int16_t tick_energy;
    int16_t ticks_applied;
    int16_t ticks_max;
};

struct status_effect_list;

struct status_effect {
    uint32_t status_effect_pre;

    uint32_t uid;
    enum se_ids template_id;

    const char *name;
    const char *description;
    const char *origin;
    const char *on_first_plr;
    const char *on_first_msr;
    const char *on_exit_plr;
    const char *on_exit_msr;

    bitfield32_t setting_flags;

    bitfield32_t check_flags;
    int check_type;
    int8_t check_difficulty;

    bitfield32_t heal_flags;
    int8_t heal_difficulty;
    enum se_ids heal_evolve_tid;

    struct se_type_struct effects[STATUS_EFFECT_MAX_NR_EFFECTS];

    /* operational variables */
    int duration_energy_min;
    int duration_energy_max;
    int duration_energy;

    int heal_cost;

    uint32_t status_effect_post;
};

void se_init(void);
struct status_effect *selst_get_next_status_effect(struct status_effect *prev);
struct status_effect *selst_status_effect_by_uid(uint32_t status_effect_uid);
void se_exit(void);

struct status_effect_list *se_list_init(void);
void se_list_exit(struct status_effect_list *se_list);
bool se_verify_list(struct status_effect_list *se_list);
int se_list_size(struct status_effect_list *se_list);
struct status_effect *se_list_get_next_status_effect(struct status_effect_list *se_list, struct status_effect *prev);
void se_process(struct msr_monster *monster);
void se_remove_all_non_permanent(struct msr_monster *monster);

bool se_verify_status_effect(struct status_effect *se);

struct status_effect *se_get_status_effect_tid(struct msr_monster *monster, enum se_ids tid);
bool se_has_non_healable_permanent_effect(struct msr_monster *monster, enum status_effect_effect_flags effect);
bool se_has_flag(struct status_effect *se, enum status_effect_flags flag);
bool se_has_effect(struct msr_monster *monster, enum status_effect_effect_flags effect);
bool se_has_tid(struct status_effect_list *se_list, enum se_ids tid);
int se_status_effect_strength(struct msr_monster *monster, enum status_effect_effect_flags effect, int param); /* set param to -1 for any. */
bool se_add_energy(struct status_effect *se, int32_t energy);

struct status_effect *se_create(enum se_ids tid); /* Do NOT use directly unless you know what you are doing. */
bool se_add_to_list(struct msr_monster *monster, struct status_effect *con);

bool se_add_status_effect(struct msr_monster *monster, uint32_t tid, const char *origin);
bool se_remove_status_effect(struct msr_monster *monster, struct status_effect *con);
bool se_remove_effects_by_tid(struct msr_monster *monster, uint32_t tid);

bool se_heal_status_effect(struct msr_monster *monster, struct msr_monster *healer, struct status_effect *con, bool magic);

bool se_add_critical_hit(struct msr_monster *monster, int critical_dmg, enum msr_hit_location mhl, enum dmg_type type);

void se_dbg_check_all(void);


static inline bool effect_set_flag(struct se_type_struct *ces, enum status_effect_setting_flags flag) {
    return set_bf(ces->effect_setting_flags, flag);
}
static inline bool effect_clr_flag(struct se_type_struct *ces, enum status_effect_setting_flags flag) {
    return clr_bf(ces->effect_setting_flags, flag);
}
static inline bool effect_has_flag(struct se_type_struct *ces, enum status_effect_setting_flags flag) {
    return test_bf(ces->effect_setting_flags, flag);
}

static inline bool check_effect_set_flag(struct status_effect *c, enum se_check_flags flag) {
    return set_bf(c->check_flags, flag);
}
static inline bool check_effect_clr_flag(struct status_effect *c, enum se_check_flags flag) {
    return clr_bf(c->check_flags, flag);
}
static inline bool check_effect_has_flag(struct status_effect *c, enum se_check_flags flag) {
    return test_bf(c->check_flags, flag);
}
static inline bool effect_heal_set_flag(struct status_effect *c, enum se_heal_flags flag) {
    return set_bf(c->heal_flags, flag);
}
static inline bool effect_heal_clr_flag(struct status_effect *c, enum se_heal_flags flag) {
    return clr_bf(c->heal_flags, flag);
}
static inline bool effect_heal_has_flag(struct status_effect *c, enum se_heal_flags flag) {
    return test_bf(c->heal_flags, flag);
}


static inline bool status_effect_set_flag(struct status_effect *c, enum status_effect_flags flag) {
    return set_bf(c->setting_flags, flag);
}
static inline bool status_effect_clr_flag(struct status_effect *c, enum status_effect_flags flag) {
    return clr_bf(c->setting_flags, flag);
}
static inline bool status_effect_has_flag(struct status_effect *c, enum status_effect_flags flag) {
    return test_bf(c->setting_flags, flag);
}

#endif /* STATUS_EFFECTS_H */
