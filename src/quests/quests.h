#ifndef QUESTS_H
#define QUESTS_H

#include "heresyrl_def.h"
#include "monster/monster.h"
#include "dungeon/dungeon_map.h"

enum quest_types {
    QST_TYPE_NONE,
    QST_TYPE_KILL_ALL,
    QST_TYPE_KILL_LEADER,
    QST_TYPE_FETCH,
};

struct quest {
    uint32_t tid;
    enum quest_types type;

    const char *description;
    const char *start;
    const char *end;
    int xp_reward;
    int gp_reward;

    struct {
        enum dm_dungeon_type type;
        int weight;
    } dungeon[5];

    struct {
        enum msr_race race;
        int weight;
    } enemies[5];

    /* used by the code */
    int state;
    uint32_t params[10];
};

struct quest *qst_spawn(int level);

void qst_process_quest_start(struct quest *quest, struct dm_map *map);
void qst_process_quest_end(struct quest *quest, struct dm_map *map);
void qst_process_quest_during(struct quest *quest, struct dm_map *map);

#endif /* QUESTS_H */
