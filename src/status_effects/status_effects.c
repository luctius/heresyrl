#include <assert.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <string.h>

#include "status_effects.h"
#include "monster/monster.h"
#include "logging.h"
#include "heresyrl_def.h"
#include "random.h"
#include "turn_tick.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "dungeon/dungeon_map.h"

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

struct se_entry {
    struct status_effect status_effect;
    LIST_ENTRY(se_entry) entries;
};

struct status_effect_list {
    uint32_t pre_check;
    LIST_HEAD(se_list_head, se_entry) head;
    uint32_t post_check;
};

struct status_effect_list_entry {
    struct se_entry se;
    TAILQ_ENTRY(status_effect_list_entry) entries;
};
static TAILQ_HEAD(status_effects_list, status_effect_list_entry) status_effects_list_head;
static bool status_effects_list_initialised = false;

#include "status_effects_static.c"

/* Garanteed to be random, rolled it myself ;)  */
#define STATUS_EFFECT_PRE_CHECK (68731)
#define STATUS_EFFECT_POST_CHECK (138)
#define STATUS_EFFECT_LIST_PRE_CHECK (45867)
#define STATUS_EFFECT_LIST_POST_CHECK (7182)

void se_init(void) {
    for (unsigned int i = 0; i < SEID_MAX; i++) {
        const struct status_effect *status_effect = &static_status_effect_list[i];
        if (status_effect->template_id != i) {
            fprintf(stderr, "Status Effects list integrity check failed! [%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }

    if (status_effects_list_initialised == false) {
        TAILQ_INIT(&status_effects_list_head);
        status_effects_list_initialised = true;
    }
}

void se_exit(void) {
    struct status_effect_list_entry *e = NULL;
    while (status_effects_list_head.tqh_first != NULL) {
        e = status_effects_list_head.tqh_first;
        TAILQ_REMOVE(&status_effects_list_head, status_effects_list_head.tqh_first, entries);
        free(e);
    }
    status_effects_list_initialised = false;
}

struct status_effect *selst_get_next_status_effect(struct status_effect *prev) {
    if (status_effects_list_initialised == false) return NULL;

    if (prev == NULL) {
        if (status_effects_list_head.tqh_first != NULL) return &status_effects_list_head.tqh_first->se.status_effect;
        return NULL;
    }
    struct se_entry *se = container_of(prev, struct se_entry, status_effect);
    struct status_effect_list_entry *sele = container_of(se, struct status_effect_list_entry, se);
    if (sele == NULL) return NULL;
    return &sele->entries.tqe_next->se.status_effect;
}

struct status_effect *selst_status_effect_by_uid(int status_effect_uid) {
    if (status_effects_list_initialised == false) return false;
    struct status_effect_list_entry *sele = status_effects_list_head.tqh_first;

    while (sele != NULL) {
        if (status_effect_uid == sele->se.status_effect.uid) return &sele->se.status_effect;
        sele = sele->entries.tqe_next;
    }
    return NULL;
}

static uint32_t selst_next_id(void) {
    if (status_effects_list_initialised == false) return false;
    struct status_effect_list_entry *sele = status_effects_list_head.tqh_first;
    uint32_t uid = 1;

    while (sele != NULL) {
        if (uid <= sele->se.status_effect.uid) uid = sele->se.status_effect.uid+1;
        sele = sele->entries.tqe_next;
    }
    return uid;
}

struct status_effect_list *se_list_init(void) {
    struct status_effect_list *i = calloc(1, sizeof(struct status_effect_list) );
    if (i != NULL) {
        LIST_INIT(&i->head);
        i->pre_check = STATUS_EFFECT_LIST_PRE_CHECK;
        i->post_check = STATUS_EFFECT_LIST_POST_CHECK;
    }
    return i;
}

void se_list_exit(struct status_effect_list *se_list) {
    if (se_verify_list(se_list) == false) return;

    struct se_entry *ce;
    while ( (ce = se_list->head.lh_first ) != NULL) {
        struct status_effect_list_entry *sele = container_of(ce, struct status_effect_list_entry, se);

        LIST_REMOVE(ce, entries);
        TAILQ_REMOVE(&status_effects_list_head, sele, entries);
        free(sele);
    }
    free(se_list);
}

bool se_verify_list(struct status_effect_list *se_list) {
    assert(se_list != NULL);
    assert(se_list->pre_check == STATUS_EFFECT_LIST_PRE_CHECK);
    assert(se_list->post_check == STATUS_EFFECT_LIST_POST_CHECK);
    return true;
}

/* Peek at items. if prev is NULL, it gives the first item, otherwise it gives the item after prev. */
struct status_effect *se_list_get_next_status_effect(struct status_effect_list *se_list, struct status_effect *prev) {
    if (se_verify_list(se_list) == false) return false;

    if (prev == NULL) {
        if (se_list->head.lh_first != NULL) return &se_list->head.lh_first->status_effect;
        return NULL;
    }
    struct se_entry *ce = container_of(prev, struct se_entry, status_effect);
    if (ce == NULL) return NULL;
    return &ce->entries.le_next->status_effect;
}

int se_list_size(struct status_effect_list *se_list) {
    if (se_verify_list(se_list) == false) return false;
    int i = 0;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        i++;
    }

    return i;
}

static int se_calc_strength(struct se_type_struct *ces) {
    int dmg = 0;
    switch(ces->strength) {
        default: dmg = ces->strength; break;
        case EF_STRENGTH_1D5:      dmg = random_xd5(gbl_game->random, 1); break;
        case EF_STRENGTH_1D10:     dmg = random_xd10(gbl_game->random, 1); break;;
        case EF_STRENGTH_2D10:     dmg = random_xd10(gbl_game->random, 2); break;
        case EF_STRENGTH_3D10:     dmg = random_xd10(gbl_game->random, 3); break;
        case EF_STRENGTH_4D10:     dmg = random_xd10(gbl_game->random, 4); break;
    }

    return dmg;
}

struct status_effect *se_create(enum se_ids tid) {
    if (tid == SEID_NONE) return false;
    if (tid >= SEID_MAX) return false;
    if (tid >= (int) ARRAY_SZ(static_status_effect_list) ) return false;

    const struct status_effect *se_template = &static_status_effect_list[tid];
    assert(se_template != NULL);

    struct status_effect_list_entry *sele = malloc(sizeof(struct status_effect_list_entry) );
    assert(sele != NULL);

    struct se_entry *ce = &sele->se;
    assert(ce != NULL);

    memcpy(&ce->status_effect, se_template, sizeof(struct status_effect) );
    TAILQ_INSERT_TAIL(&status_effects_list_head, sele, entries);
    struct status_effect *cc = &ce->status_effect;

    cc->status_effect_pre = STATUS_EFFECT_PRE_CHECK;
    cc->status_effect_post = STATUS_EFFECT_POST_CHECK;

    cc->uid = selst_next_id();

    int range = (cc->duration_energy_max - cc->duration_energy_min);
    cc->duration_energy = cc->duration_energy_min;
    if (range > 0) cc->duration_energy += (random_int32(gbl_game->random) % range);
    if (cc->duration_energy == 0) cc->duration_energy = 1;
    lg_debug("Creating se: %p(%s) duration: %d, max: %d", cc, cc->name, cc->duration_energy, cc->duration_energy_max);
    cc->duration_energy_max = cc->duration_energy;

    for (unsigned int i = 0; i < ARRAY_SZ(cc->effects); i++ ) {
        struct se_type_struct *ces = &cc->effects[i];
        if (effect_has_flag(ces, EF_SETT_ACTIVE) ) {
            ces->strength = se_calc_strength(ces);
        }
    }

    assert(se_verify_status_effect(cc) );
    return cc;
}

static void se_process_effect(struct msr_monster *monster, struct status_effect *c);

bool se_add_to_list(struct msr_monster *monster, struct status_effect *con) {
    if (msr_verify_monster(monster) == false) return false;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return false;
    if (se_verify_status_effect(con) == false) return false;

    lg_debug("Adding status_effect: %p(%s)", con, con->name);

    struct se_entry *ce = container_of(con, struct se_entry, status_effect);
    LIST_INSERT_HEAD(&se_list->head, ce, entries);

    se_process_effect(monster, con);

    return true;
}

bool se_add_status_effect(struct msr_monster *monster, uint32_t tid, const char *origin) {
    if (msr_verify_monster(monster) == false) return false;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return false;
    if (tid == SEID_NONE) return false;
    if (tid >= ARRAY_SZ(static_status_effect_list) ) return false;
    struct status_effect *c = NULL;

    /* Check if the status effect does allready exist in this list. */
    if (se_has_tid(se_list, tid) ) {
        c = se_get_status_effect_tid(se_list, tid);
        if (c != NULL) {

            /* If the condition is unique */
            if (status_effect_has_flag(c, SEF_UNIQUE) == true) {
                /* and permanent, we do nothing */
                if (status_effect_has_flag(c, SEF_PERMANENT) ) {
                    return c->uid;
                }
                
                /* if not permanent but is unique, then we restart the duration. */
                lg_debug("Restarting status_effect: %p(%s)", c, c->name);
                /* restart status_effect */
                c->duration_energy = c->duration_energy_max -1;
                return c->uid;
            }
        }
    }

    /* if it does not yet exist, create an new instance. */
    c = se_create(tid);
    if (c == NULL) return false;

    c->origin = origin;
    return se_add_to_list(monster, c);
}

bool se_remove_status_effect(struct status_effect_list *se_list, struct status_effect *con) {
    if (se_verify_list(se_list) == false) return false;

    struct status_effect *cc = NULL;
    while ( (cc = se_list_get_next_status_effect(se_list, cc) ) != NULL) {
        if (cc == con) {
            struct se_entry *ce = container_of(cc, struct se_entry, status_effect);
            struct status_effect_list_entry *sele = container_of(ce, struct status_effect_list_entry, se);

            LIST_REMOVE(ce, entries);
            TAILQ_REMOVE(&status_effects_list_head, sele, entries);
            free(sele);

            return true;
        }
    }

    return false;
}

bool se_remove_effects_by_tid(struct status_effect_list *se_list, uint32_t tid) {
    if (se_verify_list(se_list) == false) return false;

    bool found = false;
    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        if (c->template_id == tid) {
            se_remove_status_effect(se_list, c);
            found = true;
        }
    }
    return found;
}

bool se_verify_status_effect(struct status_effect *se) {
    assert(se != NULL);
    assert(se->status_effect_pre == STATUS_EFFECT_PRE_CHECK);
    assert(se->status_effect_post == STATUS_EFFECT_POST_CHECK);

    return true;
}

bool se_has_flag(struct status_effect *se, enum status_effect_flags flag) {
    if (se_verify_status_effect(se) == false) return false;
    return status_effect_has_flag(se, flag);
}

bool se_has_tid(struct status_effect_list *se_list, enum se_ids tid) {
    if (se_verify_list(se_list) == false) return false;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        if (c->template_id == tid) {
            return true;
        }
    }
    return false;
}

