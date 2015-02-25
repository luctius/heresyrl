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
            [item_id]={.uid=0, .template_id=item_id, .quality=item_quality, .age=0, \
            .weight=item_weight, .cost=item_cost, .sd_name=cs_ITEM _sd_name cs_CLOSE , .ld_name=cs_ITEM _ld_name cs_CLOSE, \
            .icon_attr=TERM_COLOUR_SLATE, .use_delay=delay
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
            .icon='/', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, .energy_left=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .permanent_energy=true, .dropable=false

#define LIGHT(_tool_type,_energy,_light_luminem) \
            .icon='/', .stacked_quantity=1, .max_quantity=100, .item_type=ITEM_TYPE_TOOL, \
            .specific.tool={ .tool_type=_tool_type, .energy=_energy, .energy_left=_energy, \
            .light_luminem=_light_luminem, .lit=false,}, .dropable=true

#define ARMOUR(_damage_reduction,_locations,_special_quality) \
            .icon='[', .stacked_quantity=0, .max_quantity=0, .item_type=ITEM_TYPE_WEARABLE, \
            .specific.wearable={ .wearable_type=WEARABLE_TYPE_ARMOUR, .locations=_locations, \
            .special_quality=_special_quality, .damage_reduction=_damage_reduction,}, .dropable=true

#define MARTIAL(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, .convey_status_effect=SEID_NONE, }, \
            martial_desc

#define CREATURE_MELEE(wpn_cat,dmg_die,dmg_add,_dmg_type,_penetration,special) .icon=' ',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .dropable=false, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE,\
            .weapon_category=wpn_cat, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0,\
            .penetration=_penetration, .special_quality=bf(WPN_SPCQLTY_CREATURE) | special, .wpn_talent=TLT_NONE, .convey_status_effect=SEID_NONE, }, \
            creature_desc 

#define MELEE_1H(dmg_die,dmg_add,_dmg_type,_penetration,special,talent) .icon=')',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=WEAPON_CATEGORY_1H_MELEE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true, melee_desc

#define MELEE_2H(dmg_die,dmg_add,_dmg_type,_penetration,special,talent) .icon=')',.stacked_quantity=0, .max_quantity=0,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_MELEE, .weapon_category=WEAPON_CATEGORY_2H_MELEE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, .dropable=true, melee_desc

#define THROWN_WEAPON(_dmg_type,dmg_die,dmg_add,_penetration,_range,special,talent) .icon='}',.stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_WEAPON, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=SEID_NONE, }, throw_desc, .dropable=true

#define THROWN_GRENADE(_dmg_type,dmg_die,dmg_add,_penetration,_range,special,talent, cid) .icon='}',.stacked_quantity=1, .max_quantity=100,\
            .item_type=ITEM_TYPE_WEAPON, .specific.weapon={.weapon_type=WEAPON_TYPE_THROWN, .weapon_category=WEAPON_CATEGORY_THROWN_GRENADE, \
            .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, .penetration=_penetration, \
            .special_quality=special, .upgrades=0, .wpn_talent=talent, .convey_status_effect=cid, }, throw_desc, .dropable=true

#define RANGED_1H(_dmg_type,dmg_die,dmg_add,_penetration,_range,special,talent) \
            .icon='}', .stacked_quantity=0, .max_quantity=0, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_1H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=1, .rof[WEAPON_ROF_SETTING_SEMI]=0, .rof[WEAPON_ROF_SETTING_AUTO]=0, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=1, .magazine_left=1, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_ARROW, .ammo_used_template_id=IID_ARROW, .special_quality=special, .upgrades=0, \
            .wpn_talent=talent, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define RANGED_2H(_dmg_type,dmg_die,dmg_add,_penetration,_range,special,talent) \
            .icon='}', .stacked_quantity=0, .max_quantity=0, .item_type=ITEM_TYPE_WEAPON, .specific.weapon={ \
            .weapon_category=WEAPON_CATEGORY_2H_RANGED, .dmg_type=_dmg_type, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=_range, \
            .rof[WEAPON_ROF_SETTING_SINGLE]=1, .rof[WEAPON_ROF_SETTING_SEMI]=0, .rof[WEAPON_ROF_SETTING_AUTO]=0, \
            .rof_set=WEAPON_ROF_SETTING_SINGLE, .magazine_sz=1, .magazine_left=1, .penetration=_penetration, \
            .ammo_type=AMMO_TYPE_ARROW, .ammo_used_template_id=IID_ARROW, .special_quality=special, .upgrades=0, \
            .wpn_talent=talent, .jammed=false, .convey_status_effect=SEID_NONE, }, .dropable=true, ranged_desc

