#include "inventory.h"

/*
   This file is included in items.c.
   This is purely my sanity that it is seperate.
   TODO create a nice solution.
 */

#include "item_descriptions.h"

#define ITEM(item_id,_sd_name,_ld_name,_availability,item_quality,item_weight,item_cost,delay) \
            [item_id]={.uid=0, .template_id=item_id, .availability=_availability, .quality=item_quality, \
            .age=0, .weight=item_weight, .cost=item_cost, .sd_name=_sd_name, .ld_name=_ld_name, \
            .icon_attr=TERM_COLOUR_SLATE, .use_delay=delay, .dropable=true
#define ITEM_END }

#define LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='(', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, .energy_left=_energy, \
            .light_luminem=_light_luminem, .lit=false,}

#define ARMOUR(_damage_reduction,_locations,_special_quality) \
            .icon=']', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEARABLE, \
            .specific.wearable={ .wearable_type=WEARABLE_TYPE_ARMOUR, .locations=_locations, \
            .special_quality=_special_quality, .damage_reduction=_damage_reduction,}

#define CREATURE_MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=special, .wpn_talent=bf(WPNTLT_CREATURE_WPN_TALENT), }

#define MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=wpn_cat, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, }

#define THROWN_WEAPON(dmg_die,dmg_add,_penetration,_range,_dmg_type,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_WEAPON, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, }

#define THROWN_GRENADE(dmg_die,dmg_add,_penetration,_range,_dmg_type,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_GRENADE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, }

#define PISTOL_SP(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_SP, .ammo_used_template_id=IID_PISTOL_AMMO_SP, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=bf(WPNTLT_PISTOL_WPN_TRNG_SP), .jammed=false,}

#define PISTOL_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_LAS, .ammo_used_template_id=IID_PISTOL_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=bf(WPNTLT_PISTOL_WPN_TRNG_LAS), .jammed=false,}

#define BASIC_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_BASIC, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_BASIC_LAS, .ammo_used_template_id=IID_BASIC_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=bf(WPNTLT_BASIC_WPN_TRNG_LAS), .jammed=false,}

#define AMMO(_ammo_type,_energy) .icon='^', .stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_AMMO, .specific.ammo={ .ammo_type=_ammo_type, .energy=_energy, .energy_left=_energy,}