bool se_has_non_healable_permanent_effect(struct status_effect_list *se_list, enum status_effect_effect_flags effect) {
    if (se_verify_list(se_list) == false) return false;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                if (status_effect_has_flag(c, SEF_PERMANENT) ) {
                    if (effect_heal_has_flag(c, EF_HEAL_ACTIVE) == false) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

struct status_effect *se_get_status_effect_tid(struct status_effect_list *se_list, enum se_ids tid) {
    if (se_verify_list(se_list) == false) return NULL;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        if (c->template_id == tid) {
            return c;
        }
    }
    return NULL;
}

/* Check if there is another status_effect with the same effect besides the one given */
bool se_has_effect_skip(struct status_effect_list *se_list, enum status_effect_effect_flags effect, struct status_effect *status_effect) {
    if (se_verify_list(se_list) == false) return false;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        if ( (status_effect != NULL) && (status_effect == c) ) continue;

        for (unsigned int  i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                return true;
            }
        }
    }

    return false;
}

bool se_has_effect(struct status_effect_list *se_list, enum status_effect_effect_flags effect) {
    return se_has_effect_skip(se_list, effect, NULL);
}

int se_status_effect_strength(struct status_effect_list *se_list, enum status_effect_effect_flags effect, int param) {
    if (se_verify_list(se_list) == false) return -1;
    struct status_effect *c = NULL;

    int strength = 0;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                if (param != -1 && c->effects[i].param == param) {
                    strength += c->effects[i].strength;
                }
            }
        }
    }

    return strength;
}

