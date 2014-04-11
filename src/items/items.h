#ifndef ITEMS_H_
#define ITEMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"
#include "enums.h"
#include "inventory.h"

enum item_types {
    ITEM_TYPE_WEARABLE,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_FOOD,
    ITEM_TYPE_AMMO,
    ITEM_TYPE_TOOL,
    ITEM_TYPE_MAX,
    ITEM_TYPE_RANDOM,
};

/*
enum item_material {
    ITEM_MATERIAL_LIQUID,
    ITEM_MATERIAL_WAX,
    ITEM_MATERIAL_VEGGY,
    ITEM_MATERIAL_FLESH,
    ITEM_MATERIAL_PAPER,
    ITEM_MATERIAL_CLOTH,
    ITEM_MATERIAL_LEATHER,
    ITEM_MATERIAL_WOOD,
    ITEM_MATERIAL_BONE,
    ITEM_MATERIAL_DRAGON_HIDE,
    ITEM_MATERIAL_IRON,
    ITEM_MATERIAL_METAL,
    ITEM_MATERIAL_COPPER,
    ITEM_MATERIAL_SILVER,
    ITEM_MATERIAL_GOLD,
    ITEM_MATERIAL_PLATINUM,
    ITEM_MATERIAL_MITHRIL,
    ITEM_MATERIAL_PLASTIC,
    ITEM_MATERIAL_GLASS,
    ITEM_MATERIAL_GEMSTONE,
    ITEM_MATERIAL_MINERAL,
    ITEM_MATERIAL_MAX,
};
*/

enum item_weapon_type {
    WEAPON_TYPE_RANGED,
    WEAPON_TYPE_MELEE,
    WEAPON_TYPE_THROWN,
    WEAPON_TYPE_MAX,
    WEAPON_TYPE_RANDOM,
};

enum item_food_type {
    FOOD_TYPE_SOLID,
    FOOD_TYPE_LIQUID,
    FOOD_TYPE_MAX,
    FOOD_TYPE_RANDOM,
};

enum item_tool_type {
    TOOL_TYPE_LIGHT,
    TOOL_TYPE_CONTAINER,
    TOOL_TYPE_MAX,
    TOOL_TYPE_RANDOM,
};

enum item_ammo_type {
    AMMO_TYPE_ARROW,
    AMMO_TYPE_PISTOL_SP,
    AMMO_TYPE_PISTOL_SHOTGUN,
    AMMO_TYPE_PISTOL_LAS,
    AMMO_TYPE_PISTOL_PLASMA,
    AMMO_TYPE_PISTOL_MELTA,
    AMMO_TYPE_PISTOL_FLAME,
    AMMO_TYPE_PISTOL_BOLT,
    AMMO_TYPE_PISTOL_SHURIKEN,
    AMMO_TYPE_BASIC_SP,
    AMMO_TYPE_BASIC_SHOTGUN,
    AMMO_TYPE_BASIC_LAS,
    AMMO_TYPE_BASIC_PLASMA,
    AMMO_TYPE_BASIC_MELTA,
    AMMO_TYPE_BASIC_FLAME,
    AMMO_TYPE_BASIC_BOLT,
    AMMO_TYPE_BASIC_SHURIKEN,
    AMMO_TYPE_BASIC_GRENADE,
    AMMO_TYPE_BASIC_ROCKET,
    AMMO_TYPE_HEAVY_SP,
    AMMO_TYPE_HEAVY_LAS,
    AMMO_TYPE_HEAVY_PLASMA,
    AMMO_TYPE_HEAVY_MELTA,
    AMMO_TYPE_HEAVY_FLAME,
    AMMO_TYPE_HEAVY_BOLT,
    AMMO_TYPE_HEAVY_GRENADE,
    AMMO_TYPE_HEAVY_ROCKET,
    AMMO_TYPE_MAX,
    AMMO_TYPE_RANDOM,
};

enum item_wearable_type {
    WEARABLE_TYPE_ARMOUR,
    WEARABLE_TYPE_BOOTS,
    WEARABLE_TYPE_VISOR,
    WEARABLE_TYPE_CLOTHING,
    WEARABLE_TYPE_RING,
    WEARABLE_TYPE_GLOVES,
    WEARABLE_TYPE_MAX,
    WEARABLE_TYPE_RANDOM,
};

