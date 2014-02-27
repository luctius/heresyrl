#pragma once
#ifndef MONSTER_H_
#define MONSTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "items.h"

struct monster_list;
extern struct monster_list *monster_list_head;

enum msr_gender {
    MSR_GENDER_MALE,
    MSR_GENDER_FEMALE,
    MSR_GENDER_IT,
    MSR_GENDER_MAX,
    MSR_GENDER_RANDOM,
};

enum msr_characteristic {
    MSR_CHAR_WEAPON_SKILL,
    MSR_CHAR_BALISTIC_SKILL,
    MSR_CHAR_STRENGTH,
    MSR_CHAR_TOUCHNESS,
    MSR_CHAR_AGILITY,
    MSR_CHAR_INTELLIGIENCE,
    MSR_CHAR_PERCEPTION,
    MSR_CHAR_WILLPOWER,
    MSR_CHAR_FELLOWSHIP,
    MSR_CHAR_MAX,
};

struct msr_char {
    uint8_t base_value;
    uint8_t advancement;
};

struct msr_monster {
    int x_pos;
    int y_pos;
    char icon;
    uint8_t colour;
    uint8_t visibility;
    bool is_player;
    enum msr_gender gender;

    struct msr_char characteristic[MSR_CHAR_MAX];
};

struct msr_monster_list_entry {
    struct msr_monster monster;
    LIST_ENTRY(msr_monster_list_entry) entries;
};

void msr_monster_list_init(void);
void msr_monster_list_exit(void);

struct msr_monster *msr_create(void);
void msr_die(struct msr_monster *monster, struct dc_map *map);
bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, int x_togo, int y_togo);
bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, int x_togo, int y_togo);
bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map);

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);

#endif /*MONSTER_H_*/
