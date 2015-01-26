#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include <ncurses.h>
#include <assert.h>
#include <float.h>

#include "monster.h"
#include "monster_static.h"
#include "status_effects/status_effects.h"
#include "game.h"
#include "random.h"
#include "tiles.h"
#include "turn_tick.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "dungeon/dungeon_map.h"
#include "dowear.h"

static TAILQ_HEAD(monster_list, msr_monster_list_entry) monster_list_head;
static bool monster_list_initialised = false;

#include "monster_static.c"

struct msr_monster_list_entry {
    struct msr_monster monster;
    TAILQ_ENTRY(msr_monster_list_entry) entries;
};

void msrlst_monster_list_init(void) {
    for (unsigned int i = 1; i < ARRAY_SZ(static_monster_list); i++) {
        struct msr_monster *template_monster = &static_monster_list[i];
        if (template_monster->icon == '\0') {
            fprintf(stderr, "Monster list integrity check failed! [%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }

    if (monster_list_initialised == false) {
        monster_list_initialised = true;
        TAILQ_INIT(&monster_list_head);
    }
}

void msrlst_monster_list_exit(void) {
    struct msr_monster_list_entry *e = NULL;
    while (monster_list_head.tqh_first != NULL) {
        e = monster_list_head.tqh_first;
        TAILQ_REMOVE(&monster_list_head, monster_list_head.tqh_first, entries);
        msr_clear_controller(&e->monster);
        se_list_exit(e->monster.status_effects);
        inv_exit(e->monster.inventory);
        free(e);
    }
    monster_list_initialised = false;
}

struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev) {
    if (prev == NULL) {
        if (monster_list_head.tqh_first != NULL) return &monster_list_head.tqh_first->monster;
        return NULL;
    }
    struct msr_monster_list_entry *mle = container_of(prev, struct msr_monster_list_entry, monster);
    if (mle == NULL) return NULL;
    return &mle->entries.tqe_next->monster;
}

static uint32_t msrlst_next_id(void) {
    if (monster_list_initialised == false) return false;
    struct msr_monster_list_entry *me = monster_list_head.tqh_first;
    uint32_t uid = 1;

    while (me != NULL) {
        if (uid <= me->monster.uid) uid = me->monster.uid+1;
        me = me->entries.tqe_next;
    }
    return uid;
}

int msr_spawn(double roll, int level, enum dm_dungeon_type dt) {
    int sz = ARRAY_SZ(static_monster_list);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0;

    int idx = MID_NONE;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = MID_NONE+1; i < sz; i++) {
        bool valid = false;
        if (level <= static_monster_list[i].level) {

            if (test_bf(static_monster_list[i].dungeon_locale, dt) ||
                 test_bf(static_monster_list[i].dungeon_locale, DUNGEON_TYPE_ALL) ){
                valid = true;
                sum += static_monster_list[i].weight;
            }
        }

        if (!valid) cumm_prob_arr[i] = DBL_MAX;
    }

    double cumm = 0;
    for (int i = MID_NONE+1; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = static_monster_list[i].weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    for (int i = sz-1; i > MID_NONE+1; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    return idx;
}

#define MONSTER_PRE_CHECK (10477)
#define MONSTER_POST_CHECK (10706)
void msr_give_items(struct msr_monster *m, int level, struct random *r);
static void creature_weapon(struct msr_monster *monster);

struct msr_monster *msr_create(enum msr_ids template_id) {
    if (monster_list_initialised == false) msrlst_monster_list_init();
    if (template_id >= (int) ARRAY_SZ(static_monster_list)) return NULL;
    if (template_id == MID_NONE) return NULL;
    struct msr_monster *template_monster = &static_monster_list[template_id];

    struct msr_monster_list_entry *m = calloc(1,sizeof(struct msr_monster_list_entry) );
    assert(m != NULL);

    memcpy(&m->monster, template_monster, sizeof(struct msr_monster) );
    m->monster.controller.ai.ai_ctx = NULL;
    m->monster.controller.controller_cb = NULL;
    m->monster.pos = cd_create(0,0);
    m->monster.uid = msrlst_next_id();
    m->monster.template_id = template_id;
    m->monster.energy = TT_ENERGY_FULL;
    m->monster.faction = 1;
    m->monster.inventory = NULL;
    m->monster.status_effects = se_list_init();
    if (m->monster.description == NULL) {
        m->monster.description = "none";
    }
    assert(m->monster.description != NULL);

    m->monster.monster_pre = MONSTER_PRE_CHECK;
    m->monster.monster_post = MONSTER_POST_CHECK;

    switch (m->monster.race) {
        case MSR_RACE_DWARF:
        case MSR_RACE_ELF:
        case MSR_RACE_HALFLING:
        case MSR_RACE_HUMAN:
            m->monster.inventory = inv_init(inv_loc_human);
            break;
        case MSR_RACE_BEAST:
        case MSR_RACE_DOMESTIC:
            m->monster.inventory = inv_init(inv_loc_animal);
            break;
        default:
            free(m);
            assert(false && "Unkown Race");
            return NULL;
            break;
    }

    creature_weapon(&m->monster);
    msr_give_items(&m->monster, m->monster.level, gbl_game->random);

    lg_debug("creating monster[%d, %s, %c]", m->monster.uid, m->monster.ld_name, m->monster.icon);

    TAILQ_INSERT_TAIL(&monster_list_head, m, entries);
    return &m->monster;
}

void msr_destroy(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return;
    struct msr_monster_list_entry *target_mle = container_of(monster, struct msr_monster_list_entry, monster);

    if (map != NULL) {
        if (dm_verify_map(map) == false) return;
        msr_remove_monster(monster, map);
    }
    inv_exit(monster->inventory);
    se_list_exit(monster->status_effects);

    if (monster->unique_name != NULL) free(monster->unique_name);

    TAILQ_REMOVE(&monster_list_head, target_mle, entries);
    free(target_mle);
}

bool msr_verify_monster(struct msr_monster *monster) {
    assert(monster != NULL);
    assert(monster->monster_pre == MONSTER_PRE_CHECK);
    assert(monster->monster_post == MONSTER_POST_CHECK);
    assert(inv_verify_inventory(monster->inventory) == true );
    assert(se_verify_list(monster->status_effects) == true);

    if (monster->dead == true) return false;

    return true;
}

struct msr_monster *msr_get_monster_by_uid(uint32_t uid) {
    if (monster_list_initialised == false) return NULL;
    struct msr_monster_list_entry *me = monster_list_head.tqh_first;

    while (me != NULL) {
        if (uid == me->monster.uid) return &me->monster;
        me = me->entries.tqe_next;
    }
    return NULL;
}

void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller) {
    if (msr_verify_monster(monster) == false) return;
    if (controller == NULL) return;
    memcpy(&monster->controller, controller, sizeof(struct monster_controller) );
}

void msr_clear_controller(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;
    ai_monster_free(monster);
    memset(&monster->controller, 0x0, sizeof(struct monster_controller) );
}

bool msr_insert_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dm_map_entity *me_future = dm_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (me_future->monster == NULL) {
            me_future->monster = monster;
            monster->pos = *pos;
            lg_ai_debug(monster, "inserting \'%s\'(%c) to (%d,%d)",
                        monster->sd_name, monster->icon, monster->pos.x, monster->pos.y);
            retval = true;
        }
    }

    return retval;
}

