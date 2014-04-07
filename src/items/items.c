#include <assert.h>
#include <string.h>
#include <sys/queue.h>

#include "items.h"
#include "items_static.h"
#include "random.h"
#include "tiles.h"
#include "inventory.h"
#include "fight.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"

struct itm_item_list_entry {
    struct itm_item item;
    LIST_ENTRY(itm_item_list_entry) entries;
};

static LIST_HEAD(items_list, itm_item_list_entry) items_list_head;
static bool items_list_initialised = false;

#include "items_static.c"

void itmlst_items_list_init(void) {
    if (items_list_initialised == false) {
        items_list_initialised = true;
        LIST_INIT(&items_list_head);
    }
}

void itmlst_items_list_exit(void) {
    struct itm_item_list_entry *e = NULL;
    while (items_list_head.lh_first != NULL) {
        e = items_list_head.lh_first;
        LIST_REMOVE(items_list_head.lh_first, entries);
        free(e);
    }
        items_list_initialised = false;
}

struct itm_item *itmlst_get_next_item(struct itm_item *prev) {
    if (prev == NULL) {
        if (items_list_head.lh_first != NULL) return &items_list_head.lh_first->item;
        return NULL;
    }
    struct itm_item_list_entry *ile = container_of(prev, struct itm_item_list_entry, item);
    if (ile == NULL) return NULL;
    return &ile->entries.le_next->item;
}

struct itm_item *itmlst_item_by_uid(uint32_t item_uid) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.lh_first;

    while (ie != NULL) {
        if (item_uid == ie->item.uid) return &ie->item;
        ie = ie->entries.le_next;
    }
    return NULL;
}

static uint32_t itmlst_next_id(void) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.lh_first;
    uint32_t uid = 1;

    while (ie != NULL) {
        if (uid <= ie->item.uid) uid = ie->item.uid+1;
        ie = ie->entries.le_next;
    }
    return uid;
}

struct itm_item *itm_generate(enum item_types type) {
    if (items_list_initialised == false) itmlst_items_list_init();
    return NULL;
}

#define ITEM_PRE_CHECK (11867)
#define ITEM_POST_CHECK (8708)

struct itm_item *itm_create(int template_id) {
    if (template_id >= (int) ARRAY_SZ(static_item_list)) return NULL;
    if (items_list_initialised == false) itmlst_items_list_init();

    struct itm_item_list_entry *i = calloc(1, sizeof(struct itm_item_list_entry) );
    if (i == NULL) return NULL;

    memcpy(&i->item, &static_item_list[template_id], sizeof(static_item_list[template_id]));
    LIST_INSERT_HEAD(&items_list_head, i, entries);
    i->item.item_pre = ITEM_PRE_CHECK;
    i->item.item_post = ITEM_POST_CHECK;
    i->item.icon_attr = get_colour(i->item.icon_attr);
    i->item.uid = itmlst_next_id();
    i->item.owner_type = ITEM_OWNER_NONE;

    return &i->item;
}

void itm_destroy(struct itm_item *item) {
    struct itm_item_list_entry *ile = container_of(item, struct itm_item_list_entry, item);

    LIST_REMOVE(ile, entries);
    free(ile);
}

bool itm_verify_item(struct itm_item *item) {
    assert(item != NULL);
    assert(item->item_pre == ITEM_PRE_CHECK);
    assert(item->item_post == ITEM_POST_CHECK);
    return true;
}

bool itm_insert_item(struct itm_item *item, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (itm_verify_item(item) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;
    if (item->dropable == false) return false;

    struct dm_map_entity *target = dm_get_map_me(pos, map);
    if (inv_has_item(target->inventory, item) == false) {
        if (inv_add_item(target->inventory, item) == true) {
            item->owner_type = ITEM_OWNER_MAP;
            item->owner.owner_map_entity = target;
            retval = true;

            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "itm", "Inserting item %s (%c) [uid:%d, tid:%d] to (%d,%d)", 
                    item->sd_name, item->icon, item->uid, item->template_id, pos->x, pos->y);
        }
    }

    return retval;
}

