#pragma once
#ifndef GAME_H_
#define GAME_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "player.h"

struct dc_map;
struct sgt_sight;
struct logging;

struct gm_game {
    struct random *game_random;
    struct random *map_random;
    struct random *spawn_random;
    struct random *ai_random;

    struct sgt_sight *sight;

    struct dc_map *current_map;
    struct pl_player player_data;
    int turn;
    bool running;
};

extern struct gm_game *gbl_game;

void game_init(struct pl_player *plr, unsigned long initial_seed);
bool game_init_map(void);
bool game_new_tick(void);
bool game_exit(void);

#endif /*GAME_H_*/