enum item_quality {
    ITEM_QLTY_NONE,
    ITEM_QLTY_POOR,
    ITEM_QLTY_AVERAGE,
    ITEM_QLTY_GOOD,
    ITEM_QLTY_BEST,
    ITEM_QLTY_MAX,
    ITEM_QLTY_RANDOM,
};

enum item_availability {
    ITEM_AVAIL_NONE,
    ITEM_AVAIL_PLENTIFUL,
    ITEM_AVAIL_AVERAGE,
    ITEM_AVAIL_COMMON,
    ITEM_AVAIL_POOR,
    ITEM_AVAIL_SCARCE,
    ITEM_AVAIL_RARE,
    ITEM_AVAIL_VERY_RARE,
    ITEM_AVAIL_MAX,
    ITEM_AVAIL_RANDOM,
};

enum weapon_dmg_type {
    WEAPON_DMG_TYPE_IMPACT,
    WEAPON_DMG_TYPE_EXPLOSIVE,
    WEAPON_DMG_TYPE_ENERGY,
    WEAPON_DMG_TYPE_RENDING,
    WEAPON_DMG_TYPE_MAX,
    WEAPON_DMG_TYPE_RANDOM,
};

enum item_weapon_category {
    /* Ranged weapons*/
    WEAPON_CATEGORY_PISTOL,
    WEAPON_CATEGORY_BASIC,
    WEAPON_CATEGORY_HEAVY,

    /* Melee weapons*/
    WEAPON_CATEGORY_1H_MELEE,
    WEAPON_CATEGORY_2H_MELEE,

    /* thrown weapons*/
    WEAPON_CATEGORY_THROWN_WEAPON,
    WEAPON_CATEGORY_THROWN_GRENADE,

    WEAPON_CATEGORY_MAX,
    WEAPON_CATEGORY_RANDOM,
};

enum wpn_rof_setting {
    WEAPON_ROF_SETTING_SINGLE,
    WEAPON_ROF_SETTING_SEMI,
    WEAPON_ROF_SETTING_AUTO,
    WEAPON_ROF_SETTING_MAX,
};

struct item_weapon_specific {
    enum item_weapon_type weapon_type;
    enum item_weapon_category weapon_category;
    enum weapon_dmg_type dmg_type;
    uint8_t nr_dmg_die;     /*0 is 1d5*/
    int8_t dmg_addition;    /* this is the +X in 1d10 +X*/
    uint8_t range;          /* range in meters*/
    uint8_t rof[WEAPON_ROF_SETTING_MAX]; /* number of bullers spend (and possible it) with each setting. single must either be 0 or 1. */
    enum wpn_rof_setting rof_set;       /* the current setting, single. semi or auto. */
    uint8_t magazine_sz;
    uint8_t magazine_left;
    uint8_t penetration;
    enum item_ammo_type ammo_type;
    uint32_t ammo_used_template_id; /* item template id of the ammo currently used. this is used to unload the ammo and check for special attributes. */
    bitfield_t special_quality;
    bitfield_t upgrades;
    bitfield_t wpn_talent;  /* talent required for this weapon to operate.*/
    bool jammed;
};

struct item_wearable_specific {
    enum item_wearable_type wearable_type;
    enum inv_locations locations;
    uint8_t damage_reduction;
    bitfield_t special_quality;
};

struct item_tool_specific {
    enum item_tool_type tool_type;
    int energy;
    int energy_left;
    int light_luminem;
    bool lit;
};

struct item_food_specific {
    enum item_food_type food_type;
    int nutrition;
    int nutrition_left;
};

struct item_ammo_specific {
    enum item_ammo_type ammo_type;
    int energy;
    int energy_left;
};

enum item_owner {
    ITEM_OWNER_NONE,
    ITEM_OWNER_MAP,
    ITEM_OWNER_MONSTER,
    ITEM_OWNER_MAX,
};

struct itm_item {
    int item_pre;

    /* unique id of this item instance. */
    uint32_t uid; 

    /* id of the items base copy. */
    uint32_t template_id;
    enum item_types item_type;
    /*enum item_material material;*/
    enum item_availability availability;
    enum item_quality quality;
    int weight;
    int cost;
    const char *sd_name;
    const char *ld_name;
    const char *description;
    char icon;
    int icon_attr;
    float use_delay;
    uint8_t stacked_quantity;
    uint8_t max_quantity;

