#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include <ncurses.h>
#include <assert.h>

#include "monster.h"
#include "game.h"
#include "random.h"
#include "tiles.h"
#include "turn_tick.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "dungeon/dungeon_map.h"

static LIST_HEAD(monster_list, msr_monster_list_entry) monster_list_head;
static bool monster_list_initialised = false;

#include "monster_static.c"

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
        inv_exit(e->monster.inventory);
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

static uint32_t msrlst_next_id(void) {
    if (monster_list_initialised == false) return false;
    struct msr_monster_list_entry *me = monster_list_head.lh_first;
    uint32_t uid = 1;

    while (me != NULL) {
        if (uid <= me->monster.uid) uid = me->monster.uid+1;
        me = me->entries.le_next;
    }
    return uid;
}

#define MONSTER_PRE_CHECK (10477)
#define MONSTER_POST_CHECK (10706)
static struct itm_item *msr_unarmed_weapon(struct msr_monster *monster);

struct msr_monster *msr_create(uint32_t template_id) {
    if (monster_list_initialised == false) msrlst_monster_list_init();
    if (template_id >= (int) ARRAY_SZ(static_monster_list)) return NULL;
    struct msr_monster *template_monster = template_monster = &static_monster_list[template_id];

    struct msr_monster_list_entry *m = calloc(1,sizeof(struct msr_monster_list_entry) );
    if (m != NULL) {
        memcpy(&m->monster, template_monster, sizeof(struct msr_monster) );
        m->monster.controller.controller_ctx = NULL;
        m->monster.controller.controller_cb = NULL;
        m->monster.pos = cd_create(0,0);
        m->monster.uid = msrlst_next_id();
        m->monster.energy = TT_ENERGY_FULL;
        m->monster.faction = 1;
        m->monster.inventory = NULL;

        m->monster.monster_pre = MONSTER_PRE_CHECK;
        m->monster.monster_post = MONSTER_POST_CHECK;

        switch (m->monster.race) {
            case MSR_RACE_HUMAN:
                m->monster.inventory = inv_init(inv_loc_human);
                break;
            case MSR_RACE_BEAST:
            case MSR_RACE_DOMESTIC:
                m->monster.inventory = inv_init(inv_loc_animal);
                break;
            default:
                free(m);
                assert(false);
                return NULL;
                break;
        }

        if (inv_loc_empty(m->monster.inventory, INV_LOC_CREATURE_WIELD1) ) {
            struct itm_item *item = msr_unarmed_weapon(&m->monster);
            if (inv_add_item(m->monster.inventory, item) == true) {
                assert(inv_move_item_to_location(m->monster.inventory, item, INV_LOC_CREATURE_WIELD1) );
            }
        }

        LIST_INSERT_HEAD(&monster_list_head, m, entries);
        return &m->monster;
    }
    return NULL;
}

void msr_destroy(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return;
    if (dm_verify_map(map) == false) return;
    struct msr_monster_list_entry *target_mle = container_of(monster, struct msr_monster_list_entry, monster);

    msr_remove_monster(monster, map);
    inv_exit(monster->inventory);

    LIST_REMOVE(target_mle, entries);
    free(target_mle);
}

bool msr_verify_monster(struct msr_monster *monster) {
    assert(monster != NULL);
    assert(monster->monster_pre == MONSTER_PRE_CHECK);
    assert(monster->monster_post == MONSTER_POST_CHECK);
    assert(inv_verify_inventory(monster->inventory) == true );
    if (monster->dead == true) return false;

    return true;
}

void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller) {
    if (msr_verify_monster(monster) == false) return;
    if (controller == NULL) return;
    memcpy(&monster->controller, controller, sizeof(struct monster_controller) );
}

void msr_clear_controller(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;
    free(monster->controller.controller_ctx);
    memset(&monster->controller, 0x0, sizeof(struct monster_controller) );
}

