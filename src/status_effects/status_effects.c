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
        struct status_effect *status_effect = &static_status_effect_list[i];
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

struct status_effect *selst_status_effect_by_uid(uint32_t status_effect_uid) {
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

void se_process_grnd() {
    if (status_effects_list_initialised == false) return;
    struct status_effect *se = NULL;

    while ( (se = selst_get_next_status_effect(se) ) != NULL) {
        if (se->permissible_on_ground) {
            if (se->me != NULL) {
                se->grnd_duration_energy -= TT_ENERGY_TICK;
                if (se->grnd_duration_energy < 0) {
                    se->me->status_effect = NULL;
                    se->me = NULL;

                    struct se_entry *ce = container_of(se, struct se_entry, status_effect);
                    struct status_effect_list_entry *sele = container_of(ce, struct status_effect_list_entry, se);

                    TAILQ_REMOVE(&status_effects_list_head, sele, entries);
                    free(sele);
                }
                else if ( (se->grnd_duration_energy % TT_ENERGY_TURN) == 0) {
                    if (se->me->monster != NULL) {
                        assert(se_add_status_effect(se->me->monster, se->template_id) );
                    }
                }
            }
        }
    }
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
        default: 
        case SE_STRENGTH_NONE:     dmg = 0;  break;
        case SE_STRENGTH_ONE:      dmg = 1;  break;
        case SE_STRENGTH_TWO:      dmg = 2;  break;
        case SE_STRENGTH_THREE:    dmg = 3;  break;
        case SE_STRENGTH_FIVE:     dmg = 5;  break;
        case SE_STRENGTH_TEN:      dmg = 10; break;
        case SE_STRENGTH_1D5:      dmg = random_xd5(gbl_game->random, 1); break;
        case SE_STRENGTH_1D10:     dmg = random_xd10(gbl_game->random, 1); break;;
        case SE_STRENGTH_2D10:     dmg = random_xd10(gbl_game->random, 2); break;
        case SE_STRENGTH_3D10:     dmg = random_xd10(gbl_game->random, 3); break;
        case SE_STRENGTH_4D10:     dmg = random_xd10(gbl_game->random, 4); break;
    }

    return dmg;
}

struct status_effect *se_create(enum se_ids tid) {
    if (tid == SEID_NONE) return false;
    if (tid >= SEID_MAX) return false;
    if (tid >= (int) ARRAY_SZ(static_status_effect_list) ) return false;

    struct status_effect *se_template = &static_status_effect_list[tid];
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

    if (cc->permissible_on_ground) {
        int range = (cc->grnd_duration_energy_max - cc->grnd_duration_energy_min);
        cc->grnd_duration_energy = cc->grnd_duration_energy_min;
        if (range > 0) cc->grnd_duration_energy += (random_int32(gbl_game->random) % range);
        if (cc->grnd_duration_energy == 0) cc->grnd_duration_energy = 1;
        lg_debug("Creating se grnd: %p(%s) duration: %d, max: %d", cc, cc->name, cc->grnd_duration_energy, cc->grnd_duration_energy_max);
        cc->grnd_duration_energy_max = cc->grnd_duration_energy;
        cc->icon_attr = get_colour(cc->icon_attr);
    }

    int range = (cc->duration_energy_max - cc->duration_energy_min);
    cc->duration_energy = cc->duration_energy_min;
    if (range > 0) cc->duration_energy += (random_int32(gbl_game->random) % range);
    if (cc->duration_energy == 0) cc->duration_energy = 1;
    lg_debug("Creating se: %p(%s) duration: %d, max: %d", cc, cc->name, cc->duration_energy, cc->duration_energy_max);
    cc->duration_energy_max = cc->duration_energy;

    for (unsigned int i = 0; i < ARRAY_SZ(cc->effects); i++ ) {
        struct se_type_struct *ces = &cc->effects[i];
        if (effect_has_flag(ces, SESF_ACTIVE) ) {
            ces->strength = se_calc_strength(ces);
        }
    }