    /* total amount of time this item exists, in energy*/
    unsigned long age;

    /* 
       amount of energy this item has, 
       if energy_action == false, this is not used.

       if energy_action == true, there is an action attached 
       to this item when its energy reaches <= 0.

       this can be used by grenades and torches for example.
     */
    bool energy_action;
    int energy;

    /* 
       whether or not this item appears on the ground after a creature is dead. 
       non-dropable items will not be saved (since they will not be modified in 
       a meaningfull way), thus any item which will be generated everytime a 
       game is loaded should be non-dropable.
     */
    bool dropable;

    /* description of the owner of this item*/
    enum item_owner owner_type;
    union owner_union {
        struct dm_map_entity *owner_map_entity;
        struct msr_monster *owner_monster;
    } owner;

    union item_specific {
        struct item_weapon_specific weapon;
        struct item_ammo_specific ammo;
        struct item_food_specific food;
        struct item_tool_specific tool;
        struct item_wearable_specific wearable;
    } specific;

    int item_post;
};

/* global init/exit of items list*/
void itmlst_items_list_init(void);
void itmlst_items_list_exit(void);

/* get next item from global items list.
   if prev == NULL, get the first item, 
   otherwise the item after prev. */
struct itm_item *itmlst_get_next_item(struct itm_item *prev);
/* search the items list for the item with this uid. */
struct itm_item *itmlst_item_by_uid(uint32_t uid);

/* generate an item by its general type*/
struct itm_item *itm_generate(enum item_types type);

/* create an item instance of this template id*/
struct itm_item *itm_create(int template_id);

/* destroy the item and remove it from the global item list */
void itm_destroy(struct itm_item *item);

/*i verify struct intergrity*/
bool itm_verify_item(struct itm_item *item);

/* instert item at map pos, and change ownership*/
bool itm_insert_item(struct itm_item *item, struct dm_map *map, coord_t *pos);

/* remove item from pos, and remove any ownership info. */
bool itm_remove_item(struct itm_item *item, struct dm_map *map, coord_t *pos);

/* get the grid position this item is on */
coord_t itm_get_pos(struct itm_item *item);

/* true if this item has this quality*/
bool itm_has_quality(struct itm_item *item, enum item_quality q);

/* fuctions which is called when item->energy reaches zero and energy_action = true. */
bool itm_energy_action(struct itm_item *item, struct dm_map *map);

/* change the item's energy by this much, true if succefull.  */
bool itm_change_energy(struct itm_item *item, int energy);

int itm_get_energy(struct itm_item *item);

/* true if the item is a weapon and is of this weapon type */
bool wpn_is_type(struct itm_item *item, enum item_weapon_type type);

/* true if the item is a weapon and is of this weapon catergory */
bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat);

/* check if the rof settings of this weapon makes sense. */
bool wpn_ranged_weapon_rof_set_check(struct itm_item *item);

/* cycle to the next valid rof setting */
bool wpn_ranged_next_rof_set(struct itm_item *item);

/* true if the weapon has this upgrade*/
bool wpn_has_upgrade(struct itm_item *item, enum weapon_upgrades u);

/* true if the weapon has this special quality*/
bool wpn_has_spc_quality(struct itm_item *item, enum weapon_special_quality q);

/* true if this item is wearable and is of this type*/
bool wbl_is_type(struct itm_item *item, enum item_wearable_type type);

/* true if the wearable has this special quality*/
bool wbl_has_spc_quality(struct itm_item *item, enum wearable_special_quality q);

/* true if the item is ammo, and the ammo is of this type */
bool ammo_is_type(struct itm_item *item, enum item_ammo_type type);

/* true if the item is a tool, and the tool is of this type */
bool tool_is_type(struct itm_item *item, enum item_tool_type type);

const char *itm_quality_string(struct itm_item *item);
const char *wpn_ammo_string(struct itm_item *item);
const char *wpn_spec_quality_name(enum weapon_special_quality spq);
const char *wpn_spec_quality_description(enum weapon_special_quality spq);

#endif /*ITEMS_H_*/