/*
   This lookup table retrieves the starting entry of the 
   critical hit table. The damage is then added to that 
   to get to the correct entry.
*/
static const enum se_ids dmg_type_to_id_lot[MSR_HITLOC_MAX][DMG_TYPE_MAX] = {
    [MSR_HITLOC_LEFT_LEG] = { 
        [DMG_TYPE_ARROW]    = SEID_NONE,
        [DMG_TYPE_BLUNT]    = SEID_NONE,
        [DMG_TYPE_BULLET]   = SEID_NONE,
        [DMG_TYPE_CLAW]     = SEID_NONE,
        [DMG_TYPE_CUTTING]  = SEID_NONE,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_NONE,
        [DMG_TYPE_SHRAPNEL] = SEID_NONE,
        [DMG_TYPE_UNARMED]  = SEID_NONE,
    },
    [MSR_HITLOC_RIGHT_LEG] = { 
        [DMG_TYPE_ARROW]    = SEID_NONE,
        [DMG_TYPE_BLUNT]    = SEID_NONE,
        [DMG_TYPE_BULLET]   = SEID_NONE,
        [DMG_TYPE_CLAW]     = SEID_NONE,
        [DMG_TYPE_CUTTING]  = SEID_NONE,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_NONE,
        [DMG_TYPE_SHRAPNEL] = SEID_NONE,
        [DMG_TYPE_UNARMED]  = SEID_NONE,
    },
    [MSR_HITLOC_LEFT_ARM] = { 
        [DMG_TYPE_ARROW]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_BLUNT]    = SEID_BLUNT_LARM_1,
        [DMG_TYPE_BULLET]   = SEID_BLUNT_LARM_1,
        [DMG_TYPE_CLAW]     = SEID_BLUNT_LARM_1,
        [DMG_TYPE_CUTTING]  = SEID_BLUNT_LARM_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_SHRAPNEL] = SEID_BLUNT_LARM_1,
        [DMG_TYPE_UNARMED]  = SEID_BLUNT_LARM_1,
    },
    [MSR_HITLOC_RIGHT_ARM] = { 
        [DMG_TYPE_ARROW]    = SEID_BLUNT_RARM_1,
        [DMG_TYPE_BLUNT]    = SEID_BLUNT_RARM_1,
        [DMG_TYPE_BULLET]   = SEID_BLUNT_RARM_1,
        [DMG_TYPE_CLAW]     = SEID_BLUNT_RARM_1,
        [DMG_TYPE_CUTTING]  = SEID_BLUNT_RARM_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_BLUNT_RARM_1,
        [DMG_TYPE_SHRAPNEL] = SEID_BLUNT_RARM_1,
        [DMG_TYPE_UNARMED]  = SEID_BLUNT_RARM_1,
    },
    [MSR_HITLOC_BODY] = { 
        [DMG_TYPE_ARROW]    = SEID_NONE,
        [DMG_TYPE_BLUNT]    = SEID_NONE,
        [DMG_TYPE_BULLET]   = SEID_NONE,
        [DMG_TYPE_CLAW]     = SEID_NONE,
        [DMG_TYPE_CUTTING]  = SEID_NONE,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_NONE,
        [DMG_TYPE_SHRAPNEL] = SEID_NONE,
        [DMG_TYPE_UNARMED]  = SEID_NONE,
    },
    [MSR_HITLOC_HEAD] = { 
        [DMG_TYPE_ARROW]    = SEID_NONE,
        [DMG_TYPE_BLUNT]    = SEID_NONE,
        [DMG_TYPE_BULLET]   = SEID_NONE,
        [DMG_TYPE_CLAW]     = SEID_NONE,
        [DMG_TYPE_CUTTING]  = SEID_NONE,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_NONE,
        [DMG_TYPE_SHRAPNEL] = SEID_NONE,
        [DMG_TYPE_UNARMED]  = SEID_NONE,
    },
};

