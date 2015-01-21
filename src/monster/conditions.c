#include <assert.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <string.h>

#include "conditions.h"
#include "monster.h"
#include "logging.h"
#include "heresyrl_def.h"
#include "random.h"
#include "turn_tick.h"
#include "items/items_static.h"
#include "dungeon/dungeon_map.h"

static inline bool effect_set_flag(struct condition_effect_struct *ces, enum condition_effect_setting_flags flag) {
    return set_bf(ces->effect_setting_flags, flag);
}
static inline bool effect_clr_flag(struct condition_effect_struct *ces, enum condition_effect_setting_flags flag) {
    return clr_bf(ces->effect_setting_flags, flag);
}
static inline bool effect_has_flag(struct condition_effect_struct *ces, enum condition_effect_setting_flags flag) {
    return set_bf(ces->effect_setting_flags, flag);
}

static inline bool condition_set_flag(struct cdn_condition *c, enum condition_flags flag) {
    return set_bf(c->setting_flags, flag);
}
static inline bool condition_clr_flag(struct cdn_condition *c, enum condition_flags flag) {
    return clr_bf(c->setting_flags, flag);
}
static inline bool condition_has_flag(struct cdn_condition *c, enum condition_flags flag) {
    return test_bf(c->setting_flags, flag);
}

struct cdn_entry {
    struct cdn_condition condition;
    LIST_ENTRY(cdn_entry) entries;
};

struct cdn_condition_list {
    uint32_t pre_check;
    LIST_HEAD(cdn_list_head, cdn_entry) head;
    uint32_t post_check;
};

#include "conditions_static.c"

/* Garanteed to be random, rolled it myself ;)  */
#define CONDITION_PRE_CHECK (68731)
#define CONDITION_POST_CHECK (138)
#define CONDITION_LIST_PRE_CHECK (45867)
#define CONDITION_LIST_POST_CHECK (7182)

void cdn_init(void) {
    for (unsigned int i = 0; i < CID_MAX; i++) {
        struct cdn_condition *condition = &static_condition_list[i];
        if (condition->template_id != i) {
            fprintf(stderr, "Condition list integrity check failed! [%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }
}

void cdn_exit(void) {
}

struct cdn_condition_list *cdn_list_init(void) {
    struct cdn_condition_list *i = calloc(1, sizeof(struct cdn_condition_list) );
    if (i != NULL) {
        LIST_INIT(&i->head);
        i->pre_check = CONDITION_LIST_PRE_CHECK;
        i->post_check = CONDITION_LIST_POST_CHECK;
    }
    return i;
}

void cdn_list_exit(struct cdn_condition_list *cdn_list) {
    if (cdn_verify_list(cdn_list) == false) return;

    struct cdn_entry *ce;
    while ( (ce = cdn_list->head.lh_first ) != NULL) {
        LIST_REMOVE(cdn_list->head.lh_first, entries);
        free(ce);
    }
    free(cdn_list);
}

bool cdn_verify_list(struct cdn_condition_list *cdn_list) {
    assert(cdn_list != NULL);
    assert(cdn_list->pre_check == CONDITION_LIST_PRE_CHECK);
    assert(cdn_list->post_check == CONDITION_LIST_POST_CHECK);
    return true;
}

/* Peek at items. if prev is NULL, it gives the first item, otherwise it gives the item after prev. */
struct cdn_condition *cdn_list_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev) {
    if (cdn_verify_list(cdn_list) == false) return false;

    if (prev == NULL) {
        if (cdn_list->head.lh_first != NULL) return &cdn_list->head.lh_first->condition;
        return NULL;
    }
    struct cdn_entry *ce = container_of(prev, struct cdn_entry, condition);
    if (ce == NULL) return NULL;
    return &ce->entries.le_next->condition;
}

int cdn_list_size(struct cdn_condition_list *cdn_list) {
    if (cdn_verify_list(cdn_list) == false) return false;
    int i = 0;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        i++;
    }

    return i;
}

static int cdn_calc_uid(struct cdn_condition_list *cdn_list) {
    if (cdn_verify_list(cdn_list) == false) return false;
    int id = 0;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (c->uid >= id) id = c->uid+1;
    }

    return id;
}

