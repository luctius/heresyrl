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
    uint32_t id = 0;

    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (c->uid >= id) id = c->uid+1;
    }

    return id;
}

static int cdn_calc_damage(struct condition_effect_struct *ces) {
    int dmg = 0;
    bool health = false;
    switch(ces->damage) {
        default: 
        case CDN_DAMAGE_NONE: dmg = 0;  break;

        case CDN_HEALTH_ONE:  health = true;
        case CDN_DAMAGE_ONE:  dmg = 1;  break;

        case CDN_HEALTH_TWO:  health = true;
        case CDN_DAMAGE_TWO:  dmg = 2;  break;

        case CDN_HEALTH_THREE:  health = true;
        case CDN_DAMAGE_THREE:  dmg = 3;  break;

        case CDN_HEALTH_FIVE: health = true;
        case CDN_DAMAGE_FIVE: dmg = 5;  break;

        case CDN_HEALTH_TEN:  health = true;
        case CDN_DAMAGE_TEN:  dmg = 10; break;

        case CDN_HEALTH_1D5:  health = true;
        case CDN_DAMAGE_1D5:  dmg = random_xd5(gbl_game->random, 1); break;

        case CDN_HEALTH_1D10: health = true;
        case CDN_DAMAGE_1D10: dmg = random_xd10(gbl_game->random, 1); break;;

        case CDN_HEALTH_2D10: health = true;
        case CDN_DAMAGE_2D10: dmg = random_xd10(gbl_game->random, 2); break;

        case CDN_HEALTH_3D10: health = true;
        case CDN_DAMAGE_3D10: dmg = random_xd10(gbl_game->random, 3); break;

        case CDN_HEALTH_4D10: health = true;
        case CDN_DAMAGE_4D10: dmg = random_xd10(gbl_game->random, 4); break;
    }

    if (health == false) dmg *= -1;
    return dmg;
}

bool cdn_add_condition(struct cdn_condition_list *cdn_list, uint32_t tid) {
    if (cdn_verify_list(cdn_list) == false) return false;
    if (tid == CID_NONE) return false;
    if (tid >= ARRAY_SZ(static_condition_list) ) return false;

    struct cdn_condition *cdn_template = &static_condition_list[tid];
    if (cdn_template == NULL) return false;

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
    cc->duration_energy_max = cc->duration_energy;

    cc->duration_energy_max *= TT_ENERGY_TURN;
    cc->duration_energy_min *= TT_ENERGY_TURN;
    cc->duration_energy     *= TT_ENERGY_TURN;

    for (unsigned int i = 0; i < ARRAY_SZ(cc->effects); i++ ) {
        struct condition_effect_struct *ces = &cc->effects[i];
        if (test_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE) ) {
            ces->tick_energy_max *= TT_ENERGY_TURN;
            ces->damage = cdn_calc_damage(ces);
        }
    }

    LIST_INSERT_HEAD(&cdn_list->head, ce, entries);

    return true;
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

struct cdn_condition *cdn_get_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid) {
    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (c->template_id == tid) {
            return c;
        }
    }
    return NULL;
}

bool cdn_condition_has_effect(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect) {
    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        for (unsigned int  i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                return true;
            }
        }
    }

    return false;
}

int cdn_condition_effect_damage(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect) {
    struct cdn_condition *c = NULL;

    int damage = 0;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                damage += c->effects[i].damage;
            }
        }
    }

    return damage;
}

enum cdn_priority cdn_condition_effect_priority(struct cdn_condition_list *cdn_list, enum condition_effect_flags effect) {
    struct cdn_condition *c = NULL;

    enum cdn_priority priority = CDN_PRIORITY_NONE;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        for (unsigned int i = 0; i < ARRAY_SZ(c->effects); i++) {
            if (c->effects[i].effect == effect) {
                if (priority < c->effects[i].priority) {
                    priority = c->effects[i].priority;
                }
            }
        }
    }

    return priority;
}

