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
#include <stdint.h>
#include <inttypes.h>

#include <time.h>

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
#include "turn_tick.h"

#include "careers_static_def.h"

static time_t session_time_start = 0;

void cr_init() {
    for (int i = 0; i < ARRAY_SZ(static_homeworld_list); i++) {
        if (static_homeworld_list[i].homeworld_talent != 0)
            assert(msr_get_tier(static_homeworld_list[i].homeworld_talent) == MSR_TALENT_TIER_MISC);
    }

    for (int i = 0; i < ARRAY_SZ(static_background_list); i++) {
        if (static_background_list[i].talents != 0)
            assert(msr_get_tier(static_background_list[i].talents) == MSR_TALENT_TIER_T1);
        if (static_background_list[i].background_talent != 0)
            assert(msr_get_tier(static_background_list[i].background_talent) == MSR_TALENT_TIER_MISC);
    }

    for (int i = 0; i < ARRAY_SZ(static_role_list); i++) {
        if (static_role_list[i].talents != 0)
            assert(msr_get_tier(static_role_list[i].talents) == MSR_TALENT_TIER_T1);
        if (static_role_list[i].role_talent != 0)
            assert(msr_get_tier(static_role_list[i].role_talent) == MSR_TALENT_TIER_MISC);
    }
}

void cr_exit(struct pl_player *plr) {
    plr->career.play_seconds += time(NULL) - session_time_start;
}