static int cdn_calc_strength(struct condition_effect_struct *ces) {
    int dmg = 0;
    switch(ces->strength) {
        default: 
        case CDN_STRENGTH_NONE:     dmg = 0;  break;
        case CDN_STRENGTH_ONE:      dmg = 1;  break;
        case CDN_STRENGTH_TWO:      dmg = 2;  break;
        case CDN_STRENGTH_THREE:    dmg = 3;  break;
        case CDN_STRENGTH_FIVE:     dmg = 5;  break;
        case CDN_STRENGTH_TEN:      dmg = 10; break;
        case CDN_STRENGTH_1D5:      dmg = random_xd5(gbl_game->random, 1); break;
        case CDN_STRENGTH_1D10:     dmg = random_xd10(gbl_game->random, 1); break;;
        case CDN_STRENGTH_2D10:     dmg = random_xd10(gbl_game->random, 2); break;
        case CDN_STRENGTH_3D10:     dmg = random_xd10(gbl_game->random, 3); break;
        case CDN_STRENGTH_4D10:     dmg = random_xd10(gbl_game->random, 4); break;
    }

    return dmg;
}

struct cdn_condition *cdn_create(struct cdn_condition_list *cdn_list, enum cdn_ids tid) {
    if (tid == CID_NONE) return false;
    if (tid >= CID_MAX) return false;
    if (tid >= (int) ARRAY_SZ(static_condition_list) ) return false;

    struct cdn_condition *cdn_template = &static_condition_list[tid];
    assert(cdn_template != NULL);

    struct cdn_entry *ce = malloc(sizeof(struct cdn_entry) );
    if (ce == NULL) return false;

    memcpy(&ce->condition, cdn_template, sizeof(struct cdn_condition) );
    struct cdn_condition *cc = &ce->condition;

    cc->condition_pre = CONDITION_PRE_CHECK;
    cc->condition_post = CONDITION_POST_CHECK;

    cc->uid = cdn_calc_uid(cdn_list);

    int range = (cc->duration_energy_max - cc->duration_energy_min);
    cc->duration_energy = cc->duration_energy_min;
    if (range > 0) cc->duration_energy += (random_int32(gbl_game->random) % range);
    if (cc->duration_energy == 0) cc->duration_energy = 1;
    lg_debug("Creating cdn: %p(%s) duration: %d, max: %d", cc, cc->name, cc->duration_energy, cc->duration_energy_max);
    cc->duration_energy_max = cc->duration_energy;

    for (unsigned int i = 0; i < ARRAY_SZ(cc->effects); i++ ) {
        struct condition_effect_struct *ces = &cc->effects[i];
        if (effect_has_flag(ces, CDN_ESF_ACTIVE) ) {
            ces->strength = cdn_calc_strength(ces);
        }
    }

    assert(cdn_verify_condition(cc) );
    return cc;
}

bool cdn_add_to_list(struct cdn_condition_list *cdn_list, struct cdn_condition *con) {
    if (cdn_verify_list(cdn_list) == false) return false;
    if (cdn_verify_condition(con) == false) return false;

    lg_debug("Adding condition: %p(%s)", con, con->name);

    struct cdn_entry *ce = container_of(con, struct cdn_entry, condition);
    LIST_INSERT_HEAD(&cdn_list->head, ce, entries);
    return true;
}

bool cdn_add_condition(struct cdn_condition_list *cdn_list, uint32_t tid) {
    if (cdn_verify_list(cdn_list) == false) return false;
    if (tid == CID_NONE) return false;
    if (tid >= ARRAY_SZ(static_condition_list) ) return false;
    struct cdn_condition *c = NULL;

    if (cdn_has_tid(cdn_list, tid) ) {
        c = cdn_get_condition_tid(cdn_list, tid);
        if (c != NULL) {
            if (condition_has_flag(c, CDN_CF_UNIQUE) ) {
                if (condition_has_flag(c, CDN_CF_PERMANENT) ) {
                    return false;
                }

                lg_debug("Restarting condition: %p(%s)", c, c->name);
                /* restart condition */
                c->duration_energy = c->duration_energy_max -1;
                return false;
            }
        }
    }

    c = cdn_create(cdn_list, tid);
    if (c == NULL) return false;

    return cdn_add_to_list(cdn_list, c);
}

bool cdn_remove_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *con) {
    if (cdn_verify_list(cdn_list) == false) return false;

    struct cdn_condition *cc = NULL;
    while ( (cc = cdn_list_get_next_condition(cdn_list, cc) ) != NULL) {
        if (cc == con) {
            struct cdn_entry *ce = container_of(cc, struct cdn_entry, condition);

            LIST_REMOVE(ce, entries);
            free(ce);

            return true;
        }
    }

    return false;
}

