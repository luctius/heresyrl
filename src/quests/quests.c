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
#include <float.h>
#include <string.h>

#include "quests.h"
#include "quests_static.h"

#include "items/items.h"

#include "quests_static_def.h"

struct quest *qst_spawn(int level) {
    struct quest *quest = &static_quest_list[QSTID_WISE_WOMAN];
    quest->state = 0;
    memset(quest->params, 0x0, sizeof(quest->params) );
    return quest;
}

enum dm_dungeon_type qst_select_dungeon(struct quest *quest, double roll) {
    int sz = QUEST_SZ;
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0.f;

    int idx = DUNGEON_TYPE_PLAIN;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = 0; i < sz; i++) {
        bool use = false;
        if (quest->dungeon[i].weight != 0) use = true;

        if (use) {
            sum += quest->dungeon[i].weight;
            cumm_prob_arr[i] = 0.f;
        }
        else cumm_prob_arr[i] = DBL_MAX;
    }

    double cumm = 0.f;
    for (int i = 0; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = quest->dungeon[i].weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    for (int i = sz-1; i > 0; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    return quest->dungeon[idx].type;
}

enum msr_race qst_select_enemy(struct quest *quest, double roll) {
    int sz = QUEST_SZ;
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0.f;

    int idx = MSR_RACE_BEAST;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = 0; i < sz; i++) {
        bool use = false;
        if (quest->enemies[i].weight != 0) use = true;

        if (use) {
            sum += quest->enemies[i].weight;
            cumm_prob_arr[i] = 0.f;
        }
        else cumm_prob_arr[i] = DBL_MAX;
    }

    double cumm = 0.f;
    for (int i = 0; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = quest->enemies[i].weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    for (int i = sz-1; i > 0; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    return quest->enemies[idx].race;
}

void qst_process_quest_start(struct quest *quest, struct dm_map *map) {
    switch (quest->type) {
        case QST_TYPE_FETCH: {
                /* Distribute Items */
                quest->state = QST_FTCH_FETCHING;
        } break;
        default: break;
    }

    if (quest->start != NULL) GM_msg("%s", quest->start);
}

void qst_process_quest_end(struct quest *quest, struct dm_map *map) {
    if (qst_is_quest_done(quest, map) ) {
        if (quest->end != NULL) GM_msg("%s", quest->end);
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
                    bool done = true;

                    while ( (item = inv_get_next_item(inv, item) ) != NULL) {
                        for (int i = 0; i < QUEST_SZ && quest->qst_params[i] != IID_NONE; i+= 2) {
                            if (item->uid == quest->qst_params[i]) {
                                quest->qst_params[i+1] -= item->stacked_quantity;
                            }
                        }
                    }

                    for (int i = 0; i < QUEST_SZ && quest->qst_params[i] != IID_NONE; i+= 2) {
                        if (quest->qst_params[i+1] > 0) done = false;
                    }

                    if (done) {
                        if (quest->state == QST_FTCH_FETCHING) You(player, "You now posses the items to complete your quest.");
                        quest->state = QST_FTCH_END;
                    }
                    else {
                        if (quest->state == QST_FTCH_END) You(player, "You no longer posses the items to complete your quest.");
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