bool se_add_critical_hit(struct msr_monster *monster, int dmg, enum msr_hit_location mhl, enum dmg_type type) {
    if (msr_verify_monster(monster) == false) return false;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return false;

    enum se_ids tid = dmg_type_to_id_lot[mhl][type];
    if (tid == SEID_NONE) return false;

    if (dmg > (STATUS_EFFECT_CRITICAL_MAX) ) dmg = (STATUS_EFFECT_CRITICAL_MAX);
    int crit_effect = tid +(dmg * STATUS_EFFECT_CRITICAL_RATIO);

    switch(mhl) {
        case MSR_HITLOC_LEFT_LEG: 
            if (se_has_non_healable_permanent_effect(monster->status_effects, EF_DISABLED_LLEG) ) return false;
        case MSR_HITLOC_RIGHT_LEG:
            if (se_has_non_healable_permanent_effect(monster->status_effects, EF_DISABLED_RLEG) ) return false;
        case MSR_HITLOC_LEFT_ARM:
            if (se_has_non_healable_permanent_effect(monster->status_effects, EF_DISABLED_LARM) ) return false;
        case MSR_HITLOC_RIGHT_ARM:
            if (se_has_non_healable_permanent_effect(monster->status_effects, EF_DISABLED_RARM) ) return false;
        case MSR_HITLOC_BODY: break;
        case MSR_HITLOC_HEAD: break;
    }

    /* TODO: update this when more critical hits become available */
    return se_add_status_effect(monster, crit_effect, "critical");
}