bool cdn_verify_condition(struct cdn_condition *cdn) {
    assert(cdn != NULL);
    assert(cdn->condition_pre == CONDITION_PRE_CHECK);
    assert(cdn->condition_post == CONDITION_POST_CHECK);

    return true;
}

bool cdn_has_tid(struct cdn_condition_list *cdn_list, enum cdn_ids tid) {
    if (cdn_verify_list(cdn_list) == false) return false;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (c->template_id == tid) {
            return true;
        }
    }
    return false;
}

struct cdn_condition *cdn_get_condition_tid(struct cdn_condition_list *cdn_list, enum cdn_ids tid) {
    if (cdn_verify_list(cdn_list) == false) return NULL;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (c->template_id == tid) {
            return c;
        }
    }
    return NULL;
}

/* Check if there is another condition with the same effect besides the one given */
bool cdn_has_effect_skip(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect, struct cdn_condition *condition) {
    if (cdn_verify_list(cdn_list) == false) return false;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if ( (condition != NULL) && (condition == c) ) continue;

        for (unsigned int  i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                return true;
            }
        }
    }

    return false;
}


bool cdn_has_effect(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect) {
    return cdn_has_effect_skip(cdn_list, effect, NULL);
}

int cdn_condition_effect_strength(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect) {
    if (cdn_verify_list(cdn_list) == false) return -1;
    struct cdn_condition *c = NULL;

    int strength = 0;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                strength += c->effects[i].strength;
            }
        }
    }

    return strength;
}

static enum cdn_ids dmg_type_to_id_lot[MSR_HITLOC_MAX][DMG_TYPE_MAX] = {
    [MSR_HITLOC_LEFT_LEG] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_LLEGS_1, 
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_LLEGS_1, 
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_LLEGS_1, 
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_LLEGS_1, 
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
    [MSR_HITLOC_RIGHT_LEG] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_RLEGS_1, 
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_RLEGS_1, 
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_RLEGS_1, 
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_RLEGS_1,
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
    [MSR_HITLOC_LEFT_ARM] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_LARMS_1, 
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_LARMS_1, 
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_LARMS_1, 
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_LARMS_1,
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
    [MSR_HITLOC_RIGHT_ARM] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_RARMS_1, 
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_RARMS_1, 
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_RARMS_1, 
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_RARMS_1,
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
    [MSR_HITLOC_BODY] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_BODY_1,  
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_BODY_1,  
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_BODY_1,  
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_BODY_1,
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
    [MSR_HITLOC_HEAD] = { 
        [DMG_TYPE_IMPACT]    = CID_IMPACT_CRITICAL_HEAD_1,  
        [DMG_TYPE_EXPLOSIVE] = CID_EXPLOSIVE_CRITICAL_HEAD_1,  
        [DMG_TYPE_ENERGY]    = CID_ENERGY_CRITICAL_HEAD_1,  
        [DMG_TYPE_RENDING]   = CID_RENDING_CRITICAL_HEAD_1,
        [DMG_TYPE_MAGIC]     = CID_NONE, 
    },
};

bool cdn_add_critical_hit(struct cdn_condition_list *cdn_list, int critical_dmg, enum msr_hit_location mhl, enum dmg_type type) {
    if (cdn_verify_list(cdn_list) == false) return false;
    if (critical_dmg > (CDN_NR_CRITICAL_HITS_PER_LOCATION * 2) ) critical_dmg = (CDN_NR_CRITICAL_HITS_PER_LOCATION * 2);
    critical_dmg /= 2; /* every 2 damage is a new critical hit effect */
    critical_dmg -= 1; /* idx to offset */

    enum cdn_ids tid = dmg_type_to_id_lot[mhl][type];
    if (tid == CID_NONE) return false;
    return cdn_add_condition(cdn_list, tid +critical_dmg);
}