bool msr_insert_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dm_map_entity *me_future = dm_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (me_future->monster == NULL) {
            me_future->monster = monster;
            monster->pos = *pos;
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "msr", "Inserting monster %s (%c) [uid:%d, tid:%d] to (%d,%d)", 
                        monster->sd_name, monster->icon, monster->uid, monster->template_id, monster->pos.x, monster->pos.y);
            retval = true;
        }
    }

    return retval;
}

bool msr_move_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;
    if (cd_equal(&monster->pos, pos) == true ) return false;

    struct dm_map_entity *me_current = dm_get_map_me(&monster->pos, map);
    struct dm_map_entity *me_future = dm_get_map_me(pos, map);

    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        /*Speed of one for now*/
        if (cd_neighbour(&monster->pos, pos) == false) return false;
        /*coord_t mon_pos_new = cd_add(&monster->pos, pos);*/

        if (msr_insert_monster(monster, map, pos) == true) {
            me_current->monster = NULL;
            retval = true;
        }
        /*
        else if (msr_move_monster(me_future->monster, map, &mon_pos_new) ) {
            retval = msr_move_monster(monster, map, pos);
        }
        */
    }

    return retval;
}
bool msr_give_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == true) return false;

    if (inv_add_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_MONSTER;
        item->owner.owner_monster = monster;
    }
    return true;
}

bool msr_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (inv_remove_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_NONE;
        item->owner.owner_monster = NULL;
    }
    return true;
}

int msr_get_near_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 1.5f) / 10) +1;
}

int msr_get_medium_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 2) / 10) +1;
}

int msr_get_far_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return ( (msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION) * 2.5f) / 10) +1;
}

bool msr_drop_inventory(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(monster->inventory, item) ) != NULL) {
        if (itm_verify_item(item) == true) {
            if (inv_remove_item(monster->inventory, item) ) {
                if (itm_insert_item(item, map, &monster->pos) == false) {
                    itm_destroy(item);
                }
                item = NULL;
            }
        }
    }

    return true;
}

bool msr_remove_monster(struct msr_monster *monster, struct dm_map *map) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct dm_map_entity *me_current = dm_get_map_me(&monster->pos, map);
    if (me_current->monster == monster) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Monster", "removed (%d,%d)", monster->pos.x, monster->pos.y);
        me_current->monster = NULL;
        monster->pos = cd_create(0,0);
        retval = true;
    }

    return retval;
}

int msr_get_energy(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return monster->energy;
}

bool msr_change_energy(struct msr_monster *monster, int energy) {
    if (msr_verify_monster(monster) == false) return false;
    monster->energy += energy;
    if (monster->energy < 0) monster->energy = 0;
    return true;
}

struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (mhl >= MSR_HITLOC_MAX) return NULL;
    struct itm_item *item = NULL;

    switch(mhl) {
        case MSR_HITLOC_CHEST:     item = inv_get_item_from_location(monster->inventory, INV_LOC_CHEST); break;
        case MSR_HITLOC_LEFT_LEG:
        case MSR_HITLOC_RIGHT_LEG: item = inv_get_item_from_location(monster->inventory, INV_LOC_LEGS); break;
        case MSR_HITLOC_LEFT_ARM:
        case MSR_HITLOC_RIGHT_ARM: item = inv_get_item_from_location(monster->inventory, INV_LOC_ARMS); break;
        case MSR_HITLOC_HEAD:      item = inv_get_item_from_location(monster->inventory, INV_LOC_HEAD); break;
        default: break;
    }
    return item;
}

int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return -1;
    if (mhl >= MSR_HITLOC_MAX) return 0;
    struct itm_item *item = msr_get_armour_from_hitloc(monster, mhl);
    int armour = 0;

    if (item != NULL) {
        if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
            armour += item->specific.wearable.damage_reduction;
        }
    }

    /* Add Armour talents here */

    return armour;
}

enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll) {
    if (hit_roll > 99) hit_roll %= 100;

    /* Human hitloc */
    if (hit_roll >= 85) return MSR_HITLOC_LEFT_LEG;
    if (hit_roll >= 70) return MSR_HITLOC_RIGHT_LEG;
    if (hit_roll >= 30) return MSR_HITLOC_CHEST;
    if (hit_roll >= 20) return MSR_HITLOC_LEFT_ARM;
    if (hit_roll >= 10) return MSR_HITLOC_RIGHT_ARM;
    return MSR_HITLOC_HEAD;
}

static bool msr_die(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    You(monster, "died...");
    Monster(monster, "dies.");

    msr_drop_inventory(monster, map);
    msr_remove_monster(monster, map);

    monster->dead = true;
    return true;
}

bool msr_do_dmg(struct msr_monster *monster, int dmg, enum msr_hit_location mhl, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;

    if (dmg > 0) {
        if (monster->cur_wounds > 0) {
            monster->cur_wounds -= dmg;
        }
        if (monster->cur_wounds <= 0) {
            /* do critical hits! */

            /* if dead.. */
            msr_die(monster, map);
        }
        return true;
    }
    return false;
}

bool msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return false;
    int charac = msr_calculate_characteristic(monster, chr);
    assert(charac >= 0);

    return ((random_int32(gbl_game->game_random)%100) < charac);
}

int msr_skill_check(struct msr_monster *monster, enum skills skill, int mod) {
    if (msr_verify_monster(monster) == false) return false;

    /*TODO lookup table for charac <> skill */
    int charac = msr_calculate_characteristic(monster, MSR_CHAR_PERCEPTION);
    assert(charac >= 0);
    lg_print("You characteristic is (%d)", charac);

    lg_print("Check modifier is: %d (%d)", mod, charac + mod);
    charac += mod;

    enum skill_rate r = msr_has_skill(monster, skill);
    switch(r) {
        case MSR_SKILL_RATE_EXPERT:     charac += 20; lg_print("You are an expert (%d)", charac); break;
        case MSR_SKILL_RATE_ADVANCED:   charac += 10; lg_print("You are advanced (%d)", charac); break;
        case MSR_SKILL_RATE_BASIC:      lg_print("You have the skill (%d)", charac); break;
        case MSR_SKILL_RATE_NONE:       charac /= 2; lg_print("You do not have this skill (%d)", charac); break;
        default: assert(false); break;
    }

    uint32_t roll = (random_int32(gbl_game->game_random)%100);
    int result = charac - roll;
    int DoS = result /10;
    if (roll < charac) DoS += 1;

    lg_print("roll %d, result %d, DoS %d", roll, result, DoS);

    return DoS;
}

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX) return -1;
    return monster->characteristic[chr].base_value + (monster->characteristic[chr].advancement * 5);
}

int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX) return -1;
    if (chr == MSR_CHAR_WEAPON_SKILL) return -1;
    if (chr == MSR_CHAR_BALISTIC_SKILL) return -1;
    return ( (monster->characteristic[chr].base_value + (monster->characteristic[chr].advancement * 5)) / 10);
}

enum skill_rate msr_has_skill(struct msr_monster *monster, enum skills skill) {
    enum skill_rate r = MSR_SKILL_RATE_NONE;
    if ((monster->skills[MSR_SKILL_RATE_BASIC]    & skill) > 0) r = MSR_SKILL_RATE_BASIC;
    if ((monster->skills[MSR_SKILL_RATE_ADVANCED] & skill) > 0) r = MSR_SKILL_RATE_ADVANCED;
    if ((monster->skills[MSR_SKILL_RATE_EXPERT]   & skill) > 0) r = MSR_SKILL_RATE_EXPERT;
    return r;
}

const char *msr_gender_string(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return "nil";
    switch (monster->gender) {
        case MSR_GENDER_MALE: return "Male";
        case MSR_GENDER_FEMALE: return "Female";
        case MSR_GENDER_IT: return "None";
        default: break;
    }
    return "";
}

