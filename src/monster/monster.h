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

#ifndef MONSTER_H_
#define MONSTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"
#include "enums.h"
#include "ai/ai.h"
#include "monster_static.h"

#include "items/items.h"
#include "items/items_static.h"

#define MSR_NR_DEFAULT_WEAPONS_MAX 3
#define MSR_FATIQUE_RECOVER_DELAY (100)

#define MSR_WEAPON_DAMAGE_INSTA_DEATH (10)

#define MSR_MOVEMENT_MIN 0
#define MSR_MOVEMENT_MAX 9

enum msr_gender {
    MSR_GENDER_MALE,
    MSR_GENDER_FEMALE,
    MSR_GENDER_IT,
    MSR_GENDER_MAX,
};

enum msr_characteristic {
    MSR_CHAR_COMBAT,
    MSR_CHAR_STRENGTH,
    MSR_CHAR_TOUGHNESS,
    MSR_CHAR_AGILITY,
    MSR_CHAR_INTELLIGENCE,
    MSR_CHAR_WILLPOWER,
    MSR_CHAR_PERCEPTION,
    MSR_CHAR_MAX,
};

enum msr_size {
    MSR_SIZE_MINISCULE,
    MSR_SIZE_PUNY,
    MSR_SIZE_SCRAWNY,
    MSR_SIZE_AVERAGE,
    MSR_SIZE_HULKING,
    MSR_SIZE_ENORMOUS,
    MSR_SIZE_MASSIVE,
};

enum msr_weapon_selection {
    MSR_WEAPON_SELECT_MAIN_HAND,
    MSR_WEAPON_SELECT_OFF_HAND,
    MSR_WEAPON_SELECT_DUAL_HAND,
    MSR_WEAPON_SELECT_BOTH_HAND,
    MSR_WEAPON_SELECT_CREATURE1,
    MSR_WEAPON_SELECT_MAX,
};

enum msr_evasions {
    MSR_EVASION_MAIN_HAND,
    MSR_EVASION_OFF_HAND,
    MSR_EVASION_DODGE,
    MSR_EVASION_MAX,
};

enum msr_hit_location {
    MSR_HITLOC_LEFT_LEG,
    MSR_HITLOC_RIGHT_LEG,
    MSR_HITLOC_LEFT_ARM,
    MSR_HITLOC_RIGHT_ARM,
    MSR_HITLOC_BODY,
    MSR_HITLOC_HEAD,
    MSR_HITLOC_MAX,
    MSR_HITLOC_NONE,
};

struct msr_char {
    uint8_t base_value;
    uint8_t advancement;
    int8_t mod;
};

struct monster_controller {
    bool interruptable;  /* this monster should be interrupted if anythin interresting happens near it. */
    bool interrupted;    /* this monster *is* interrupted. */
    struct ai ai;        /* private struct for this monster */
    bool (*controller_cb)(struct msr_monster *monster); /* ai/player callback. */
};

struct msr_monster {
    uint32_t monster_pre;

    /* unique id of this monsters instance*/
    uint32_t uid;

    /* id of the base template of this monster */
    enum msr_ids tid;

    /* current grid position */
    coord_t pos;

    icon_t icon;
    int icon_attr;

    /* true if this is the player*/
    bool is_player;

    /* monster gender*/
    enum msr_gender gender;

    /* unique name for this unique monster, default is NULL, contains also the players name. */
    wchar_t *unique_name;
    const wchar_t *sd_name;
    const wchar_t *ld_name;
    const wchar_t *description;

    struct {
        /* current number of wounds, more is better */
        int8_t curr;

        /* maxmimum number of wounds, more is better */
        int8_t max;

        /* number of wounds added due to career advancements. */
        int8_t added;
    } wounds;

    enum msr_size size;

    /* current number of fate points, not used. */
    uint8_t fate_points;

    /* current number of insanity points, not used. */
    uint8_t insanity_points;

    /* current number of corruption points, not used. */
    uint8_t corruption_points;

    /* current energy of this monster */
    int energy;

    int idle_counter;

    /* bitfield of the monster's talents, divided in tiers */
    bitfield64_t talents[MSR_TALENT_TIER_MAX];

    /*
       skills, divided in basic (has the skill), advanced and expert.
       a skill in advanced should also be in basic, etc.
     */
    bitfield32_t skills[MSR_SKILL_RATE_MAX];

    /* special creature traits. */
    bitfield64_t creature_traits;

    /* base characteric score, +advancement. every advancement worth 5 points for
           basic characteristics and 1 point for secondary characteristics. */
    struct msr_char characteristic[MSR_CHAR_MAX];

    /* ai or player callback structure */
    struct monster_controller controller;

    /* current weapon selection */
    enum msr_weapon_selection wpn_sel;

    /* inventory of this monster. */
    struct inv_inventory *inventory;

    /* status_effects effecting this monster. */
    struct status_effect_list *status_effects;

    int faction;

    /* Monster creation. */
    struct {
        int weight;
        int cost;
        int level;
        int group_chance;

        /* faction this monster belongs to. should probably be a bitfield. */
        bitfield32_t dungeon_locale;
    } spwn;

    enum item_ids crtr_wpn;
    enum item_group def_items[MSR_NR_DEFAULT_WEAPONS_MAX];

    uint32_t monster_post;
};

/* init/exit global monster list*/
void msrlst_monster_list_init(void);
void msrlst_monster_list_exit(void);

/*
   get next monster from global monster list,
   if prev is NULL, return the first monster,
   otherwise return the one after prev.
*/
struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev);