bool msr_move_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;
    if (cd_equal(&monster->pos, pos) == true ) return false;

    struct dm_map_entity *me_current = dm_get_map_me(&monster->pos, map);
    struct dm_map_entity *me_future = dm_get_map_me(pos, map);

    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        /*Speed of one for now*/
        if (cd_neighbour(&monster->pos, pos) == false) return false;
        /*coord_t mon_pos_new = cd_add(&monster->pos, pos);*/

        if (msr_insert_monster(monster, map, pos) == true) {
            me_current->monster = NULL;
            retval = true;
        }
        /*
        else if (msr_move_monster(me_future->monster, map, &mon_pos_new) ) {
            retval = msr_move_monster(monster, map, pos);
        }
        */
    }

    return retval;
}
bool msr_give_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == true) return false;

    if (inv_add_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_MONSTER;
        item->owner.owner_monster = monster;
    }
    return true;
}

bool msr_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (inv_remove_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_NONE;
        item->owner.owner_monster = NULL;
    }
    return true;
}

int msr_get_near_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_near = 5 + ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 0.5f) / 10) +1;
    if (msr_has_talent(monster, TLT_NIGHT_VISION) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_near < nv_sight) sight_near = nv_sight;
    }

    if (se_has_effect(monster->status_effects, SETF_BLINDNESS) ) sight_near = 0;
    return sight_near;
}