void cdn_process_effects_first(struct condition_effect_struct *ces, struct msr_monster *monster, struct cdn_condition *c) {
    if (msr_verify_monster(monster) == false) return;
    if (cdn_verify_condition(c) == false) return;

    int mod = 1;
    switch(ces->effect) {
        case CDN_EF_DECREASE_MOVEMENT: break;
        case CDN_EF_INCREASE_MOVEMENT: break;
        case CDN_EF_STUMBLE: break;
        case CDN_EF_BLINDNESS: break;
        case CDN_EF_DEAFNESS: break;
        case CDN_EF_STUNNED: break;
        case CDN_EF_HALUCINATING: break;
        case CDN_EF_CONFUSED: break;
        case CDN_EF_INHIBIT_FATE_POINT: break;
        case CDN_EF_PINNED: break;
        case CDN_EF_ON_FIRE: break;
        case CDN_EF_PSYCHIC_ENHANCE: break;
        case CDN_EF_BLOODLOSS: break;
        case CDN_EF_DETOX: break;
        case CDN_EF_DECREASE_FATIQUE: break;
        case CDN_EF_INCREASE_FATIQUE: break;
        case CDN_EF_INCREASE_ALL_SKILLS: break;
        case CDN_EF_DECREASE_ALL_SKILLS: break;
        case CDN_EF_DECREASE_MAX_WOUNDS: break;
        case CDN_EF_INCREASE_MAX_WOUNDS: break;
        case CDN_EF_DECREASE_WS: break;
        case CDN_EF_INCREASE_WS: break;
        case CDN_EF_DECREASE_BS: break;
        case CDN_EF_INCREASE_BS: break;
        case CDN_EF_DECREASE_STR: break;
        case CDN_EF_INCREASE_STR: break;
        case CDN_EF_DECREASE_AG: break;
        case CDN_EF_INCREASE_AG: break;
        case CDN_EF_DECREASE_TGH: break;
        case CDN_EF_INCREASE_TGH: break;
        case CDN_EF_DECREASE_PER: break;
        case CDN_EF_INCREASE_PER: break;
        case CDN_EF_DECREASE_WILL: break;
        case CDN_EF_INCREASE_WILL: break;
        case CDN_EF_DECREASE_INT: break;
        case CDN_EF_INCREASE_INT: break;
        case CDN_EF_DECREASE_FEL: break;
        case CDN_EF_INCREASE_FEL: break;

        case CDN_EF_SET_WS: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value;
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = strength;
        } break;
        case CDN_EF_SET_BS: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value;
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value = strength;
        } break;
        case CDN_EF_SET_STR: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_STRENGTH].base_value;
            monster->characteristic[MSR_CHAR_STRENGTH].base_value = strength;
        } break;
        case CDN_EF_SET_AG: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_AGILITY].base_value;
            monster->characteristic[MSR_CHAR_AGILITY].base_value = strength;
        } break;
        case CDN_EF_SET_TGH: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_TOUGHNESS].base_value;
            monster->characteristic[MSR_CHAR_TOUGHNESS].base_value = strength;
        } break;
        case CDN_EF_SET_PER: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_PERCEPTION].base_value;
            monster->characteristic[MSR_CHAR_PERCEPTION].base_value = strength;
        } break;
        case CDN_EF_SET_WILL: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_WILLPOWER].base_value;
            monster->characteristic[MSR_CHAR_WILLPOWER].base_value = strength;
        } break;
        case CDN_EF_SET_INT: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value;
            monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value = strength;
        } break;
        case CDN_EF_SET_FEL: {
            int8_t strength = ces->strength;
            ces->strength = monster->characteristic[MSR_CHAR_FELLOWSHIP].base_value;
            monster->characteristic[MSR_CHAR_FELLOWSHIP].base_value = strength;
        } break;

        case CDN_EF_DISABLE_LLEG: break;
        case CDN_EF_DISABLE_RLEG: break;
        case CDN_EF_DISABLE_LARM:
            inv_disable_location(monster->inventory, INV_LOC_OFFHAND_WIELD);
            msr_weapon_next_selection(monster);
            break;
        case CDN_EF_DISABLE_RARM:
            inv_disable_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
            msr_weapon_next_selection(monster);
            break;

        case CDN_EF_DAMAGE:
            effect_clr_flag(ces, CDN_ESF_ACTIVE); 
            break;
        case CDN_EF_DAMAGE_TICK: break;

        case CDN_EF_HEALTH: 
            effect_clr_flag(ces, CDN_ESF_ACTIVE); 
            break;
        case CDN_EF_HEALTH_TICK: break;

        case CDN_EF_EXPLODE: {
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
        case CDN_EF_INSTANT_DEATH: {
            effect_clr_flag(ces, CDN_ESF_ACTIVE);
            msr_die(monster, gbl_game->current_map);
            return;
        } break;

        default: 
            effect_clr_flag(ces, CDN_ESF_ACTIVE); break;
    }
}

