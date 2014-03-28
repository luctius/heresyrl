#ifndef TURN_TICK_H
#define TURN_TICK_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

enum tt_energy {
    TT_ENERGY_TICK = 10,
    TT_ENERGY_TURN = 100,
    TT_ENERGY_FULL = 1000,
};

void tt_init(void);
void tt_exit(void);

void tt_process(struct dc_map *map);
bool tt_interrupt_monster(uint32_t monster_uid);

#endif /* TURN_TICK_H */