int msr_get_medium_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_medium = 5 + ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 1) / 10) +1;
    if (msr_has_talent(monster, TLT_NIGHT_VISION) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_medium < nv_sight) sight_medium = nv_sight;
    }

    if (se_has_effect(monster->status_effects, SETF_BLINDNESS) ) sight_medium = 0;
    return  sight_medium;
}

int msr_get_far_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_far = 5 + ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 2) / 10) +1;
    if (msr_has_talent(monster, TLT_NIGHT_VISION) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_far < nv_sight) sight_far = nv_sight;
    }

    if (se_has_effect(monster->status_effects, SETF_BLINDNESS) ) sight_far = 0;
    return sight_far;
}

bool msr_drop_inventory(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(monster->inventory, item) ) != NULL) {
        if (itm_verify_item(item) == true) {
            if (inv_remove_item(monster->inventory, item) ) {
                if (itm_insert_item(item, map, &monster->pos) == false) {
                    itm_destroy(item);
                }
                item = NULL;
            }
        }
    }

    return true;
}

bool msr_remove_monster(struct msr_monster *monster, struct dm_map *map) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct dm_map_entity *me_current = dm_get_map_me(&monster->pos, map);
    if (me_current->monster == monster) {
        lg_ai_debug(monster, "removed (%d,%d)", monster->pos.x, monster->pos.y);
        me_current->monster = NULL;
        monster->pos = cd_create(0,0);
        retval = true;
    }

    return retval;
}

int msr_get_energy(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return monster->energy;
}

bool msr_change_energy(struct msr_monster *monster, int energy) {
    if (msr_verify_monster(monster) == false) return false;
    monster->energy += energy;
    if (monster->energy < 0) monster->energy = 0;
    //lg_ai_debug(monster, "energy: %d", monster->energy);
    return true;
}

struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (mhl >= MSR_HITLOC_MAX) return NULL;
    struct itm_item *item = NULL;

    switch(mhl) {
        case MSR_HITLOC_BODY:      item = inv_get_item_from_location(monster->inventory, INV_LOC_BODY); break;
        case MSR_HITLOC_LEFT_LEG:
        case MSR_HITLOC_RIGHT_LEG: item = inv_get_item_from_location(monster->inventory, INV_LOC_LEGS); break;
        case MSR_HITLOC_LEFT_ARM:
        case MSR_HITLOC_RIGHT_ARM: item = inv_get_item_from_location(monster->inventory, INV_LOC_ARMS); break;
        case MSR_HITLOC_HEAD:      item = inv_get_item_from_location(monster->inventory, INV_LOC_HEAD); break;
        default: break;
    }
    return item;
}

int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return -1;
    if (mhl >= MSR_HITLOC_MAX) return 0;
    struct itm_item *item = msr_get_armour_from_hitloc(monster, mhl);
    int armour = 0;

    if (item != NULL) {
        if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
            armour += item->specific.wearable.damage_reduction;
        }
    }

    /* Add Armour talents here */

    return armour;
}

