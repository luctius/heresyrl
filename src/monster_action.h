#ifndef MONSTER_ACTION_H
#define MONSTER_ACTION_H

#include "heresyrl_def.h"
#include "coord.h"

struct monster_controller {
    void *controller;
    struct msr_monster *monster;
    event_callback_t callback;
};

bool ma_do_guard(struct monster_controller *mc);
bool ma_do_move(struct monster_controller *mc, coord_t *newpos);
bool ma_do_wear(struct monster_controller *mc, struct itm_item *item);
bool ma_do_drop(struct monster_controller *mc, struct itm_item *item);
bool ma_do_get(struct monster_controller *mc, struct itm_item *item);
bool ma_do_throw(struct monster_controller *mc, struct itm_item *item);


#endif /* MONSTER_ACTION_H */