    assert(se_verify_status_effect(cc) );
    return cc;
}

struct status_effect *se_create_ground(enum se_ids tid, struct dm_map_entity *me) {
    struct status_effect *se = se_create(tid);
    assert(se != NULL);
    assert(se->permissible_on_ground);
    se->me = me;

    return se;
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

bool se_add_status_effect(struct msr_monster *monster, uint32_t tid) {
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
            if (status_effect_has_flag(c, SEF_NOT_UNIQUE) == false) {
                /* and permanent, we do nothing */
                if (status_effect_has_flag(c, SEF_PERMANENT) ) {
                    return false;
                }
                
                /* if not permanent but is unique, then we restart the duration. */
                lg_debug("Restarting status_effect: %p(%s)", c, c->name);
                /* restart status_effect */
                c->duration_energy = c->duration_energy_max -1;
                return false;
            }
        }
    }

    /* if it does not yet exist, or the NOT_UNIQUE flag is set, create an new instance. */
    c = se_create(tid);
    if (c == NULL) return false;

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
bool se_has_effect_skip(struct status_effect_list *se_list, enum status_effect_type_flags effect, struct status_effect *status_effect) {
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


bool se_has_effect(struct status_effect_list *se_list, enum status_effect_type_flags effect) {
    return se_has_effect_skip(se_list, effect, NULL);
}

int se_status_effect_strength(struct status_effect_list *se_list, enum status_effect_type_flags effect) {
    if (se_verify_list(se_list) == false) return -1;
    struct status_effect *c = NULL;

    int strength = 0;
    while ( (c = se_list_get_next_status_effect(se_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                strength += c->effects[i].strength;
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
static enum se_ids dmg_type_to_id_lot[MSR_HITLOC_MAX][DMG_TYPE_MAX] = {
    [MSR_HITLOC_LEFT_LEG] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_LLEGS_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_LLEGS_1,
    },
    [MSR_HITLOC_RIGHT_LEG] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_RLEGS_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_RLEGS_1,
    },
    [MSR_HITLOC_LEFT_ARM] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_LARMS_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_LARMS_1,
    },
    [MSR_HITLOC_RIGHT_ARM] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_RARMS_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_RARMS_1,
    },
    [MSR_HITLOC_BODY] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_BODY_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_BODY_1,
    },
    [MSR_HITLOC_HEAD] = { 
        [DMG_TYPE_ARROW]    = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_BLUNT]    = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_BULLET]   = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_CLAW]     = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_CUTTING]  = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_ENERGY]   = SEID_NONE,
        [DMG_TYPE_PIERCING] = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_SHRAPNEL] = SEID_GENERAL_CRITICAL_HEAD_1,
        [DMG_TYPE_UNARMED]  = SEID_GENERAL_CRITICAL_HEAD_1,
    },
};

static int crit_value_table[10][10] = {
       /*+1 +2 +3 +4 +5 +6 +7 +8 +9 +10*/
    [0]={ 5, 7, 9,10,10,10,10,10,10,10,},
    [1]={ 5, 6, 8, 9,10,10,10,10,10,10,},
    [2]={ 4, 6, 8, 9, 9,10,10,10,10,10,},
    [3]={ 4, 5, 7, 8, 9, 9,10,10,10,10,},
    [4]={ 3, 5, 7, 8, 8, 9, 9,10,10,10,},
    [5]={ 3, 4, 6, 7, 8, 8, 9, 9,10,10,},
    [6]={ 2, 4, 6, 7, 7, 8, 8, 9, 9,10,},
    [7]={ 2, 3, 5, 6, 7, 7, 8, 8, 9, 9,},
    [8]={ 1, 3, 5, 6, 6, 7, 7, 8, 8, 9,},
    [9]={ 1, 2, 4, 5, 6, 6, 7, 7, 8, 8,},
};