bool msr_can_use_evasion(struct msr_monster *monster, enum msr_evasions evasion) {
    if (msr_verify_monster(monster) == false) return false;
    if (monster->evasion_last_used[evasion] < (gbl_game->turn +TT_ENERGY_TURN) ) return false;

    struct inv_inventory *inv = monster->inventory;
    if (inv_verify_inventory(monster->inventory) == false) return false;

    switch(evasion) {
        case MSR_EVASION_MAIN_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
            if (wpn_is_type(witem, WEAPON_TYPE_MELEE) == false) return false;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
            break; }
        case MSR_EVASION_OFF_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (wpn_is_type(witem, WEAPON_TYPE_MELEE) == false) return false;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
            break; }
        case MSR_EVASION_DODGE:
            if (msr_has_skill(monster, MSR_SKILLS_DODGE) == false) return false;
            break;
        case MSR_EVASION_MAX:
        default:
            return false;
    }

    return true;
}

bool msr_use_evasion(struct msr_monster *monster, struct msr_monster *attacker, struct itm_item *atk_wpn, enum msr_evasions evasion, int to_hit_DoS, int mod) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_can_use_evasion(monster, evasion) == false) return false;

    FIX_UNUSED(attacker);

    struct inv_inventory *inv = monster->inventory;
    if (inv_verify_inventory(inv) == false) return false;

    if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_FLEXIBLE) ) return false;
    if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_FAST) ) mod -= 10;
    if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_SLOW) ) mod += 10;

    int roll = 0;
    switch(evasion) {
        case MSR_EVASION_MAIN_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_DEFENSIVE) ) mod += 10;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNWIELDY) ) mod -= 10;
            roll = msr_characteristic_check(monster, MSR_CHAR_WEAPON_SKILL, mod);
            Info("Using main-hand, unable to parry with it for one turn.");
            break; }
        case MSR_EVASION_OFF_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_DEFENSIVE) ) mod += 10;
            if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNWIELDY) ) mod -= 10;
            roll = msr_characteristic_check(monster, MSR_CHAR_WEAPON_SKILL, mod);
            Info("Using off-hand, unable to parry or attack with it for one turn.");
            break; }
        case MSR_EVASION_DODGE: {
            roll = msr_skill_check(monster, MSR_SKILLS_DODGE, mod);
            Info("Using dodge, cannot dodge anymore for one turn.");
            break; }
        case MSR_EVASION_MAX:
        default:
            return false;
    }

    monster->evasion_last_used[evasion] = gbl_game->turn;
    return roll >= to_hit_DoS;
}

enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll) {
    if (msr_verify_monster(monster) == false) return MSR_HITLOC_NONE;
    if (hit_roll > 99) hit_roll %= 100;

    /* Human hitloc */
    if (hit_roll >= 85) return MSR_HITLOC_LEFT_LEG;
    if (hit_roll >= 70) return MSR_HITLOC_RIGHT_LEG;
    if (hit_roll >= 30) return MSR_HITLOC_BODY;
    if (hit_roll >= 20) return MSR_HITLOC_LEFT_ARM;
    if (hit_roll >= 10) return MSR_HITLOC_RIGHT_ARM;
    return MSR_HITLOC_HEAD;
}

bool msr_die(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    if (monster->is_player && (monster->fate_points > 0) ) {
        You(monster, "fall unconsious.");
        Monster(monster, "falls unconsious.");
    }
    else {
        You(monster, "die...");
        Monster(monster, "dies.");

        msr_drop_inventory(monster, map);
        msr_remove_monster(monster, map);
    }

    monster->dead = true;
    return true;
}

