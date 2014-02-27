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

struct msr_monster {
    int hp;
    int x_pos;
    int y_pos;
    char icon;
    uint8_t colour;
    uint8_t visibility;
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

#endif /*MONSTER_H_*/
