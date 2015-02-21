#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include <float.h>

#include "careers.h"
#include "careers_static.h"
#include "random.h"
#include "inventory.h"
#include "fight.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "items/items.h"
#include "fov/sight.h"

#include "careers_static_def.h"

enum career_ids cr_spawn(double roll, enum msr_race race) {
    int sz = ARRAY_SZ(static_career_list);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0;

    int idx = CRID_NONE;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = CRID_NONE; i < sz; i++) {
        if (static_career_list[i].available[race] == true) {
            sum += static_career_list[i].weight[race];
            cumm_prob_arr[i] = 0.f;
        }
        else cumm_prob_arr[i] = DBL_MAX;
    }

    double cumm = 0;
    for (int i = CRID_NONE; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = static_career_list[i].weight[race] / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    for (int i = sz-1; i > CRID_NONE; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    return idx;
}

struct cr_career *cr_get_career_by_id(enum career_ids template_id) {
    if (template_id < CRID_NONE) return NULL;
    if (template_id >= CRID_MAX) return NULL;

    lg_debug("creating career tid: %d", template_id);
    return &static_career_list[template_id];
}

enum career_ids cr_get_next_career_id_for_race(enum msr_race race, enum career_ids prev_tid) {
    if (prev_tid < CRID_NONE) return CRID_NONE;

    for (int i = prev_tid; i < (int) ARRAY_SZ(static_career_list); i++) {
        if (static_career_list[i].available[race] == true) return i;
    }
    return CRID_NONE;
}

bool cr_give_trappings_to_player(struct cr_career *car, struct msr_monster *player) {
    if (msr_verify_monster(player) == false) return false;

    for (int i = 0; i < (int) ARRAY_SZ(car->trappings); i++) {
        if (car->trappings[i] != IID_NONE) {
            struct itm_item *item = itm_create(car->trappings[i]);
            assert(item != NULL);
            assert( (msr_give_item(player, item) ) );
        }
    }
    return true;
}

bool cr_generate_allies(struct cr_career *car, struct msr_monster *player, struct dm_map *map) {
    if (msr_verify_monster(player) == false) return false;

    for (int i = 0; i < (int) ARRAY_SZ(car->allies_ids); i++) {
        if (car->allies_ids[i] != MID_NONE) {
            struct msr_monster *monster = msr_create(car->allies_ids[i]);
            coord_t c = sgt_scatter(map, gbl_game->random, &player->pos, 5);
            msr_insert_monster(monster, map, &c);
            monster->faction = 0;
            ai_monster_init(monster, 0);
        }
    }

    return true;
}

bool cr_can_upgrade_characteristic(struct cr_career *cr, struct msr_monster *player, enum msr_characteristic c) {
    if (msr_verify_monster(player) == false) return false;
    if (cr == NULL) return false;

    if (player->characteristic[c].advancement >= cr->char_advancements[c]) return false;
    return true;
}

bool cr_can_upgrade_wounds(struct cr_career *cr, struct msr_monster *player) {
    if (msr_verify_monster(player) == false) return false;
    if (cr == NULL) return false;

    if (player->wounds_added >= cr->wounds) return false;
    return true;
}

/* TODO: this only checks for untrained. we should find a way to do this for basic and advanced. */
bool cr_can_upgrade_skill(struct cr_career *cr, struct msr_monster *player, enum msr_skills skill) {
    if (skill == 0) return false;
    if (msr_verify_monster(player) == false) return false;
    if (cr == NULL) return false;
    if (msr_has_skill(player, skill) > MSR_SKILL_RATE_NONE) return false;

    if (test_bf(cr->skills, skill) == false) return false;
    return true;
}

bool cr_can_upgrade_talent(struct cr_career *cr, struct msr_monster *player, enum msr_talents talent) {
    if (talent == TLT_NONE) return false;
    if (msr_verify_monster(player) == false) return false;
    if (cr == NULL) return false;
    if (msr_has_talent(player, talent) == true) return false;

    for (int i = 0; i < CR_TALENTS_MAX; i++) {
        if (cr->talents[i] == talent) return true;
    }

    return false;
}

bool cr_upgrade_characteristic(struct cr_career *cr, struct msr_monster *player, enum msr_characteristic c) {
    if (cr_can_upgrade_characteristic(cr, player, c) == false) return false;

    switch (c) {
        case MSR_CHAR_WEAPON_SKILL:
        case MSR_CHAR_BALISTIC_SKILL:
        case MSR_CHAR_STRENGTH:
        case MSR_CHAR_TOUGHNESS:
        case MSR_CHAR_AGILITY:
        case MSR_CHAR_INTELLIGENCE:
        case MSR_CHAR_WILLPOWER:
        case MSR_CHAR_PERCEPTION:
            player->characteristic[c].advancement += 5;
            break;
        case MSR_SEC_CHAR_ATTACKS:
        case MSR_SEC_CHAR_MOVEMENT:
        case MSR_SEC_CHAR_MAGIC:
            player->characteristic[c].advancement += 1;
            break;
        default: assert(false && "Unknown Characteristic");
    }

    return true;
}

bool cr_upgrade_wounds(struct cr_career *cr, struct msr_monster *player) {
    if (cr_can_upgrade_wounds(cr, player) == false) return false;

    player->cur_wounds += 1;
    player->max_wounds += 1;
    player->wounds_added += 1;
    return true;
}

/* TODO: this only checks for untrained. we should find a way to do this for basic and advanced. */
bool cr_upgrade_skill(struct cr_career *cr, struct msr_monster *player, enum msr_skills skill) {
    if (cr_can_upgrade_skill(cr, player, skill) == false) return false;

    return msr_set_skill(player, skill, MSR_SKILL_RATE_BASIC);
}

bool cr_upgrade_talent(struct cr_career *cr, struct msr_monster *player, enum msr_talents talent) {
    if (cr_can_upgrade_talent(cr, player, talent) == false) return false;
    return msr_set_talent(player, talent);
}

