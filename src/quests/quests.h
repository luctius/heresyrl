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

#ifndef QUESTS_H
#define QUESTS_H

#include "quests/quests_static.h"
#include "heresyrl_def.h"
#include "monster/monster.h"
#include "dungeon/dungeon_map.h"

#define QUEST_SZ 10

enum quest_types {
    QST_TYPE_NONE,
    QST_TYPE_KILL_ALL,
    QST_TYPE_KILL_LEADER,
    QST_TYPE_FETCH,
};

enum qst_fetch_states {
    QST_FTCH_START,
    QST_FTCH_FETCHING,
    QST_FTCH_END,
};

struct qst_dungeon {
    enum dm_dungeon_type type;
    int weight;
    int dungeon_levels;
    int item_chance;
    int monster_chance;
    coord_t size;
};

struct qst_enemies {
    int faction;
    int weight;
};

struct quest {
    enum qst_ids tid;
    enum quest_types type;
    int qst_params[QUEST_SZ];

    const wchar_t *description;
    const wchar_t *start;
    const wchar_t *end;
    int xp_reward;
    int gp_reward;
    int weight;
    int min_level;
    int max_level;

    struct qst_dungeon dungeon[QUEST_SZ];
    struct qst_enemies enemies[QUEST_SZ];

    /* used by the code */
    int state;
    uint32_t params[QUEST_SZ];
};

struct quest *qst_by_tid(enum qst_ids tid);
struct quest *qst_spawn(int level, int32_t roll);
struct qst_dungeon *qst_select_dungeon(struct quest *quest, int32_t roll);
struct qst_enemies *qst_select_enemy(struct quest *quest, int32_t roll);

void qst_process_quest_start(struct quest *quest, struct dm_map *map, struct random *r);
void qst_process_quest_end(struct quest *quest, struct dm_map *map);
void qst_process_quest_during(struct quest *quest, struct dm_map *map);

bool qst_is_quest_done(struct quest *quest, struct dm_map *map);

void qst_get_description(struct quest *quest, wchar_t *str, int max_length);
#endif /* QUESTS_H */