static bool cr_give_trappings_to_player(struct cr_background *car, struct msr_monster *player) {
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

void cr_init_career(struct pl_player *plr, enum homeworld_ids htid, enum background_ids btid, enum role_ids rtid) {
    session_time_start = time(NULL);
    plr->career.play_seconds = 0;

    plr->career.b_tid = btid;
    plr->career.r_tid = btid;

    plr->career.xp_current = 0;
    plr->career.xp_spend   = 0;

    plr->player->wounds.max  = cr_get_homeworld_by_id(htid)->wounds + random_xd5(gbl_game->random, 1);
    plr->player->wounds.curr = plr->player->wounds.max;

    plr->career.aptitudes = bf(APTITUDE_GENERAL);

    plr->career.killer = NULL;
    for (int i = 0; i < ACHIEVEMENTS_MAX; i++) {
        plr->career.achievements[i].turn = -1;
        plr->career.achievements[i].achievement = NULL;
    }

    if (cr_get_homeworld_by_id(htid)->aptitudes != 0)   plr->career.aptitudes  |= cr_get_homeworld_by_id(htid)->aptitudes;
    if (cr_get_background_by_id(btid)->aptitudes != 0)  plr->career.aptitudes  |= cr_get_background_by_id(btid)->aptitudes;
    if (cr_get_role_by_id(rtid)->aptitudes != 0)        plr->career.aptitudes  |= cr_get_role_by_id(rtid)->aptitudes;

    //if (cr_get_homeworld_by_id(htid)->talents != 0)     plr->player->talents[MSR_TALENT_TIER_T1] |= cr_get_homeworld_by_id(htid)->talents;
    if (cr_get_background_by_id(btid)->talents != 0)    plr->player->talents[MSR_TALENT_TIER_T1] |= cr_get_background_by_id(btid)->talents;
    if (cr_get_role_by_id(htid)->talents != 0)          plr->player->talents[MSR_TALENT_TIER_T1] |= cr_get_role_by_id(rtid)->talents;

    if (cr_get_homeworld_by_id(htid)->homeworld_talent != 0)    msr_set_talent(plr->player, cr_get_homeworld_by_id(htid)->homeworld_talent);
    if (cr_get_background_by_id(btid)->background_talent != 0)  msr_set_talent(plr->player, cr_get_background_by_id(btid)->background_talent);
    if (cr_get_role_by_id(rtid)->role_talent != 0)              msr_set_talent(plr->player, cr_get_role_by_id(rtid)->role_talent);

    cr_give_trappings_to_player(cr_get_background_by_id(btid), plr->player);
    msr_verify_monster(plr->player);
}

struct cr_homeworld *cr_get_homeworld_by_id(enum homeworld_ids tid) {
    if (tid < CR_HWID_NONE) return NULL;
    if (tid >= CR_HWID_MAX) return NULL;
    return &static_homeworld_list[tid];
}

struct cr_background *cr_get_background_by_id(enum background_ids tid) {
    if (tid < CR_BCKGRNDID_NONE) return NULL;
    if (tid >= CR_BCKGRNDID_MAX) return NULL;
    return &static_background_list[tid];
}

struct cr_role *cr_get_role_by_id(enum role_ids tid) {
    if (tid < CR_ROLEID_NONE) return NULL;
    if (tid >= CR_ROLEID_MAX) return NULL;
    return &static_role_list[tid];
}

int cr_skill_cost(struct pl_player *plr, enum msr_skills skill) {
    if (plr == NULL) return -1;
    if (skill <= MSR_SKILLS_NONE) return -1;
    if (skill >= MSR_SKILLS_MAX) return -1;

    int mult = 3;
    if (test_bf(plr->career.aptitudes, aptitude_skill_list[skill].apt1) ) mult -= 1;
    if (test_bf(plr->career.aptitudes, aptitude_skill_list[skill].apt2) ) mult -= 1;

    int mult2 = msr_has_skill(plr->player, skill) +1;
    if (mult2 >= MSR_SKILL_RATE_MAX) return -1;

    return mult * mult2 * 100;
}

int cr_talent_cost(struct pl_player *plr, enum msr_talents talent) {
    if (plr == NULL) return -1;
    if (talent <= TLT_NONE) return -1;
    if (talent >= TLT_MAX) return -1;
    if (msr_has_talent(plr->player, talent) ) return -1;

    int tier = talent >> MSR_TALENT_HEADER_SHIFT;
    for (int i = 0; i < ARRAY_SZ(aptitude_talent_list); i++) {
        if (aptitude_talent_list[i].talent != talent) continue;
        int has = 0;
        if (test_bf(plr->career.aptitudes, aptitude_talent_list[i].apt1) ) has += 1;
        if (test_bf(plr->career.aptitudes, aptitude_talent_list[i].apt2) ) has += 1;

        return talent_cost_list[tier][has];
    }
    return -1;
}

bool cr_has_aptitude(struct pl_player *plr, enum aptitude_enum aptitude) {
    if (plr == NULL) return NULL;
    return test_bf(plr->career.aptitudes, aptitude);
}

void cr_set_aptitude(struct pl_player *plr, enum aptitude_enum aptitude) {
    assert (plr != NULL);
    set_bf(plr->career.aptitudes, aptitude);
}
const char *cr_aptitude_name(enum aptitude_enum aptitude) {
    if (aptitude <= 0) return NULL;
    if (aptitude >= APTITUDE_MAX) return NULL;
    return aptitude_names[aptitude];
}

void cr_add_achievement(struct pl_player *plr, int turn, const char *achievement) {
    if (plr == NULL) return;
    if (turn == -1) return;
    if (achievement == NULL) return;

    for (int i = 0; i < ACHIEVEMENTS_MAX; i++) {
        if (plr->career.achievements[i].turn >= 0) continue;

        plr->career.achievements[i].turn = turn;
        plr->career.achievements[i].achievement = achievement;

        return;
    }
}

void cr_print_morgue_file(struct pl_player *plr) {
    if (plr == NULL) return;
    plr->career.play_seconds += time(NULL) - session_time_start;

    printf("%s played for %lu seconds and %" PRIu64 ".%" PRIu64 " turns\n", plr->player->unique_name, (unsigned long int) plr->career.play_seconds, gbl_game->turn / TT_ENERGY_TURN, gbl_game->turn % TT_ENERGY_TURN      );
    printf("%s still owed the loan-shark %d throne guilders\n", plr->player->unique_name, plr->loan);

    printf("%s's states were: \n", plr->player->unique_name);
}