bool itm_remove_item(struct itm_item *item, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (itm_verify_item(item) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dm_map_entity *target = dm_get_map_me(pos, map);
    if (inv_has_item(target->inventory, item) == true) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "itm", "removed (%d,%d)", pos->x, pos->y);
        if (inv_remove_item(target->inventory, item) ) {
            item->owner_type = ITEM_OWNER_NONE;
            item->owner.owner_map_entity = NULL;
            retval = true;
        }
    }

    return retval;
}

coord_t itm_get_pos(struct itm_item *item) {
    if (itm_verify_item(item) == false) return cd_create(0,0);

    switch (item->owner_type) {
        case ITEM_OWNER_MAP:        return item->owner.owner_map_entity->pos;
        case ITEM_OWNER_MONSTER:    return item->owner.owner_monster->pos;
        default:
            return cd_create(0,0);
    }
}
bool itm_has_quality(struct itm_item *item, enum item_quality q) {
    if (itm_verify_item(item) == false) return false;
    return (item->quality == q);
}

bool itm_energy_action(struct itm_item *item, struct dm_map *map) {
    if (itm_verify_item(item) == false) return false;
    if (item->energy > 0) return false;

    /*
       this should be given it's own place, but for now lets hack it.
     */
    switch(item->item_type) {
        case ITEM_TYPE_TOOL:
            if (tool_is_type(item, TOOL_TYPE_LIGHT) ) {
                item->specific.tool.energy_left = 0;
                item->specific.tool.lit = false;
                msg("A %s switches off as it runs out of juice.", item->sd_name);
            }
        case ITEM_TYPE_WEAPON: {
                if (wpn_is_catergory(item, WEAPON_CATEGORY_THROWN_GRENADE) ) {
                    fght_explosion(gbl_game->random, item, map);
                    coord_t pos = itm_get_pos(item);
                    if (itm_remove_item(item, map, &pos) ) {
                        itm_destroy(item);
                    }
                }
            }
        case ITEM_TYPE_WEARABLE:
        case ITEM_TYPE_AMMO:
        case ITEM_TYPE_FOOD:
        default: return false;
    }

    return true;
}

/* change the item's energy by this much, true if succefull. */
bool itm_change_energy(struct itm_item *item, int energy) {
    if (itm_verify_item(item) == false) return false;
    if (item->energy_action == false) return false;

    item->energy += energy;
    return true;
}

int itm_get_energy(struct itm_item *item) {
    if (itm_verify_item(item) == false) return -1;
    if (item->energy_action == false) return -1;
    return item->energy;
}

bool wpn_is_type(struct itm_item *item, enum item_weapon_type type) {
    if (item == NULL) return false;
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (item->specific.weapon.weapon_type == type) return true;
    return false;
}

bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (item->specific.weapon.weapon_category == cat) return true;
    return false;
}

bool wpn_ranged_weapon_rof_set_check(struct itm_item *item) {
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    if (item->specific.weapon.rof[item->specific.weapon.rof_set] > 0) return true;
    return false;
}

bool wpn_ranged_next_rof_set(struct itm_item *item) {
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    do {
        item->specific.weapon.rof_set++;
        item->specific.weapon.rof_set %= WEAPON_ROF_SETTING_MAX;
    } while (wpn_ranged_weapon_rof_set_check(item) == false);
    return true;
}

bool wpn_has_spc_quality(struct itm_item *item, enum weapon_special_quality q) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    return ( (item->specific.weapon.special_quality & q) > 0);
}

bool wbl_is_type(struct itm_item *item, enum item_wearable_type type) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEARABLE) return false;
    if (item->specific.wearable.wearable_type == type) return true;
    return false;
}

bool wbl_has_spc_quality(struct itm_item *item, enum wearable_special_quality q) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEARABLE) return false;
    return ( (item->specific.wearable.special_quality & q) > 0);
}

bool ammo_is_type(struct itm_item *item, enum item_ammo_type type) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_AMMO) return false;
    if (item->specific.ammo.ammo_type == type) return true;
    return false;
}

bool tool_is_type(struct itm_item *item, enum item_tool_type type) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_TOOL) return false;
    if (item->specific.tool.tool_type == type) return true;
    return false;
}
