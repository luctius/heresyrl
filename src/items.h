#ifndef ITEMS_H_
#define ITEMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"

enum item_types {
    ITEM_TYPE_WEARABLE,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_FOOD,
    ITEM_TYPE_AMMO,
    ITEM_TYPE_TOOL,
    ITEM_TYPE_MAX,
    ITEM_TYPE_RANDOM,
};

enum item_attributes {
    ITEM_ATTRIBUTE_NONE         = (0),
    ITEM_ATTRIBUTE_STACKABLE    = (1<<0),
    ITEM_ATTRIBUTE_WARPED       = (1<<1),
    ITEM_ATTRIBUTE_EXPLOSIVE    = (1<<2),
    ITEM_ATTRIBUTE_UNSTABLE     = (1<<3),
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
    WEAPON_TYPE_CREATURE,
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
    AMMO_TYPE_PISTOL,
    AMMO_TYPE_BASIC,
    AMMO_TYPE_ROCKET,
    AMMO_TYPE_FLAMER,
    AMMO_TYPE_SHOTGUN,
    AMMO_TYPE_LAS_PACK,
    AMMO_TYPE_PLASMA_PACK,
    AMMO_TYPE_MAX,
    AMMO_TYPE_RANDOM,
};

enum item_wearable_type {
    WEARABLE_TYPE_FEET,
    WEARABLE_TYPE_LEGS,
    WEARABLE_TYPE_CHEST,
    WEARABLE_TYPE_SHOULDERS,
    WEARABLE_TYPE_ARMS,
    WEARABLE_TYPE_HANDS,
    WEARABLE_TYPE_FINGERS,
    WEARABLE_TYPE_HEAD,
    WEARABLE_TYPE_FACE,
    WEARABLE_TYPE_BACK,
    WEARABLE_TYPE_ARMOUR_CHEST,
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
enum weapon_special_quality {
    WEAPON_SPEC_QUALITY_NONE        = (0<<0),
    WEAPON_SPEC_QUALITY_RELIABLE    = (1<<0),
};

enum item_weapon_category {
    WEAPON_CATEGORY_PISTOL,
    WEAPON_CATEGORY_BASIC,
    WEAPON_CATEGORY_HEAVY,
    WEAPON_CATEGORY_1H_MELEE,
    WEAPON_CATEGORY_2H_MELEE,
    WEAPON_CATEGORY_THROWN,
    WEAPON_CATEGORY_MELEE_THROWN,
    WEAPON_CATEGORY_MAX,
    WEAPON_CATEGORY_RANDOM,
};

enum wpn_rof_setting {
    WPN_ROF_SETTING_SINGLE,
    WPN_ROF_SETTING_SEMI,
    WPN_ROF_SETTING_AUTO,
    WPN_ROF_SETTING_MAX,
};


struct item_weapon_specific {
    enum item_weapon_type weapon_type;
    enum item_weapon_category weapon_category;
    enum weapon_dmg_type dmg_type;
    uint8_t nr_dmg_die; /*0 is 1d5*/
    uint8_t dmg_addition;
    uint8_t range;
    uint8_t rof[WPN_ROF_SETTING_MAX];
    uint8_t magazine_sz;
    uint8_t magazine_left;
    uint8_t reload_delay;
    uint8_t penetration;
    enum weapon_special_quality special_quality;
    enum item_ammo_type ammo_type;
    bool jammed;
};

struct item_wearable_specific {
    enum item_wearable_type wearable_type;
    uint8_t damage_reduction;
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
    uint8_t energy;
    uint8_t energy_left;
};

enum item_owner {
    ITEM_OWNER_NONE,
    ITEM_OWNER_MAP,
    ITEM_OWNER_MONSTER,
    ITEM_OWNER_MAX,
};

struct itm_item {
    uint32_t id;
    uint32_t list_id;
    enum item_types item_type;
    /*enum item_material material;*/
    enum item_availability availability;
    enum item_quality quality;
    enum item_attributes attributes;
    unsigned long age;
    int weight;
    int cost;
    const char *sd_name;
    const char *ld_name;
    const char *description;
    char icon;
    int icon_attr;
    uint8_t use_delay;
    uint8_t stacked_quantity;
    uint8_t max_quantity;
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
};

struct itm_item_list_entry {
    struct itm_item item;
    LIST_ENTRY(itm_item_list_entry) entries;
};

void itmlst_items_list_init(void);
void itmlst_items_list_exit(void);
struct itm_item *itmlst_get_next_item(struct itm_item *prev);

struct itm_item *itm_generate(enum item_types type);
struct itm_item *itm_create_specific(int idx);
struct itm_item *itm_create_type(enum item_types type, int specific_id);
void itm_destroy(struct itm_item *item);
bool itm_insert_item(struct itm_item *item, struct dc_map *map, coord_t *pos);
bool itm_remove_item(struct itm_item *item, struct dc_map *map, coord_t *pos);
coord_t itm_get_pos(struct itm_item *item);

bool wpn_is_type(struct itm_item *item, enum item_weapon_type type);
bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat);

#endif /*ITEMS_H_*/

