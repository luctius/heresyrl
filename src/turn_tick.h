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

#ifndef TURN_TICK_H
#define TURN_TICK_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

enum tt_energy {
    TT_ENERGY_TICK      = 5,
    TT_ENERGY_TURN_MINI = 10,
    TT_ENERGY_TURN      = 100,
    TT_ENERGY_FULL      = 200,
};

void tt_init(void);
void tt_exit(void);

void tt_process(struct dm_map *map);
bool tt_interrupt_monster(uint32_t monster_uid);

#endif /* TURN_TICK_H */