/* retrieves a monster template id based on parameters */
int msr_spawn(int32_t roll, int lvl_min, int lvl_max, enum dm_dungeon_type dt);

/* Equipes the monster with weapons according to its profile. */
void msr_populate_inventory(struct msr_monster *monster, int lvl_min, int lvl_max, struct random *r);

/* create a instance of this monster template. */
struct msr_monster *msr_create(enum msr_ids tid);

/* remove monster from map and global monster list. */
void msr_destroy(struct msr_monster *monster, struct dm_map *map);

/* find a monster with the specified uid. return NULL if not found. */
struct msr_monster *msr_get_monster_by_uid(uint32_t uid);

/* verify monster structure. */
bool msr_verify_monster(struct msr_monster *monster);

/* assign controller structure to this monster*/
void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller);

/* free ai context and set everything to 0. */
void msr_clear_controller(struct msr_monster *monster);

/* inster monster on this map pos, set monster->pos to that pos. */
bool msr_insert_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos);

/* check if we can move to that position, then remove monster from current map pos*/
bool msr_move_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos);

/* remove monster from map, monster->pos = {0,0} */
bool msr_remove_monster(struct msr_monster *monster, struct dm_map *map);

/* put this item in monsters inventory. */
bool msr_give_item(struct msr_monster *monster, struct itm_item *item);

/* remove item from monsters inventory */
bool msr_remove_item(struct msr_monster *monster, struct itm_item *item);

/* roll a d100, 1 is succes, > 1 is DoS. 0 is failure, < 0 is DoF */
int msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr, int mod);

/* roll a d100, 1 is succes, > 1 is DoS. 0 is failure, < 0 is DoF */
int msr_skill_check(struct msr_monster *monster, enum msr_skills skill, int mod);

/* get current characteristic value, including talents. */
int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);

/* get current characteristic bonus ( normally (characteristic / 10) ), including talents */
int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr);

int msr_calculate_skill(struct msr_monster *monster, enum msr_skills skill);

int msr_calculate_fatique(struct msr_monster *monster);

int msr_calculate_carrying_capacity(struct msr_monster *monster);

/* get the current worn armour on that location, or NULL */
struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl);

/* get the damage reduction of that hit location*/
int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location hitloc);

bool msr_can_use_evasion(struct msr_monster *monster, enum msr_evasions evasion);
bool msr_use_evasion(struct msr_monster *monster, struct msr_monster *attacker, struct itm_item *atk_wpn, enum msr_evasions evasion, int to_hit_DoS, int mod);

/* do damage to that hit location, including critical hits, and handle the first part of monster death. */
bool msr_do_dmg(struct msr_monster *monster, const wchar_t *origin, int dmg, enum dmg_type dmg_type, enum msr_hit_location mhl);
bool msr_die(struct msr_monster *monster, const wchar_t *origin, struct dm_map *map);

/* given a number between 0 and 99, return the monsters hit location*/
enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll);

/* get current monster energy */
int msr_get_energy(struct msr_monster *monster);

/* change the monster's energy by this much, true if succefull. */
bool msr_change_energy(struct msr_monster *monster, int energy);

struct ai *msr_get_ai_ctx(struct msr_monster *monster);

/*
   get range of the sight.
   far includes medium, medium includes near.
 */
int msr_get_near_sight_range(struct msr_monster *monster);
int msr_get_medium_sight_range(struct msr_monster *monster);
int msr_get_far_sight_range(struct msr_monster *monster);

bool msr_has_creature_trait(struct msr_monster *monster, enum msr_creature_traits trait);
bool msr_set_creature_trait(struct msr_monster *monster, enum msr_creature_traits trait);
bool msr_clr_creature_trait(struct msr_monster *monster, enum msr_creature_traits trait);
enum msr_talent_tiers msr_get_tier(bitfield64_t t);
bool msr_has_talent(struct msr_monster *monster, enum msr_talents talent);
bool msr_set_talent(struct msr_monster *monster, enum msr_talents talent);
bool msr_clr_talent(struct msr_monster *monster, enum msr_talents talent);
enum msr_skill_rate msr_has_skill(struct msr_monster *monster,  enum msr_skills skill);
bool msr_set_skill(struct msr_monster *monster, enum msr_skills skill, enum msr_skill_rate rate);
uint8_t msr_get_movement_rate(struct msr_monster *monster);

/* check if the current weapon selection of this monster is correct. */
bool msr_weapons_check(struct msr_monster *monster);

/* check if the monster wields a weapon of this type */
bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type);

/* cycle weapon selection towards the next correct setting, if possible. */
bool msr_weapon_next_selection(struct msr_monster *monster);

const wchar_t *msr_ldname(struct msr_monster *monster);

/* Male / Female / It */
const wchar_t *msr_gender_string(struct msr_monster *monster);

/* he / his -- her / hers, etc.*/
const wchar_t *msr_gender_name(struct msr_monster *monster, bool possesive);

const wchar_t *msr_char_names(enum msr_characteristic c);
const wchar_t *msr_char_descriptions(enum msr_characteristic c);
const wchar_t *msr_skill_names(enum msr_skills s);
const wchar_t *msr_skill_descriptions(enum msr_skills s);
const wchar_t *msr_skillrate_names(enum msr_skill_rate sr);
const wchar_t *msr_talent_names(enum msr_talents t);
const wchar_t *msr_talent_descriptions(enum msr_talents t);
const wchar_t *msr_hitloc_name(struct msr_monster *monster, enum msr_hit_location mhl);

void msr_dbg_check_all(void);

#endif /*MONSTER_H_*/
