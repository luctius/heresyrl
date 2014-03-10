#ifndef MONSTER_TURN_H
#define MONSTER_TURN_H

#include <limits.h>

#include "heresyrl_def.h"
#include "coord.h"

#define MT_ENERGY_FULL (UINT32_MAX - 1000)
#define MT_ENERGY_TURN (100)
#define MT_ENERGY_TICK (10)

typedef bool (*mt_callback_t)(struct msr_monster *monster, void *controller);

void mt_init(void);
void mt_exit(void);

bool mt_procces(void);
bool mt_interrupt_event(uint32_t monster_uid);

bool mt_do_move(struct msr_monster *monster, coord_t *pos, mt_callback_t cb, void *controller);
bool mt_do_idle(struct msr_monster *monster, mt_callback_t cb, void *controller);
bool mt_do_guard(struct msr_monster *monster, mt_callback_t cb, void *controller);


#endif /* MONSTER_TURN_H */
