#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include <float.h>

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
    TAILQ_ENTRY(itm_item_list_entry) entries;
};

static TAILQ_HEAD(items_list, itm_item_list_entry) items_list_head;
static bool items_list_initialised = false;

#include "items_static.c"

#define ITEM_PRE_CHECK (11867)
#define ITEM_POST_CHECK (8708)

void itmlst_items_list_init(void) {
    for (unsigned int i = 0; i < IID_MAX; i++) {
        struct itm_item *item = &static_item_list[i];
        if (item->template_id != i) {
            fprintf(stderr, "Item list integrity check failed! [%d]\n", i);
            fprintf(stderr, "Item: %s.\n", static_item_list[i].sd_name);
            exit(EXIT_FAILURE);
        }
        item->item_pre  = ITEM_PRE_CHECK;
        item->item_post = ITEM_POST_CHECK;
    }

    for (int i = 0; i < IID_MAX; i++) {
        const char *string = itm_descs[i];
        if (string == NULL) {
            fprintf(stderr, "Item description list integrity check failed! (%s [%d])\n", static_item_list[i].sd_name, i);
            exit(EXIT_FAILURE);
        }
    }

    if (items_list_initialised == false) {
        items_list_initialised = true;
        TAILQ_INIT(&items_list_head);
    }
}

void itmlst_items_list_exit(void) {
    struct itm_item_list_entry *e = NULL;
    while (items_list_head.tqh_first != NULL) {
        e = items_list_head.tqh_first;
        TAILQ_REMOVE(&items_list_head, items_list_head.tqh_first, entries);
        free(e);
    }
    items_list_initialised = false;
}

struct itm_item *itmlst_get_next_item(struct itm_item *prev) {
    if (items_list_initialised == false) return NULL;

    if (prev == NULL) {
        if (items_list_head.tqh_first != NULL) return &items_list_head.tqh_first->item;
        return NULL;
    }
    struct itm_item_list_entry *ile = container_of(prev, struct itm_item_list_entry, item);
    if (ile == NULL) return NULL;
    return &ile->entries.tqe_next->item;
}

struct itm_item *itmlst_item_by_uid(uint32_t item_uid) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.tqh_first;

    while (ie != NULL) {
        if (item_uid == ie->item.uid) return &ie->item;
        ie = ie->entries.tqe_next;
    }
    return NULL;
}

static uint32_t itmlst_next_id(void) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.tqh_first;
    uint32_t uid = 1;

    while (ie != NULL) {
        if (uid <= ie->item.uid) uid = ie->item.uid+1;
        ie = ie->entries.tqe_next;
    }
    return uid;
}

static bool itm_is_in_group(struct itm_item *item, enum item_group ig) {
    switch(ig) {
        case ITEM_GROUP_NONE: 
            return false;
        case ITEM_GROUP_1H_MELEE:
            return wpn_is_catergory(item, WEAPON_CATEGORY_1H_MELEE);
        case ITEM_GROUP_2H_MELEE:
            return wpn_is_catergory(item, WEAPON_CATEGORY_2H_MELEE);
        case ITEM_GROUP_ARMOUR:
            return wbl_is_type(item, WEARABLE_TYPE_ARMOUR);
        case ITEM_GROUP_POTION: break;
        case ITEM_GROUP_RANGED:
            return wpn_is_type(item, WEAPON_TYPE_RANGED);
        case ITEM_GROUP_SHIELD: break;
        case ITEM_GROUP_TRAP: break;
        case ITEM_GROUP_ANY: return true;
        default: break;
    }
    return false;
}

int itm_spawn(double roll, int level, enum item_group ig) {
    int sz = ARRAY_SZ(static_item_list);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0.f;

    if (ig == ITEM_GROUP_NONE) return IID_NONE;
    int idx = IID_NONE;

    cumm_prob_arr[0] = DBL_MAX;
    for (int i = IID_NONE; i < sz; i++) {
        if ( (level >= static_item_list[i].spawn_level) &&
             (itm_is_in_group(&static_item_list[i], ig) ) ) {
            sum += static_item_list[i].spawn_weight;
            cumm_prob_arr[i] = 0.f;
        }
        else cumm_prob_arr[i] = DBL_MAX;
    }

    if (sum == 0) {
        printf("bla");
    }

    double cumm = 0.f;
    for (int i = IID_NONE; i < sz; i++) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        prob_arr[i] = static_item_list[i].spawn_weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }

    if (cumm == 0) {
        printf("bla");
    }

    for (int i = sz-1; i > IID_NONE; i--) {
        if (cumm_prob_arr[i] == DBL_MAX) continue;
        if (roll < cumm_prob_arr[i]) idx = i;
    }

    if (idx == IID_NONE) {
        printf("bla");
    }

    return idx;
}

