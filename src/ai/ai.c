#include <sys/param.h>
#include <string.h>

#include "ai.h"
#include "ai_utils.h"
#include "pathfinding.h"
#include "tiles.h"
#include "game.h"

#include "monster/monster.h"
#include "monster/monster_action.h"
#include "items/items.h"
#include "dungeon/dungeon_creator.h"


/* 
   For now, this file contains every ai posible, 
   and gives an monster its ai based on race.
   We can later modify it by given out on other parameters.
 */

/* state context of the beast ai */
struct beast_ai_struct {
    coord_t last_pos;
    int time_last_pos;
    struct pf_context *pf_ctx;
};

static bool ai_beast_loop(struct msr_monster *monster, void *controller) {
    if (monster->dead == true) {
        msr_clear_controller(monster);
    }
    struct dc_map *map = gbl_game->current_map;
    struct beast_ai_struct *ai = controller;
    bool has_action = false;
    monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "ai_beast_loop for [uid: %d, tid: %d]", monster->uid, monster->template_id);

    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) ) {
        struct msr_monster *enemy = NULL;
        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] sees an enemy (melee)", monster->uid, monster->template_id);
            ai->last_pos = enemy->pos;
            ai->time_last_pos = 1;

            if (cd_pyth(&monster->pos, &enemy->pos) > 1) {
                int radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster) +1;
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
                lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] attacks an enemy", monster->uid, monster->template_id);
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

struct human_ai_struct {
    coord_t last_pos;
    int time_last_pos;
    struct pf_context *pf_ctx;
};
static bool ai_human_loop(struct msr_monster *monster, void *controller) {
    if (monster->dead == true) {
        msr_clear_controller(monster);
    }
    struct dc_map *map = gbl_game->current_map;
    struct human_ai_struct *ai = controller;
    bool has_action = false;
    monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "ai_human_loop for [uid: %d, tid: %d]", monster->uid, monster->template_id);

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
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] sees an enemy (ranged)", monster->uid, monster->template_id);
            has_action = ma_do_fire(monster, &enemy->pos);
        }
    }


    if (msr_weapon_type_check(monster, WEAPON_TYPE_MELEE) ) {
        struct msr_monster *enemy = NULL;
        if ( (enemy = aiu_get_nearest_enemy(monster, 0, map) ) != NULL) {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] sees an enemy (melee)", monster->uid, monster->template_id);
            ai->last_pos = enemy->pos;
            ai->time_last_pos = 1;

            if (cd_pyth(&monster->pos, &enemy->pos) > 1) {
                int radius = msr_get_near_sight_range(monster) + msr_get_far_sight_range(monster) +1;
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
                lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "ai", "[uid: %d, tid: %d] attacks an enemy", monster->uid, monster->template_id);
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

static void init_human_ai(struct msr_monster *monster) {
    struct human_ai_struct *ai = calloc(1, sizeof(struct human_ai_struct) );
    if (ai != NULL) {
        struct monster_controller mc = {
            .controller_ctx = ai,
            .controller_cb = ai_human_loop,
        };

        msr_assign_controller(monster, &mc);
    }
}

static void init_bestial_ai(struct msr_monster *monster) {
    struct beast_ai_struct *ai = calloc(1, sizeof(struct beast_ai_struct) );
    if (ai != NULL) {
        struct monster_controller mc = {
            .controller_ctx = ai,
            .controller_cb = ai_beast_loop,
        };

        msr_assign_controller(monster, &mc);
    }
}

void ai_monster_init(struct msr_monster *monster) {
    switch(monster->race) {
        case MSR_RACE_HUMAN: init_human_ai(monster); break;

        default:
        case MSR_RACE_BEAST: init_bestial_ai(monster); break;
    }
}

