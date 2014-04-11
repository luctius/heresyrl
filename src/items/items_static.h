#ifndef ITEMS_STATIC_H
#define ITEMS_STATIC_H

#include "items.h"

enum item_ids {
    /* Wearables*/
    IID_FLAK_HELMET,
    IID_FLAK_GAUNTLETS,
    IID_FLAK_LIGHT_COAT,
    IID_FLAK_VEST,
    IID_FLAK_JACKET,
    IID_FLAK_CLOAK,
    IID_FLAK_GUARD_ARMOUR,

    /* Tools */
    /* Fixed Items */
    IID_FIXED_LIGHT,

    /* Tools */
    IID_GLOW_GLOBE, //GLOWGLOBE
    //Screamer
    //Respirator
    //photovisor
    //medipack
    //infra-red goggles
    //filter plugs
    //bio-scanner

    /* Weapons */
    /* Creature Attacks */
    IID_HUMAN_UNARMED,
    IID_CREATURE_BITE_UNTRAINED,
    IID_CREATURE_BITE_TRAINED,

    /* Pistols*/
    IID_STUB_AUTOMATIC,
    IID_STUB_REVOLVER,
    IID_LAS_PISTOL,

    /* Basic weapons */
    IID_LAS_GUN,

    /* Thrown */
    IID_THROWING_KNIFE,

    /* Grenages */
    IID_FRAG_GRENADE,

    /* Ammo */
    IID_ARROW,
    IID_PISTOL_AMMO_SP,
    IID_PISTOL_AMMO_SP_DUMDUM,
    IID_PISTOL_AMMO_SP_MANSTOPPER,
    IID_PISTOL_AMMO_LAS,
    IID_PISTOL_AMMO_PLASMA,
    IID_PISTOL_AMMO_MELTA,
    IID_PISTOL_AMMO_FLAME,
    IID_PISTOL_AMMO_BOLT,
    IID_PISTOL_AMMO_SHURIKEN,
    IID_BASIC_AMMO_SP,
    IID_BASIC_AMMO_SP_DUMDUM,
    IID_BASIC_AMMO_SP_MANSTOPPER,
    IID_BASIC_AMMO_SHOTGUN,
    IID_BASIC_AMMO_LAS,
    IID_BASIC_AMMO_PLASMA,
    IID_BASIC_AMMO_MELTA,
    IID_BASIC_AMMO_FLAME,
    IID_BASIC_AMMO_BOLT,
    IID_BASIC_AMMO_SHURIKEN,
    IID_BASIC_AMMO_GRENADE,
    IID_HEAVY_AMMO_SP,
    IID_HEAVY_AMMO_LAS,
    IID_HEAVY_AMMO_PLASMA,
    IID_HEAVY_AMMO_MELTA,
    IID_HEAVY_AMMO_FLAME,
    IID_HEAVY_AMMO_BOLT,
    IID_HEAVY_AMMO_GRENADE,
    IID_HEAVY_AMMO_ROCKET,

    /* Last Entry */
    IID_MAX,
};

#endif /* ITEMS_STATIC_H */
