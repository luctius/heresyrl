#ifndef CAREER_H
#define CAREER_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "careers/careers_static.h"
#include "monster/monster.h"

#define CR_EXIT_IDS_MAX  10
#define CR_TRAPPINGS_MAX 10
struct cr_career {
    enum career_ids template_id;

    const char *title;
    const char *description;

    int char_advancements[MSR_CHAR_MAX];
    bitfield64_t skills;

    uint8_t talents[MSR_NR_TALENTS_MAX];

    enum item_ids trappings[CR_TRAPPINGS_MAX];
    enum career_ids exit_template_ids[CR_EXIT_IDS_MAX];

    /* career generation */
    int weight[MSR_RACE_MAX];
};

struct cr_career *cr_get_career_by_id(enum career_ids template_id);
enum career_ids cr_get_next_career_id_for_race(enum msr_race race, enum career_ids prev_tid);
enum career_ids cr_spawn(double roll, enum msr_race race);

bool cr_give_trappings_to_player(struct msr_monster *monster, struct cr_career *car);

#endif /* CAREER_H */
