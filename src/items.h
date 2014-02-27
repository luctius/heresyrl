#pragma once
#ifndef ITEMS_H_
#define ITEMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "dungeon_creator.h"

struct items_list;
extern struct items_list *items_list_head;

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
    ITEM_ATTRIBUTE_MAX,
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
    ITEM_WEAPON_TYPE_HANDGUN,
    ITEM_WEAPON_TYPE_PISTOL,
    ITEM_WEAPON_TYPE_BASIC,
    ITEM_WEAPON_TYPE_MELEE,
    ITEM_WEAPON_TYPE_THROWN,
    ITEM_WEAPON_TYPE_MELEE_THROWN,
    ITEM_WEAPON_TYPE_MAX,
    ITEM_WEAPON_TYPE_RANDOM,
};

enum item_food_type {
    ITEM_FOOD_TYPE_SOLID,
    ITEM_FOOD_TYPE_LIQUID,
    ITEM_FOOD_TYPE_MAX,
    ITEM_FOOD_TYPE_RANDOM,
};

enum item_tool_type {
    ITEM_TOOL_TYPE_LIGHT,
    ITEM_TOOL_TYPE_CONTAINER,
    ITEM_TOOL_TYPE_MAX,
    ITEM_TOOL_TYPE_RANDOM,
};

enum item_ammo_type {
    ITEM_AMMO_TYPE_ARROW,
    ITEM_AMMO_TYPE_PISTOL,
    ITEM_AMMO_TYPE_BASIC,
    ITEM_AMMO_TYPE_ROCKET,
    ITEM_AMMO_TYPE_FLAMER,
    ITEM_AMMO_TYPE_SHOTGUN,
    ITEM_AMMO_TYPE_LAS_PACK,
    ITEM_AMMO_TYPE_PLASMA_PACK,
    ITEM_AMMO_TYPE_MAX,
    ITEM_AMMO_TYPE_RANDOM,
};

enum item_wearable_type {
    ITEM_WEARABLE_TYPE_FEET,
    ITEM_WEARABLE_TYPE_LEGS,
    ITEM_WEARABLE_TYPE_CHEST,
    ITEM_WEARABLE_TYPE_SHOULDERS,
    ITEM_WEARABLE_TYPE_ARMS,
    ITEM_WEARABLE_TYPE_HANDS,
    ITEM_WEARABLE_TYPE_FINGERS,
    ITEM_WEARABLE_TYPE_HEAD,
    ITEM_WEARABLE_TYPE_FACE,
    ITEM_WEARABLE_TYPE_BACK,
    ITEM_WEARABLE_TYPE_ARMOUR_CHEST,
    ITEM_WEARABLE_TYPE_MAX,
    ITEM_WEARABLE_TYPE_RANDOM,
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

struct item_weapon_specific {
    enum item_weapon_type weapon_type;
    enum weapon_dmg_type dmg_type;
    uint8_t nr_dmg_die; /*0 is 1d5*/
    uint8_t dmg_addition;
    uint8_t range;
    struct rate_of_fire {
        uint8_t rof_single;
        uint8_t rof_semi;
        uint8_t rof_auto;
    } rof;
    uint8_t penetration;
    uint8_t special_quality;
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

struct itm_items {
    uint32_t id;
    enum item_types item_type;
    /*enum item_material material;*/
    enum item_availability availability;
    enum item_quality quality;
    enum item_attributes attributes;
    unsigned long age;
    int weight;
    int cost;
    const char *name;
    char icon;
    uint8_t colour;
    uint8_t use_delay;
    uint8_t stacked_quantity;

    union item_specific {
        struct item_weapon_specific weapon;
        struct item_ammo_specific ammo;
        struct item_food_specific food;
        struct item_tool_specific tool;
        struct item_wearable_specific wearable;
    } specific;
};

struct itm_items_list_entry {
    struct itm_items item;
    LIST_ENTRY(itm_items_list_entry) entries;
};

void itm_items_list_init(void);
void itm_items_list_exit(void);

struct itm_items *itm_generate(enum item_types type);
struct itm_items *itm_create_specific(int idx);
struct itm_items *itm_create_type(enum item_types type, int specific_id);
void itm_destroy(struct itm_items *item);
bool itm_insert_item(struct itm_items *item, struct dc_map *map, int x_togo, int y_togo);
bool itm_remove_item(struct itm_items *item, struct dc_map *map, int x_pos, int y_pos);

#endif /*ITEMS_H_*/

