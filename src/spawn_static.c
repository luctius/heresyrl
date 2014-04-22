#include "spawn_static.h"

static struct spwn_item item_weights[] = {
    {.id = IID_GLOW_GLOBE,       .weight=1 },

    {.id = IID_PISTOL_AMMO_SP,   .weight=2 },
    {.id = IID_PISTOL_AMMO_LAS,  .weight=2 },
    {.id = IID_BASIC_AMMO_LAS,   .weight=2 },

    {.id = IID_STUB_AUTOMATIC,   .weight=2 },
    {.id = IID_STUB_REVOLVER,    .weight=2, },
    {.id = IID_LAS_PISTOL,       .weight=2, },
    {.id = IID_LAS_GUN,          .weight=2, },
};

static struct spwn_monster monster_weights[] = {
    /*
    {.weight=1, .id = MID_HIVE_GANGER,
        .items = {  {  .id = IID_FLAK_LIGHT_COAT,   .min=1,.max=1,.wear=true}, 
                    {  .id = IID_LAS_PISTOL,        .min=1,.max=1,.wear=true}, 
                    {  .id = IID_PISTOL_AMMO_LAS,   .min=0,.max=3,.wear=false},{ 0,0,0,0,} },},
    */
    {.weight=4, .id = MID_HIVE_GANGER,
        .items = {  {  .id= IID_STUB_AUTOMATIC,    .min=1,.max=1,.wear=true}, 
                    {  .id = IID_PISTOL_AMMO_SP,    .min=5,.max=50,.wear=false},{ 0,0,0,0,} },},
    {.weight=1, .id = MID_VICIOUS_DOG, .items={{0,0,0,0,} },},
};