bool se_add_critical_hit(struct msr_monster *monster, int critical_dmg, enum msr_hit_location mhl, enum dmg_type type) {
    if (msr_verify_monster(monster) == false) return false;

    struct status_effect_list *se_list = monster->status_effects;
    if (se_verify_list(se_list) == false) return false;

    if (critical_dmg > 10) critical_dmg = 10;

    int rand = random_xd10(gbl_game->random, 1);
    int idx  = crit_value_table[rand-1][critical_dmg-1] -1;

    enum se_ids tid = dmg_type_to_id_lot[mhl][type];
    if (tid == SEID_NONE) return false;

    /* TODO: update this when more critical hits become available */
    return se_add_status_effect(monster, tid +(idx/2) );
}


void se_process_effects_first(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    switch(ces->effect) {
        case SETF_DECREASE_MOVEMENT: break;
        case SETF_INCREASE_MOVEMENT: break;
        case SETF_STUMBLE: break;
        case SETF_BLINDNESS: break;
        case SETF_DEAFNESS: break;
        case SETF_STUNNED: break;
        case SETF_HALUCINATING: break;
        case SETF_CONFUSED: break;
        case SETF_INHIBIT_FATE_POINT: break;
        case SETF_PINNED: break;
        case SETF_ON_FIRE: break;
        case SETF_PSYCHIC_ENHANCE: break;
        case SETF_BLOODLOSS: break;
        case SETF_DETOX: break;
        case SETF_DECREASE_FATIQUE: break;
        case SETF_INCREASE_FATIQUE: break;
        case SETF_INCREASE_ALL_SKILLS: break;
        case SETF_DECREASE_ALL_SKILLS: break;
        case SETF_DECREASE_MAX_WOUNDS: break;
        case SETF_INCREASE_MAX_WOUNDS: break;
        case SETF_DECREASE_WS: break;
        case SETF_INCREASE_WS: break;
        case SETF_DECREASE_BS: break;
        case SETF_INCREASE_BS: break;
        case SETF_DECREASE_STR: break;
        case SETF_INCREASE_STR: break;
        case SETF_DECREASE_AG: break;
        case SETF_INCREASE_AG: break;
        case SETF_DECREASE_TGH: break;
        case SETF_INCREASE_TGH: break;
        case SETF_DECREASE_PER: break;
        case SETF_INCREASE_PER: break;
        case SETF_DECREASE_WILL: break;
        case SETF_INCREASE_WILL: break;
        case SETF_DECREASE_INT: break;
        case SETF_INCREASE_INT: break;

        case SETF_TALENT:
            if (msr_has_talent(monster, ces->optional) ) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
            else msr_set_talent(monster, ces->optional);
            break;
        case SETF_TRAIT: 
            if (msr_has_creature_trait(monster, ces->optional) ) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
            else msr_set_creature_trait(monster, ces->optional);
            break;

        case SETF_SET_WS: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value;
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = strength;
        } break;
        case SETF_SET_BS: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value;
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value = strength;
        } break;
        case SETF_SET_STR: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_STRENGTH].base_value;
            monster->characteristic[MSR_CHAR_STRENGTH].base_value = strength;
        } break;
        case SETF_SET_AG: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_AGILITY].base_value;
            monster->characteristic[MSR_CHAR_AGILITY].base_value = strength;
        } break;
        case SETF_SET_TGH: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_TOUGHNESS].base_value;
            monster->characteristic[MSR_CHAR_TOUGHNESS].base_value = strength;
        } break;
        case SETF_SET_PER: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_PERCEPTION].base_value;
            monster->characteristic[MSR_CHAR_PERCEPTION].base_value = strength;
        } break;
        case SETF_SET_WILL: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_WILLPOWER].base_value;
            monster->characteristic[MSR_CHAR_WILLPOWER].base_value = strength;
        } break;
        case SETF_SET_INT: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value;
            monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value = strength;
        } break;

        case SETF_DISABLE_EYE: break;
        case SETF_DISABLE_LLEG: break;
        case SETF_DISABLE_RLEG: break;
        case SETF_DISABLE_LARM:
            inv_disable_location(monster->inventory, INV_LOC_OFFHAND_WIELD);
            msr_weapon_next_selection(monster);
            break;
        case SETF_DISABLE_RARM:
            inv_disable_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
            msr_weapon_next_selection(monster);
            break;

        case SETF_DAMAGE:
            effect_clr_flag(ces, SESF_ACTIVE); 
            break;
        case SETF_DAMAGE_TICK: break;

        case SETF_HEALTH: 
            effect_clr_flag(ces, SESF_ACTIVE); 
            break;
        case SETF_HEALTH_TICK: break;

        case SETF_EXPLODE: {
            /* place bodypart grenade here */

            /* create item */
            struct itm_item *item = itm_create(IID_BODYPART_GRENADE);

            /* TODO: change name of bodypart */
            item->ld_name = "a head";

            /* light fuse */
            item->energy = TT_ENERGY_TICK;
            item->energy_action = true;

            /* put item in map at the tile of monster */
            itm_insert_item(item, gbl_game->current_map, &monster->pos);
        } /* No Break */
        case SETF_INSTANT_DEATH: {
            effect_clr_flag(ces, SESF_ACTIVE);
            msr_die(monster, gbl_game->current_map);
            return;
        } break;

        default: 
            effect_clr_flag(ces, SESF_ACTIVE); break;
    }
}