void se_process_effects_first(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    switch(ces->effect) {
        case EF_NONE: break;
        case EF_MAX: break;

        case EF_ALLY: break;
        case EF_BLEEDING: break;
        case EF_BLINDED: break;
        case EF_BROKEN: break;
        case EF_CONFUSED: break;
        case EF_COWERING: break;
        case EF_DAMAGE: break;
        case EF_DAZED: break;
        case EF_DAZZLED: break;
        case EF_DEAD: break;
        case EF_DEAFENED: break;
        case EF_DISABLED_LLEG: break;
        case EF_DISABLED_RLEG: break;
        case EF_DISABLED_EYE: break;
        case EF_ENTANGLED: break;
        case EF_EXHAUSTED: break;
        case EF_FLAT_FOOTED: break;
        case EF_FRIGHTENED: break;
        case EF_GRAPPLED: break;
        case EF_HEALTH: break;
        case EF_HELPLESS: break;
        case EF_INHIBIT_FATE_POINT: break;
        case EF_INVISIBLE: break;
        case EF_NAUSEATED: break;
        case EF_ON_FIRE: break;
        case EF_PANICKED: break;
        case EF_PARALYZED: break;
        case EF_PETRIFIED: break;
        case EF_POISON: break;
        case EF_PINNED: break;
        case EF_PRONE: break;
        case EF_SHAKEN: break;
        case EF_SICKENED: break;
        case EF_SINKING: break;
        case EF_STABLE: break;
        case EF_STAGGERED: break;
        case EF_STUNNED: break;
        case EF_SUMMONED: break;
        case EF_UNCONSCIOUS: break;
        case EF_SKILL: break;
        case EF_EVOLVES: break;

        case EF_DECREASE_FATIQUE: break;
        case EF_INCREASE_FATIQUE: break;
        case EF_DECREASE_MAX_WOUNDS: break;
        case EF_INCREASE_MAX_WOUNDS: break;
        case EF_DECREASE_CHAR: break;
        case EF_INCREASE_CHAR: break;
        case EF_DECREASE_SKILL: break;
        case EF_INCREASE_SKILL: break;

        case EF_TALENT:
            if (msr_has_talent(monster, ces->param) ) {
                effect_clr_flag(ces, EF_SETT_ACTIVE);
            }
            else msr_set_talent(monster, ces->param);
            break;
        case EF_TRAIT:
            if (msr_has_creature_trait(monster, ces->param) ) {
                effect_clr_flag(ces, EF_SETT_ACTIVE);
            }
            else msr_set_creature_trait(monster, ces->param);
            break;

        case EF_SET_CHAR: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[ces->param].base_value;
            monster->characteristic[ces->param].base_value = strength;
        } break;

        case EF_DISABLED_LARM:
        case EF_DISABLED_RARM:
            {
                uint32_t location = INV_LOC_MAINHAND_WIELD;
                uint32_t o_location = INV_LOC_OFFHAND_WIELD;

                if (ces->effect == EF_DISABLED_LARM) {
                    location = INV_LOC_OFFHAND_WIELD;
                    o_location = INV_LOC_MAINHAND_WIELD;
                }

                struct itm_item *item = inv_get_item_from_location(monster->inventory, location);
                if (item == inv_get_item_from_location(monster->inventory, o_location) ) {
                    /* 2 handed item */
                    if (inv_move_item_to_location(monster->inventory, item, INV_LOC_NONE) ) {
                        You(monster, "are unable to wield %s.", item->ld_name);
                        Monster(monster, "is unable to wield %s.", item->ld_name, msr_gender_name(monster, true));
                    }
                }
                else {
                    /* one handed item, lets drop it. */
                    if (msr_remove_item(monster, item) == true) {
                        itm_insert_item(item, gbl_game->current_map, &monster->pos);

                        You(monster, "let go of %s.", item->ld_name);
                        Monster(monster, "droppes %s from %s hand.", item->ld_name, msr_gender_name(monster, true));
                    }
                }
                inv_disable_location(monster->inventory, location);
                msr_weapon_next_selection(monster);
            } break;

        case EF_EXPLODE: {
            /* place bodypart grenade here */

            /* create item */
            struct itm_item *item = itm_create(IID_BODYPART_GRENADE);

            /* TODO: change name of bodypart */
            item->ld_name = "something";

            /* light fuse */
            item->energy = TT_ENERGY_TICK;
            item->energy_action = true;

            /* put item in map at the tile of monster */
            itm_insert_item(item, gbl_game->current_map, &monster->pos);
        } /* No Break */
        case EF_INSTANT_DEATH: {
            effect_clr_flag(ces, EF_SETT_ACTIVE);
            msr_die(monster, gbl_game->current_map);
        } break;

        default: effect_clr_flag(ces, EF_SETT_ACTIVE); break;
    }
}