#define AMMO(_ammo_type,cid) .icon='\'', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_AMMO, .specific.ammo={ .ammo_type=_ammo_type, .convey_status_effect=cid, }

#define DRAUGHT(cid) .icon='!', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_FOOD, .specific.food={ .food_type=FOOD_TYPE_LIQUID, .nutrition=0, .nutrition_left=0, .convey_status_effect=cid, }

#define MUSHROOM(cid) .icon='&', .stacked_quantity=1, .max_quantity=100, .dropable=true, \
            .item_type=ITEM_TYPE_FOOD, .specific.food={ .food_type=FOOD_TYPE_SOLID, .nutrition=0, .nutrition_left=0, .convey_status_effect=cid, }

static const char *itm_descs[] = {
    [IID_NONE]              = "none",
    [IID_FIXED_LIGHT]       = "",
    [IID_TORCH]             = "This a generic torch",

    /* Armour */
	[IID_LEATHER_SKULLCAP]		= "none",
	[IID_LEATHER_JERKIN]		= "none",
	[IID_LEATHER_JACK]		    = "none",
	[IID_LEATHER_LEGGINS]		= "none",
	[IID_LEATHER_FULL]		    = "none",
	[IID_STDD_LEATHER_SKULLCAP]	= "none",
	[IID_STDD_LEATHER_JERKIN]	= "none",
	[IID_STDD_LEATHER_JACK]		= "none",
	[IID_STDD_LEATHER_LEGGINS]	= "none",
	[IID_STDD_LEATHER_FULL]		= "none",
	[IID_MAIL_COIF]		        = "none",
	[IID_MAIL_SHIRT]		    = "none",
	[IID_MAIL_SLEEVED_SHIRT]	= "none",
	[IID_MAIL_COAT]		        = "none",
	[IID_MAIL_SLEEVED_COAT]		= "none",
	[IID_MAIL_LEGGINS]		    = "none",
	[IID_MAIL_FULL]		        = "none",

    /* Melee */
    [IID_KNIFE]                 = "a basic knife",
    [IID_AXE_POOR]              = "",
    [IID_PICK_POOR]             = "",
    [IID_HAMMER_POOR]           = "",
    [IID_SWORD_POOR]            = "",
    [IID_AXE]                   = "",
    [IID_PICK]                  = "",
    [IID_HAMMER]                = "",
    [IID_SWORD]                 = "",
    [IID_SWORD_GOOD]            = "",
    [IID_AXE_GOOD]              = "",
    [IID_HAMMER_GOOD]           = "",
    [IID_PICK_GOOD]             = "",

    /* Ranged */
    [IID_THROWING_KNIFE]        = "",
    [IID_SHORT_BOW]             = "",
    [IID_LONG_BOW]              = "",
    [IID_FIRE_BOMB]             = "",

    /* Ammo */
    [IID_ARROW]                 = "",

    [IID_DRAUGHT_HEALING_MINOR]  = "A healing Draught",

    [IID_MUSHROOM_MAD_CAP] = "A mad cap mushroom",

    /* Debug and internal items */
    [IID_HUMAN_UNARMED]           = "",
    [IID_CREATURE_BITE_UNTRAINED] = "",
    [IID_CREATURE_BITE_TRAINED]   = "",
    [IID_BODYPART_GRENADE]  = "used in status effects",
    [IID_STIMM_DEATH]      = "Death stimm injector *debug*",
};

static struct itm_item static_item_list[] = {
    /* Tools */


    /* Lights */
    /*    ID              short name    long name    quality          weight,cost,delay             tool type         energy  luminem*/
    ITEM(IID_FIXED_LIGHT, "torch",      "a torch",   ITEM_QLTY_AVERAGE, 1,     1,   1), FIXED_LIGHT(TOOL_TYPE_LIGHT,      1,    8),  ITEM_END,
    ITEM(IID_TORCH,       "torch",      "a torch",   ITEM_QLTY_AVERAGE, 1,     1,   1),       LIGHT(TOOL_TYPE_LIGHT,   1000,    6), CREATION(1,1), ITEM_END,

