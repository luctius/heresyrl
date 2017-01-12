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

#include <sys/param.h>
#include <string.h>
#include <assert.h>

#include "ai.h"
#include "ai_utils.h"
#include "pathfinding.h"
#include "game.h"

#include "monster/monster.h"
#include "monster/monster_action.h"
#include "items/items.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "status_effects/status_effects.h"


/*
   For now, this file contains every ai posible,
   and gives an monster its ai based on race.
   We can later modify it by given out on other parameters.
 */


/* state context of the beast ai */
struct beast_ai_struct {
    coord_t last_pos;
    int time_last_pos;
    struct pf_context *pf_ctx; /* TODO: free this correctly. */
};

static bool ai_beast_loop(struct msr_monster *monster) {
    struct dm_map *map = gbl_game->current_map;
    struct ai *ai_s = msr_get_ai_ctx(monster);
    assert(ai_s != NULL);

    struct beast_ai_struct *ai = ai_s->ai_ctx;
    assert(ai != NULL);

    /*struct msr_monster *leader = msr_get_monster_by_uid(ai_s->leader_uid);*/

    bool has_action = false;
    monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;

    //lg_debug("ai_beast_loop for [uid: %d, tid: %d]", monster->uid, monster->tid);

    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) ) {
        struct msr_monster *enemy = NULL;
        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            lg_ai_debug(monster, "attacks an enemy (melee)");
            ai->last_pos = enemy->pos;
            ai->time_last_pos = 1;

            if (cd_pyth(&monster->pos, &enemy->pos) > 1) {
                //int radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster) +1;
                if (aiu_generate_astar(&ai->pf_ctx, map, &monster->pos, &enemy->pos, 0) == true) {
                    coord_t *coord_lst;
                    int coord_lst_sz =  pf_calculate_path(ai->pf_ctx, &monster->pos, &enemy->pos, &coord_lst);
                    if (coord_lst_sz > 1) {
                        has_action = ma_do_move(monster, &coord_lst[1]);
                        free(coord_lst);
                    }
                }
            }
            else {
                lg_ai_debug(monster, "attacks an enemy (ranged)");
                if (ma_do_melee(monster, &enemy->pos) == true) {
                    has_action = true;
                }
            }
        }
        else if (ai->time_last_pos > 0) {
            if (aiu_generate_astar(&ai->pf_ctx, map, &monster->pos, &ai->last_pos, 0) == true) {
                coord_t *coord_lst;
                int coord_lst_sz = pf_calculate_path(ai->pf_ctx, &monster->pos, &ai->last_pos, &coord_lst);
                if (coord_lst_sz > 1) {
                    has_action = ma_do_move(monster, &coord_lst[1]);
                    free(coord_lst);
                }
            }
            ai->time_last_pos++;
            if (cd_pyth(&monster->pos, &ai->last_pos) == 0) ai->time_last_pos = 0; /* forget enemy, we lost him */
        }
    }

    if (has_action == false) {
        return ma_do_guard(monster);
    }

    return false;
}

void ai_beast_free(void *ai_ctx) {
    struct dm_map *map = gbl_game->current_map;
    assert(ai_ctx != NULL);

    struct beast_ai_struct *ai = ai_ctx;
    assert(ai != NULL);

    pf_exit(ai->pf_ctx);
    ai->pf_ctx = NULL;
}

struct human_ai_struct {
    coord_t last_pos;
    int time_last_pos;
    struct pf_context *pf_ctx; /* TODO: free this correctly. */
};