bool msr_weapons_check(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    if (monster->wpn_sel >= MSR_WEAPON_SELECT_MAX) return false;

    struct inv_inventory *inv = monster->inventory;

    /* check if the monster has any weapons */
    if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) && (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
    }

    /* If we have a single hand, test that for emptiness and weaponness. */
    if (monster->wpn_sel== MSR_WEAPON_SELECT_OFF_HAND) {
        /* if location is unsupported by this monster*/
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;
        /* if the location is empty */
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        /* if there is no weapon at that location*/
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        /* or if it is the same as the mainhand weapon */
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if ( (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_BASIC) == false) &&
             (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_HEAVY) == false) &&
             (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_2H_MELEE) == false) ) {
            return false;
        }
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
        if (inv_support_location(inv, INV_LOC_CREATURE_WIELD1) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1)->item_type != ITEM_TYPE_WEAPON) return false;
    }
    else return false;

    return true;
}

bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type) {
    if (msr_weapons_check(monster) == false) return false;
    if (type >= WEAPON_TYPE_MAX) return false;
    struct inv_inventory *inv = monster->inventory;

    if (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
         if ( (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) &&
              (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1), type) == false) return false;
    }
    else return false;


    return true;
}

bool msr_weapon_next_selection(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    if ( (inv_loc_empty(monster->inventory, INV_LOC_MAINHAND_WIELD) == true) &&
         (inv_loc_empty(monster->inventory, INV_LOC_OFFHAND_WIELD) == true) ) return false;

    do {
        monster->wpn_sel++;
        monster->wpn_sel %= MSR_WEAPON_SELECT_MAX;
    } while (msr_weapons_check(monster) == false);
    return true;
}

static struct itm_item *msr_unarmed_weapon(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    struct itm_item *item = NULL;
    
    switch (monster->race) {
        case MSR_RACE_HUMAN:
            item = itm_create(ITEM_ID_HUMAN_UNARMED);
            break;
        case MSR_RACE_BEAST:
            item = itm_create(ITEM_ID_CREATURE_BITE_TRAINED);
            break;
        case MSR_RACE_DOMESTIC:
            item = itm_create(ITEM_ID_CREATURE_BITE_TRAINED);
            break;
        default:
            assert(false);
            break;
    }
    return item;
}

bool msr_has_creature_trait(struct msr_monster *monster,  bitfield_t trait) {
    if (msr_verify_monster(monster) == false) return false;
    return ( (monster->creature_traits & trait) > 0);
}

bool msr_has_talent(struct msr_monster *monster,  bitfield_t talent) {
    if (msr_verify_monster(monster) == false) return false;
    int idx = (talent >> TALENTS_IDX_OFFSET) & 0x0F; 
    return ( (monster->talents[idx] & talent) == talent);
}

bool msr_set_talent(struct msr_monster *monster, bitfield_t talent) {
    if (msr_verify_monster(monster) == false) return false;
    int idx = (talent >> TALENTS_IDX_OFFSET) & 0x0F; 
    monster->talents[idx] |= talent;
    return true;
}

uint8_t msr_get_movement_rate(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    return MIN( (msr_calculate_characteristic_bonus(monster, MSR_CHAR_AGILITY) * 10), 90);
}

const char *msr_ldname(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return "unkown";

    if (monster->is_player) return "you";
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) return "something";
    return monster->ld_name;
}

const char *msr_gender_name(struct msr_monster *monster, bool possesive) {
    if (msr_verify_monster(monster) == false) return "unkown";

    enum msr_gender gender = monster->gender;
    if (monster->gender >= MSR_GENDER_MAX) gender = MSR_GENDER_IT;
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) gender = MSR_GENDER_IT;

    switch(gender) {
        case MSR_GENDER_MALE:        return (possesive) ? "his" : "he"; break;
        case MSR_GENDER_FEMALE:      return (possesive) ? "her" : "she"; break;
        case MSR_GENDER_IT: default: return (possesive) ? "its" : "it"; break;
    }
}