void cdn_process_effects_last(struct condition_effect_struct *ces, struct msr_monster *monster, struct cdn_condition *c) {
    if (msr_verify_monster(monster) == false) return;
    if (cdn_verify_condition(c) == false) return;

    int mod = -1;
    switch(ces->effect) {
        case CDN_EF_DECREASE_MOVEMENT: break;
        case CDN_EF_INCREASE_MOVEMENT: break;
        case CDN_EF_STUMBLE: break;
        case CDN_EF_BLINDNESS: break;
        case CDN_EF_DEAFNESS: break;
        case CDN_EF_STUNNED: break;
        case CDN_EF_HALUCINATING: break;
        case CDN_EF_CONFUSED: break;
        case CDN_EF_INHIBIT_FATE_POINT: break;
        case CDN_EF_PINNED: break;
        case CDN_EF_ON_FIRE: break;
        case CDN_EF_PSYCHIC_ENHANCE: break;
        case CDN_EF_DECREASE_FATIQUE: break;
        case CDN_EF_INCREASE_FATIQUE: break;
        case CDN_EF_INCREASE_ALL_SKILLS: break;
        case CDN_EF_DECREASE_ALL_SKILLS: break;
        case CDN_EF_DAMAGE: break;
        case CDN_EF_DAMAGE_TICK: break;
        case CDN_EF_HEALTH: break;
        case CDN_EF_HEALTH_TICK: break;
        case CDN_EF_BLOODLOSS: break;

        case CDN_EF_SET_WS:
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = ces->strength;
            break;
        case CDN_EF_SET_BS: 
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].base_value = ces->strength;
            break;
        case CDN_EF_SET_STR: 
            monster->characteristic[MSR_CHAR_STRENGTH].base_value = ces->strength;
            break;
        case CDN_EF_SET_AG:
            monster->characteristic[MSR_CHAR_AGILITY].base_value = ces->strength;
            break;
        case CDN_EF_SET_TGH:
            monster->characteristic[MSR_CHAR_TOUGHNESS].base_value = ces->strength;
            break;
        case CDN_EF_SET_PER:
            monster->characteristic[MSR_CHAR_PERCEPTION].base_value = ces->strength;
            break;
        case CDN_EF_SET_WILL:
            monster->characteristic[MSR_CHAR_WILLPOWER].base_value = ces->strength;
            break;
        case CDN_EF_SET_INT:
            monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value = ces->strength;
            break;
        case CDN_EF_SET_FEL:
            monster->characteristic[MSR_CHAR_FELLOWSHIP].base_value = ces->strength;
            break;

        case CDN_EF_DECREASE_WS: mod = 1;
        case CDN_EF_INCREASE_WS:
                monster->characteristic[MSR_CHAR_WEAPON_SKILL].mod += (ces->strength * mod) * ces->ticks_applied;
                break;

        case CDN_EF_DECREASE_BS: mod = 1;
        case CDN_EF_INCREASE_BS:
                monster->characteristic[MSR_CHAR_BALISTIC_SKILL].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_STR: mod = 1;
        case CDN_EF_INCREASE_STR:
                monster->characteristic[MSR_CHAR_STRENGTH].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_AG: mod = 1;
        case CDN_EF_INCREASE_AG:
                monster->characteristic[MSR_CHAR_AGILITY].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_TGH: mod = 1;
        case CDN_EF_INCREASE_TGH:
                monster->characteristic[MSR_CHAR_TOUGHNESS].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_PER: mod = 1;
        case CDN_EF_INCREASE_PER:
                monster->characteristic[MSR_CHAR_PERCEPTION].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_WILL: mod = 1;
        case CDN_EF_INCREASE_WILL:
                monster->characteristic[MSR_CHAR_WILLPOWER].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_INT: mod = 1;
        case CDN_EF_INCREASE_INT:
                monster->characteristic[MSR_CHAR_INTELLIGENCE].mod += (ces->strength * mod) * ces->ticks_applied;
                break;
        case CDN_EF_DECREASE_FEL: mod = 1;
        case CDN_EF_INCREASE_FEL:
                monster->characteristic[MSR_CHAR_FELLOWSHIP].mod += (ces->strength * mod) * ces->ticks_applied;
                break;

        case CDN_EF_DECREASE_MAX_WOUNDS: mod = 1;
        case CDN_EF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += (ces->strength * mod) * ces->ticks_applied;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case CDN_EF_DISABLE_LLEG: break;
        case CDN_EF_DISABLE_RLEG: break;
        case CDN_EF_DISABLE_LARM:
            if (cdn_has_effect_skip(monster->conditions, CDN_EF_DISABLE_LARM, c) )
                break;
            inv_enable_location(monster->inventory, INV_LOC_OFFHAND_WIELD);
            break;
        case CDN_EF_DISABLE_RARM:
            if (cdn_has_effect_skip(monster->conditions, CDN_EF_DISABLE_RARM, c) ) 
                break;
            inv_enable_location(monster->inventory, INV_LOC_MAINHAND_WIELD);
            break;

        case CDN_EF_DETOX: {
            /* Remove all conditions which are 'DETOXABLE' */
            struct cdn_condition *c_temp = NULL;
            struct cdn_condition_list *cdn_list = monster->conditions;
            while ( (c_temp = cdn_list_get_next_condition(cdn_list, c_temp) ) != NULL) {
                if (condition_has_flag(c, CDN_CF_DETOXABLE) ) {
                    cdn_remove_condition(cdn_list, c_temp);
                    c_temp = NULL;
                }
            }
        } break;

        case CDN_EF_EXPLODE:
        case CDN_EF_INSTANT_DEATH: {
            effect_clr_flag(ces, CDN_ESF_ACTIVE);
            msr_die(monster, gbl_game->current_map);
            return;
        } break;

        default: 
            effect_clr_flag(ces, CDN_ESF_ACTIVE); break;
    }

    ces->tick_energy = 0;
    effect_clr_flag(ces, CDN_ESF_ACTIVE);
}