void se_process_effects_last(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    int mod = -1;
    switch(ces->effect) {
        case SETF_DECREASE_MOVEMENT: break;
        case SETF_INCREASE_MOVEMENT: break;
        case SETF_STUMBLE: break;
        case SETF_BLINDNESS: break;
        case SETF_DEAFNESS: break;
        case SETF_STUNNED: break;
        case SETF_HALUCINATING: break;
        case SETF_CONFUSED: break;
        case SETF_INHIBIT_FATE_POINT: break;
        case SETF_PINNED: break;
        case SETF_ON_FIRE: break;
        case SETF_PSYCHIC_ENHANCE: break;
        case SETF_DECREASE_FATIQUE: break;
        case SETF_INCREASE_FATIQUE: break;
        case SETF_INCREASE_ALL_SKILLS: break;
        case SETF_DECREASE_ALL_SKILLS: break;
        case SETF_DAMAGE: break;
        case SETF_DAMAGE_TICK: break;
        case SETF_HEALTH: break;
        case SETF_HEALTH_TICK: break;
        case SETF_BLOODLOSS: break;

        case SETF_TALENT:
            msr_clr_talent(monster, ces->optional);
            break;
        case SETF_TRAIT: 
            msr_set_creature_trait(monster, ces->optional);
            break;

        case SETF_SET_WS:
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = ces->strength;
            break;
        case SETF_SET_BS: 
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value = ces->strength;
            break;
        case SETF_SET_STR: 
            monster->characteristic[MSR_CHAR_STRENGTH].base_value = ces->strength;
            break;
        case SETF_SET_AG:
            monster->characteristic[MSR_CHAR_AGILITY].base_value = ces->strength;
            break;
        case SETF_SET_TGH:
            monster->characteristic[MSR_CHAR_TOUGHNESS].base_value = ces->strength;
            break;
        case SETF_SET_PER:
            monster->characteristic[MSR_CHAR_PERCEPTION].base_value = ces->strength;
            break;
        case SETF_SET_WILL:
            monster->characteristic[MSR_CHAR_WILLPOWER].base_value = ces->strength;
            break;
        case SETF_SET_INT:
            monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value = ces->strength;
            break;

        case SETF_DECREASE_WS: mod = 1;
        case SETF_INCREASE_WS:
                monster->characteristic[MSR_CHAR_WEAPON_SKILL].mod += (ces->strength * mod) * ces->ticks_applied;
                break;

        case SETF_DECREASE_BS: mod = 1;
        case SETF_INCREASE_BS:
                monster->characteristic[MSR_CHAR_BALISTIC_SKILL].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_STR: mod = 1;
        case SETF_INCREASE_STR:
                monster->characteristic[MSR_CHAR_STRENGTH].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_AG: mod = 1;
        case SETF_INCREASE_AG:
                monster->characteristic[MSR_CHAR_AGILITY].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_TGH: mod = 1;
        case SETF_INCREASE_TGH:
                monster->characteristic[MSR_CHAR_TOUGHNESS].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_PER: mod = 1;
        case SETF_INCREASE_PER:
                monster->characteristic[MSR_CHAR_PERCEPTION].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_WILL: mod = 1;
        case SETF_INCREASE_WILL:
                monster->characteristic[MSR_CHAR_WILLPOWER].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case SETF_DECREASE_INT: mod = 1;
        case SETF_INCREASE_INT:
                monster->characteristic[MSR_CHAR_INTELLIGENCE].mod += (ces->strength * mod) * ces->ticks_applied;
                break;

        case SETF_DECREASE_MAX_WOUNDS: mod = 1;
        case SETF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += (ces->strength * mod) * ces->ticks_applied;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case SETF_DISABLE_EYE: break;
        case SETF_DISABLE_LLEG: break;
        case SETF_DISABLE_RLEG: break;
        case SETF_DISABLE_LARM:
            if (se_has_effect_skip(monster->status_effects, SETF_DISABLE_LARM, c) )
                break;
            inv_enable_location(monster->inventory, INV_LOC_OFFHAND_WIELD);
            break;
        case SETF_DISABLE_RARM:
            if (se_has_effect_skip(monster->status_effects, SETF_DISABLE_RARM, c) ) 
                break;
            inv_enable_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
            break;

        case SETF_DETOX: {
            /* Remove all status_effects which are 'DETOXABLE' */
            struct status_effect *c_temp = NULL;
            struct status_effect_list *se_list = monster->status_effects;
            while ( (c_temp = se_list_get_next_status_effect(se_list, c_temp) ) != NULL) {
                if (status_effect_has_flag(c, SEF_DETOXABLE) ) {
                    se_remove_status_effect(se_list, c_temp);
                    c_temp = NULL;
                }
            }
        } break;

        case SETF_EXPLODE:
        case SETF_INSTANT_DEATH: {
            effect_clr_flag(ces, SESF_ACTIVE);
            msr_die(monster, gbl_game->current_map);
            return;
        } break;

        default: 
            effect_clr_flag(ces, SESF_ACTIVE); break;
    }

    ces->tick_energy = 0;
    effect_clr_flag(ces, SESF_ACTIVE);
}

