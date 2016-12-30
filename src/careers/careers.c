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

#include <assert.h>
#include <string.h>
#include <sys/queue.h>

#include "careers.h"
#include "careers_static.h"
#include "random.h"
#include "inventory.h"
#include "fight.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "items/items.h"
#include "fov/sight.h"
#include "random_generator.h"

#include "careers_static_def.h"


struct cr_spawn_weigh_struct {
    enum msr_race race;
};

static int32_t cr_spawn_weight(void *ctx, int idx) {
    assert (ctx != NULL);
    struct cr_spawn_weigh_struct *sws = ctx;

    if (static_career_list[idx].available[sws->race] == true) {
        return static_career_list[idx].weight[sws->race];
    }

    return RANDOM_GEN_WEIGHT_IGNORE;
}

enum career_ids cr_spawn(int32_t roll, enum msr_race race) {
    struct cr_spawn_weigh_struct sws = {
        .race = race,
    };

    struct random_gen_settings s = {
        .start_idx = 0,
        .end_idx = ARRAY_SZ(static_career_list),
        .roll = roll,
        .ctx = &sws,
        .weight = cr_spawn_weight,
    };

    return random_gen_spawn(&s);
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

    if (player->wounds.added >= cr->wounds) return false;
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

    player->wounds.curr += 1;
    player->wounds.max += 1;
    player->wounds.added += 1;
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

