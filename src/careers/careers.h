#ifndef CAREER_H
#define CAREER_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "enums.h"
#include "careers/careers_static.h"
#include "monster/monster.h"

#define CR_EXIT_IDS_MAX  10
#define CR_TRAPPINGS_MAX 10
#define CR_TALENTS_MAX   10
#define CR_ALLIES_MAX    2
struct cr_career {
    enum career_ids template_id;

    const char *title;
    const char *description;

    int char_advancements[MSR_CHAR_MAX];
    bitfield64_t skills;

    int wounds;

    uint8_t talents[CR_TALENTS_MAX];

    enum item_ids trappings[CR_TRAPPINGS_MAX];
    enum career_ids exit_template_ids[CR_EXIT_IDS_MAX];
    enum msr_ids allies_ids[CR_ALLIES_MAX];

    /* career generation */
    int weight[MSR_RACE_MAX];
    bool available[MSR_RACE_MAX];
};

struct cr_career *cr_get_career_by_id(enum career_ids template_id);
enum career_ids cr_get_next_career_id_for_race(enum msr_race race, enum career_ids prev_tid);
enum career_ids cr_spawn(double roll, enum msr_race race);

bool cr_give_trappings_to_player(struct cr_career *car, struct msr_monster *monster);
bool cr_generate_allies(struct cr_career *car, struct msr_monster *player, struct dm_map *map);

bool cr_can_upgrade_characteristic(struct cr_career *car, struct msr_monster *monster, enum msr_characteristic c);
bool cr_can_upgrade_wounds(struct cr_career *car, struct msr_monster *monster);
bool cr_can_upgrade_skill(struct cr_career *car, struct msr_monster *monster, enum msr_skills skill);
bool cr_can_upgrade_talent(struct cr_career *car, struct msr_monster *monster, enum msr_talents talent);

bool cr_upgrade_characteristic(struct cr_career *car, struct msr_monster *monster, enum msr_characteristic c);
bool cr_upgrade_wounds(struct cr_career *car, struct msr_monster *monster);
bool cr_upgrade_skill(struct cr_career *car, struct msr_monster *monster, enum msr_skills skill);
bool cr_upgrade_talent(struct cr_career *car, struct msr_monster *monster, enum msr_talents talent);

#endif /* CAREER_H */
