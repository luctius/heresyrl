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
    ITEM_QUALITY_POOR,
    ITEM_QUALITY_AVERAGE,
    ITEM_QUALITY_GOOD,
    ITEM_QUALITY_BEST,
    ITEM_QUALITY_MAX,
    ITEM_QUALITY_RANDOM,
};

enum item_availability {
    ITEM_AVAILABILITY_PLENTIFUL,
    ITEM_AVAILABILITY_AVERAGE,
    ITEM_AVAILABILITY_COMMON,
    ITEM_AVAILABILITY_POOR,
    ITEM_AVAILABILITY_SCARCE,
    ITEM_AVAILABILITY_RARE,
    ITEM_AVAILABILITY_VERY_RARE,
    ITEM_AVAILABILITY_MAX,
    ITEM_AVAILABILITY_RANDOM,
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
    WEAPON_CATEGORY_PISTOL,
    WEAPON_CATEGORY_BASIC,
    WEAPON_CATEGORY_HEAVY,
    WEAPON_CATEGORY_1H_MELEE,
    WEAPON_CATEGORY_2H_MELEE,
    WEAPON_CATEGORY_THROWN,
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
    uint8_t nr_dmg_die; /*0 is 1d5*/
    int8_t dmg_addition;
    uint8_t range;
    uint8_t rof[WEAPON_ROF_SETTING_MAX];
    enum wpn_rof_setting rof_set;
    uint8_t magazine_sz;
    uint8_t magazine_left;
    uint8_t penetration;
    enum item_ammo_type ammo_type;
    uint32_t ammo_used_template_id;
    bitfield_t special_quality;
    bitfield_t upgrades;
    bitfield_t wpn_talent;
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

    uint32_t uid;
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
    unsigned long age;
    bool dropable;

    enum item_owner owner_type;
    union owner_union {
        struct dc_map_entity *owner_map_entity;
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

void itmlst_items_list_init(void);
void itmlst_items_list_exit(void);
struct itm_item *itmlst_get_next_item(struct itm_item *prev);
struct itm_item *itmlst_item_by_uid(uint32_t uid);

struct itm_item *itm_generate(enum item_types type);
struct itm_item *itm_create(int template_id);
void itm_destroy(struct itm_item *item);
bool itm_verify_item(struct itm_item *item);
bool itm_insert_item(struct itm_item *item, struct dc_map *map, coord_t *pos);
bool itm_remove_item(struct itm_item *item, struct dc_map *map, coord_t *pos);
coord_t itm_get_pos(struct itm_item *item);

bool wpn_is_type(struct itm_item *item, enum item_weapon_type type);
bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat);
bool wpn_ranged_weapon_rof_set_check(struct itm_item *item);
bool wpn_ranged_next_rof_set(struct itm_item *item);

bool wearable_is_type(struct itm_item *item, enum item_wearable_type type);
bool ammo_is_type(struct itm_item *item, enum item_ammo_type type);
bool tool_is_type(struct itm_item *item, enum item_tool_type type);
#endif /*ITEMS_H_*/

