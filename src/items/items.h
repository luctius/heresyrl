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

#ifndef ITEMS_H_
#define ITEMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"
#include "enums.h"
#include "inventory.h"
#include "status_effects/status_effects_static.h"
#include "status_effects/ground_effects.h"
#include "items_static.h"

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
    FOOD_TYPE_INJECTION,
    FOOD_TYPE_MAX,
    FOOD_TYPE_RANDOM,
};

enum item_tool_type {
    TOOL_TYPE_LIGHT,
    TOOL_TYPE_CONTAINER,
    TOOL_TYPE_MONEY,
    TOOL_TYPE_MAX,
    TOOL_TYPE_RANDOM,
};

enum item_ammo_type {
    AMMO_TYPE_NONE,
    AMMO_TYPE_ARROW,
    AMMO_TYPE_PISTOL_SP,
    AMMO_TYPE_PISTOL_LAS,
    AMMO_TYPE_PISTOL_PLASMA,
    AMMO_TYPE_PISTOL_MELTA,
    AMMO_TYPE_PISTOL_FLAME,
    AMMO_TYPE_PISTOL_BOLT,
    AMMO_TYPE_PISTOL_SHURIKEN,
    AMMO_TYPE_PISTOL_SHOTGUN,
                             
    AMMO_TYPE_BASIC_GRENADE,
    AMMO_TYPE_BASIC_SP,
    AMMO_TYPE_BASIC_LAS,
    AMMO_TYPE_BASIC_PLASMA,
    AMMO_TYPE_BASIC_MELTA,
    AMMO_TYPE_BASIC_FLAME,
    AMMO_TYPE_BASIC_BOLT,
    AMMO_TYPE_BASIC_SHOTGUN,
    AMMO_TYPE_BASIC_SHURIKEN,
    AMMO_TYPE_BASIC_ROCKET,
                             
    AMMO_TYPE_HEAVY_GRENADE,
    AMMO_TYPE_HEAVY_SP,
    AMMO_TYPE_HEAVY_LAS,
    AMMO_TYPE_HEAVY_PLASMA,
    AMMO_TYPE_HEAVY_FLAME,
    AMMO_TYPE_HEAVY_MELTA,
    AMMO_TYPE_HEAVY_BOLT,
    AMMO_TYPE_HEAVY_ROCKET,
    AMMO_TYPE_MAX,
    AMMO_TYPE_RANDOM,
};

enum item_wearable_type {
    WEARABLE_TYPE_ARMOUR,
    WEARABLE_TYPE_VISOR,
    WEARABLE_TYPE_CLOTHING,
    WEARABLE_TYPE_RING,
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

enum item_weapon_category {
    /* Ranged weapons*/
    WEAPON_CATEGORY_1H_RANGED,
    WEAPON_CATEGORY_2H_RANGED,

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
    enum dmg_type dmg_type;
    uint8_t nr_dmg_die;     /*0 is 1d5*/
    int8_t dmg_addition;    /* this is the +X in 1d10 +X*/
    uint8_t range;          /* range in meters*/
    uint8_t rof[WEAPON_ROF_SETTING_MAX]; /* number of bullers spend (and possible it) with each setting. single must either be 0 or 1. */
    enum wpn_rof_setting rof_set;       /* the current setting, single. semi or auto. */
    uint8_t magazine_sz;
    uint8_t magazine_left;
    uint8_t penetration;
    enum item_ammo_type ammo_type;
    enum item_ids ammo_used_tid; /* <enum item_ids> item template id of the ammo currently used. this is used to unload the ammo and check for special attributes. */
    bitfield64_t special_quality;
    bitfield64_t upgrades;
    bitfield64_t wpn_talent;  /* talent required for this weapon to operate.*/
    bool jammed;
    enum se_ids convey_status_effect;
    enum ge_ids convey_ground_effect;
};

struct item_wearable_specific {
    enum item_wearable_type wearable_type;
    enum inv_locations locations;
    uint8_t damage_reduction;
    bitfield64_t special_quality;
};

struct item_tool_specific {
    enum item_tool_type tool_type;
    int energy;
    int light_luminem;
    bool lit;
};

struct item_food_specific {
    enum item_food_type food_type;
    int nutrition;
    int nutrition_left;
    enum se_ids convey_status_effect;