void se_process_effects_last(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    int mod = -1;
    switch(ces->effect) {
        case EF_NONE: break;
        case EF_MAX: break;

        case EF_ALLY: break;
        case EF_BLEEDING: break;
        case EF_BLINDED: break;
        case EF_BROKEN: break;
        case EF_CONFUSED: break;
        case EF_COWERING: break;
        case EF_DAZED: break;
        case EF_DAZZLED: break;
        case EF_DEAD: break;
        case EF_DEAFENED: break;
        case EF_DISABLED_LLEG: break;
        case EF_DISABLED_RLEG: break;
        case EF_DISABLED_EYE: break;
        case EF_ENTANGLED: break;
        case EF_EXHAUSTED: break;
        case EF_FLAT_FOOTED: break;
        case EF_FRIGHTENED: break;
        case EF_GRAPPLED: break;
        case EF_HELPLESS: break;
        case EF_INHIBIT_FATE_POINT: break;
        case EF_INVISIBLE: break;
        case EF_NAUSEATED: break;
        case EF_ON_FIRE: break;
        case EF_PANICKED: break;
        case EF_PARALYZED: break;
        case EF_PETRIFIED: break;
        case EF_POISON: break;
        case EF_PINNED: break;
        case EF_PRONE: break;
        case EF_SHAKEN: break;
        case EF_SICKENED: break;
        case EF_SINKING: break;
        case EF_STABLE: break;
        case EF_STAGGERED: break;
        case EF_STUNNED: break;
        case EF_SUMMONED: break;
        case EF_UNCONSCIOUS: break;
        case EF_SKILL: break;

        case EF_DECREASE_FATIQUE: break;
        case EF_INCREASE_FATIQUE: break;
        case EF_DAMAGE: break;
        case EF_HEALTH: break;
        case EF_DECREASE_SKILL: break;
        case EF_INCREASE_SKILL: break;

        case EF_EVOLVES: se_add_status_effect(monster, ces->param, c->name); break;

        case EF_TALENT:
            /* if the monster already had the talent, the effect is set to be inactive and this doesn't apply. */
            msr_clr_talent(monster, ces->param);
            break;
        case EF_TRAIT: 
            /* if the monster already had the trait, the effect is set to be inactive and this doesn't apply. */
            msr_set_creature_trait(monster, ces->param);
            break;

        case EF_SET_CHAR:
            monster->characteristic[ces->param].base_value = ces->strength;
            break;

        case EF_DECREASE_CHAR: mod = 1;
        case EF_INCREASE_CHAR:
            monster->characteristic[ces->param].mod += (ces->strength * mod) * ces->ticks_applied;
            break;

        case EF_DECREASE_MAX_WOUNDS: mod = 1;
        case EF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += (ces->strength * mod) * ces->ticks_applied;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case EF_DISABLED_RARM:
        case EF_DISABLED_LARM:
            if (se_has_effect_skip(monster->status_effects, ces->effect, c) == false) {
                inv_enable_location(monster->inventory, ces->effect == EF_DISABLED_RARM ? INV_LOC_MAINHAND_WIELD : INV_LOC_OFFHAND_WIELD);
            }
            break;

        case EF_EXPLODE:
        case EF_INSTANT_DEATH: {
            effect_clr_flag(ces, EF_SETT_ACTIVE);
            msr_die(monster, gbl_game->current_map);
        } break;

        default: effect_clr_flag(ces, EF_SETT_ACTIVE); break;
    }

    ces->ticks_applied = 0;
    ces->tick_energy = 0;
    effect_clr_flag(ces, EF_SETT_ACTIVE);
}

