#include "spawn_static.h"

static struct spwn_item item_weights[] = {
    {.id=ITEM_ID_TORCH,          .weight=1 },
    {.id=ITEM_ID_PISTOL_AMMO,    .weight=1 },
    {.id=ITEM_ID_STUB_AUTOMATIC, .weight=2 },
    {.id=ITEM_ID_STUB_REVOLVER,  .weight=1, },
};

static struct spwn_monster monster_weights[] = {
    {.weight=1, .id=MSR_ID_BASIC_FERAL, },
};