void cdn_process_effects_during(struct condition_effect_struct *ces, struct msr_monster *monster, struct cdn_condition *c) {
    if (msr_verify_monster(monster) == false) return;
    if (cdn_verify_condition(c) == false) return;

    int mod = 1;
    switch(ces->effect) {
        case CDN_EF_DECREASE_MOVEMENT: break;
        case CDN_EF_INCREASE_MOVEMENT: break;
        case CDN_EF_STUMBLE: break;
        case CDN_EF_BLINDNESS: break;
        case CDN_EF_DEAFNESS: break;
        case CDN_EF_STUNNED: break;
        case CDN_EF_HALUCINATING: break;
        case CDN_EF_CONFUSED: break;
        case CDN_EF_INHIBIT_FATE_POINT: break;
        case CDN_EF_PINNED: break;
        case CDN_EF_ON_FIRE: break;
        case CDN_EF_PSYCHIC_ENHANCE: break;
        case CDN_EF_EXPLODE: break;
        case CDN_EF_INSTANT_DEATH: break;
        case CDN_EF_SET_WS: break;
        case CDN_EF_SET_BS: break;
        case CDN_EF_SET_STR: break;
        case CDN_EF_SET_AG: break;
        case CDN_EF_SET_TGH: break;
        case CDN_EF_SET_PER: break;
        case CDN_EF_SET_WILL: break;
        case CDN_EF_SET_INT: break;
        case CDN_EF_SET_FEL: break;
        case CDN_EF_INCREASE_ALL_SKILLS: break;
        case CDN_EF_DECREASE_ALL_SKILLS: break;
        case CDN_EF_DISABLE_LLEG: break;
        case CDN_EF_DISABLE_RLEG: break;
        case CDN_EF_DISABLE_LARM: break;
        case CDN_EF_DISABLE_RARM: break;
        case CDN_EF_DETOX: break;

        case CDN_EF_DECREASE_FATIQUE: mod = -1;
        case CDN_EF_INCREASE_FATIQUE:
            monster->fatique += ces->strength * ( -1 * mod);
            monster->fatique_turn = gbl_game->turn;
            break;

        case CDN_EF_DECREASE_WS: mod = -1;
        case CDN_EF_INCREASE_WS:
            monster->characteristic[MSR_CHAR_WEAPON_SKILL].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_BS: mod = -1;
        case CDN_EF_INCREASE_BS:
            monster->characteristic[MSR_CHAR_BALISTIC_SKILL].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_STR: mod = -1;
        case CDN_EF_INCREASE_STR:
            monster->characteristic[MSR_CHAR_STRENGTH].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_AG: mod = -1;
        case CDN_EF_INCREASE_AG:
            monster->characteristic[MSR_CHAR_AGILITY].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_TGH: mod = -1;
        case CDN_EF_INCREASE_TGH:
            monster->characteristic[MSR_CHAR_TOUGHNESS].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_PER: mod = -1;
        case CDN_EF_INCREASE_PER:
            monster->characteristic[MSR_CHAR_PERCEPTION].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_WILL: mod = -1;
        case CDN_EF_INCREASE_WILL:
            monster->characteristic[MSR_CHAR_WILLPOWER].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_INT: mod = -1;
        case CDN_EF_INCREASE_INT:
            monster->characteristic[MSR_CHAR_INTELLIGENCE].mod += ces->strength * mod;
            break;
        case CDN_EF_DECREASE_FEL: mod = -1;
        case CDN_EF_INCREASE_FEL:
            monster->characteristic[MSR_CHAR_FELLOWSHIP].mod += ces->strength * mod;
            break;

        case CDN_EF_DAMAGE: break;
        case CDN_EF_DAMAGE_TICK: 
            msr_do_dmg(monster, ces->strength, DMG_TYPE_MAGIC, MSR_HITLOC_NONE);
            break;

        case CDN_EF_HEALTH: break;
        case CDN_EF_HEALTH_TICK:
            monster->cur_wounds += ces->strength;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case CDN_EF_DECREASE_MAX_WOUNDS: mod = -1;
        case CDN_EF_INCREASE_MAX_WOUNDS:
            monster->max_wounds += ces->strength * mod;
            monster->cur_wounds = (monster->cur_wounds < monster->max_wounds) ? 
                                    monster->cur_wounds : monster->max_wounds;
            break;

        case CDN_EF_BLOODLOSS:
            if (random_d100(gbl_game->random) < 5) {
                You(monster, "die due to bloodless.");
                Monster(monster, "dies in a large pool of blood.");
                msr_die(monster, gbl_game->current_map);
            }
            break;

        default: break;
            effect_clr_flag(ces, CDN_ESF_ACTIVE); break;
    }

    ces->tick_energy = ces->tick_energy_max;
    ces->ticks_applied++;
}

