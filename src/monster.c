#include <stdlib.h>
#include <stddef.h>
#include <ncurses.h>

#include "dungeon_creator.h"
#include "monster.h"
#include "items.h"
#include "tiles.h"
#include "inventory.h"

static LIST_HEAD(monster_list, msr_monster_list_entry) monster_list_head;
static bool monster_list_initialised = false;

struct msr_monster_list_entry {
    struct msr_monster monster;
    LIST_ENTRY(msr_monster_list_entry) entries;
};

void msrlst_monster_list_init(void) {
    if (monster_list_initialised == false) {
        monster_list_initialised = true;
        LIST_INIT(&monster_list_head);
    }
}

void msrlst_monster_list_exit(void) {
    struct msr_monster_list_entry *e = NULL;
    while (monster_list_head.lh_first != NULL) {
        e = monster_list_head.lh_first;
        LIST_REMOVE(monster_list_head.lh_first, entries);
        free(e);
    }
    monster_list_initialised = false;
}

struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev) {
    if (prev == NULL) {
        if (monster_list_head.lh_first != NULL) return &monster_list_head.lh_first->monster;
        return NULL;
    }
    struct msr_monster_list_entry *mle = container_of(prev, struct msr_monster_list_entry, monster);
    if (mle == NULL) return NULL;
    return &mle->entries.le_next->monster;
}

struct msr_monster *msr_create(enum msr_race race) {
    if (monster_list_initialised == false) msrlst_monster_list_init();

    struct msr_monster_list_entry *m = malloc(sizeof(struct msr_monster_list_entry) );
    if (m != NULL) {

        m->monster.pos = cd_create(0,0);
        m->monster.icon = 'm';
        m->monster.icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL);
        m->monster.visibility = 100;
        m->monster.is_player = false;
        m->monster.inventory = NULL;
        m->monster.sd_name = "";
        m->monster.ld_name = "";
        m->monster.description = "";
        m->monster.gender = MSR_GENDER_MALE;
        m->monster.race = race;

        switch (race) {
            case MSR_RACE_HUMAN:
                m->monster.inventory = inv_init(inv_loc_human);
                break;
            default:
                free(m);
                return NULL;
                break;
        }

        LIST_INSERT_HEAD(&monster_list_head, m, entries);
        return &m->monster;
    }
    return NULL;
}

void msr_die(struct msr_monster *monster, struct dc_map *map) {
    if (monster == NULL) return;
    if (map == NULL) return;
    struct msr_monster_list_entry *target_mle = container_of(monster, struct msr_monster_list_entry, monster);

    msr_remove_monster(monster, map);

    LIST_REMOVE(target_mle, entries);
    free(target_mle);
}

bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dc_map_entity *me_future = sd_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (me_future->monster == NULL) {
            me_future->monster = monster;
            monster->pos = *pos;
            retval = true;
        }
    }

    return retval;
}

bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;
    if (cd_equal(&monster->pos, pos) == true ) return false;

    struct dc_map_entity *me_current = sd_get_map_me(&monster->pos, map);
    struct dc_map_entity *me_future = sd_get_map_me(pos, map);

    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        int x_diff = monster->pos.x - pos->x;
        int y_diff = monster->pos.y - pos->y;
        if (abs(x_diff) + abs(y_diff) > 1) return false; /*Speed of one for now*/
        coord_t mon_pos_new = cd_add(&monster->pos, pos);

        if (msr_insert_monster(monster, map, pos) == true) {
            me_current->monster = NULL;
            retval = true;
        }
        else if (msr_move_monster(me_future->monster, map, &mon_pos_new) ) {
            retval = msr_move_monster(monster, map, pos);
        }
    }

    return retval;
}
bool msr_give_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == true) return false;

    if (inv_add_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_MONSTER;
        item->owner.owner_monster = monster;
        You("picked up %s.", item->ld_name);
    }
    return true;
}

bool msr_use_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (monster->inventory == NULL) {
        Your("inventory is empty.");
        return false;
    }
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (item->item_type == ITEM_TYPE_TOOL && item->specific.tool.tool_type == ITEM_TOOL_TYPE_LIGHT) {
        item->specific.tool.lit = true;
        You("light %s.", item->ld_name);
    }
    return true;
}

bool msr_remove_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (inv_remove_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_NONE;
        item->owner.owner_monster = NULL;
    }
    return true;
}

int msr_get_near_sight_range(struct msr_monster *monster) {
    if (monster == NULL) return -1;
    return 2;
}

int msr_get_far_sight_range(struct msr_monster *monster) {
    if (monster == NULL) return -1;
    return 4;
}

bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;

    struct dc_map_entity *me_current = sd_get_map_me(&monster->pos, map);
    if (me_current->monster == monster) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Monster", "removed (%d,%d)", monster->pos.x, monster->pos.y);
        me_current->monster = NULL;
        monster->pos = cd_create(0,0);
        retval = true;
    }

    return retval;
}

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr) {
    if (monster == NULL) return -1;
    return 30;
}

char *msr_gender_string(struct msr_monster *monster) {
    if (monster == NULL) return "nil";
    switch (monster->gender) {
        case MSR_GENDER_MALE: return "Male";
        case MSR_GENDER_FEMALE: return "Female";
        case MSR_GENDER_IT: return "None";
    }
    return "";
}

