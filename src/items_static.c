#include "inventory.h"

/*
   This file is included in items.c.
   This is purely my sanity that it is seperate.
   TODO create a nice solution.
 */

#define GLOW_GLOBE_DESC "This a generic glow globe."
#define STUB_AUTOMATIC_DESC "This a generic stub automatic."
#define STUB_REVOLVER_DESC "This a generic stub revolver."
#define LAS_PISTOL_DESC "This a generic las pistol."
#define LAS_GUN_DESC "This a generic las gun."

#define ITEM(item_id,_sd_name,_ld_name,item_desc,_availability,item_quality,item_weight,item_cost,delay) \
    [item_id]={.uid=0, .template_id=item_id, .availability=_availability, .quality=item_quality, \
    .age=0, .weight=item_weight, .cost=item_cost, .sd_name=_sd_name, .ld_name=_ld_name, \
    .description=item_desc, .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, .dropable=true
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
            .penetration=_penetration, .special_quality=special, .wpn_talent=TALENTS0_CREATURE_TALENT_HANDS, }

#define MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=wpn_cat, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special .upgrades=_upgrades, .wpn_talent=talent, }

#define PISTOL_SP(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_SP, .ammo_used_template_id=ITEM_ID_PISTOL_AMMO_SP, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TALENTS0_PISTOL_WEAPON_TRAINING_SOLID_PROJECTILE, .jammed=false,}

#define PISTOL_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_LAS, .ammo_used_template_id=ITEM_ID_PISTOL_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TALENTS0_PISTOL_WEAPON_TRAINING_LAS, .jammed=false,}

#define BASIC_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_BASIC, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_BASIC_LAS, .ammo_used_template_id=ITEM_ID_BASIC_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TALENTS0_BASIC_WEAPON_TRAINING_LAS, .jammed=false,}

#define AMMO(_ammo_type,_energy) .icon='^', .stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_AMMO, .specific.ammo={ .ammo_type=_ammo_type, .energy=_energy, .energy_left=_energy,}

struct itm_item static_item_list[] = {
    /* Tools */
    /* Lights */
    /*    ID                   short name  long name    description          availability               quality              weight,cost,delay*/
    ITEM(ITEM_ID_FIXED_LIGHT,  "torch",    "a torch",   GLOW_GLOBE_DESC,  ITEM_AVAILABILITY_AVERAGE, ITEM_QUALITY_AVERAGE, 1,     1,   1),
        /*        tool type     energy  luminem*/
        LIGHT(TOOL_TYPE_LIGHT,  65000,     12), ITEM_END,

    ITEM(ITEM_ID_GLOW_GLOBE,    "glow globe",    "a glow globe",    GLOW_GLOBE_DESC,  ITEM_AVAILABILITY_AVERAGE, ITEM_QUALITY_AVERAGE, 1,     1,   1),
        LIGHT(TOOL_TYPE_LIGHT,   100,      12), ITEM_END,

    /* Wearables */
    /*    ID                     short name  long name                       description          availability               quality          weight,cost,delay*/
    ITEM(ITEM_ID_LIGHT_FLAK,  "light flack", "a piece of light flak armour",  "",           ITEM_AVAILABILITY_AVERAGE, ITEM_QUALITY_AVERAGE,   300,    0,   5),
        /*     dr   locations                     special qualities*/
        ARMOUR(5,   INV_LOC_CHEST | INV_LOC_ARMS, 0),
        ITEM_END,

    /* Weapons */
    /* Creature Attacks */
    /*    ID                     short name  long name    description          availability               quality              weight,cost,delay*/
    ITEM(ITEM_ID_HUMAN_UNARMED,  "hands",    "hands",     "",                  0,                         0,                    0,    0,   0),
        /*                 CATEGORY             xd10  +X     dmg type              upgrades       special qualities             talent */
        CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,0,    -3,  WEAPON_DMG_TYPE_IMPACT, 0,         WEAPON_SPEC_QUALITY_UNARMED), 
        ITEM_END,

    /* Melee */

    /* Pistols */
    /*    ID                            short name         long name          description           availability            quality   weight,cost,delay*/
    ITEM(ITEM_ID_STUB_AUTOMATIC,"stub automatic","a stub automatic",STUB_AUTOMATIC_DESC,ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE, 15,    50,  1),
        /*          dmg type           xd10  +x range  single semi  auto  mag_sz  pen  upgrades  special   talent*/
        PISTOL_SP(WEAPON_DMG_TYPE_IMPACT,1, 3,  30,    1,    3,    0,    9,      0,   0, WEAPON_SPEC_QUALITY_NONE), 
        ITEM_END,

    ITEM(ITEM_ID_STUB_REVOLVER,"stub revolver",  "a stub revolver", STUB_REVOLVER_DESC, ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE, 15,    40,  2),
        PISTOL_SP(WEAPON_DMG_TYPE_IMPACT,1, 3,  30,    1,    0,    0,    6,      0,   0, WEAPON_SPEC_QUALITY_RELIABLE), 
        ITEM_END,

    ITEM(ITEM_ID_LAS_PISTOL,"las pistol",       "a las pistol",    LAS_PISTOL_DESC,    ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE, 17,    50,  1),
        PISTOL_LAS(WEAPON_DMG_TYPE_ENERGY,1, 2, 30,    1,    0,    0,   30,      0,   0, WEAPON_SPEC_QUALITY_RELIABLE), 
        ITEM_END,

    ITEM(ITEM_ID_LAS_GUN,"las gun",       "a las gun",    LAS_GUN_DESC,    ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE, 17,    50,  1),
        BASIC_LAS(WEAPON_DMG_TYPE_ENERGY,1, 2, 30,    1,    0,    0,   30,      0,   0, WEAPON_SPEC_QUALITY_RELIABLE), 
        ITEM_END,

    /* Ammo */
    /*    ID                    short name    long name           description    availability             quality           weight,cost,delay*/
    ITEM(ITEM_ID_PISTOL_AMMO_SP,  "solid pistol ammo","A clip of solid projectile pistol ammo","",    ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE,  0,     1,   0),
        /*      ammo Type      energy level*/
        AMMO(AMMO_TYPE_PISTOL_SP,     0), ITEM_END,

    ITEM(ITEM_ID_PISTOL_AMMO_LAS,  "pistol charge pack","A pistol charge pack","",    ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE,  0,     1,   0),
        AMMO(AMMO_TYPE_PISTOL_LAS,     1000), ITEM_END,

    ITEM(ITEM_ID_BASIC_AMMO_LAS,  "basic charge pack","A basic weapon charge pack","",    ITEM_AVAILABILITY_AVERAGE,ITEM_QUALITY_AVERAGE,  0,     1,   0),
        AMMO(AMMO_TYPE_BASIC_LAS,     1000), ITEM_END,
};