const char *itm_descs[] = {
    [IID_FIXED_LIGHT]       = "",
    [IID_GLOW_GLOBE]        = "This a generic glow globe",

    [IID_FLAK_HELMET]       = "This a generic flak helmet",
    [IID_FLAK_GAUNTLETS]    = "These are generic flak gauntlets",
    [IID_FLAK_LIGHT_COAT]   = "These are generic light flak coat",
    [IID_FLAK_VEST]         = "These are generic flak vest",
    [IID_FLAK_JACKET]       = "These are generic flak jacket",
    [IID_FLAK_CLOAK]        = "These are generic flak cloak",
    [IID_FLAK_GUARD_ARMOUR] = "These are generic guard flak armour",

    [IID_HUMAN_UNARMED]           = "",
    [IID_CREATURE_BITE_UNTRAINED] = "",
    [IID_CREATURE_BITE_TRAINED]   = "",

    [IID_FRAG_GRENADE]      = "",
    [IID_THROWING_KNIFE]    = "",

    [IID_STUB_AUTOMATIC]    = "Just as common as the revolver variant, the stub automatic allows for a greater rate of fire and clip capacity, though at the cost of reliability",
    [IID_STUB_REVOLVER]     = "Based on an ancient and well-tested design, the stub revolver is the ideal backup weapon",
    [IID_LAS_PISTOL]        = "The las pistol is a light, compact and reliable weapon, common throughout the Imperium. Designs vary wildy and can range from elaborate heirloom "
                                    "devices inscribed with ornate carvings and gold filigree, to simplistic but brutaly robust weapons used by gangs and criminals",
    [IID_LAS_GUN]           = "Produced in a multitude of different styles and patterns, the lasgun can be found on almost every world of the Imperium, where its robust design "
                                    "and dependability make it a favoured weapon of both Emperor's faithfull and many of their foes",

    [IID_PISTOL_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_PISTOL_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",

    [IID_BASIC_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_BASIC_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",
};

struct itm_item static_item_list[] = {
    /* Tools */


    /* Lights */
    /*    ID              short name    long name       availability        quality          weight,cost,delay       tool type         energy  luminem*/
    ITEM(IID_FIXED_LIGHT, "torch",      "a torch",      ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE, 1,     1,   1), LIGHT(TOOL_TYPE_LIGHT,  65000,   10), .dropable = false, ITEM_END,
    ITEM(IID_GLOW_GLOBE,  "glow globe", "a glow globe", ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE, 1,     1,   1), LIGHT(TOOL_TYPE_LIGHT,   1000,    10), ITEM_END,

    /* Wearables */
    /*    ID                         short name           long name         availability        quality          weight, cost, delay         dr   locations                                             special qualities*/
    ITEM(IID_FLAK_HELMET,       "flak helmet",       "a flak helmet",       ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   20,    25,   2), ARMOUR(2,   INV_LOC_HEAD,                                                 0), ITEM_END,
    ITEM(IID_FLAK_GAUNTLETS,    "flak gauntlets",    "two flak gauntlets",  ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   10,    50,   2), ARMOUR(2,   INV_LOC_ARMS,                                                 0), ITEM_END,
    ITEM(IID_FLAK_LIGHT_COAT,   "light flak coat",   "a light flak coat",   ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   40,    80,   4), ARMOUR(2,   INV_LOC_ARMS | INV_LOC_CHEST | INV_LOC_LEGS,                  0), ITEM_END,
    ITEM(IID_FLAK_VEST,         "flak vest",         "a flak vest",         ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   50,    50,   3), ARMOUR(3,   INV_LOC_CHEST,                                                0), ITEM_END,
    ITEM(IID_FLAK_JACKET,       "flak jacket",       "a flak jacket",       ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   60,    100,  4), ARMOUR(3,   INV_LOC_ARMS | INV_LOC_CHEST | INV_LOC_LEGS,                  0), ITEM_END,
    ITEM(IID_FLAK_CLOAK,        "flak cloak",        "a flak cloak",        ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   80,    80,   3), ARMOUR(3,   INV_LOC_CHEST,                                                0), ITEM_END,
    ITEM(IID_FLAK_GUARD_ARMOUR, "guard flak armour", "a guard flak armour", ITEM_AVAIL_AVERAGE, ITEM_QLTY_AVERAGE,   110,   300,  3), ARMOUR(4,   INV_LOC_ARMS | INV_LOC_LEGS | INV_LOC_CHEST | INV_LOC_HEAD,   0), ITEM_END,

    /* Weapons */
    /* Creature Attacks */
    /*    ID                          short name  long name  availability     quality       weight,cost,delay                CATEGORY                xd10  +X   dmg type                 upgrades   special qualities*/
    ITEM(IID_HUMAN_UNARMED,           "hands",    "hands",   ITEM_AVAIL_NONE, ITEM_QLTY_NONE, 0,    0,   0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,0,    -3,  WEAPON_DMG_TYPE_IMPACT,   0,        bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM(IID_CREATURE_BITE_UNTRAINED,"teeth",     "teeth",   ITEM_AVAIL_NONE, ITEM_QLTY_NONE, 0,    0,   0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1,    -3,  WEAPON_DMG_TYPE_RENDING,  0,        bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM(IID_CREATURE_BITE_TRAINED,  "teeth",     "teeth",   ITEM_AVAIL_NONE, ITEM_QLTY_NONE, 0,    0,   0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1,     0,  WEAPON_DMG_TYPE_RENDING,  0,        0), ITEM_END,

    /* Melee */


    /* Thrown */
    /*    ID               short name         long name          availability         quality         weight,cost,delay  CATEGORY     xd10 +X pen,range   dmg type              upgrades    special qualities         talent */
    ITEM(IID_FRAG_GRENADE, "frag grenade",   "a frag grenade",   ITEM_AVAIL_AVERAGE,  ITEM_QLTY_AVERAGE, 5,  10,  1),    THROWN_GRENADE(2,  0, 0,  3,  WEAPON_DMG_TYPE_EXPLOSIVE, 0, bf(WPN_SPCQLTY_BLAST_4),   bf(WPNTLT_CREATURE_WPN_TALENT)), ITEM_END,
    ITEM(IID_THROWING_KNIFE,"throwing knife","a throwing knife", ITEM_AVAIL_PLENTIFUL,ITEM_QLTY_AVERAGE, 5,  5,   1),    THROWN_WEAPON( 0,  0, 0,  5,  WEAPON_DMG_TYPE_RENDING,   0, bf(WPN_SPCQLTY_PRIMITIVE), bf(WPNTLT_THROWN_WPN_TRNG_PRIMITIVE)), ITEM_END,

    /* Pistols */
    /*    ID                short name        long name         availability       quality         weight,cost,delay          dmg type            xd10 +x range  single semi  auto  mag_sz  pen  upgrades  special*/
    ITEM(IID_STUB_AUTOMATIC,"stub automatic","a stub automatic",ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE, 15, 50,  1), PISTOL_SP(WEAPON_DMG_TYPE_IMPACT, 1, 3, 30,    1,    3,    0,     9,      0,   0,        0                     ), ITEM_END,
    ITEM(IID_STUB_REVOLVER, "stub revolver", "a stub revolver", ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE, 15, 40,  2), PISTOL_SP(WEAPON_DMG_TYPE_IMPACT, 1, 3, 30,    1,    0,    0,     6,      0,   0,     bf(WPN_SPCQLTY_RELIABLE) ), ITEM_END,
    ITEM(IID_LAS_PISTOL,    "las pistol",    "a las pistol",    ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE, 17, 50,  1), PISTOL_LAS(WEAPON_DMG_TYPE_ENERGY,1, 2, 30,    1,    0,    0,    30,      0,   0,     bf(WPN_SPCQLTY_RELIABLE) ), ITEM_END,


    /* Basic weapons */
    /*    ID                short name        long name         availability       quality         weight,cost,delay          dmg type            xd10 +x range  single semi  auto  mag_sz  pen  upgrades  special*/
    ITEM(IID_LAS_GUN,       "las gun",       "a las gun",       ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE, 40, 50,  1), BASIC_LAS(WEAPON_DMG_TYPE_ENERGY, 1, 3, 100,   1,    3,    0,    60,      0,   0,     bf(WPN_SPCQLTY_RELIABLE) ), ITEM_END,

    /* Ammo */
    /*    ID                  short name            long name                               availability       quality          weight,cost,delay         ammo Type         energy level*/
    ITEM(IID_PISTOL_AMMO_SP,  "solid pistol ammo", "a clip of solid projectile pistol ammo",ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE,  0,     1,   0), AMMO(AMMO_TYPE_PISTOL_SP,  0   ), ITEM_END,
    ITEM(IID_PISTOL_AMMO_LAS, "pistol charge pack","a pistol charge pack",                  ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE,  0,     1,   0), AMMO(AMMO_TYPE_PISTOL_LAS, 1000), ITEM_END,

    ITEM(IID_BASIC_AMMO_SP,   "solid basic ammo", "a clip of solid projectile basic ammo",  ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE,  0,     1,   0), AMMO(AMMO_TYPE_BASIC_SP,   0   ), ITEM_END,
    ITEM(IID_BASIC_AMMO_LAS,  "basic charge pack", "a basic weapon charge pack",            ITEM_AVAIL_AVERAGE,ITEM_QLTY_AVERAGE,  0,     1,   0), AMMO(AMMO_TYPE_BASIC_LAS,  1000), ITEM_END,
};

static const char *item_quality_strings[] = {
    [ITEM_QLTY_POOR]    =  "poor",
    [ITEM_QLTY_AVERAGE] =  "average",
    [ITEM_QLTY_GOOD]    =  "good",
    [ITEM_QLTY_BEST]    =  "best",
};

static const char *ammo_type_strings[] = {
    [AMMO_TYPE_ARROW]  =  "arrows",

    [AMMO_TYPE_PISTOL_SP]       =  "pistol solid projectile ammo",
    [AMMO_TYPE_PISTOL_LAS]      =  "pistol charge packs",
    [AMMO_TYPE_PISTOL_PLASMA]   =  "pistol plasma flasks",
    [AMMO_TYPE_PISTOL_MELTA]    =  "pistol melta canisters",
    [AMMO_TYPE_PISTOL_FLAME]    =  "pistol flame fuel",
    [AMMO_TYPE_PISTOL_BOLT]     =  "pistol bolt clips",
    [AMMO_TYPE_PISTOL_SHURIKEN] =  "pistol shuriken clips",
    [AMMO_TYPE_PISTOL_SHOTGUN]  =  "pistol shotgun shells",

    [AMMO_TYPE_BASIC_GRENADE]   =  "basic rifle grenades",
    [AMMO_TYPE_BASIC_SP]        =  "basic solid projectile ammo",
    [AMMO_TYPE_BASIC_LAS]       =  "basic charge packs",
    [AMMO_TYPE_BASIC_PLASMA]    =  "basic plasma flasks",
    [AMMO_TYPE_BASIC_MELTA]     =  "basic melta canisters",
    [AMMO_TYPE_BASIC_FLAME]     =  "basic flame fuel",
    [AMMO_TYPE_BASIC_BOLT]      =  "basic bolt clips",
    [AMMO_TYPE_BASIC_SHOTGUN]   =  "basic shotgun shells",
    [AMMO_TYPE_BASIC_SHURIKEN]  =  "basic shuriken clips",
    [AMMO_TYPE_BASIC_ROCKET]    =  "basic rockets",

    [AMMO_TYPE_HEAVY_GRENADE]   =  "heavy rifle grenades",
    [AMMO_TYPE_HEAVY_SP]        =  "heavy solid projectile ammo",
    [AMMO_TYPE_HEAVY_LAS]       =  "heavy charge packs",
    [AMMO_TYPE_HEAVY_PLASMA]    =  "heavy plasma flasks",
    [AMMO_TYPE_HEAVY_FLAME]     =  "heavy flame fuel",
    [AMMO_TYPE_HEAVY_MELTA]     =  "heavy melta canisters",
    [AMMO_TYPE_HEAVY_BOLT]      =  "heavy bolt clips",
    [AMMO_TYPE_HEAVY_ROCKET]    =  "heavy rockets",
};

static const char *wpn_spcqlty_name[] = {
    [WPN_SPCQLTY_ACCURATE]	    = "accurate",
    [WPN_SPCQLTY_BALANCED]      = "balanced",
    [WPN_SPCQLTY_BLAST_1]	    = "blast 1",
    [WPN_SPCQLTY_BLAST_2]	    = "blast 2",
    [WPN_SPCQLTY_BLAST_3]       = "blast 3",
    [WPN_SPCQLTY_BLAST_4]       = "blast 4",
    [WPN_SPCQLTY_DEFENSIVE]	    = "defensive",
    [WPN_SPCQLTY_FLAME]	        = "flame",
    [WPN_SPCQLTY_FLEXIBLE]      = "flexible",
    [WPN_SPCQLTY_INACCURATE]    = "inaccurate",
    [WPN_SPCQLTY_OVERHEATS]     = "overheats",
    [WPN_SPCQLTY_POWER_FIELD]   = "power field",
    [WPN_SPCQLTY_PRIMITIVE]	    = "primitive",
    [WPN_SPCQLTY_RECHARGE]	    = "recharge",
    [WPN_SPCQLTY_RELIABLE]	    = "reliable",
    [WPN_SPCQLTY_SCATTER]	    = "scatter",
    [WPN_SPCQLTY_SHOCKING]	    = "shocking",
    [WPN_SPCQLTY_SMOKE]	        = "smoke",
    [WPN_SPCQLTY_SNARE]	        = "snare",
    [WPN_SPCQLTY_TEARING]	    = "tearing",
    [WPN_SPCQLTY_TOXIC]	        = "toxic",
    [WPN_SPCQLTY_UNARMED]	    = "unarmed",
    [WPN_SPCQLTY_UNBALANCED]	= "unbalanced",
    [WPN_SPCQLTY_UNRELIABLE]	= "unreliable",
    [WPN_SPCQLTY_UNSTABLE]	    = "unstable",
    [WPN_SPCQLTY_UNWIELDY]	    = "unwieldy",
};

static const char *wpn_spcqlty_desc[] = {
    [WPN_SPCQLTY_ACCURATE]	    = " Additional +10 to hit when used with the aim action ",
    [WPN_SPCQLTY_BALANCED]      = " +10 Parry ",
    [WPN_SPCQLTY_BLAST_1]	    = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_BLAST_2]	    = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_BLAST_3]       = "",
    [WPN_SPCQLTY_BLAST_4]       = "",
    [WPN_SPCQLTY_DEFENSIVE]	    = " +15 Parry, -10 to hit  ",
    [WPN_SPCQLTY_FLAME]	        = " No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. ",
    [WPN_SPCQLTY_FLEXIBLE]	    = " Cannot be Parried. ",
    [WPN_SPCQLTY_INACCURATE]	= " No bonus with aim action ",
    [WPN_SPCQLTY_OVERHEATS]	    = " 90+ -> Overheat ",
    [WPN_SPCQLTY_POWER_FIELD]	= " When this is parried, 75% chance to destroy parrying weapon ",
    [WPN_SPCQLTY_PRIMITIVE]	    = " AP doubled, unless armour is also primitive ",
    [WPN_SPCQLTY_RECHARGE]	    = " Fire only every other round ",
    [WPN_SPCQLTY_RELIABLE]	    = " If jam, 10 on 1d10 to actually jam. <I> ",
    [WPN_SPCQLTY_SCATTER]	    = " Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. ",
    [WPN_SPCQLTY_SHOCKING]	    = " If damage, test Toughness or be stunned. ",
    [WPN_SPCQLTY_SMOKE]	        = " Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds. ",
    [WPN_SPCQLTY_SNARE]	        = " Ag. test or be immobilised ",
    [WPN_SPCQLTY_TEARING]	    = " Roll twice, take highest damage ",
    [WPN_SPCQLTY_TOXIC]	        = " if damage, teest toughness -5 for every damage taken. if failed, take 1d10 extra impact damage. ",
    [WPN_SPCQLTY_UNARMED]	    = " 1d5-3 +Str, Armour doubles ",
    [WPN_SPCQLTY_UNBALANCED]	= " -10 when parry ",
    [WPN_SPCQLTY_UNRELIABLE]	= " jams on 90+ ",
    [WPN_SPCQLTY_UNSTABLE]	    = " on hit, roll 1d10: 1: half dmg, 2-9: normal, 10: double damage ",
    [WPN_SPCQLTY_UNWIELDY]	    = " Cannot be used to parry. ",
};