bool msr_do_dmg(struct msr_monster *monster, int dmg, enum dmg_type dmg_type, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return false;
    bool critic = false;
    int wounds_above_zero = 0;

    if (monster->cur_wounds < 0) critic = true;
    else wounds_above_zero = monster->cur_wounds;

    if (dmg > 0) {
        monster->cur_wounds -= dmg;

        if (monster->cur_wounds < 0) {
            if (critic == false) {
                You(monster, "are criticly wounded.");
                Monster(monster, "is criticly wounded.");
            }

            if ( (dmg - wounds_above_zero) > 10) {
                msr_die(monster, gbl_game->current_map);
            }
            /* do critical hits! */
            else if (mhl != MSR_HITLOC_NONE) {
                se_add_critical_hit(monster, dmg - wounds_above_zero, mhl, dmg_type);
                monster->insanity_points++;
            }
            else if (monster->cur_wounds < -30) {
                msr_die(monster, gbl_game->current_map);
            }
        }
        return true;
    }
    return false;
}

int msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr, int mod) {
    if (msr_verify_monster(monster) == false) return false;
    int charac = msr_calculate_characteristic(monster, chr);
    assert(charac >= 0);
    lg_print("You characteristic is (%d)", charac);

    lg_print("Check modifier is: %d (%d)", mod, charac + mod);

    int con_mod = 0;
    if (monster->fatique > 0) {
        con_mod -= (monster->fatique * 10);
    }

    charac += mod;
    lg_print("Conditions modifier is: %d (%d)", con_mod, charac);

    int roll = (int) (random_int32(gbl_game->random)%100);
    int result = (charac - roll);
    int DoS = result / 10;
    if (roll < charac) DoS += 1;
    return DoS;
}

int msr_skill_check(struct msr_monster *monster, enum msr_skills skill, int mod) {
    if (msr_verify_monster(monster) == false) return false;

    int con_mod = 0;
    int charac = msr_calculate_characteristic(monster, msr_skill_charac[skill]);
    assert(charac >= 0);
    lg_print("You characteristic is (%d)", charac);

    lg_print("Check modifier is: %d (%d)", mod, charac + mod);

    if (se_has_effect(monster->status_effects, SETF_INCREASE_ALL_SKILLS) ) {
        con_mod += se_status_effect_strength(monster->status_effects, SETF_INCREASE_ALL_SKILLS);
    }
    if (se_has_effect(monster->status_effects, SETF_DECREASE_ALL_SKILLS) ) {
        con_mod += se_status_effect_strength(monster->status_effects, SETF_DECREASE_ALL_SKILLS);
    }

    if (monster->fatique > 0) {
        con_mod -= (monster->fatique * 10);
    }

    mod += con_mod;
    charac += mod;
    lg_print("Conditions modifier is: %d (%d)", con_mod, charac);

    enum msr_skill_rate r = msr_has_skill(monster, skill);
    switch(r) {
        case MSR_SKILL_RATE_EXPERT:     charac += 20; lg_print("You are an expert (%d)", charac); break;
        case MSR_SKILL_RATE_ADVANCED:   charac += 10; lg_print("You are advanced (%d)", charac); break;
        case MSR_SKILL_RATE_BASIC:      lg_print("You have the skill (%d)", charac); break;
        case MSR_SKILL_RATE_NONE:       charac /= 2; lg_print("You do not have this skill (%d)", charac); break;
        default: assert(false); break;
    }

    int roll = (random_int32(gbl_game->random)%100);
    int result = charac - roll;
    int DoS = result /10;
    if (roll < charac) DoS += 1;

    lg_print("roll %d, result %d, DoS %d", roll, result, DoS);

    return DoS;
}

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX) return -1;
    int mod = 0;

    if (chr == MSR_CHAR_AGILITY) {
        if (inv_wears_wearable_with_spcqlty(monster->inventory, WBL_SPCQLTY_PLATE) ) {
            mod -= 20;
        }
        else if (inv_wears_wearable_with_spcqlty(monster->inventory, WBL_SPCQLTY_SCALE) ) {
            mod -= 10;
        }
        else if (inv_wears_wearable_with_spcqlty(monster->inventory, WBL_SPCQLTY_MAIL) ) {
            mod -= 10;
        }
    }

    int adv = 0;
    switch(chr) {
        case MSR_CHAR_WEAPON_SKILL:
        case MSR_CHAR_BALISTIC_SKILL:
        case MSR_CHAR_STRENGTH:
        case MSR_CHAR_TOUGHNESS:
        case MSR_CHAR_AGILITY:
        case MSR_CHAR_WILLPOWER:
        case MSR_CHAR_INTELLIGENCE:
        case MSR_CHAR_PERCEPTION:
            adv = (monster->characteristic[chr].advancement * 5);
            break;
        case MSR_SEC_CHAR_MOVEMENT:
        case MSR_SEC_CHAR_ATTACKS:
        case MSR_SEC_CHAR_MAGIC:
            adv = (monster->characteristic[chr].advancement * 1);
            break;
        default: assert(false && "unkown characteristic.");
    }

    return monster->characteristic[chr].base_value + adv + monster->characteristic[chr].mod + mod;
}

