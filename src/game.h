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

#pragma once
#ifndef GAME_H_
#define GAME_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "player.h"

struct dm_map;
struct logging;

void game_init(struct pl_player *plr, unsigned long initial_seed);

bool game_load(void);

bool game_init_map(void);
bool game_new_tick(void);
bool game_exit(void);

#endif /*GAME_H_*/
