/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
   This file is included in items.c.
   This is purely my sanity that it is seperate.
   TODO create a nice solution.


Glyphs:
% is for food
) is for weapons
[ is for armor
} is for evokables
" is for amulets
= is for rings
| is for staves and wands
! is for potions
? is for scrolls
+ is for books
^ is for traps
' is for ammunition
$ is for money
& is for mushrooms
*/

/* TODO: create general arrays for this, which should also be used for tools, food and wearables and for improvised weapons. */
#define ranged_desc \
    .you_use_desc = {"shoot",  "take aim",  "fire", NULL, }, \
    .msr_use_desc = {"shoots", "takes aim", "fires", NULL, }

#define melee_desc \
    .you_use_desc = {"slash",   "chop",  "bash",   "hack", "lung"}, \
    .msr_use_desc = {"slashes", "chops", "bashes", "hacks", "lunges"}

#define creature_desc \
    .you_use_desc = {"bite",  "claw",  "rend",  "gnaw", NULL, }, \
    .msr_use_desc = {"bites", "claws", "rends", "gnaws", NULL, }

#define martial_desc \
    .you_use_desc = {"kick",  "punch", "slam", NULL, }, \
    .msr_use_desc = {"kicks", "punches", "slams", NULL, }

#define throw_desc \
    .you_use_desc = {"throw",  "lob",  "fling", "toss", NULL, }, \
    .msr_use_desc = {"throws", "lobs", "flings", "tosses", NULL, }

#define ITEM(item_id,_sd_name,_ld_name,item_quality,item_weight,item_cost,delay) \
            [item_id]={.uid=0, .tid=item_id, .quality=item_quality, .age=0, \
            .weight=item_weight, .cost=item_cost, .sd_name=cs_ITEM _sd_name cs_CLOSE , .ld_name=cs_ITEM _ld_name cs_CLOSE, \
            .icon_attr=TERM_COLOUR_SLATE, .use_delay=delay, .identified=true, .identify_last_try = 0
#define ITEM_END }

#define ITEM_NONE(item_id,_sd_name,_ld_name,item_weight,item_cost,delay) \
                ITEM(item_id,_sd_name,_ld_name,ITEM_QLTY_NONE,item_weight,item_cost,delay)
#define ITEM_POOR(item_id,_sd_name,_ld_name,item_weight,item_cost,delay) \
                ITEM(item_id,_sd_name,_ld_name,ITEM_QLTY_POOR,item_weight,item_cost,delay)
#define ITEM_AVG(item_id,_sd_name,_ld_name,item_weight,item_cost,delay) \
                ITEM(item_id,_sd_name,_ld_name,ITEM_QLTY_AVERAGE,item_weight,item_cost,delay)
#define ITEM_GOOD(item_id,_sd_name,_ld_name,item_weight,item_cost,delay) \
                ITEM(item_id,_sd_name,_ld_name,ITEM_QLTY_GOOD,item_weight,item_cost,delay)
#define ITEM_BEST(item_id,_sd_name,_ld_name,item_weight,item_cost,delay) \
                ITEM(item_id,_sd_name,_ld_name,ITEM_QLTY_BEST,item_weight,item_cost,delay)

#define CREATION(wght, lvl) \
            .spawn_weight=wght, .spawn_level=lvl

#define FIXED_LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='0', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .permanent_energy=true, .dropable=false

#define LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='0', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .dropable=true

#define MONEY() \
            .icon='$', .stacked_quantity=1, .max_quantity=UINT32_MAX -2, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=TOOL_TYPE_MONEY, .energy=0, \
            .light_luminem=0, .lit=false,}, .dropable=true

#define ARMOUR(_damage_reduction,_locations,_special_quality) \
            .icon='[', .stacked_quantity=0, .max_quantity=0, .item_type=ITEM_TYPE_WEARABLE, \
            .specific.wearable={ .wearable_type=WEARABLE_TYPE_ARMOUR, .locations=_locations, \
            .special_quality=_special_quality, .damage_reduction=_damage_reduction,}, .dropable=true

#define MARTIAL(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, \
            .convey_status_effect=SEID_NONE, }, martial_desc

#define CREATURE_MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, \
            .convey_status_effect=SEID_NONE, }, creature_desc

#define MELEE_1H(dmg_die,dmg_add,_dmg_type,_penetration,special,talent) .icon=')',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=WEAPON_CATEGORY_1H_MELEE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true, melee_desc

