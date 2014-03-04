#include <sys/queue.h>
#include "inventory.h"

struct inv_entry {
    struct itm_items *item;
    LIST_ENTRY(inv_entry) entry;
};

struct inventory {
    LIST_HEAD(invhead, inv_entry) head;
};

struct inventory *inv_init(void) {
}

void inv_exit() {
}