void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster) {
    if (cdn_verify_list(cdn_list) == false) return;
    if (msr_verify_monster(monster) == false) return;

    struct cdn_condition *c_prev = NULL;
    struct cdn_condition *c = NULL;
    while ( (c = cdn_list_get_next_condition(cdn_list, c) ) != NULL) {
        if (monster->dead) return;
        if (cdn_verify_condition(c) == false) return;

        bool destroy = false;
        bool first_time = false;
        bool last_time = true;
        if (c->duration_energy == c->duration_energy_max) first_time = true;
        if (c->duration_energy > 0) last_time = false;

        {   /* Pre checks */
            if (first_time) {

                {
                    struct condition_effect_struct *ces = &c->effects[0];
                    if (test_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE) == false) {
                        destroy = true;
                    }
                }

                if (c->template_id == CID_NONE) destroy = true;

                int DoS_best = -100;
                int DoS = -100;

                if (test_bf(c->setting_flags, CDN_SF_REQ_WILL_CHECK) ) {
                    if ( (DoS = msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                    if (DoS_best < DoS) DoS_best = DoS;
                }
                if (test_bf(c->setting_flags, CDN_SF_REQ_TGH_CHECK) ) {
                    if ( (DoS = msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                    if (DoS_best < DoS) DoS_best = DoS;
                }
                if (test_bf(c->setting_flags, CDN_SF_REQ_AG_CHECK) ) {
                    if ( (DoS = msr_characteristic_check(monster, MSR_CHAR_AGILITY, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                    if (DoS_best < DoS) DoS_best = DoS;
                }
                if (test_bf(c->setting_flags, CDN_SF_REQ_CHEM_USE_CHECK) ) {
                    if ( (DoS = msr_skill_check(monster, MSR_SKILLS_CHEM_USE, c->difficulty) ) >= 1) {
                        destroy = true;
                    }
                    if (DoS_best < DoS) DoS_best = DoS;
                }

                if (DoS_best > -100) {
                    c->duration_energy_max += (abs(DoS_best) / 2) * TT_ENERGY_TURN;
                    c->duration_energy_min = c->duration_energy_max;
                    c->duration_energy = c->duration_energy_max;
                }

                if (destroy == false) {
                    if (test_bf(c->setting_flags, CDN_SF_INVISIBLE) == false) {
                        if (c->on_apply_plr != NULL) You(monster, "%s.", c->on_apply_plr);
                        if (c->on_apply_msr != NULL) Monster(monster, "%s.", c->on_apply_msr);
                    }
                }
            }
            else if (last_time) {
                if (test_bf(c->setting_flags, CDN_SF_INVISIBLE) == false) {
                    if (c->on_exit_plr != NULL) You(monster, "%s.", c->on_exit_plr);
                    if (c->on_exit_msr != NULL) Monster(monster, "%s.", c->on_exit_msr);
                }
                destroy = true;
            }

            if (destroy) {
                if (c->continues_to_id != CID_NONE) {
                    cdn_add_condition(cdn_list, c->continues_to_id);
                }
                cdn_remove_condition(cdn_list, c);
                c = c_prev;
                continue;
            }
        }

        for (unsigned int i = 0; i < (ARRAY_SZ(c->effects) ); i++) {
            struct condition_effect_struct *ces = &c->effects[i];
            if (test_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE) == false) continue;

            /* Checks per tick */
            if (test_bf(ces->effect_setting_flags, CDN_ESF_REQ_WILL_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_WILLPOWER, ces->difficulty) >= 1) {
                    clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                }
            }
            if (test_bf(ces->effect_setting_flags, CDN_ESF_REQ_TGH_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_TOUGHNESS, ces->difficulty) >= 1) {
                    clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                }
            }
            if (test_bf(ces->effect_setting_flags, CDN_ESF_REQ_AG_CHECK) ) {
                if (msr_characteristic_check(monster, MSR_CHAR_AGILITY, ces->difficulty) >= 1) {
                    clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                }
            }
            if (test_bf(ces->effect_setting_flags, CDN_ESF_REQ_CHEM_USE_CHECK) ) {
                if (msr_skill_check(monster, MSR_SKILLS_CHEM_USE, ces->difficulty) >= 1) {
                    clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                }
            }
            if (test_bf(ces->effect_setting_flags, CDN_ESF_INACTIVE_IF_LESS_PRIORITY) ) {
                enum cdn_priority priority = cdn_condition_effect_priority(cdn_list, ces->effect);
                if (priority > ces->priority) {
                    clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                }
            }

            bool process = false;

            if (test_bf(ces->effect_setting_flags, CDN_ESF_TICK) && (ces->tick_energy == 0) ) {
                process = true;
            }
            else if (test_bf(ces->effect_setting_flags, CDN_ESF_TICK) && (ces->tick_energy > 0) ) {
                ces->tick_energy -= MIN(TT_ENERGY_TICK, ces->tick_energy);
            }
            else if (first_time) {
                process = true;
            }

            if (process) {
                switch(ces->effect) {
                    case CDN_EF_MODIFY_FATIQUE: break;
                    case CDN_EF_MODIFY_MOVEMENT: break;
                    case CDN_EF_MODIFY_WS: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_BS: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_WEAPON_SKILL].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_STR: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_STRENGTH].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_AG: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_AGILITY].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_TGH: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_TOUGHNESS].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_PER: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_PERCEPTION].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_WILL: 
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_WILLPOWER].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_INT:
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_INTELLIGENCE].base_value = ces->damage;
                            break;
                        }
                    case CDN_EF_MODIFY_FEL: {
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_MODIFY_BASE) ) {
                            monster->characteristic[MSR_CHAR_FELLOWSHIP].base_value = ces->damage;
                            break;
                        }
                        else if (first_time == false) {
                            if (ces->damage != 0) {
                                if (ces->damage > 0) ces->damage -= 1;
                                else if (ces->damage < 0) ces->damage += 1;
                            }
                        }
                    }break;

                    case CDN_EF_MODIFY_ALL_SKILLS: break;

                    case CDN_EF_DISABLE_LLEG: break;

                    case CDN_EF_DISABLE_RLEG: break;

                    case CDN_EF_DISABLE_LARM: break;

                    case CDN_EF_DISABLE_RARM: break;

                    case CDN_EF_DISABLE_CHEST: break;

                    case CDN_EF_DAMAGE_TICK: {
                        enum msr_hit_location mhl = msr_get_hit_location(monster, random_d100(gbl_game->random) );
                        enum dmg_type dmg_type = DMG_TYPE_IMPACT;
                        if (test_bf(ces->effect_setting_flags, CDN_ESF_DMG_TYPE_ENERGY) ) dmg_type = DMG_TYPE_ENERGY;
                        else if (test_bf(ces->effect_setting_flags, CDN_ESF_DMG_TYPE_RENDING) ) dmg_type = DMG_TYPE_RENDING;
                        else if (test_bf(ces->effect_setting_flags, CDN_ESF_DMG_TYPE_EXPLODING) ) dmg_type = DMG_TYPE_EXPLOSIVE;
                        else if (test_bf(ces->effect_setting_flags, CDN_ESF_DMG_TYPE_IMPACT) ) dmg_type = DMG_TYPE_IMPACT;

                        msr_do_dmg(monster, ces->damage, dmg_type, mhl, gbl_game->current_map);
                    } break;

                    case CDN_EF_HEALTH_TICK: {
                        if (monster->cur_wounds < monster->max_wounds) {
                            monster->cur_wounds += 1;
                        }
                    } break;

                    case CDN_EF_BLINDNESS: break;

                    case CDN_EF_DEAFNESS: break;

                    case CDN_EF_STUNNED: break;

                    case CDN_EF_HALUCINATING: break;

                    case CDN_EF_CONFUSED: break;

                    case CDN_EF_INHIBIT_FATE_POINT: break;

                    case CDN_EF_PINNED: break;

                    case CDN_EF_ON_FIRE: break;

                    case CDN_EF_PSYCHIC_ENHANCE: break;

                    case CDN_EF_DETOX: break;

                    case CDN_EF_DEATH: {
                        clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE);
                        monster->dead = true; 
                        return;
                    } break;

                    default: 
                        clr_bf(ces->effect_setting_flags, CDN_ESF_ACTIVE); break;
                }

                ces->tick_energy = ces->tick_energy_max;
            }
        }

        if (c->duration_energy > 0) {
            if ( (test_bf(c->setting_flags, CDN_SF_PERMANENT) == false) || (first_time) ) {
                c->duration_energy -= MIN(TT_ENERGY_TICK, c->duration_energy);
            }
        }

        c_prev = c;
    }
}