int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX)            return -1;

    return ( msr_calculate_characteristic(monster, chr) / 10);
}

enum msr_skill_rate msr_has_skill(struct msr_monster *monster, enum msr_skills skill) {
    enum msr_skill_rate r = MSR_SKILL_RATE_NONE;
    if ((monster->skills[MSR_SKILL_RATE_BASIC]    & bf(skill) ) > 0) r = MSR_SKILL_RATE_BASIC;
    if ((monster->skills[MSR_SKILL_RATE_ADVANCED] & bf(skill) ) > 0) r = MSR_SKILL_RATE_ADVANCED;
    if ((monster->skills[MSR_SKILL_RATE_EXPERT]   & bf(skill) ) > 0) r = MSR_SKILL_RATE_EXPERT;
    return r;
}

const char *msr_gender_string(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return "nil";
    switch (monster->gender) {
        case MSR_GENDER_MALE: return "Male";
        case MSR_GENDER_FEMALE: return "Female";
        case MSR_GENDER_IT: return "None";
        default: break;
    }
    return "";
}

bool msr_weapons_check(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    if (monster->wpn_sel >= MSR_WEAPON_SELECT_MAX) return false;

    struct inv_inventory *inv = monster->inventory;

    /* check if the monster has any weapons */
    if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) && (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
    }

    /* If we have a single hand, test that for emptiness and weaponness. */
    if (monster->wpn_sel== MSR_WEAPON_SELECT_OFF_HAND) {
        /* if location is unsupported by this monster*/
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;
        /* if the location is empty */
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        /* if there is no weapon at that location*/
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        /* or if it is the same as the mainhand weapon */
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if ( (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_2H_RANGED) == false) &&
             (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_2H_MELEE) == false) ) {
            return false;
        }
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
        if (inv_support_location(inv, INV_LOC_CREATURE_WIELD1) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1)->item_type != ITEM_TYPE_WEAPON) return false;
    }
    else return false;

    return true;
}

bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type) {
    if (msr_weapons_check(monster) == false) return false;
    if (type >= WEAPON_TYPE_MAX) return false;
    struct inv_inventory *inv = monster->inventory;

    if (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
         if ( (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) &&
              (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1), type) == false) return false;
    }
    else return false;


    return true;
}

bool msr_weapon_next_selection(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    if ( (inv_loc_empty(monster->inventory, INV_LOC_MAINHAND_WIELD) == true) &&
         (inv_loc_empty(monster->inventory, INV_LOC_OFFHAND_WIELD) == true) ) {
        monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
        return false;
    }

    do {
        monster->wpn_sel++;
        monster->wpn_sel %= MSR_WEAPON_SELECT_MAX;
    } while (msr_weapons_check(monster) == false);
    return true;
}