    /* Wearables */
    /*    ID                           short name                    long name                             (wgt,cst,dly)      dr   locations                                     special qualities*/
    ITEM_AVG(IID_LEATHER_SKULLCAP,     "leather helmet",             "a leather helmet",                  10, 3,2), ARMOUR(2,INV_LOC_HEAD,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_LEATHER_JERKIN,       "leather jerkin",             "a leather jerkin",                  40, 6,2), ARMOUR(2,INV_LOC_BODY,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_LEATHER_JACK,         "light leather jack",         "a light leather jack",              50,12,4), ARMOUR(2,INV_LOC_ARMS|INV_LOC_BODY,                          bf(WBL_SPCQLTY_LEATHER) ), CREATION(5,1), ITEM_END,
    ITEM_AVG(IID_LEATHER_LEGGINS,      "leather leggings",           "a pair of leather leggings",        20,10,3), ARMOUR(2,INV_LOC_LEGS,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_LEATHER_FULL,         "full leather armour",        "a full leather armour",             80,25,4), ARMOUR(3,INV_LOC_HEAD|INV_LOC_ARMS|INV_LOC_BODY|INV_LOC_LEGS,bf(WBL_SPCQLTY_LEATHER) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_STDD_LEATHER_SKULLCAP,"studded leather helmet",     "a studded leather helmet",          10, 3,2), ARMOUR(4,INV_LOC_HEAD,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_STDD_LEATHER_JERKIN,  "studded leather jerkin",     "a studded leather jerkin",          40, 6,2), ARMOUR(4,INV_LOC_BODY,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(5,1), ITEM_END,
    ITEM_AVG(IID_STDD_LEATHER_JACK,    "light studded leather jack", "a light studded leather jack",      50,12,4), ARMOUR(4,INV_LOC_ARMS|INV_LOC_BODY,                          bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_STDD_LEATHER_LEGGINS, "studded leather leggings",   "a pair of studded leather leggings",20,10,3), ARMOUR(4,INV_LOC_LEGS,                                       bf(WBL_SPCQLTY_LEATHER) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_STDD_LEATHER_FULL,    "full studded leather armour","a full studded leather armour",     80,25,4), ARMOUR(5,INV_LOC_HEAD|INV_LOC_ARMS|INV_LOC_BODY|INV_LOC_LEGS,bf(WBL_SPCQLTY_LEATHER) ), CREATION(4,1), ITEM_END,
    ITEM_AVG(IID_MAIL_COIF,            "mail coif",                  "a mail coif",                       10, 3,2), ARMOUR(6,INV_LOC_HEAD,                                       bf(WBL_SPCQLTY_MAIL) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_MAIL_SHIRT,           "mail shirt",                 "a mail shirt",                      40, 6,2), ARMOUR(6,INV_LOC_BODY,                                       bf(WBL_SPCQLTY_MAIL) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_MAIL_SLEEVED_SHIRT,   "sleeved mail shirt",         "a sleeved mail shirt",              50,12,4), ARMOUR(6,INV_LOC_ARMS|INV_LOC_BODY,                          bf(WBL_SPCQLTY_MAIL) ), CREATION(5,1), ITEM_END,
    ITEM_AVG(IID_MAIL_COAT,            "mail coat",                  "a mail coat",                       40, 6,2), ARMOUR(6,INV_LOC_BODY|INV_LOC_LEGS,                          bf(WBL_SPCQLTY_MAIL) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_MAIL_SLEEVED_COAT,    "sleeved mail coat",          "a sleeved mail coat",               50,12,4), ARMOUR(6,INV_LOC_ARMS|INV_LOC_BODY|INV_LOC_LEGS,             bf(WBL_SPCQLTY_MAIL) ), CREATION(5,1), ITEM_END,
    ITEM_AVG(IID_MAIL_LEGGINS,         "mail leggings",              "a pair of mail leggings",           20,10,3), ARMOUR(6,INV_LOC_LEGS,                                       bf(WBL_SPCQLTY_MAIL) ), CREATION(6,1), ITEM_END,
    ITEM_AVG(IID_MAIL_FULL,            "full mail armour",           "a full mail armour",                80,25,4), ARMOUR(7,INV_LOC_HEAD|INV_LOC_ARMS|INV_LOC_BODY|INV_LOC_LEGS,bf(WBL_SPCQLTY_MAIL) ), CREATION(4,1), ITEM_END,

    /* Melee */
    /*    ID                 short name,   long name              (wgt,cst,dly)      xd10 +X, dmg type         pen, special qualities,         talent*/
    ITEM_AVG(IID_KNIFE,       "knife",     "a knife",              15, 50, 0),MELEE_1H(1,-4,  DMG_TYPE_PIERCING,0,  bf(WPN_SPCQLTY_LIGHT),    TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_POOR(IID_AXE_POOR,   "axe",       "an shoddy axe",        75, 10, 0),MELEE_1H(1,-2,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(40,1), ITEM_END,
    ITEM_POOR(IID_PICK_POOR,  "pick",      "an shoddy pick",       75, 10, 0),MELEE_1H(1,-2,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(40,1), ITEM_END,
    ITEM_POOR(IID_HAMMER_POOR,"hammer",    "an shoddy hammer",     75, 10, 0),MELEE_1H(1,-2,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(40,1), ITEM_END,
    ITEM_POOR(IID_SWORD_POOR, "sword",     "a shoddy sword",       75, 10, 0),MELEE_1H(1,-2,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(40,1), ITEM_END,
    ITEM_AVG(IID_AXE,         "axe",       "an axe",               75, 10, 0),MELEE_1H(1, 0,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_AVG(IID_PICK,        "pick",      "an pick",              75, 10, 0),MELEE_1H(1, 0,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_AVG(IID_HAMMER,      "hammer",    "an hammer",            75, 10, 0),MELEE_1H(1, 0,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_AVG(IID_SWORD,       "sword",     "a sword",              75, 10, 0),MELEE_1H(1, 0,  DMG_TYPE_PIERCING,0,  0,                        TLT_NONE), CREATION(20,1), ITEM_END,
    ITEM_GOOD(IID_AXE_GOOD,   "axe",       "a military axe",       75,100, 0),MELEE_1H(1, 0,  DMG_TYPE_CUTTING, 0,  bf(WPN_SPCQLTY_IMPACT),   TLT_NONE), CREATION(10,2), ITEM_END,
    ITEM_GOOD(IID_HAMMER_GOOD,"hammer",    "a military hammer",    75,100, 0),MELEE_1H(1, 0,  DMG_TYPE_BLUNT,   0,  bf(WPN_SPCQLTY_PUMMELING),TLT_NONE), CREATION(10,2), ITEM_END,
    ITEM_GOOD(IID_PICK_GOOD,  "pick",      "a military pick",      75,100, 0),MELEE_1H(1, 0,  DMG_TYPE_PIERCING,1,  bf(WPN_SPCQLTY_SLOW),     TLT_NONE), CREATION(10,2), ITEM_END,
    ITEM_GOOD(IID_SWORD_GOOD, "sword",     "a military sword",     75,100, 0),MELEE_1H(1, 0,  DMG_TYPE_CUTTING, 0,  bf(WPN_SPCQLTY_DEFENSIVE),TLT_NONE), CREATION(10,2), ITEM_END,

    /* Ranged */
    /*    ID                    short name       long name        (wgt,cst,dly)              dmg type       xd10 +x pen range  special qualities     talents*/
    ITEM_AVG(IID_SHORT_BOW,     "short bow",     "a short bow",     50, 7, 0.5),RANGED_2H(   DMG_TYPE_PIERCING,1, 3, 0, 16,     0,                   TLT_NONE),                    CREATION(10,1), ITEM_END,
    ITEM_AVG(IID_LONG_BOW,      "long bow",      "a long bow",      90,15, 0.5),RANGED_2H(   DMG_TYPE_PIERCING,1, 3, 1, 32,     0,                   TLT_SPEC_WPN_GRP_LONGBOW),    CREATION(10,1), ITEM_END,
    ITEM_AVG(IID_THROWING_KNIFE,"throwing knife","a throwing knife", 5, 5, 1),THROWN_WEAPON( DMG_TYPE_PIERCING,1,-3, 0,  2,     0,                   TLT_SPEC_WPN_GRP_THROWING),   CREATION(40,1), ITEM_END,
    ITEM_AVG(IID_FIRE_BOMB,     "fire bomb",     "a fire bomb",      5,10, 1),THROWN_GRENADE(DMG_TYPE_SHRAPNEL,1,-5, 0,  2,bf(WPN_SPCQLTY_BLAST_1),  TLT_NONE, SEID_WEAPON_FLAME), CREATION(30,1), ITEM_END,

    /* Ammo */
    /*    ID                    short name       long name        (wgt,cst,dly)     ammo type       status effect id*/
    ITEM_AVG(IID_ARROW,         "arrows",        "arrows",           1, 1, 1), AMMO(AMMO_TYPE_ARROW,SEID_NONE), CREATION(10,1), ITEM_END,

    /* Creature Attacks */
    /*    ID                            hort name long name  (wgt,cst,dly)            CATEGORY               xd10  +X  dmg type     upgrades   special qualities*/
    ITEM_NONE(IID_HUMAN_UNARMED,          "hands",   "hands",   0, 0, 0), MARTIAL(       WEAPON_CATEGORY_2H_MELEE,1, -4, DMG_TYPE_UNARMED,0,  bf(WPN_SPCQLTY_UNARMED) | bf(WPN_SPCQLTY_PRIMITIVE) ), ITEM_END,
    ITEM_NONE(IID_CREATURE_BITE_UNTRAINED,"teeth",   "teeth",   0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1, -4, DMG_TYPE_CLAW,   0,  bf(WPN_SPCQLTY_UNARMED) ), ITEM_END,
    ITEM_NONE(IID_CREATURE_BITE_TRAINED,  "teeth",   "teeth",   0, 0, 0), CREATURE_MELEE(WEAPON_CATEGORY_2H_MELEE,1,  0, DMG_TYPE_CLAW,   0,  0), ITEM_END,

    /* Potions */
    /*    ID                                short name        long name              (wgt,cst,dly)            status_effect*/
    ITEM_POOR(IID_DRAUGHT_HEALING_MINOR,  "healing draught", "a minor healing draught", 0, 1, 1), DRAUGHT(SEID_MINOR_HEALING), CREATION(10,1), ITEM_END,

    /* Mushrooms */
    /*    ID                                short name        long name       (wgt,cst,dly)           status_effect*/
    ITEM_AVG(IID_MUSHROOM_MAD_CAP,  "mad cap mushroom", "a black cap mushroom", 0, 1, 1), MUSHROOM(SEID_MAD_CAP), ITEM_END,

    /* status effect items */
    ITEM(IID_BODYPART_GRENADE,"critical","",ITEM_QLTY_AVERAGE,5,10,1),THROWN_GRENADE(DMG_TYPE_SHRAPNEL,1,0,0,3,bf(WPN_SPCQLTY_BLAST_2),TLT_NONE,SEID_NONE),ITEM_END,

    /* debug items */
    ITEM(IID_STIMM_DEATH,   "death debug",  "an injector with a deadly liquid", ITEM_QLTY_AVERAGE, 0, 1, 1), DRAUGHT(SEID_DEATH_STIMM), ITEM_END,
};

static const char *item_quality_strings[] = {
    [ITEM_QLTY_POOR]    =  "poor",
    [ITEM_QLTY_AVERAGE] =  "average",
    [ITEM_QLTY_GOOD]    =  "good",
    [ITEM_QLTY_BEST]    =  "best",
};

static const char *ammo_type_strings[] = {
    [AMMO_TYPE_NONE]   =  "none",
    [AMMO_TYPE_ARROW]  =  "arrows",
    [AMMO_TYPE_MAX]    =  "max",
};

static const char *wpn_spcqlty_name[] = {
    [WPN_SPCQLTY_ACCURATE]	    = "Accurate",
    [WPN_SPCQLTY_BALANCED]      = "Balanced",
    [WPN_SPCQLTY_BLAST_1]	    = "Blast 1",
    [WPN_SPCQLTY_BLAST_2]	    = "Blast 2",
    [WPN_SPCQLTY_BLAST_3]       = "Blast 3",
    [WPN_SPCQLTY_BLAST_4]       = "Blast 4",
    [WPN_SPCQLTY_DEFENSIVE]	    = "Defensive",
    [WPN_SPCQLTY_EXPERIMENTAL]  = "Experimental",
    [WPN_SPCQLTY_FAST]	        = "Fast",
    [WPN_SPCQLTY_FLAME]	        = "Flame",
    [WPN_SPCQLTY_FLEXIBLE]      = "Flexible",
    [WPN_SPCQLTY_GUNPOWDER]     = "Gunpowder",
    [WPN_SPCQLTY_IMPACT]        = "Impact",
    [WPN_SPCQLTY_INACCURATE]    = "Inaccurate",
    [WPN_SPCQLTY_JAMS]          = "Jams",
    [WPN_SPCQLTY_LIGHT]         = "Light",
    [WPN_SPCQLTY_OVERHEATS]     = "Overheats",
    [WPN_SPCQLTY_PRIMITIVE]	    = "Primitive",
    [WPN_SPCQLTY_PRECISE]       = "Precise",
    [WPN_SPCQLTY_PUMMELING]     = "Pummeling",
    [WPN_SPCQLTY_RELIABLE]	    = "Reliable",
    [WPN_SPCQLTY_SCATTER]	    = "Scatter",
    [WPN_SPCQLTY_SHOCKING]	    = "Shocking",
    [WPN_SPCQLTY_SHIELD]        = "Shield",
    [WPN_SPCQLTY_SHRAPNEL]      = "Shrapnel",
    [WPN_SPCQLTY_SLOW]          = "Slow",
    [WPN_SPCQLTY_SMOKE]	        = "Smoke",
    [WPN_SPCQLTY_SNARE]	        = "Snare",
    [WPN_SPCQLTY_TEARING]	    = "Tearing",
    [WPN_SPCQLTY_TOXIC]	        = "Toxic",
    [WPN_SPCQLTY_UNARMED]	    = "Unarmed",
    [WPN_SPCQLTY_UNBALANCED]	= "Unbalanced",
    [WPN_SPCQLTY_UNRELIABLE]	= "Unreliable",
    [WPN_SPCQLTY_UNSTABLE]	    = "Unstable",
    [WPN_SPCQLTY_UNWIELDY]	    = "Unwieldy",
};

static const char *wpn_spcqlty_desc[] = {
    [WPN_SPCQLTY_ACCURATE]	    = " Additional +10 to hit when used with the aim action ",
    [WPN_SPCQLTY_BALANCED]      = " +10 Parry ",
    [WPN_SPCQLTY_BLAST_1]	    = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_BLAST_2]	    = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_BLAST_3]       = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_BLAST_4]       = " All within the weapon's blast radius in squares is hit ",
    [WPN_SPCQLTY_DEFENSIVE]	    = " +15 Parry, -10 to hit  ",
    [WPN_SPCQLTY_EXPERIMENTAL]  = " Unstable, 96-98, jam, 99-00 -> explodes. ",
    [WPN_SPCQLTY_FAST]	        = " -10% to parry or dodge against ",
    [WPN_SPCQLTY_FLAME]	        = " No BS Test, All in arc take Agility Test or take damage, 2nd Ag. test to avoid catch fire. ",
    [WPN_SPCQLTY_FLEXIBLE]	    = " Cannot be Parried. ",
    [WPN_SPCQLTY_GUNPOWDER]     = " Gunpowder. ",
    [WPN_SPCQLTY_IMPACT]        = " Damage is rolled twice and the highest is picked. ",
    [WPN_SPCQLTY_INACCURATE]	= " No bonus with aim action ",
    [WPN_SPCQLTY_JAMS]          = " Can Jam on 95% ",
    [WPN_SPCQLTY_LIGHT]         = " This light weapon is easier to handle in your off-hand ",
    [WPN_SPCQLTY_OVERHEATS]	    = " 90+ -> Overheat ",
    [WPN_SPCQLTY_PRIMITIVE]	    = " AP doubled, unless armour is also primitive ",
    [WPN_SPCQLTY_PRECISE]       = " +2 Critical hit value ",
    [WPN_SPCQLTY_PUMMELING]     = " +10% on Strength for Strike to Stun ",
    [WPN_SPCQLTY_RELIABLE]	    = " If jam, 10 on 1d10 to actually jam.",
    [WPN_SPCQLTY_SCATTER]	    = " Point Blank: 2 DoS score another hit, AP doubled at Long and Extreme ranges. ",
    [WPN_SPCQLTY_SHOCKING]	    = " If damage, test Toughness or be stunned. ",
    [WPN_SPCQLTY_SHIELD]        = " Gives ranged attackers a -10% to hit.",
    [WPN_SPCQLTY_SHRAPNEL]      = " Fires in a Cone, requires an Agility Check, no Ballistic Check ",
    [WPN_SPCQLTY_SLOW]          = " Defenders can parry this weapon easier.",
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
    [WBL_SPCQLTY_PRIMITIVE]	= "Primitive",
    [WBL_SPCQLTY_LEATHER]	= "Leather",
    [WBL_SPCQLTY_MAIL]	    = "Mail",
    [WBL_SPCQLTY_SCALE]	    = "Scale",
    [WBL_SPCQLTY_PLATE]	    = "Plate",
};

static const char *wbl_spcqlty_desc[] = {
    [WBL_SPCQLTY_PRIMITIVE]	= "Halfs armour bonus against non-primitive weapons",
    [WBL_SPCQLTY_LEATHER]	= "Basic armour which give no penalties",
    [WBL_SPCQLTY_MAIL]    	= "Armour which give penalties",
    [WBL_SPCQLTY_SCALE]    	= "Armour which give penalties",
    [WBL_SPCQLTY_PLATE]    	= "Armour which give penalties",
};

