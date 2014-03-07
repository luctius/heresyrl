#ifndef DOWEAR_H
#define DOWEAR_H

#include "heresyrl_def.h"

bool dw_wear_item(struct msr_monster *monster, struct itm_item *item);
bool dw_remove_item(struct msr_monster *monster, struct itm_item *item);
bool dw_use_item(struct msr_monster *monster, struct itm_item *item);

#endif /* DOWEAR_H */