static bool ai_human_loop(struct msr_monster *monster) {
    struct dm_map *map = gbl_game->current_map;
    struct ai *ai_s = msr_get_ai_ctx(monster);
    assert (ai_s != NULL);

    struct human_ai_struct *ai = ai_s->ai_ctx;
    assert (ai != NULL);

    /* struct msr_monster *leader = msr_get_monster_by_uid(ai_s->leader_uid); */
    ai_s->emo_state = "Stupid";

    bool has_action = false;
    monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;

    //lg_debug("ai_human_loop for [uid: %d, tid: %d]", monster->uid, monster->tid);

    for (int i = 0; i < MSR_WEAPON_SELECT_MAX; i++) {
        monster->wpn_sel = i;
        if (msr_weapons_check(monster) ) {
            if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) ) break;
            if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) ) break;
        }
    }

    if (msr_weapon_type_check(monster, WEAPON_TYPE_RANGED) ) {
        struct msr_monster *enemy = NULL;
        struct itm_item *item = fght_get_weapon(monster, WEAPON_TYPE_RANGED, FGHT_MAIN_HAND);
        if (item == NULL) item = fght_get_weapon(monster, WEAPON_TYPE_RANGED, FGHT_OFF_HAND);
        struct item_weapon_specific *wpn = &item->specific.weapon;

        if (wpn->magazine_left == 0 || wpn->jammed) {
            struct itm_item *items[] = {item};
            has_action = ma_do_reload_carried(monster, NULL);
            if (has_action == false) has_action = ma_do_drop(monster, items, 1);
            monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
        }

        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            lg_debug("[uid: %d, tid: %d] sees an enemy (ranged)", monster->uid, monster->tid);
            has_action = ma_do_fire(monster, &enemy->pos);
        }
    }

    if (aiu_next_thrown_weapon(monster, 0) != NULL) {
        struct msr_monster *enemy = NULL;
        struct itm_item *item = aiu_next_thrown_weapon(monster, 0);

        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            if (cd_pyth(&monster->pos, &enemy->pos) > 2) {
                lg_debug("[uid: %d, tid: %d] sees an enemy (throw)", monster->uid, monster->tid);
                has_action = ma_do_throw(monster, &enemy->pos, item);
            }
        }
    }

    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) ) {
        struct msr_monster *enemy = NULL;
        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            lg_ai_debug(monster, "sees an enemy (melee)");
            ai->last_pos = enemy->pos;
            ai->time_last_pos = 1;

            if (cd_pyth(&monster->pos, &enemy->pos) > 1) {
                //int radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster) +1;
                if (aiu_generate_astar(&ai->pf_ctx, map, &monster->pos, &enemy->pos, 0) == true) {
                    coord_t *coord_lst;
                    int coord_lst_sz =  pf_calculate_path(ai->pf_ctx, &monster->pos, &enemy->pos, &coord_lst);
                    if (coord_lst_sz > 1) {
                        has_action = ma_do_move(monster, &coord_lst[1]);
                        free(coord_lst);
                    }
                }
            }
            else {
                lg_ai_debug(monster, "attacks an enemy");
                if (ma_do_melee(monster, &enemy->pos) == true) {
                    has_action = true;
                }
            }
        }
        else if (ai->time_last_pos > 0) {
            if (aiu_generate_astar(&ai->pf_ctx, map, &monster->pos, &ai->last_pos, 0) == true) {
                coord_t *coord_lst;
                int coord_lst_sz = pf_calculate_path(ai->pf_ctx, &monster->pos, &ai->last_pos, &coord_lst);
                if (coord_lst_sz > 1) {
                    has_action = ma_do_move(monster, &coord_lst[1]);
                    free(coord_lst);
                }
            }
            ai->time_last_pos++;
            if (cd_pyth(&monster->pos, &ai->last_pos) == 0) ai->time_last_pos = 0; /* forget enemy, we lost him */
        }
    }

    if (has_action == false) {
        return ma_do_guard(monster);
    }

    return false;
}

void ai_human_free(void *ai_ctx) {
    struct dm_map *map = gbl_game->current_map;
    assert(ai_ctx != NULL);

    struct human_ai_struct *ai = ai_ctx;
    assert(ai != NULL);

    pf_exit(ai->pf_ctx);
    ai->pf_ctx = NULL;
}


static void init_human_ai(struct msr_monster *monster) {
    struct human_ai_struct *ai = calloc(1, sizeof(struct human_ai_struct) );
    if (ai != NULL) {
        struct monster_controller mc = {
            .ai = {
                .ai_ctx = ai,
                .free_func = ai_human_free,
            },
            .controller_cb = ai_human_loop,
        };

        msr_assign_controller(monster, &mc);
    }
}

static void init_bestial_ai(struct msr_monster *monster) {
    struct beast_ai_struct *ai = calloc(1, sizeof(struct beast_ai_struct) );
    if (ai != NULL) {
        struct monster_controller mc = {
            .ai = {
                .ai_ctx = ai,
                .free_func = ai_beast_free,
            },
            .controller_cb = ai_beast_loop,
        };

        msr_assign_controller(monster, &mc);
    }
}

void ai_monster_init(struct msr_monster *monster, uint32_t leader_uid) {
    switch(monster->race) {
        case MSR_RACE_GREENSKIN:
        case MSR_RACE_HUMAN: init_human_ai(monster); break;

        default:
        case MSR_RACE_DOMESTIC:
        case MSR_RACE_BEAST: init_bestial_ai(monster); break;
    }

    struct ai *ai_s = msr_get_ai_ctx(monster);
    assert(ai_s != NULL);
    if (leader_uid != 0) ai_s->leader_uid = leader_uid;
}

void ai_monster_free(struct msr_monster *monster) {
    if (monster->is_player) return;
    if (monster->controller.ai.ai_ctx != NULL)  {
        if (monster->controller.ai.free_func != NULL)  {
            monster->controller.ai.free_func(monster->controller.ai.ai_ctx);
        }
        free(monster->controller.ai.ai_ctx);
    }
}