#define MELEE_2H(dmg_die,dmg_add,_dmg_type,_penetration,special,talent) .icon=')',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=WEAPON_CATEGORY_2H_MELEE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true, melee_desc

#define LOW_TECH(_dmg_type,dmg_die,dmg_add,_penetration,_range,special) \
            .icon='}', .stacked_quantity=0, .max_quantity=0, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_type=WEAPON_TYPE_RANGED,.weapon_category=WEAPON_CATEGORY_2H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, \
            .dmg_addition=dmg_add, .range=_range, .rof[WEAPON_ROF_SETTING_SINGLE]=1, .rof[WEAPON_ROF_SETTING_SEMI]=0, .rof[WEAPON_ROF_SETTING_AUTO]=0, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=1, .magazine_left=1, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_ARROW, .ammo_used_tid=IID_ARROW, .special_quality=bf(WPN_SPCQLTY_PRIMITIVE) | special, .upgrades=0, \
            .wpn_talent=TLT_1_WEAPON_TRAINING_LOW_TECH, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define PISTOL_SP(_dmg_type,dmg_die,dmg_add,_penetration,_range,rof_single,rof_semi,rof_auto,mag_sz,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_type=WEAPON_TYPE_RANGED,.weapon_category=WEAPON_CATEGORY_1H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, \
            .dmg_addition=dmg_add, .range=_range, .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, \
            .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, \
            .penetration=_penetration, .ammo_type=AMMO_TYPE_PISTOL_SP, .ammo_used_tid=IID_PISTOL_AMMO_SP, \
            .special_quality=bf(WPN_SPCQLTY_JAMS) | special, .upgrades=_upgrades, .wpn_talent=TLT_1_WEAPON_TRAINING_SP, .jammed=false, \
            .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define PISTOL_LAS(_dmg_type,dmg_die,dmg_add,_penetration,_range,rof_single,rof_semi,rof_auto,mag_sz,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_type=WEAPON_TYPE_RANGED,.weapon_category=WEAPON_CATEGORY_1H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, \
            .dmg_addition=dmg_add, .range=_range, .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, \
            .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, \
            .penetration=_penetration, .ammo_type=AMMO_TYPE_PISTOL_LAS, .ammo_used_tid=IID_PISTOL_AMMO_LAS, \
            .special_quality=bf(WPN_SPCQLTY_JAMS) | special, .upgrades=_upgrades, .wpn_talent=TLT_1_WEAPON_TRAINING_LAS, .jammed=false, \
            .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define BASIC_SP(_dmg_type,dmg_die,dmg_add,_penetration,_range,rof_single,rof_semi,rof_auto,mag_sz,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_type=WEAPON_TYPE_RANGED,.weapon_category=WEAPON_CATEGORY_2H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, \
            .dmg_addition=dmg_add, .range=_range, .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, \
            .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, \
            .penetration=_penetration, .ammo_type=AMMO_TYPE_BASIC_SP, .ammo_used_tid=IID_BASIC_AMMO_SP, \
            .special_quality=bf(WPN_SPCQLTY_JAMS) | special, .upgrades=_upgrades, .wpn_talent=TLT_1_WEAPON_TRAINING_SP, .jammed=false, \
            .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define BASIC_LAS(_dmg_type,dmg_die,dmg_add,_penetration,_range,rof_single,rof_semi,rof_auto,mag_sz,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_type=WEAPON_TYPE_RANGED,.weapon_category=WEAPON_CATEGORY_2H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, \
            .dmg_addition=dmg_add, .range=_range, .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, \
            .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, \
            .penetration=_penetration, .ammo_type=AMMO_TYPE_BASIC_LAS, .ammo_used_tid=IID_BASIC_AMMO_LAS, \
            .special_quality=bf(WPN_SPCQLTY_JAMS) | special, .upgrades=_upgrades, .wpn_talent=TLT_1_WEAPON_TRAINING_LAS, .jammed=false, \
            .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define THROWN_WEAPON(_dmg_type,dmg_die,dmg_add,_penetration,_range,_upgrade,special,talent) .icon='}',.stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_WEAPON, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrade, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, throw_desc, .dropable=true

#define THROWN_GRENADE(_dmg_type,dmg_die,dmg_add,_penetration,_range,special,talent, cid) .icon='}',.stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_GRENADE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=cid, }, throw_desc, .dropable=true


#define AMMO(_ammo_type,_energy, cid) .icon='^', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_AMMO, .specific.ammo={ .ammo_type=_ammo_type, .energy=_energy, .energy_left=_energy, .convey_status_effect=cid, }

#define STIMM(_food_type,sips,cid) .icon='?', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_FOOD, .specific.food={ .food_type=_food_type, .nutrition=sips, .nutrition_left=sips, .convey_status_effect=cid, }

#define MUSHROOM(cid) .icon='&', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_FOOD, .specific.food={ .food_type=FOOD_TYPE_SOLID, .nutrition=0, .nutrition_left=0, .convey_status_effect=cid, }

static const char *itm_descs[] = {
    [IID_NONE]              = "none",
    [IID_MONEY]             = "A bunch of coins in a supple leather bag",
    [IID_FIXED_LIGHT]       = "",
    [IID_GLOW_GLOBE]        = "This a generic glow globe",

    /* Armour */
    [IID_FLAK_HELMET]       = "This a generic flak helmet",
    [IID_FLAK_GAUNTLETS]    = "These are generic flak gauntlets",
    [IID_FLAK_LIGHT_COAT]   = "These are generic light flak coat",
    [IID_FLAK_VEST]         = "These are generic flak vest",
    [IID_FLAK_JACKET]       = "These are generic flak jacket",
    [IID_FLAK_CLOAK]        = "These are generic flak cloak",
    [IID_FLAK_GUARD_ARMOUR] = "These are generic guard flak armour",

    /* Melee */
    [IID_KNIFE]             = "a basic knife",
    [IID_SWORD]             = "a basic sword",
    [IID_2H_SWORD]          = "a basic 2-handed sword",

    [IID_SHORT_BOW]         = "a basic short bow",
    [IID_LONG_BOW]          = "a basic long bow",

    /* Throwing */
    [IID_FRAG_GRENADE]      = "",
    [IID_FIRE_BOMB]         = "",
    [IID_THROWING_KNIFE]    = "",

    /* Ranged */
    [IID_STUB_AUTOMATIC]    = "Just as common as the revolver variant, the stub automatic allows for a greater rate of fire and clip capacity, though at the cost of reliability",
    [IID_STUB_REVOLVER]     = "Based on an ancient and well-tested design, the stub revolver is the ideal backup weapon",
    [IID_LAS_PISTOL]        = "The las pistol is a light, compact and reliable weapon, common throughout the Imperium. Designs vary wildy and can range from elaborate heirloom "
                                    "devices inscribed with ornate carvings and gold filigree, to simplistic but brutaly robust weapons used by gangs and criminals",
    [IID_LAS_GUN]           = "Produced in a multitude of different styles and patterns, the lasgun can be found on almost every world of the Imperium, where its robust design "
                                    "and dependability make it a favoured weapon of both Emperor's faithfull and many of their foes",
    [IID_AUTO_GUN]          = "autogun",

    [IID_ARROW]             = "arrow",

    [IID_PISTOL_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_PISTOL_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",

    [IID_BASIC_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_BASIC_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",

    /* Stimms */
    [IID_STIMM_HEALTH_MINOR]  = "Health stimm injector",

    /* Environment */
    [IID_MUSHROOM_MAD_CAP] = "A mad cap mushroom",

    /* Debug and internal items */
    [IID_HUMAN_UNARMED]           = "error",
    [IID_CREATURE_BITE_UNTRAINED] = "error",
    [IID_CREATURE_BITE_TRAINED]   = "error",
    [IID_BODYPART_GRENADE]        = "error",
    [IID_STIMM_DEATH]             = "Death stimm injector *debug*",
};

static struct itm_item static_item_list[] = {
    /* Money */
    /*    ID        short name    long name             weight,cost,delay  tool type  */
    ITEM_AVG(IID_MONEY, "coins",      "a bunch of coins", 0,     1,   1),  MONEY(),  ITEM_END,

    /* Tools */

    /* Lights */
    /*    ID              short name        long name    weight,cost,delay              tool type         energy  luminem*/
    ITEM_AVG(IID_FIXED_LIGHT, "glow-globe","a glow-globe",  1,     1,   1), FIXED_LIGHT(TOOL_TYPE_LIGHT,      1,    8),  ITEM_END,
    ITEM_AVG(IID_GLOW_GLOBE,  "glow-globe","a glow-globe",  1,     1,   1),       LIGHT(TOOL_TYPE_LIGHT,    100,    6), CREATION(1,1), ITEM_END,

    /* Wearables */
    /*    ID                        short name           long name              (wgt,cst,dly)      dr   locations                                                 special qualities*/
    ITEM_AVG(IID_FLAK_HELMET,       "flak helmet",       "a flak helmet",        20, 25, 2), ARMOUR(2, INV_LOC_HEAD,                                              bf(WBL_SPCQLTY_FLAK) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_FLAK_GAUNTLETS,    "flak gauntlets",    "two flak gauntlets",   10, 50, 2), ARMOUR(2, INV_LOC_ARMS,                                              bf(WBL_SPCQLTY_FLAK) ), CREATION(5,1), ITEM_END,
    ITEM_AVG(IID_FLAK_LIGHT_COAT,   "light flak coat",   "a light flak coat",    40, 80, 4), ARMOUR(2, INV_LOC_ARMS | INV_LOC_BODY | INV_LOC_LEGS,                bf(WBL_SPCQLTY_FLAK) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_FLAK_VEST,         "flak vest",         "a flak vest",          50, 50, 3), ARMOUR(3, INV_LOC_BODY,                                              bf(WBL_SPCQLTY_FLAK) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_FLAK_JACKET,       "flak jacket",       "a flak jacket",        60, 100,4), ARMOUR(3, INV_LOC_ARMS | INV_LOC_BODY | INV_LOC_LEGS,                bf(WBL_SPCQLTY_FLAK) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_FLAK_CLOAK,        "flak cloak",        "a flak cloak",         80, 80, 3), ARMOUR(3, INV_LOC_BODY,                                              bf(WBL_SPCQLTY_FLAK) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_FLAK_GUARD_ARMOUR, "guard flak armour", "a guard flak armour",  110,300,3), ARMOUR(4, INV_LOC_ARMS | INV_LOC_LEGS | INV_LOC_BODY | INV_LOC_HEAD, bf(WBL_SPCQLTY_FLAK) ), CREATION(1,1), ITEM_END,

    /* Melee */
    /*    ID                 short name,   long name              (wgt,cst,dly)      xd10 +X, dmg type         pen, special qualities,         talent*/
    ITEM_AVG(IID_KNIFE,       "knife",     "a knife",               5,  5, 0),MELEE_1H(1,-4,  DMG_TYPE_IMPACT,0,  bf(WPN_SPCQLTY_LIGHT),    TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_AVG(IID_SWORD,       "sword",     "a sword",              20, 10, 0),MELEE_1H(1, 0,  DMG_TYPE_IMPACT,0,  0,                        TLT_NONE), CREATION(20,1), ITEM_END,

    /*    ID                short name,         long name           (wgt,cst,dly)      xd10 +X, dmg type         pen, special qualities, talent*/
    ITEM_POOR(IID_2H_SWORD, "two-handed sword", "a two-handed sword", 30, 10, 0),MELEE_2H(1, 2, DMG_TYPE_IMPACT, 0,   0,                 TLT_NONE), CREATION(40,1), ITEM_END,

    /* Ranged */
    /*    ID                    short name       long name        (wgt,cst,dly)              dmg type    xd10 +x pen range special qualities  */
    ITEM_AVG(IID_SHORT_BOW,     "short bow",     "a short bow",     15, 7, 0.5),LOW_TECH(DMG_TYPE_IMPACT,1, 3, 0,  8,          0           ), CREATION(10,1), ITEM_END,
    ITEM_AVG(IID_LONG_BOW,      "long bow",      "a long bow",      20,15, 0.5),LOW_TECH(DMG_TYPE_IMPACT,1, 3, 1, 16,          0           ), CREATION(10,1), ITEM_END,

    /*    ID                    short name       long name        (wgt,cst,dly)            dmg type      xd10 +x pen range S/X/X) mag_sz  upgrades special qualities */
    ITEM_AVG(IID_STUB_AUTOMATIC,"stub automatic","a stub automatic",15, 50, 1), PISTOL_SP(DMG_TYPE_IMPACT,  1, 3, 0, 30,   1,3,0,  9,     0,            0                 ), CREATION(10,1), ITEM_END,
    ITEM_AVG(IID_STUB_REVOLVER, "stub revolver", "a stub revolver", 15, 40, 2), PISTOL_SP(DMG_TYPE_IMPACT,  1, 3, 0, 30,   1,0,0,  6,     0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(10,1), ITEM_END,
    ITEM_AVG(IID_LAS_PISTOL,    "las pistol",    "a las pistol",    17, 50, 1), PISTOL_LAS(DMG_TYPE_ENERGY, 1, 2, 0, 30,   1,0,0,  30,    0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(10,1), ITEM_END,


    /* Basic weapons */
    /*    ID               short name  long name   (wgt,cst,dly)           dmg type      xd10 +x pen range (S/X/X)   mag_sz upgrades  special */
    ITEM_AVG(IID_LAS_GUN,  "las gun",  "a las gun",  40, 50, 1), BASIC_LAS(DMG_TYPE_ENERGY, 1, 3, 0,  100,   1,3,0,   60,    0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(2,1), ITEM_END,
    ITEM_AVG(IID_AUTO_GUN, "autogun",  "an autogun", 40, 50, 1), BASIC_SP(DMG_TYPE_IMPACT,  1, 3, 0,  100,   1,5,10,  20,    0,     0),                         CREATION(2,1), ITEM_END,

    /* Ammo */
    /*    ID                 short name            long name                                 (wgt,cst,dly)         ammo Type         energy level*/
    ITEM_AVG(IID_PISTOL_AMMO_SP, "solid pistol ammo", "a clip of solid projectile pistol ammo",1, 1,  0), AMMO(AMMO_TYPE_PISTOL_SP,  0   , SEID_NONE), CREATION(60,1), ITEM_END,
    ITEM_AVG(IID_PISTOL_AMMO_LAS,"pistol charge pack","a pistol charge pack",                  2, 1,  0), AMMO(AMMO_TYPE_PISTOL_LAS, 1000, SEID_NONE), CREATION(60,1), ITEM_END,

    ITEM_AVG(IID_BASIC_AMMO_SP,  "solid basic ammo",  "a clip of solid projectile basic ammo", 1, 1,  0), AMMO(AMMO_TYPE_BASIC_SP,   0   , SEID_NONE), CREATION(40,1), ITEM_END,
    ITEM_AVG(IID_BASIC_AMMO_LAS, "basic charge pack", "a basic weapon charge pack",            4, 1,  0), AMMO(AMMO_TYPE_BASIC_LAS,  1000, SEID_NONE), CREATION(40,1), ITEM_END,
    ITEM_AVG(IID_ARROW,          "arrows",            "arrows",                                1, 1,  1), AMMO(AMMO_TYPE_ARROW,      0,    SEID_NONE), CREATION(10,1), ITEM_END,

    /* Thrown */
    /*    ID                short name         long name          (wgt,cst,dly)  CATEGORY     dmg_type         xd10,+X,pen,range,special qualities         talent    status_effect*/
    ITEM_AVG(IID_FRAG_GRENADE,  "frag grenade",  "a frag grenade",  5, 10, 1), THROWN_GRENADE(DMG_TYPE_EXPLOSIVE,2,  0, 0,  3,   bf(WPN_SPCQLTY_BLAST_4),  TLT_NONE,   SEID_NONE),         CREATION(30,1), ITEM_END,
    ITEM_AVG(IID_FIRE_BOMB,     "fire bomb",     "a fire bomb",     5, 10, 1), THROWN_GRENADE(DMG_TYPE_EXPLOSIVE,1,  0, 0,  3,   bf(WPN_SPCQLTY_BLAST_3),  TLT_NONE,   SEID_WEAPON_FLAME), CREATION(30,1), ITEM_END,

    /*    ID                short name         long name          (wgt,cst,dly)  CATEGORY     dmg_type         xd10,+X,pen,range,upgrades   special qualities       talent  */
    ITEM_AVG(IID_THROWING_KNIFE,"throwing knife","a throwing knife",5,  5, 1), THROWN_WEAPON(DMG_TYPE_RENDING,   0,  0, 0,  5,     0,    bf(WPN_SPCQLTY_PRIMITIVE),TLT_1_WEAPON_TRAINING_LOW_TECH),      CREATION(40,1), ITEM_END,

    /* Potions */
    /*    ID                                short name        long name              (wgt,cst,dly)            status_effect   sips*/
    ITEM_POOR(IID_STIMM_HEALTH_MINOR,     "health stimm",    "an injector with a regenerative liquid", 0, 1, 1), STIMM(FOOD_TYPE_INJECTION, 3, SEID_MINOR_HEALING), CREATION(10,1), ITEM_END,

    /* Mushrooms */
    /*    ID                                short name        long name       (wgt,cst,dly)           status_effect*/
    ITEM_AVG(IID_MUSHROOM_MAD_CAP,  "mad cap mushroom", "a black cap mushroom", 0, 1, 1), MUSHROOM(SEID_MAD_CAP), ITEM_END,

    /* Creature Attacks */
    /*    ID                            hort name long name  (wgt,cst,dly)            CATEGORY               xd10  +X  dmg type     upgrades   special qualities*/
    ITEM_NONE(IID_HUMAN_UNARMED,          "hands",   "hands",   0, 0, 0), MARTIAL(       WEAPON_CATEGORY_2H_MELEE,1, -4, DMG_TYPE_IMPACT,  0,  bf(WPN_SPCQLTY_UNARMED) | bf(WPN_SPCQLTY_PRIMITIVE) ), ITEM_END,
    ITEM_NONE(IID_CREATURE_BITE_UNTRAINED,"teeth",   "teeth",   0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1, -4, DMG_TYPE_IMPACT,  0,  bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM_NONE(IID_CREATURE_BITE_TRAINED,  "teeth",   "teeth",   0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1,  0, DMG_TYPE_IMPACT,  0,  0), ITEM_END,

    /* status effect items */
    ITEM(IID_BODYPART_GRENADE,"critical","",ITEM_QLTY_AVERAGE,5,10,1),THROWN_GRENADE(DMG_TYPE_EXPLOSIVE,1,0,0,3,bf(WPN_SPCQLTY_BLAST_2),TLT_NONE,SEID_NONE),ITEM_END,

    /* debug items */
    ITEM(IID_STIMM_DEATH,   "death debug",  "an injector with a deadly liquid", ITEM_QLTY_AVERAGE, 0, 1, 1), STIMM(FOOD_TYPE_INJECTION, 1, SEID_BLUNT_RARM_4), ITEM_END,
};

static const char *item_quality_strings[] = {
    [ITEM_QLTY_POOR]    =  "poor",
    [ITEM_QLTY_AVERAGE] =  "average",
    [ITEM_QLTY_GOOD]    =  "good",
    [ITEM_QLTY_BEST]    =  "best",
};

static const int item_food_quality_side_effect_chance[] = {
    [ITEM_QLTY_POOR]    =  40,
    [ITEM_QLTY_AVERAGE] =  20,
    [ITEM_QLTY_GOOD]    =  10,
    [ITEM_QLTY_BEST]    =  5,
};

static const char *ammo_type_strings[] = {
    [AMMO_TYPE_NONE]            =  "none",

    [AMMO_TYPE_ARROW]           =  "arrows",

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
    /*[WPN_SPCQLTY_ACCURATE]      = "Accurate",*/
    [WPN_SPCQLTY_BLAST_1]       = "Blast 1",
    [WPN_SPCQLTY_BLAST_2]       = "Blast 2",
    [WPN_SPCQLTY_BLAST_3]       = "Blast 3",
    [WPN_SPCQLTY_BLAST_4]       = "Blast 4",
    [WPN_SPCQLTY_DEFENSIVE]     = "Defensive",
    [WPN_SPCQLTY_FAST]          = "Fast",
    /*[WPN_SPCQLTY_FLAME]         = "Flame",*/
    [WPN_SPCQLTY_FLEXIBLE]      = "Flexible",
    /*[WPN_SPCQLTY_INACCURATE]    = "Inaccurate",*/
    [WPN_SPCQLTY_JAMS]          = "Jams",
    [WPN_SPCQLTY_LIGHT]         = "Light",
    /*[WPN_SPCQLTY_OVERHEATS]     = "Overheats",*/
    [WPN_SPCQLTY_PRIMITIVE]     = "Primitive",
    [WPN_SPCQLTY_RELIABLE]      = "Reliable",
    /*[WPN_SPCQLTY_SCATTER]       = "Scatter",*/
    /*[WPN_SPCQLTY_SHOCKING]      = "Shocking",*/
    [WPN_SPCQLTY_SHIELD]        = "Shield",
    [WPN_SPCQLTY_SLOW]          = "Slow",
    /*[WPN_SPCQLTY_SMOKE]         = "Smoke",*/
    [WPN_SPCQLTY_TEARING]       = "Tearing",
    /*[WPN_SPCQLTY_TOXIC]         = "Toxic",*/
    [WPN_SPCQLTY_UNARMED]       = "Unarmed",
    [WPN_SPCQLTY_UNBALANCED]    = "Unbalanced",
    [WPN_SPCQLTY_UNRELIABLE]    = "Unreliable",
    /*[WPN_SPCQLTY_UNSTABLE]      = "Unstable",*/
    [WPN_SPCQLTY_UNWIELDY]      = "Unwieldy",
};

static const char *wpn_spcqlty_desc[] = {
    /*[WPN_SPCQLTY_ACCURATE]      = "Additional +10 to hit when used with the aim action ",*/
    [WPN_SPCQLTY_BLAST_1]       = "All within 1 squares is hit",
    [WPN_SPCQLTY_BLAST_2]       = "All within 2 squares is hit",
    [WPN_SPCQLTY_BLAST_3]       = "All within 3 squares is hit",
    [WPN_SPCQLTY_BLAST_4]       = "All within 4 squares is hit",
    [WPN_SPCQLTY_DEFENSIVE]     = "Easier to parry with: +5 Parry ",
    [WPN_SPCQLTY_FAST]          = "-10% Harder to dodge or parry against",
    /*[WPN_SPCQLTY_FLAME]         = "No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire.",*/
    [WPN_SPCQLTY_FLEXIBLE]      = "Cannot be Parried",
    /*[WPN_SPCQLTY_INACCURATE]    = "No bonus with aim action",*/
    [WPN_SPCQLTY_JAMS]          = "Jams on a to-hit roll of 95+",
    [WPN_SPCQLTY_LIGHT]         = "This light weapon is easier to handle in your off-hand",
    /*[WPN_SPCQLTY_OVERHEATS]     = "90+ -> Overheat",*/
    [WPN_SPCQLTY_PRIMITIVE]     = "AP doubled, unless armour is also primitive",
    [WPN_SPCQLTY_RELIABLE]      = "Jams only on  a to-hit roll of 100",
    /*[WPN_SPCQLTY_SCATTER]       = "Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges",*/
    /*[WPN_SPCQLTY_SHOCKING]      = "If damage, test Toughness or be stunned",*/
    [WPN_SPCQLTY_SHIELD]        = "Gives ranged attackers a -10% to hit",
    [WPN_SPCQLTY_SLOW]          = "Defenders can parry this 5% weapon easier",
    /* [WPN_SPCQLTY_SMOKE]         = "Creates smoke screen 3d10 metres in diameters, lasts 2d10 Rounds", */
    [WPN_SPCQLTY_TEARING]       = "Roll an extra damage die, take highest ",
    /* [WPN_SPCQLTY_TOXIC]         = "if damage, teest toughness -5 for every damage taken. if failed, take 1d10 extra impact damage.", */
    [WPN_SPCQLTY_UNARMED]       = "1d5-3 +Str, Armour doubles",
    [WPN_SPCQLTY_UNBALANCED]    = "-10 when parry",
    [WPN_SPCQLTY_UNRELIABLE]    = "jams on 90+",
    /* [WPN_SPCQLTY_UNSTABLE]      = "on hit, roll 1d10: 1: half dmg, 2-9: normal, 10: double damage", */
    [WPN_SPCQLTY_UNWIELDY]      = "Cannot be used to parry with",
};


static const char *wbl_spcqlty_name[] = {
    [WBL_SPCQLTY_PRIMITIVE] = "Primitive",
    [WBL_SPCQLTY_FLAK]      = "Flak",
    [WBL_SPCQLTY_MAX]       = "ERROR",
};

static const char *wbl_spcqlty_desc[] = {
    [WBL_SPCQLTY_PRIMITIVE] = "Halfs armour bonus against non-primitive weapons",
    [WBL_SPCQLTY_FLAK]      = "Basic armour which give no penalties",
    [WBL_SPCQLTY_MAX]       = "ERROR",
};

struct item_food_side_effect_struct {
    int weight;
    enum se_ids side_effect_id;
};

static const struct item_food_side_effect_struct item_food_side_effects[] = {
    {
        .weight = 3,
        .side_effect_id = SEID_MAD_CAP,
    },
    {
        .weight = 1,
        .side_effect_id = SEID_MINOR_HEALING,
    },
};