struct itm_item *itm_create(int template_id) {
    if (template_id <= IID_NONE) return NULL;
    if (template_id >= IID_MAX) return NULL;
    if (template_id >= (int) ARRAY_SZ(static_item_list)) return NULL;
    if (items_list_initialised == false) itmlst_items_list_init();

    struct itm_item_list_entry *i = malloc(sizeof(struct itm_item_list_entry) );
    assert(i != NULL);

    memcpy(&i->item, &static_item_list[template_id], sizeof(static_item_list[template_id]));
    TAILQ_INSERT_TAIL(&items_list_head, i, entries);
    i->item.item_pre    = ITEM_PRE_CHECK;
    i->item.item_post   = ITEM_POST_CHECK;
    i->item.icon_attr   = get_colour(i->item.icon_attr);
    i->item.uid         = itmlst_next_id();
    i->item.owner_type  = ITEM_OWNER_NONE;
    i->item.description = itm_descs[template_id];
    assert(i->item.description != NULL);

    return &i->item;
}

void itm_destroy(struct itm_item *item) {
    struct itm_item_list_entry *ile = container_of(item, struct itm_item_list_entry, item);

    TAILQ_REMOVE(&items_list_head, ile, entries);
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

    struct dm_map_entity *target = dm_get_map_me(pos, map);
    if (inv_has_item(target->inventory, item) == false) {
        if (TILE_HAS_ATTRIBUTE(target->tile, TILE_ATTR_TRAVERSABLE) ) {
            /* non dropable items should not be able to be picked up by the player. */
            if (item->dropable == false) return false;
        }

        if (inv_add_item(target->inventory, item) == true) {
            item->owner_type = ITEM_OWNER_MAP;
            item->owner.owner_map_entity = target;
            retval = true;

            lg_debug("Inserting item %s (%c) [uid:%d, tid:%d] to (%d,%d)", 
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
        lg_debug("removed (%d,%d)", pos->x, pos->y);
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

bool itm_is_type(struct itm_item *item, enum item_types type) {
    if (itm_verify_item(item) == false) return false;
    return (item->item_type == type);
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
    if (item->permanent_energy == true) return false;

    item->energy += energy;
    return true;
}

int itm_get_energy(struct itm_item *item) {
    if (itm_verify_item(item) == false) return -1;
    if (item->energy_action == false) return -1;
    return item->energy;
}

bool itm_stack_compatible(struct itm_item *item1, struct itm_item *item2) {
    if (itm_verify_item(item1) == false) return false;
    if (itm_verify_item(item2) == false) return false;

    if (item1->template_id != item2->template_id) return false;
    if (item1->item_type != item2->item_type) return false;
    if (item1->quality != item2->quality) return false;
    if (item1->weight != item2->weight) return false;
    if (item1->cost != item2->cost) return false;
    if (item1->sd_name != item2->sd_name) return false;
    if (item1->ld_name != item2->ld_name) return false;
    if (item1->description != item2->description) return false;
    if (item1->icon != item2->icon) return false;
    if (item1->icon_attr != item2->icon_attr) return false;
    if (item1->use_delay != item2->use_delay) return false;
    if (item1->max_quantity != item2->max_quantity) return false;

    switch(item1->item_type) {
        case ITEM_TYPE_TOOL:
            if (item1->specific.tool.tool_type != item2->specific.tool.tool_type) return false;
            if (item1->specific.tool.energy_left != item2->specific.tool.energy_left) return false;
            if (item1->specific.tool.energy != item2->specific.tool.energy) return false;
            break;
        case ITEM_TYPE_WEAPON:
            if (item1->specific.weapon.upgrades != item2->specific.weapon.upgrades) return false;
            if (item1->specific.weapon.jammed != item2->specific.weapon.jammed) return false;
            break;
        case ITEM_TYPE_WEARABLE:
            if (item1->specific.wearable.wearable_type != item2->specific.wearable.wearable_type) return false;
            if (item1->specific.wearable.locations != item2->specific.wearable.locations) return false;
            if (item1->specific.wearable.damage_reduction != item2->specific.wearable.damage_reduction) return false;
            if (item1->specific.wearable.special_quality != item2->specific.wearable.special_quality) return false;
            break;
        case ITEM_TYPE_AMMO: 
            if (item1->specific.ammo.ammo_type != item2->specific.ammo.ammo_type) return false;
            if (item1->specific.ammo.upgrades != item2->specific.ammo.upgrades) return false;
            if (item1->specific.ammo.convey_status_effect != item2->specific.ammo.convey_status_effect) return false;
            break;
        case ITEM_TYPE_FOOD:
            if (item1->specific.food.food_type != item2->specific.food.food_type) return false;
            if (item1->specific.food.nutrition != item2->specific.food.nutrition) return false;
            if (item1->specific.food.nutrition_left != item2->specific.food.nutrition_left) return false;
            if (item1->specific.food.convey_status_effect != item2->specific.food.convey_status_effect) return false;
            break;
        default: return false;
    }

    return true;
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
    return ( (item->specific.weapon.special_quality & bf(q) ) > 0);
}

bool wpn_uses_ammo(struct itm_item *item) {
    if (itm_verify_item(item) == false) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    return ( (item->specific.weapon.ammo_type != AMMO_TYPE_NONE) );
}

enum item_ammo_type wpn_get_ammo_type(struct itm_item *item) {
    if (itm_verify_item(item) == false) return AMMO_TYPE_NONE;
    if (item->item_type != ITEM_TYPE_WEAPON) return AMMO_TYPE_NONE;
    return item->specific.weapon.ammo_type;
}

enum item_ids wpn_get_ammo_used_id(struct itm_item *item) {
    if (itm_verify_item(item) == false) return IID_NONE;
    if (item->item_type != ITEM_TYPE_WEAPON) return IID_NONE;
    return item->specific.weapon.ammo_used_template_id;
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
    return ( (item->specific.wearable.special_quality & bf(q) ) > 0);
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

const char *itm_you_use_desc(struct itm_item *item) {
    if (itm_verify_item(item) == false) return NULL;
    int cnt = 0;
    for (int i = 0; i < (int) ARRAY_SZ(item->you_use_desc); i++) {
        if (item->you_use_desc[i] != NULL) cnt++;
    }
    int idx = random_int32(gbl_game->random) % (cnt-1);
    return item->you_use_desc[idx];
}

const char *itm_msr_use_desc(struct itm_item *item) {
    if (itm_verify_item(item) == false) return NULL;
    int cnt = 0;
    for (int i = 0; i < (int) ARRAY_SZ(item->msr_use_desc); i++) {
        if (item->msr_use_desc[i] != NULL) cnt++;
    }
    int idx = random_int32(gbl_game->random) % (cnt-1);
    return item->msr_use_desc[idx];
}

const char *itm_quality_string(struct itm_item *item) {
    if (itm_verify_item(item) == false) return NULL;
    if (item->quality >= ITEM_QLTY_MAX) return NULL;
    return item_quality_strings[item->quality];
}

const char *itm_upgrades_string(struct itm_item *item) {
    if (itm_verify_item(item) == false) return NULL;
    if (item->quality >= ITEM_QLTY_MAX) return NULL;
    return item_quality_strings[item->quality];
}

const char *wpn_ammo_string(enum item_ammo_type iat) {
    if (iat >= AMMO_TYPE_MAX) return NULL;
    return ammo_type_strings[iat];
}

const char *wpn_spec_quality_name(enum weapon_special_quality spq) {
    if (spq >= WPN_SPCQLTY_MAX) return NULL;
    return wpn_spcqlty_name[spq];
}

const char *wpn_spec_quality_description(enum weapon_special_quality spq) {
    if (spq >= WPN_SPCQLTY_MAX) return NULL;
    return wpn_spcqlty_desc[spq];
}

const char *wbl_spec_quality_name(enum wearable_special_quality spq) {
    if (spq >= WBL_SPCQLTY_MAX) return NULL;
    return wbl_spcqlty_name[spq];
}

const char *wbl_spec_quality_description(enum wearable_special_quality spq) {
    if (spq >= WBL_SPCQLTY_MAX) return NULL;
    return wbl_spcqlty_desc[spq];
}