void se_process_effects_during(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    if (status_effect_has_flag(c, SEF_INVISIBLE) == false) {
        if (ces->on_tick_plr != NULL) You_msg(monster,      ces->on_tick_plr);
        if (ces->on_tick_msr != NULL) Monster_msg(monster,  ces->on_tick_msr, msr_ldname(monster) );
    }

    int mod = 1;
    switch(ces->effect) {
        case EF_NONE: break;
        case EF_MAX: break;

        case EF_ALLY: break;
        case EF_BLEEDING: break;
        case EF_BLINDED: break;
        case EF_BROKEN: break;
        case EF_CONFUSED: break;
        case EF_COWERING: break;
        case EF_DAZED: break;
        case EF_DAZZLED: break;
        case EF_DEAD: break;
        case EF_DEAFENED: break;
        case EF_DISABLED_LLEG: break;
        case EF_DISABLED_RLEG: break;
        case EF_DISABLED_LARM: break;
        case EF_DISABLED_RARM: break;
        case EF_DISABLED_EYE: break;
        case EF_ENTANGLED: break;
        case EF_EXHAUSTED: break;
        case EF_FLAT_FOOTED: break;
        case EF_FRIGHTENED: break;
        case EF_GRAPPLED: break;
        case EF_HELPLESS: break;
        case EF_INHIBIT_FATE_POINT: break;
        case EF_INVISIBLE: break;
        case EF_NAUSEATED: break;
        case EF_ON_FIRE: break;
        case EF_PANICKED: break;
        case EF_PARALYZED: break;
        case EF_PETRIFIED: break;
        case EF_POISON: break;
        case EF_PINNED: break;
        case EF_PRONE: break;
        case EF_SHAKEN: break;
        case EF_SICKENED: break;
        case EF_SINKING: break;
        case EF_STABLE: break;
        case EF_STAGGERED: break;
        case EF_STUNNED: break;
        case EF_SUMMONED: break;
        case EF_UNCONSCIOUS: break;
        case EF_SKILL: break;
        case EF_EVOLVES: break;

        case EF_EXPLODE: break;
        case EF_INSTANT_DEATH: break;
        case EF_SET_CHAR: break;
        case EF_DECREASE_SKILL: break;
        case EF_INCREASE_SKILL: break;
        case EF_TALENT: break;
        case EF_TRAIT: break;

        case EF_DECREASE_FATIQUE: break;
        case EF_INCREASE_FATIQUE: break;

        case EF_DECREASE_CHAR: mod = -1;
        case EF_INCREASE_CHAR:
            monster->characteristic[ces->param].mod += ces->strength * mod;
            break;

        case EF_DAMAGE: {
                int hitloc = MSR_HITLOC_NONE;
                int dmg_type = DMG_TYPE_ENERGY;
                if (effect_has_flag(ces, EF_SETT_HITLOC_RANDOM) ) hitloc = msr_get_hit_location(monster, random_d100(gbl_game->random));
                msr_do_dmg(monster, ces->strength, dmg_type, hitloc);
            } break;

        case EF_HEALTH:
            monster->cur_wounds += ces->strength;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case EF_DECREASE_MAX_WOUNDS: mod = -1;
        case EF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += ces->strength * mod;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        default: effect_clr_flag(ces, EF_SETT_ACTIVE); break;
    }

    ces->tick_energy = ces->tick_interval_energy;
    ces->ticks_applied++;
    if ( (ces->ticks_applied > ces->ticks_max) && (ces->ticks_max > 0) ) {
        se_process_effects_last(ces, monster, c);
        effect_clr_flag(ces, EF_SETT_ACTIVE);
    }
}

