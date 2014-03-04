#ifndef INVENTORY_H
#define INVENTORY_H

#include "items.h"

struct inventory;

struct inventory *inv_init(void);
void inv_exit(struct inventory *inv);

#endif /* INVENTORY_H */
