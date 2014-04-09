#include "spawn_static.h"

static struct spwn_item item_weights[] = {
    {.id=ITEM_ID_GLOW_GLOBE,       .weight=1 },

    {.id=ITEM_ID_PISTOL_AMMO_SP,   .weight=2 },
    {.id=ITEM_ID_PISTOL_AMMO_LAS,  .weight=2 },
    {.id=ITEM_ID_BASIC_AMMO_LAS,   .weight=2 },

    {.id=ITEM_ID_STUB_AUTOMATIC,   .weight=2 },
    {.id=ITEM_ID_STUB_REVOLVER,    .weight=2, },
    {.id=ITEM_ID_LAS_PISTOL,       .weight=2, },
    {.id=ITEM_ID_LAS_GUN,          .weight=2, },
};

static struct spwn_monster monster_weights[] = {
    {.weight=1, .id=MSR_ID_HIVE_GANGER,
        .items = {  {.id=ITEM_ID_FLAK_LIGHT_COAT,.min=1,.max=1,.wear=true}, 
                    {.id=ITEM_ID_LAS_PISTOL,.min=1,.max=1,.wear=true}, 
                    {.id=ITEM_ID_PISTOL_AMMO_LAS,.min=0,.max=3,.wear=false},{ 0,0,0,0,} },},
    {.weight=1, .id=MSR_ID_HIVE_GANGER,
        .items = {  {.id=ITEM_ID_STUB_AUTOMATIC,.min=1,.max=1,.wear=true}, 
                    {.id=ITEM_ID_PISTOL_AMMO_SP,.min=5,.max=50,.wear=false},{ 0,0,0,0,} },},
    {.weight=1, .id=MSR_ID_VICIOUS_DOG, .items={{0,0,0,0,} },},
};

