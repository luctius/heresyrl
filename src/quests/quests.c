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
#include <string.h>
#include <stdio.h>

#include "random_generator.h"

#include "quests.h"
#include "quests_static.h"

#include "items/items.h"

#include "quests_static_def.h"

struct quest *qst_by_tid(enum qst_ids tid) {
    struct quest *quest = &static_quest_list[tid];
    quest->state = 0;
    memset(quest->params, 0x0, sizeof(quest->params) );
    return quest;
}

static struct quest *qst_select(int idx) {
    struct quest *quest = &static_quest_list[idx];
    quest->state = 0;
    memset(quest->params, 0x0, sizeof(quest->params) );
    return quest;
}

struct quest *qst_spawn(int level) {
    int idx = 1;

    return qst_select(idx);
}

static int32_t qst_dungeon_spawn_weight(void *ctx, int idx) {
    assert (ctx != NULL);
    struct quest *quest = ctx;

    if (quest->dungeon[idx].weight != 0) return quest->dungeon[idx].weight;

    return RANDOM_GEN_WEIGHT_IGNORE;
}

enum dm_dungeon_type qst_select_dungeon(struct quest *quest, int32_t roll) {
    assert(quest != NULL);

    struct random_gen_settings s = {
        .start_idx = 0,
        .end_idx = QUEST_SZ,
        .roll = roll,
        .ctx = quest,
        .weight = qst_dungeon_spawn_weight,
    };

    int32_t idx = random_gen_spawn(&s);
    if (idx >= 0 && idx < QUEST_SZ) return quest->dungeon[idx].type;
    return DUNGEON_TYPE_PLAIN;
}

static int32_t qst_enemies_spawn_weight(void *ctx, int idx) {
    assert (ctx != NULL);
    struct quest *quest = ctx;

    if (quest->enemies[idx].weight != 0) quest->enemies[idx].weight;

    return RANDOM_GEN_WEIGHT_IGNORE;
}


enum msr_race qst_select_enemy(struct quest *quest, int32_t roll) {
    assert(quest != NULL);

    struct random_gen_settings s = {
        .start_idx = 0,
        .end_idx = QUEST_SZ,
        .roll = roll,
        .ctx = quest,
        .weight = qst_enemies_spawn_weight,
    };

    int32_t idx = random_gen_spawn(&s);
    if (idx >= 0 && idx < QUEST_SZ) return quest->enemies[idx].race;
    return MSR_RACE_BEAST;
}

void qst_process_quest_start(struct quest *quest, struct dm_map *map, struct random *r) {
    switch (quest->type) {
        case QST_TYPE_FETCH: {
                /* Distribute Items */
                lg_debug("Distributing quest items.");
                for (int i = 0; i < QUEST_SZ && quest->qst_params[i] != IID_NONE; i+= 2) {
                    int nrj = quest->qst_params[i+1];
                    for (int j = 0; j < nrj; j++) {
                        coord_t p = dm_scatter(map, r);
                        if (p.x == 0 && p.y == 0) {
                            lg_debug("Placement of quest item failed.");
                            continue;
                        }

                        struct itm_item *item = itm_create(quest->qst_params[i]);
                        inv_add_item(dm_get_map_me(&p,map)->inventory, item);
                    }
                }
                quest->state = QST_FTCH_FETCHING;
        } break;
        default: break;
    }

    if (quest->start != NULL) GM_msg("%s", quest->start);
}

void qst_process_quest_end(struct quest *quest, struct dm_map *map) {
    if (qst_is_quest_done(quest, map) ) {
        if (quest->end != NULL) GM_msg("%s", quest->end);
        struct itm_item *money_item = inv_get_item_by_template_id(gbl_game->player_data.player->inventory, IID_MONEY);
        if (money_item == NULL) {
            money_item = itm_create(IID_MONEY);
            msr_give_item(gbl_game->player_data.player, money_item);
            money_item->stacked_quantity = 0;
        }
        money_item->stacked_quantity     += quest->gp_reward;
        gbl_game->player_data.xp_current += quest->xp_reward;
    }
}

void qst_process_quest_during(struct quest *quest, struct dm_map *map) {
    switch (quest->type) {
        case QST_TYPE_FETCH:
            switch (quest->state) {
                case QST_FTCH_END:
                case QST_FTCH_FETCHING: {
                    /* Check Number of Items */
                    struct msr_monster *player = gbl_game->player_data.player;
                    struct inv_inventory *inv =  player->inventory;
                    struct itm_item *item = NULL;
                    bool done = false;
                    int nrdone = 0;
                    int qst_nr = 0;

                    for (int i = 0; i < QUEST_SZ && quest->qst_params[i] != IID_NONE; i+= 2) {
                        qst_nr++;
                    }

                    while ( (item = inv_get_next_item(inv, item) ) != NULL) {
                        for (int i = 0; i < QUEST_SZ && quest->qst_params[i] != IID_NONE; i+= 2) {
                            if (item->template_id == quest->qst_params[i]) {
                                if (quest->qst_params[i+1] <= item->stacked_quantity ) {
                                    nrdone++;
                                }
                            }
                        }
                    }

                    if (qst_nr <= nrdone) done = true;

                    if (done) {
                        if (quest->state == QST_FTCH_FETCHING) You(player, "now posses the items to complete your quest.");
                        quest->state = QST_FTCH_END;
                    }
                    else {
                        if (quest->state == QST_FTCH_END) You(player, "no longer posses the items to complete your quest.");
                        quest->state = QST_FTCH_FETCHING;
                    }
                } break;
            }
            break;
        default: break;
    }
}

bool qst_is_quest_done(struct quest *quest, struct dm_map *map) {
    switch (quest->type) {
        case QST_TYPE_FETCH:
            switch (quest->state) {
                case QST_FTCH_END:
                    return true;
                    break;
                default: break;
            }
            break;
        default: break;
    }
    return false;
}

void qst_get_description(struct quest *quest, char *str, int max_length) {
    switch (quest->type) {
        case QST_TYPE_FETCH: {
                struct itm_item *item = itm_create(quest->qst_params[0]);
                snprintf(str, max_length, quest_description_templates[quest->type], quest->qst_params[1], item->sd_name);
                itm_destroy(item);
            } break;
        default: 
                snprintf(str, max_length, "none.");
            break;
    }
    str[max_length-1] = 0;
}