static void se_process_effect(struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return;
    /* 
       if the monster is dead, do nothing.
       we do this here because an effect can cause death.
     */
    if (monster->dead) return;


    bool first_time = false;
    bool last_time  = true;

    struct status_effect *c_prev = NULL;

    if (status_effect_has_flag(c, SEF_ACTIVE) ) {
        /* Check if this status_effect is new, or maybe even for the last time. */
        if (c->duration_energy == c->duration_energy_max) first_time = true;
        if ( (status_effect_has_flag(c, SEF_PERMANENT) == true) ) last_time = false;
        else if (c->duration_energy > 0) last_time = false;

        {   /* Pre checks */
            if (first_time) {
                lg_debug("Condition %p(%s) is processed for the first time.", c, c->name);

                int inactive = 0;
                for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
                    struct se_type_struct *ces = &c->effects[i];
                    if (ces->effect != EF_NONE) {
                        if (effect_has_flag(ces, EF_SETT_ACTIVE) == false) {
                            inactive++;
                        }
                    }
                }
                if (inactive == ARRAY_SZ(c->effects) ) status_effect_clr_flag(c, SEF_ACTIVE);

                if (c->template_id == SEID_NONE) status_effect_clr_flag(c, SEF_ACTIVE);

                /* Handle Checks */
                if (check_effect_has_flag(c, EF_CHECK_ACTIVE) ) {
                    bool made_check = false;

                    if (check_effect_has_flag(c, EF_CHECK_EACH_INTERVAL) == false ) {
                        if (check_effect_has_flag(c, EF_CHECK_CHARACTERISTIC) ) {
                            if ( (msr_characteristic_check(monster, c->check_type, c->check_difficulty) ) >= 1) {
                                made_check = true;
                            }
                        }
                        else if (check_effect_has_flag(c, EF_CHECK_SKILL) ) {
                            if ( (msr_skill_check(monster, c->check_type, c->check_difficulty) ) >= 1) {
                                made_check = true;
                            }
                        }

                        if (check_effect_has_flag(c, EF_CHECK_BENEFICIAL) ) {
                            if (made_check == false) status_effect_clr_flag(c, SEF_ACTIVE);
                        }
                        else if (made_check == true) status_effect_clr_flag(c, SEF_ACTIVE);
                    }
                }

                if (status_effect_has_flag(c, SEF_ACTIVE) ) {
                    if (status_effect_has_flag(c, SEF_INVISIBLE) == false) {
                        lg_debug("Condition %p(%s) is to be applyed.", c, c->name);
                        if (c->on_first_plr != NULL) You_msg(monster, c->on_first_plr);
                        if (c->on_first_msr != NULL) Monster_msg(monster, c->on_first_msr, msr_ldname(monster) );
                    }
                }
                else {
                    first_time = false;
                }
            }
            else if (last_time) {
                lg_debug("Condition %p(%s) ends.", c, c->name);

                if (status_effect_has_flag(c, SEF_INVISIBLE) == false) {
                    if (c->on_exit_plr != NULL) You_msg(monster, c->on_exit_plr);
                    if (c->on_exit_msr != NULL) Monster_msg(monster, c->on_exit_msr, msr_ldname(monster) );
                }
                status_effect_clr_flag(c, SEF_ACTIVE);
            }
        }

        for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
            struct se_type_struct *ces = &c->effects[i];
            if (effect_has_flag(ces, EF_SETT_ACTIVE) == false) continue;

            if (effect_has_flag(ces, EF_SETT_TICK) && (ces->tick_energy > 0) ) {
                ces->tick_energy -= TT_ENERGY_TICK;
                if (!first_time && !last_time) continue;
            }

            if (first_time) {
                se_process_effects_first(ces, monster, c);
                se_process_effects_during(ces, monster, c);
            }
            else if (last_time) {
                se_process_effects_during(ces, monster, c);
                /* se_process_effects_last(ces, monster, c); */
            }
            else if (effect_has_flag(ces, EF_SETT_TICK) && (ces->tick_energy <= 0) ) {

                /* Handle Checks */
                if (check_effect_has_flag(c, EF_CHECK_ACTIVE) && check_effect_has_flag(c, EF_CHECK_EACH_INTERVAL) ) {
                    bool made_check = false;

                    if (check_effect_has_flag(c, EF_CHECK_CHARACTERISTIC) ) {
                        if ( (msr_characteristic_check(monster, c->check_type, c->check_difficulty) ) >= 1) {
                            made_check = true;
                        }
                    }
                    else if (check_effect_has_flag(c, EF_CHECK_SKILL) ) {
                        if ( (msr_skill_check(monster, c->check_type, c->check_difficulty) ) >= 1) {
                            made_check = true;
                        }
                    }

                    if (check_effect_has_flag(c, EF_CHECK_BENEFICIAL) ) {
                        if (made_check == false) status_effect_clr_flag(c, SEF_ACTIVE);
                    }
                    else if (made_check == true) status_effect_clr_flag(c, SEF_ACTIVE);
                }

                if (status_effect_has_flag(c, SEF_ACTIVE) ) se_process_effects_during(ces, monster, c);
            }
        }
    }

    if (status_effect_has_flag(c, SEF_ACTIVE) == false) {
        lg_debug("Condition %p(%s) is to be destroyed.", c, c->name);
        se_remove_status_effect(se_list, c);
        c = c_prev;
    }
    else if (c->duration_energy > 0) {
        c->duration_energy -= MIN(TT_ENERGY_TICK, c->duration_energy);
    }

    c_prev = c;
}

/* Process status_effects */
void se_process(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return;

    struct status_effect *c = NULL;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        se_process_effect(monster, c);
    }
}

void se_remove_all_non_permanent(struct msr_monster *monster) {
    struct status_effect *c = NULL;
    struct status_effect_list *se_list = monster->status_effects;

    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        if ( (status_effect_has_flag(c, SEF_PERMANENT) == false) ) {

            if ( (status_effect_has_flag(c, SEF_ACTIVE) ) ) {
                /* Cleanup effects */
                for (int i = 0; i < ( (int) ARRAY_SZ(c->effects) ); i++) {
                    struct se_type_struct *ces = &c->effects[i];

                    if (effect_has_flag(ces, EF_SETT_ACTIVE) ) se_process_effects_last(ces, monster, c);
                }
            }

            se_remove_status_effect(se_list, c);
        }
    }
}

