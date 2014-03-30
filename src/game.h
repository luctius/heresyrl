#pragma once
#ifndef GAME_H_
#define GAME_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "player.h"

struct dm_map;
struct sgt_sight;
struct logging;

void game_init(struct pl_player *plr, unsigned long initial_seed);

bool game_load(void);

bool game_init_map(void);
bool game_new_tick(void);
bool game_exit(void);

#endif /*GAME_H_*/