static void creature_weapon(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;
    assert(monster->crtr_wpn != IID_NONE);
    if (monster->crtr_wpn == IID_NONE) return;
    struct itm_item *item = itm_create(monster->crtr_wpn);

    if (itm_verify_item(item) == true) {
        assert(itm_is_type(item, ITEM_TYPE_WEAPON) );

        if (inv_add_item(monster->inventory, item) == true) {
            if (wpn_has_spc_quality(item, WPN_SPCQLTY_CREATURE) ) {
                if (inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) ) {
                    assert(inv_move_item_to_location(monster->inventory, item, INV_LOC_CREATURE_WIELD1) );
                }
            }
        }
    }

    assert(inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) == false);
}

void msr_give_items(struct msr_monster *monster, int level, struct random *r) {
    if (msr_verify_monster(monster) == false) return;
    struct itm_item *item = NULL;
    
    int mlevel = level;
    if (random_float(r) > 0.95) mlevel += 1;

    for (int i = 0; i < MSR_NR_DEFAULT_WEAPONS_MAX; i++) {
        if (monster->def_items[i] != ITEM_GROUP_NONE) {
            item = itm_create(itm_spawn(random_float(r), mlevel, monster->def_items[i]) );
            if (itm_verify_item(item) == true) {
                if (inv_add_item(monster->inventory, item) == true) {
                    if (dw_can_wear_item(monster, item) ) {
                        assert(dw_wear_item(monster, item) == true);
                    }

                    if (itm_is_type(item, ITEM_TYPE_WEAPON) ) {
                        struct item_weapon_specific *wpn = &item->specific.weapon;
                        if (msr_has_talent(monster, wpn->wpn_talent) == false) {
                            msr_set_talent(monster, wpn->wpn_talent);
                        }
                    }

                    if (wpn_uses_ammo(item) ) {
                        enum item_ids ammo_id = wpn_get_ammo_used_id(item);
                        struct itm_item *ammo = itm_create(ammo_id);
                        int nr = (random_int32(gbl_game->random) % (ammo->max_quantity / 5) ) +10;
                        ammo->stacked_quantity = nr;
                        assert (inv_add_item(monster->inventory, ammo) == true);
                    }
                }
            }
        }
    }

    assert(inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) == false);
}

bool msr_has_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    return ( (monster->creature_traits & bf(trait) ) > 0);
}

bool msr_set_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_creature_trait(monster, trait) == true) return false;
    return monster->creature_traits |= bf(trait);
}

bool msr_clr_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_creature_trait(monster, trait) == false) return false;
    monster->creature_traits &= ~bf(trait);

    return msr_has_creature_trait(monster, trait);
}

bool msr_has_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (talent == TLT_NONE) return true;
    if (talent >= TLT_MAX) return false;

    for (unsigned int i = 0; i < ARRAY_SZ(monster->talents); i++) {
        if (monster->talents[i] == talent) return true;
    }
    return false;
}

bool msr_set_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_talent(monster, talent) == true) return false;
    if (talent >= TLT_MAX) return false;

    for (unsigned int i = 0; i < ARRAY_SZ(monster->talents); i++) {
        if (monster->talents[i] == TLT_NONE) {
            monster->talents[i] = talent;
            return true;
        }
    }
    return false;
}

bool msr_clr_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_talent(monster, talent) == false) return false;

    for (unsigned int i = 0; i < ARRAY_SZ(monster->talents); i++) {
        if (monster->talents[i] == talent) {
            monster->talents[i] = talent;
            return true;
        }
    }

    return false;
}