/* Process conditions */
void cdn_process(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;

    struct cdn_condition_list *cdn_list = monster->conditions;
    if (cdn_verify_list(cdn_list) == false) return;

    struct cdn_condition *c_prev = NULL;
    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {

        /* 
           if the monster is dead, do nothing.
           we do this here because an effect can cause death.
         */
        if (monster->dead) return;

        /*  Paranoia */
        if (cdn_verify_condition(c) == false) return;

        bool destroy    = false;
        bool first_time = false;
        bool last_time  = true;

        /* Check if this condition is new, or maybe even for the last time. */
        if (c->duration_energy == c->duration_energy_max) first_time = true;
        if ( (condition_has_flag(c, CDN_CF_PERMANENT) == true) ) last_time = false;
        else if (c->duration_energy > 0) last_time = false;

        {   /* Pre checks */
            if (first_time) {
                lg_debug("Condition %p(%s) is processed for the first time.", c, c->name);

                int inactive = 0;
                for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
                    struct condition_effect_struct *ces = &c->effects[i];
                    if (ces->effect != CDN_EF_NONE) {
                        if (effect_has_flag(ces, CDN_ESF_ACTIVE) == false) {
                            inactive++;
                        }
                    }
                }
                if (inactive == ARRAY_SZ(c->effects) ) destroy = true;

                if (c->template_id == CID_NONE) destroy = true;

                if (condition_has_flag(c, CDN_CF_REQ_WILL_CHECK) ) {
                    if ( (msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                }
                if (condition_has_flag(c, CDN_CF_REQ_TGH_CHECK) ) {
                    if ( (msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                }
                if (condition_has_flag(c, CDN_CF_REQ_AG_CHECK) ) {
                    if ( (msr_characteristic_check(monster, MSR_CHAR_AGILITY, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                }
                if (condition_has_flag(c, CDN_CF_REQ_CHEM_USE_CHECK) ) {
                    if ( (msr_skill_check(monster, MSR_SKILLS_CHEM_USE, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                }

                if (destroy == false) {
                    if (condition_has_flag(c, CDN_CF_INVISIBLE) == false) {
                        lg_debug("Condition %p(%s) is to be applyed.", c, c->name);
                        if (c->on_apply_plr != NULL) You_msg(monster, c->on_apply_plr);
                        if (c->on_apply_msr != NULL) Monster_msg(monster, c->on_apply_msr, msr_ldname(monster) );
                    }
                }
                else {
                    first_time = false;
                }
            }
            else if (last_time) {
                lg_debug("Condition %p(%s) ends.", c, c->name);

                if (condition_has_flag(c, CDN_CF_INVISIBLE) == false) {
                    if (c->on_exit_plr != NULL) You_msg(monster, c->on_exit_plr);
                    if (c->on_exit_msr != NULL) Monster_msg(monster, c->on_exit_msr, msr_ldname(monster) );
                }
                destroy = true;
            }
        }

        /*
           Remove the id which this condition continues in,
           apparently this continues releaves the cravings 
           set by the continues_condition. Later offcourse,
           this condition will continue in that so it will 
           not stop it at all.
         */
        if (condition_has_flag(c, CDN_CF_REMOVE_CONTINUE) ) {
            struct cdn_condition *c_temp = NULL;
            while ( (c_temp = cdn_get_condition_tid(cdn_list, c->continues_to_id) ) != NULL) {
                if (condition_has_flag(c_temp, CDN_CF_INVISIBLE) == false) {
                    if (c_temp->on_exit_plr != NULL) You(monster, "%s.", c_temp->on_exit_plr);
                    if (c_temp->on_exit_msr != NULL) Monster(monster, "%s.", c_temp->on_exit_msr);
                }
                cdn_remove_condition(cdn_list, c_temp);
            }
        }

        for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
            struct condition_effect_struct *ces = &c->effects[i];
            if (effect_has_flag(ces, CDN_ESF_ACTIVE) == false) continue;

            if (effect_has_flag(ces, CDN_ESF_TICK) && (ces->tick_energy > 0) ) {
                ces->tick_energy -= TT_ENERGY_TICK;
                if (!first_time && !last_time) continue;
            }

            /* 
               These tests will check if the condition still has to be active.
               They are run the first time and whenever another tick happens.
             */
            if (effect_has_flag(ces, CDN_ESF_REQ_WILL_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, ces->difficulty) >= 1) {
                    effect_clr_flag(ces, CDN_ESF_ACTIVE);
                }
            }
            if (effect_has_flag(ces, CDN_ESF_REQ_TGH_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, ces->difficulty) >= 1) {
                    effect_clr_flag(ces, CDN_ESF_ACTIVE);
                }
            }
            if (effect_has_flag(ces, CDN_ESF_REQ_AG_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_AGILITY, ces->difficulty) >= 1) {
                    effect_clr_flag(ces, CDN_ESF_ACTIVE);
                }
            }
            if (effect_has_flag(ces, CDN_ESF_REQ_CHEM_USE_CHECK) ) {
                if (msr_skill_check(monster, MSR_SKILLS_CHEM_USE, ces->difficulty) >= 1) {
                    effect_clr_flag(ces, CDN_ESF_ACTIVE);
                }
            }

            if (first_time) {
                cdn_process_effects_first(ces, monster, c);
                cdn_process_effects_during(ces, monster, c);

                /* if ONCE is set, the second time this effect has been processed, 
                   disable it. the first time will be the TICK directly after the 
                   condition is applied, the second time will be the turn after 
                   that. */
                if (effect_has_flag(ces, CDN_ESF_ONCE) ) {
                    effect_clr_flag(ces, CDN_ESF_ACTIVE);
                }
            }
            else if (last_time) {
                cdn_process_effects_during(ces, monster, c);
                cdn_process_effects_last(ces, monster, c);
                destroy = true;
            }
            else if (effect_has_flag(ces, CDN_ESF_TICK) && (ces->tick_energy <= 0) ) {
                cdn_process_effects_during(ces, monster, c);
            }
        }

        if (destroy) {
            lg_debug("Condition %p(%s) is to be destroyed.", c, c->name);
            if (c->continues_to_id != CID_NONE) {
                cdn_add_condition(cdn_list, c->continues_to_id);
            }
            cdn_remove_condition(cdn_list, c);
            c = c_prev;
        }
        else if (c->duration_energy > 0) {
            c->duration_energy -= MIN(TT_ENERGY_TICK, c->duration_energy);
        }

        c_prev = c;
    }
}

void cdn_remove_all_non_permanent(struct msr_monster *monster) {
    struct cdn_condition *c = NULL;
    struct cdn_condition_list *cdn_list = monster->conditions;

    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if ( (condition_has_flag(c, CDN_CF_PERMANENT) == false) ) {
            cdn_remove_condition(cdn_list, c);
        }
    }
}

