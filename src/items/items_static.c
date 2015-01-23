/*
   This file is included in items.c.
   This is purely my sanity that it is seperate.
   TODO create a nice solution.
 */

#define ranged_desc \
    .you_use_desc = {"shoot",  "take aim",  "fire", NULL}, \
    .msr_use_desc = {"shoots", "takes aim", "fires", NULL}

#define melee_desc \
    .you_use_desc = {"slash",   "chop",  "bash",   "hack", "lung"}, \
    .msr_use_desc = {"slashes", "chops", "bashes", "hack", "lunges"}

#define creature_desc \
    .you_use_desc = {"bite",  "claw",  "rend",  "gnaw", NULL}, \
    .msr_use_desc = {"bites", "claws", "rends", "gnaws", NULL}

#define martial_desc \
    .you_use_desc = {"kick",  "punch", NULL}, \
    .msr_use_desc = {"kicks", "punches", NULL}

#define throw_desc \
    .you_use_desc = {"throw",  "lob",  "fling", NULL}, \
    .msr_use_desc = {"throws", "lobs", "flings", NULL}

#define ITEM(item_id,_sd_name,_ld_name,item_quality,item_weight,item_cost,delay) \
            [item_id]={.uid=0, .template_id=item_id, .quality=item_quality, .age=0, \
            .weight=item_weight, .cost=item_cost, .sd_name=cs_ITEM _sd_name cs_ITEM , .ld_name=cs_ITEM _ld_name cs_ITEM, \
            .icon_attr=TERM_COLOUR_SLATE, .use_delay=delay
#define ITEM_END }

#define CREATION(wght, minlvl, maxlvl) \
            .spawn_weight=wght, .spawn_min_level=minlvl, .spawn_max_level=maxlvl

#define FIXED_LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='(', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, .energy_left=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .dropable=false

#define LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='(', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, .energy_left=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .dropable=true

#define ARMOUR(_damage_reduction,_locations,_special_quality) \
            .icon=']', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEARABLE, \
            .specific.wearable={ .wearable_type=WEARABLE_TYPE_ARMOUR, .locations=_locations, \
            .special_quality=_special_quality, .damage_reduction=_damage_reduction,}, .dropable=true

#define MARTIAL(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, .convey_status_effect=SEID_NONE, }, \
            martial_desc

#define CREATURE_MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, .convey_status_effect=SEID_NONE, }, \
            creature_desc 

#define MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=1,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=wpn_cat, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true, melee_desc

#define THROWN_WEAPON(dmg_die,dmg_add,_penetration,_range,_dmg_type,_upgrades,special,talent) .icon='|',.stacked_quantity=0, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_WEAPON, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true

#define THROWN_GRENADE(dmg_die,dmg_add,_penetration,_range,_dmg_type,_upgrades,special,talent, cid) .icon='|',.stacked_quantity=0, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_GRENADE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=_upgrades, .wpn_talent=talent, .convey_status_effect=cid, }, .dropable=true

#define PISTOL_SP(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_SP, .ammo_used_template_id=IID_PISTOL_AMMO_SP, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TLT_PISTOL_WPN_TRNG_SP, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define PISTOL_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_PISTOL, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_PISTOL_LAS, .ammo_used_template_id=IID_PISTOL_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TLT_PISTOL_WPN_TRNG_LAS, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define BASIC_SP(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_BASIC, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_BASIC_SP, .ammo_used_template_id=IID_BASIC_AMMO_SP, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TLT_BASIC_WPN_TRNG_SP, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define BASIC_LAS(_dmg_type,dmg_die,dmg_add,_range,rof_single,rof_semi,rof_auto,mag_sz,_penetration,_upgrades,special) \
            .icon='|', .stacked_quantity=0, .max_quantity=1, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_BASIC, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=rof_single, .rof[WEAPON_ROF_SETTING_SEMI]=rof_semi, .rof[WEAPON_ROF_SETTING_AUTO]=rof_auto, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=mag_sz, .magazine_left=mag_sz, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_BASIC_LAS, .ammo_used_template_id=IID_BASIC_AMMO_LAS, .special_quality=special, .upgrades=_upgrades, \
            .wpn_talent=TLT_BASIC_WPN_TRNG_LAS, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define AMMO(_ammo_type,_energy, cid) .icon='^', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_AMMO, .specific.ammo={ .ammo_type=_ammo_type, .energy=_energy, .energy_left=_energy, .convey_status_effect=cid, }

#define STIMM(_food_type,cid) .icon='?', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_FOOD, .specific.food={ .food_type=_food_type, .nutrition=0, .nutrition_left=0, .convey_status_effect=cid, }

static const char *itm_descs[] = {
    [IID_NONE]              = "none",
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

    [IID_KNIFE]             = "a basic knife",

    [IID_FRAG_GRENADE]      = "",
    [IID_FIRE_BOMB]         = "",
    [IID_BODYPART_GRENADE]  = "used in status effects",
    [IID_THROWING_KNIFE]    = "",

    [IID_STUB_AUTOMATIC]    = "Just as common as the revolver variant, the stub automatic allows for a greater rate of fire and clip capacity, though at the cost of reliability",
    [IID_STUB_REVOLVER]     = "Based on an ancient and well-tested design, the stub revolver is the ideal backup weapon",
    [IID_LAS_PISTOL]        = "The las pistol is a light, compact and reliable weapon, common throughout the Imperium. Designs vary wildy and can range from elaborate heirloom "
                                    "devices inscribed with ornate carvings and gold filigree, to simplistic but brutaly robust weapons used by gangs and criminals",
    [IID_LAS_GUN]           = "Produced in a multitude of different styles and patterns, the lasgun can be found on almost every world of the Imperium, where its robust design "
                                    "and dependability make it a favoured weapon of both Emperor's faithfull and many of their foes",
    [IID_AUTO_GUN]          = "autogun",

    [IID_PISTOL_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_PISTOL_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",

    [IID_BASIC_AMMO_SP]    = "Hard rounds are common for many weapons within the Imperium and vary greatly in calibre and design",
    [IID_BASIC_AMMO_LAS]   = "Charge packs are powerful batteries used almost exclusively by las weapons",

    [IID_STIMM_HEALTH]     = "Health stimm injector",
    [IID_STIMM_DEATH]      = "Death stimm injector *debug*",
};

static struct itm_item static_item_list[] = {
    /* Tools */


    /* Lights */
    /*    ID              short name    long name       quality          weight,cost,delay             tool type         energy  luminem*/
    ITEM(IID_FIXED_LIGHT, "torch",      "a torch",      ITEM_QLTY_AVERAGE, 1,     1,   1), FIXED_LIGHT(TOOL_TYPE_LIGHT,  65000,   10),  ITEM_END,
    ITEM(IID_GLOW_GLOBE,  "glow globe", "a glow globe", ITEM_QLTY_AVERAGE, 1,     1,   1),       LIGHT(TOOL_TYPE_LIGHT,   1000,    10), CREATION(1,1,100), ITEM_END,

    /* Wearables */
    /*    ID                    short name           long name              quality           (wgt,cst,dly)      dr   locations                                       special qualities*/
    ITEM(IID_FLAK_HELMET,       "flak helmet",       "a flak helmet",       ITEM_QLTY_AVERAGE, 20, 25, 2), ARMOUR(2, INV_LOC_HEAD,                                              0), CREATION(6,1,100), ITEM_END,
    ITEM(IID_FLAK_GAUNTLETS,    "flak gauntlets",    "two flak gauntlets",  ITEM_QLTY_AVERAGE, 10, 50, 2), ARMOUR(2, INV_LOC_ARMS,                                              0), CREATION(5,1,100), ITEM_END,
    ITEM(IID_FLAK_LIGHT_COAT,   "light flak coat",   "a light flak coat",   ITEM_QLTY_AVERAGE, 40, 80, 4), ARMOUR(2, INV_LOC_ARMS | INV_LOC_BODY | INV_LOC_LEGS,                0), CREATION(4,1,100), ITEM_END,
    ITEM(IID_FLAK_VEST,         "flak vest",         "a flak vest",         ITEM_QLTY_AVERAGE, 50, 50, 3), ARMOUR(3, INV_LOC_BODY,                                              0), CREATION(4,1,100), ITEM_END,
    ITEM(IID_FLAK_JACKET,       "flak jacket",       "a flak jacket",       ITEM_QLTY_AVERAGE, 60, 100,4), ARMOUR(3, INV_LOC_ARMS | INV_LOC_BODY | INV_LOC_LEGS,                0), CREATION(4,1,100), ITEM_END,
    ITEM(IID_FLAK_CLOAK,        "flak cloak",        "a flak cloak",        ITEM_QLTY_AVERAGE, 80, 80, 3), ARMOUR(3, INV_LOC_BODY,                                              0), CREATION(4,1,100), ITEM_END,
    ITEM(IID_FLAK_GUARD_ARMOUR, "guard flak armour", "a guard flak armour", ITEM_QLTY_AVERAGE, 110,300,3), ARMOUR(4, INV_LOC_ARMS | INV_LOC_LEGS | INV_LOC_BODY | INV_LOC_HEAD, 0), CREATION(1,1,100), ITEM_END,

    /* Weapons */
    /* Creature Attacks */
    /*    ID                         short name long name  quality       (wgt,cst,dly)            CATEGORY               xd10  +X  dmg type     upgrades   special qualities*/
    ITEM(IID_HUMAN_UNARMED,          "hands",   "hands",   ITEM_QLTY_NONE, 0, 0, 0), MARTIAL(       WEAPON_CATEGORY_2H_MELEE,0, -3, DMG_TYPE_IMPACT,  0,  bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM(IID_CREATURE_BITE_UNTRAINED,"teeth",   "teeth",   ITEM_QLTY_NONE, 0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1, -3, DMG_TYPE_RENDING, 0,  bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM(IID_CREATURE_BITE_TRAINED,  "teeth",   "teeth",   ITEM_QLTY_NONE, 0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1,  0, DMG_TYPE_RENDING, 0,  0), ITEM_END,

    /* Melee */
    /*    ID      short name, long name  quality       (wgt,cst,dly)            CATEGORY          xd10 +X, dmg type        pen   upgrades   special qualities, talent*/
    ITEM(IID_KNIFE,"knife","a knife",ITEM_QLTY_AVERAGE, 15, 50, 0),MELEE(WEAPON_CATEGORY_1H_MELEE,0, 0,  DMG_TYPE_IMPACT, 0,    0,          0,                TLT_NONE), CREATION(20,1,100), ITEM_END,

    /* Thrown */
    /*    ID               short name         long name          quality       (wgt,cst,dly)  CATEGORY xd10,+X,pen,range   dmg type  upgrades   special qualities   talent    status_effect*/
    ITEM(IID_FRAG_GRENADE,"frag grenade", "a frag grenade", ITEM_QLTY_AVERAGE, 5, 10, 1), THROWN_GRENADE(2, 0, 0, 3,DMG_TYPE_EXPLOSIVE,0, bf(WPN_SPCQLTY_BLAST_4),  TLT_NONE, SEID_NONE), CREATION(30,1,100), ITEM_END,
    ITEM(IID_FIRE_BOMB,   "fire bomb",     "a fire bomb",   ITEM_QLTY_AVERAGE, 5, 10, 1), THROWN_GRENADE(1, 0, 0, 3,DMG_TYPE_EXPLOSIVE,0, bf(WPN_SPCQLTY_BLAST_3),  TLT_NONE, SEID_WEAPON_FLAME_AREA), CREATION(30,1,100), ITEM_END,
    ITEM(IID_THROWING_KNIFE,"throwing knife","a throwing knife",ITEM_QLTY_AVERAGE, 5, 5,1),THROWN_WEAPON(0, 0, 0, 5,DMG_TYPE_RENDING,  0, bf(WPN_SPCQLTY_PRIMITIVE),TLT_THROWN_WPN_TRNG_PRIMITIVE),CREATION(40,1,100), ITEM_END,

    /* Pistols */
    /*    ID                short name        long name         quality           (wgt,cst,dly)         dmg type      xd10 +x range  (S/X/X)  mag_sz pen upgrades  special*/
    ITEM(IID_STUB_AUTOMATIC,"stub automatic","a stub automatic",ITEM_QLTY_AVERAGE, 15, 50, 1), PISTOL_SP(DMG_TYPE_IMPACT, 1, 3, 30,    1,3,0,  9,     0,  0,        0                     ), CREATION(10,1,100), ITEM_END,
    ITEM(IID_STUB_REVOLVER, "stub revolver", "a stub revolver", ITEM_QLTY_AVERAGE, 15, 40, 2), PISTOL_SP(DMG_TYPE_IMPACT, 1, 3, 30,    1,0,0,  6,     0,  0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(10,1,100), ITEM_END,
    ITEM(IID_LAS_PISTOL,    "las pistol",    "a las pistol",    ITEM_QLTY_AVERAGE, 17, 50, 1), PISTOL_LAS(DMG_TYPE_ENERGY,1, 2, 30,    1,0,0,  30,    0,  0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(10,1,100), ITEM_END,


    /* Basic weapons */
    /*    ID           short name  long name     quality           (wgt,cst,dly)          dmg type      xd10 +x range (S/X/X) mag_sz pen  upgrades  special*/
    ITEM(IID_LAS_GUN,  "las gun",  "a las gun",  ITEM_QLTY_AVERAGE, 40, 50, 1), BASIC_LAS(DMG_TYPE_ENERGY, 1, 3, 100,   1,3,0, 60,    0,   0,     bf(WPN_SPCQLTY_RELIABLE) ), CREATION(2,1,100), ITEM_END,
    ITEM(IID_AUTO_GUN, "autogun",  "an autogun", ITEM_QLTY_AVERAGE, 40, 50, 1), BASIC_SP(DMG_TYPE_IMPACT,  1, 3, 100,   1,5,10, 20,    0,   0,     0),                         CREATION(2,1,100), ITEM_END,

    /* Ammo */
    /*    ID                 short name            long name                               quality           (wgt,cst,dly)         ammo Type         energy level*/
    ITEM(IID_PISTOL_AMMO_SP, "solid pistol ammo", "a clip of solid projectile pistol ammo",ITEM_QLTY_AVERAGE,  0, 1,  0), AMMO(AMMO_TYPE_PISTOL_SP,  0   , SEID_NONE), CREATION(60,1,100), ITEM_END,
    ITEM(IID_PISTOL_AMMO_LAS,"pistol charge pack","a pistol charge pack",                  ITEM_QLTY_AVERAGE,  0, 1,  0), AMMO(AMMO_TYPE_PISTOL_LAS, 1000, SEID_NONE), CREATION(60,1,100), ITEM_END,

    ITEM(IID_BASIC_AMMO_SP,  "solid basic ammo", "a clip of solid projectile basic ammo",  ITEM_QLTY_AVERAGE,  0, 1,  0), AMMO(AMMO_TYPE_BASIC_SP,   0   , SEID_NONE), CREATION(40,1,100), ITEM_END,
    ITEM(IID_BASIC_AMMO_LAS, "basic charge pack", "a basic weapon charge pack",            ITEM_QLTY_AVERAGE,  0, 1,  0), AMMO(AMMO_TYPE_BASIC_LAS,  1000, SEID_NONE), CREATION(40,1,100), ITEM_END,

    /* Stimms */
    /*    ID                  short name            long name                         quality           (wgt,cst,dly)      Food Type            status_effect*/
    ITEM(IID_STIMM_HEALTH,  "health stimm", "an injector with a regenerative liquid", ITEM_QLTY_AVERAGE,   0, 1, 1), STIMM(FOOD_TYPE_INJECTION, SEID_HEALTH_STIMM), CREATION(10,1,100), ITEM_END,

    /* status effect items */
    ITEM(IID_BODYPART_GRENADE,"","status effect",ITEM_QLTY_AVERAGE,5,10,1),THROWN_GRENADE(1,0,0,3,DMG_TYPE_EXPLOSIVE, 0, bf(WPN_SPCQLTY_BLAST_2),TLT_NONE,SEID_NONE),ITEM_END,
    /* debug items */
    ITEM(IID_STIMM_DEATH,   "death debug",  "an injector with a deadly liquid", ITEM_QLTY_AVERAGE, 0, 1, 1), STIMM(FOOD_TYPE_INJECTION, SEID_DEATH_STIMM), ITEM_END,
};

static const char *item_quality_strings[] = {
    [ITEM_QLTY_POOR]    =  "poor",
    [ITEM_QLTY_AVERAGE] =  "average",
    [ITEM_QLTY_GOOD]    =  "good",
    [ITEM_QLTY_BEST]    =  "best",
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


static const char *wbl_spcqlty_name[] = {
    [WBL_SPCQLTY_PRIMITIVE]	    = "primitive",
};

static const char *wbl_spcqlty_desc[] = {
    [WBL_SPCQLTY_PRIMITIVE]	    = "Halves AP unless weapon is also primitive",
};