uint8_t msr_get_movement_rate(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    int speed = (msr_calculate_characteristic_bonus(monster, MSR_SEC_CHAR_MOVEMENT) );
    int speed_mod = 0;
    int min_speed = MSR_MOVEMENT_MIN;
    int max_speed = MSR_MOVEMENT_MAX;

    if ( (se_has_effect(monster->status_effects, SETF_DISABLE_RLEG) == true) &&
         (se_has_effect(monster->status_effects, SETF_DISABLE_LLEG) == true) ) {
        return 0;
    }

    if (se_has_effect(monster->status_effects, SETF_DISABLE_RLEG) ) {
        max_speed -= 2;
        speed_mod -= speed / 2;
    }
    if (se_has_effect(monster->status_effects, SETF_DISABLE_LLEG) ) {
        max_speed -= 2;
        speed_mod -= speed / 2;
    }

    if (se_has_effect(monster->status_effects, SETF_INCREASE_MOVEMENT) ) {
        speed_mod += se_status_effect_strength(monster->status_effects, SETF_INCREASE_MOVEMENT);
    }
    if (se_has_effect(monster->status_effects, SETF_DECREASE_MOVEMENT) ) {
        speed_mod += se_status_effect_strength(monster->status_effects, SETF_DECREASE_MOVEMENT);
    }

    if (inv_wears_wearable_with_spcqlty(monster->inventory, WBL_SPCQLTY_PLATE) ) {
        speed_mod -= 1;
    }
    else if (inv_wears_wearable_with_spcqlty(monster->inventory, WBL_SPCQLTY_SCALE) ) {
        speed_mod -= 1;
    }

    speed += speed_mod;
    if (speed > max_speed) speed = max_speed;
    if (speed < min_speed) speed = min_speed;

    return speed;
}

struct ai *msr_get_ai_ctx(struct msr_monster *monster) {
    return &monster->controller.ai;
}

const char *msr_ldname(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return cs_MONSTER "unknown" cs_MONSTER;

    if (monster->is_player) return cs_PLAYER "you" cs_PLAYER;
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) return cs_MONSTER "something" cs_MONSTER;
    return monster->ld_name;
}

const char *msr_gender_name(struct msr_monster *monster, bool possesive) {
    if (msr_verify_monster(monster) == false) return cs_MONSTER "unknown" cs_MONSTER;

    enum msr_gender gender = monster->gender;
    if (monster->gender >= MSR_GENDER_MAX) gender = MSR_GENDER_IT;
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) gender = MSR_GENDER_IT;

    switch(gender) {
        case MSR_GENDER_MALE:        return (possesive) ? cs_MONSTER "his" cs_MONSTER : cs_MONSTER "he"  cs_MONSTER; break;
        case MSR_GENDER_FEMALE:      return (possesive) ? cs_MONSTER "her" cs_MONSTER : cs_MONSTER "she" cs_MONSTER; break;
        case MSR_GENDER_IT: default: return (possesive) ? cs_MONSTER "its" cs_MONSTER : cs_MONSTER "it"  cs_MONSTER; break;
    }
}

const char *msr_skill_names(enum msr_skills s) {
    if (s >= MSR_SKILLS_MAX) return NULL;
    return msr_skill_name[s];
}

const char *msr_skillrate_names(enum msr_skill_rate sr) {
    if (sr >= MSR_SKILL_RATE_NONE) return NULL;
    return msr_skillrate_name[sr];
}

const char *msr_talent_names(enum msr_talents t) {
    if (t >= TLT_MAX) return NULL;
    return msr_talent_name[t];
}

const char *msr_hitloc_name(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (mhl >= MSR_HITLOC_MAX) return NULL;

    switch(mhl) {
        case MSR_HITLOC_BODY:       return "thorax";
        case MSR_HITLOC_LEFT_LEG:   return "left leg";
        case MSR_HITLOC_RIGHT_LEG:  return "right leg";
        case MSR_HITLOC_LEFT_ARM:   return "left arm";
        case MSR_HITLOC_RIGHT_ARM:  return "right arm";
        case MSR_HITLOC_HEAD:       return "head";
        default: break;
    }
    return NULL;
}