void se_process_effects_during(struct se_type_struct *ces, struct msr_monster *monster, struct status_effect *c) {
    if (msr_verify_monster(monster) == false) return;
    if (se_verify_status_effect(c) == false) return;

    int mod = 1;
    switch(ces->effect) {
        case SETF_DECREASE_MOVEMENT: break;
        case SETF_INCREASE_MOVEMENT: break;
        case SETF_STUMBLE: break;
        case SETF_BLINDNESS: break;
        case SETF_DEAFNESS: break;
        case SETF_STUNNED: break;
        case SETF_HALUCINATING: break;
        case SETF_CONFUSED: break;
        case SETF_INHIBIT_FATE_POINT: break;
        case SETF_PINNED: break;
        case SETF_ON_FIRE: break;
        case SETF_PSYCHIC_ENHANCE: break;
        case SETF_EXPLODE: break;
        case SETF_INSTANT_DEATH: break;
        case SETF_SET_WS: break;
        case SETF_SET_BS: break;
        case SETF_SET_STR: break;
        case SETF_SET_AG: break;
        case SETF_SET_TGH: break;
        case SETF_SET_PER: break;
        case SETF_SET_WILL: break;
        case SETF_SET_INT: break;
        case SETF_INCREASE_ALL_SKILLS: break;
        case SETF_DECREASE_ALL_SKILLS: break;
        case SETF_DISABLE_EYE: break;
        case SETF_DISABLE_LLEG: break;
        case SETF_DISABLE_RLEG: break;
        case SETF_DISABLE_LARM: break;
        case SETF_DISABLE_RARM: break;
        case SETF_DETOX: break;

        case SETF_TALENT: break;
        case SETF_TRAIT: break;

        case SETF_DECREASE_FATIQUE: mod = -1;
        case SETF_INCREASE_FATIQUE:
            monster->fatique += ces->strength * ( -1 * mod);
            monster->fatique_turn = gbl_game->turn;
            break;

        case SETF_DECREASE_WS: mod = -1;
        case SETF_INCREASE_WS:
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_BS: mod = -1;
        case SETF_INCREASE_BS:
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_STR: mod = -1;
        case SETF_INCREASE_STR:
            monster->characteristic[MSR_CHAR_STRENGTH].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_AG: mod = -1;
        case SETF_INCREASE_AG:
            monster->characteristic[MSR_CHAR_AGILITY].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_TGH: mod = -1;
        case SETF_INCREASE_TGH:
            monster->characteristic[MSR_CHAR_TOUGHNESS].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_PER: mod = -1;
        case SETF_INCREASE_PER:
            monster->characteristic[MSR_CHAR_PERCEPTION].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_WILL: mod = -1;
        case SETF_INCREASE_WILL:
            monster->characteristic[MSR_CHAR_WILLPOWER].mod += ces->strength * mod;
            break;
        case SETF_DECREASE_INT: mod = -1;
        case SETF_INCREASE_INT:
            monster->characteristic[MSR_CHAR_INTELLIGENCE].mod += ces->strength * mod;
            break;

        case SETF_DAMAGE: break;
        case SETF_DAMAGE_TICK: 
            msr_do_dmg(monster, ces->strength, DMG_TYPE_ENERGY, MSR_HITLOC_NONE);
            break;

        case SETF_HEALTH: break;
        case SETF_HEALTH_TICK:
            monster->cur_wounds += ces->strength;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case SETF_DECREASE_MAX_WOUNDS: mod = -1;
        case SETF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += ces->strength * mod;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case SETF_BLOODLOSS:
            if (random_d100(gbl_game->random) < 5) {
                You(monster, "die due to bloodless.");
                Monster(monster, "dies in a large pool of blood.");
                msr_die(monster, gbl_game->current_map);
            }
            break;

        default: effect_clr_flag(ces, SESF_ACTIVE); break;
    }

    ces->tick_energy = ces->tick_energy_max;
    ces->ticks_applied++;
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


    bool destroy    = false;
    bool first_time = false;
    bool last_time  = true;

    struct status_effect *c_prev = NULL;
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
                if (ces->effect != SETF_NONE) {
                    if (effect_has_flag(ces, SESF_ACTIVE) == false) {
                        inactive++;
                    }
                }
            }
            if (inactive == ARRAY_SZ(c->effects) ) destroy = true;

            if (c->template_id == SEID_NONE) destroy = true;

            if (status_effect_has_flag(c, SEF_REQ_WILL_CHECK) ) {
                if ( (msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, c->difficulty) ) >= 1) {
                    destroy = true;
                }
            }
            if (status_effect_has_flag(c, SEF_REQ_TGH_CHECK) ) {
                if ( (msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, c->difficulty) ) >= 1) {
                    destroy = true;
                }
            }
            if (status_effect_has_flag(c, SEF_REQ_AG_CHECK) ) {
                if ( (msr_characteristic_check(monster, MSR_CHAR_AGILITY, c->difficulty) ) >= 1) {
                    destroy = true;
                }
            }
            if (status_effect_has_flag(c, SEF_REQ_CHEM_USE_CHECK) ) {
                if ( (msr_skill_check(monster, MSR_SKILLS_CHEM_USE, c->difficulty) ) >= 1) {
                    destroy = true;
                }
            }

            if (destroy == false) {
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
            destroy = true;
        }
    }

    /*
       Remove the id which this status_effect continues in,
       apparently this continues releaves the cravings 
       set by the continues_status_effect. Later offcourse,
       this status_effect will continue in that so it will 
       not stop it at all.
     */
    if (status_effect_has_flag(c, SEF_REMOVE_CONTINUE) ) {
        struct status_effect *c_temp = NULL;
        while ( (c_temp = se_get_status_effect_tid(se_list, c->continues_to_id) ) != NULL) {
            if (status_effect_has_flag(c_temp, SEF_INVISIBLE) == false) {
                if (c_temp->on_exit_plr != NULL) You(monster, "%s.", c_temp->on_exit_plr);
                if (c_temp->on_exit_msr != NULL) Monster(monster, "%s.", c_temp->on_exit_msr);
            }
            se_remove_status_effect(se_list, c_temp);
        }
    }

    for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
        struct se_type_struct *ces = &c->effects[i];
        if (effect_has_flag(ces, SESF_ACTIVE) == false) continue;

        if (effect_has_flag(ces, SESF_TICK) && (ces->tick_energy > 0) ) {
            ces->tick_energy -= TT_ENERGY_TICK;
            if (!first_time && !last_time) continue;
        }

        /* 
           These tests will check if the status_effect still has to be active.
           They are run the first time and whenever another tick happens.
         */
        if (effect_has_flag(ces, SESF_REQ_WILL_CHECK) ) {
            if (msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, ces->difficulty) >= 1) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
        }
        if (effect_has_flag(ces, SESF_REQ_TGH_CHECK) ) {
            if (msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, ces->difficulty) >= 1) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
        }
        if (effect_has_flag(ces, SESF_REQ_AG_CHECK) ) {
            if (msr_characteristic_check(monster, MSR_CHAR_AGILITY, ces->difficulty) >= 1) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
        }
        if (effect_has_flag(ces, SESF_REQ_CHEM_USE_CHECK) ) {
            if (msr_skill_check(monster, MSR_SKILLS_CHEM_USE, ces->difficulty) >= 1) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
        }

        if (first_time) {
            se_process_effects_first(ces, monster, c);
            se_process_effects_during(ces, monster, c);

            /* if ONCE is set, the second time this effect has been processed, 
               disable it. the first time will be the TICK directly after the 
               status_effect is applied, the second time will be the turn after 
               that. */
            if (effect_has_flag(ces, SESF_ONCE) ) {
                effect_clr_flag(ces, SESF_ACTIVE);
            }
        }
        else if (last_time) {
            se_process_effects_during(ces, monster, c);
            se_process_effects_last(ces, monster, c);
            destroy = true;
        }
        else if (effect_has_flag(ces, SESF_TICK) && (ces->tick_energy <= 0) ) {
            se_process_effects_during(ces, monster, c);
        }
    }

    if (destroy) {
        lg_debug("Condition %p(%s) is to be destroyed.", c, c->name);
        if (c->continues_to_id != SEID_NONE) {
            se_add_status_effect(monster, c->continues_to_id);
        }
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
            se_remove_status_effect(se_list, c);
        }
    }
}

bool se_tid_ground_permissible(enum se_ids tid) {
    if (tid == SEID_NONE) return false;
    if (tid >= SEID_MAX) return false;
    if (tid >= (int) ARRAY_SZ(static_status_effect_list) ) return false;
    return static_status_effect_list[tid].permissible_on_ground;
}