    uint8_t side_effect_chance;
    enum se_ids side_effect;
};

struct item_ammo_specific {
    enum item_ammo_type ammo_type;
    bitfield64_t upgrades;
    enum se_ids convey_status_effect;
    int energy;
    int energy_left;
};

enum item_owner {
    ITEM_OWNER_NONE,
    ITEM_OWNER_MAP,
    ITEM_OWNER_MONSTER,
    ITEM_OWNER_MAX,
};

enum item_group {
    ITEM_GROUP_NONE,
    ITEM_GROUP_ARMOUR,
    ITEM_GROUP_SHIELD,
    ITEM_GROUP_RANGED,
    ITEM_GROUP_GUNPOWDER,
    ITEM_GROUP_THROWING,
    ITEM_GROUP_1H_MELEE,
    ITEM_GROUP_2H_MELEE,
    ITEM_GROUP_POTION,
    ITEM_GROUP_TRAP,
    ITEM_GROUP_ANY,
};

struct itm_item {
    int item_pre;

    /* unique id of this item instance. */
    uint32_t uid;

    /* id of the items base copy. */
    uint32_t tid;
    enum item_types item_type;
    /*enum item_material material;*/

    int spawn_weight;
    int spawn_level;

    enum item_quality quality;
    int weight;
    uint32_t cost;
    const wchar_t *sd_name;
    const wchar_t *ld_name;
    const wchar_t *description;
    icon_t icon;
    int icon_attr;
    float use_delay;
    int stacked_quantity;
    int max_quantity;
    bool identified;
    int identify_last_try;

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
    bool permanent_energy; /* True if it is a permanent effect, like a magic cloak*/

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

    const wchar_t *you_use_desc[5];
    const wchar_t *msr_use_desc[5];

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

/* retrieves a item template id based on parameters */
uint32_t itm_spawn(int32_t roll, int level, enum item_group ig, struct msr_monster *monster);

/* create an item instance of this template id*/
struct itm_item *itm_create(int tid);
struct itm_item *itm_copy(struct itm_item *item);

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

bool itm_is_type(struct itm_item *item, enum item_types type);

/* true if this item has this quality*/
bool itm_has_quality(struct itm_item *item, enum item_quality q);

/* fuctions which is called when item->energy reaches zero and energy_action = true. */
bool itm_energy_action(struct itm_item *item, struct dm_map *map);

/* change the item's energy by this much, true if succefull.  */
bool itm_change_energy(struct itm_item *item, int energy);

/* return the current amount of energy of the device, or -1 if invalid. */
int itm_get_energy(struct itm_item *item);

/* handle succesfull identification of an item */
void itm_identify(struct itm_item *item);

/* true if the item is now succesfully identified, false if failed or allready identified. */
bool itm_try_identify(struct itm_item *item);

/* true if the item is a weapon and is of this weapon type */
bool wpn_is_type(struct itm_item *item, enum item_weapon_type type);

/* true if the item is a weapon and is of this weapon catergory */
bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat);

/* check if the rof settings of this weapon makes sense. */
bool wpn_ranged_weapon_rof_set_check(struct itm_item *item);

/* cycle to the next valid rof setting */
bool wpn_ranged_next_rof_set(struct itm_item *item);

/* true if the weapon has this upgrade */
bool wpn_has_upgrade(struct itm_item *item, enum weapon_upgrades u);

/* true if the weapon has this special quality */
bool wpn_has_spc_quality(struct itm_item *item, enum weapon_special_quality q);

/* true if the weapon uses ammo */
bool wpn_uses_ammo(struct itm_item *item);

/* return the id of the ammo template; AMMO_TYPE_NONE on error */
enum item_ammo_type wpn_get_ammo_type(struct itm_item *item);

/* return the id of the ammo used; IID_NONE on error */
enum item_ids wpn_get_ammo_used_id(struct itm_item *item);

/* true if this item is wearable and is of this type*/
bool wbl_is_type(struct itm_item *item, enum item_wearable_type type);

/* true if the wearable has this special quality*/
bool wbl_has_spc_quality(struct itm_item *item, enum wearable_special_quality q);

/* true if the item is ammo, and the ammo is of this type */
bool ammo_is_type(struct itm_item *item, enum item_ammo_type type);

/* true if the item is a tool, and the tool is of this type */
bool tool_is_type(struct itm_item *item, enum item_tool_type type);

/* true if both items can be stacked. */
bool itm_stack_compatible(struct itm_item *item1, struct itm_item *item2);

const wchar_t *itm_you_use_desc(struct itm_item *item);
const wchar_t *itm_msr_use_desc(struct itm_item *item);

const wchar_t *itm_quality_string(struct itm_item *item);
const wchar_t *wpn_ammo_string(enum item_ammo_type iat);

const wchar_t *wpn_spec_quality_name(enum weapon_special_quality spq);
const wchar_t *wpn_spec_quality_description(enum weapon_special_quality spq);

const wchar_t *wbl_spec_quality_name(enum wearable_special_quality spq);
const wchar_t *wbl_spec_quality_description(enum wearable_special_quality spq);

void itm_dbg_check_all(void);

#endif /*ITEMS_H_*/

