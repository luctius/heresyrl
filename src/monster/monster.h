#ifndef MONSTER_H_
#define MONSTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"
#include "enums.h"

enum msr_gender {
    MSR_GENDER_MALE,
    MSR_GENDER_FEMALE,
    MSR_GENDER_IT,
    MSR_GENDER_MAX,
};

enum msr_race {
    MSR_RACE_HUMAN,
    MSR_RACE_BEAST,
    MSR_RACE_DOMESTIC,
};

enum msr_characteristic {
    MSR_CHAR_WEAPON_SKILL,
    MSR_CHAR_BALISTIC_SKILL,
    MSR_CHAR_STRENGTH,
    MSR_CHAR_TOUGHNESS,
    MSR_CHAR_AGILITY,
    MSR_CHAR_INTELLIGENCE,
    MSR_CHAR_PERCEPTION,
    MSR_CHAR_WILLPOWER,
    MSR_CHAR_FELLOWSHIP,
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
    MSR_WEAPON_SELECT_OFF_HAND,
    MSR_WEAPON_SELECT_MAIN_HAND,
    MSR_WEAPON_SELECT_DUAL_HAND,
    MSR_WEAPON_SELECT_BOTH_HAND,
    MSR_WEAPON_SELECT_CREATURE1,
    MSR_WEAPON_SELECT_MAX,
};

enum msr_hit_location {
    MSR_HITLOC_LEFT_LEG,
    MSR_HITLOC_RIGHT_LEG,
    MSR_HITLOC_LEFT_ARM,
    MSR_HITLOC_RIGHT_ARM,
    MSR_HITLOC_CHEST,
    MSR_HITLOC_HEAD,
    MSR_HITLOC_MAX,
};

struct msr_char {
    uint8_t base_value;
    uint8_t advancement;
};

struct monster_controller {
    bool interruptable;  /* this monster should be interrupted if anythin interresting happens near it. */
    bool interrupted;    /* this monster *is* interrupted. */
    void *controller_ctx; /* private struct for this monster */
    bool (*controller_cb)(struct msr_monster *monster, void *controller_ctx); /* ai/player callback. */
};

struct msr_monster {
    uint32_t monster_pre;

    /* unique id of this monsters instance*/
    uint32_t uid;

    /* id of the base template of this monster */
    uint32_t template_id;

    /* current grid position */
    coord_t pos;

    char icon;
    int icon_attr;

    /* true if this is the player*/
    bool is_player;

    /* monster gender*/
    enum msr_gender gender;

    /* unique name for this unique monster, default is NULL, contains also the players name. */
    char *unique_name;
    const char *sd_name;
    const char *ld_name;
    const char *description;

    /* faction this monster belongs to. should probably be a bitfield. */
    int faction;

    enum msr_race race;
    enum msr_size size;

    /* current number of wounds, more is better */
    int8_t cur_wounds;

    /* maxmimum number of wounds, more is better */
    int8_t max_wounds;

    /* current number of fatepoints, not used. */
    uint8_t fatepoints;

    /* true if the monster is dead and to be cleaned up. */
    bool dead;

    /* current energy of this monster */
    int energy;

    /* array of bitfield of all the monster's talents */
    bitfield_t talents[((TALENTS_MAX >> TALENTS_IDX_OFFSET) & 0x0F)+1];

    /* 
       skills, divided in basic (has the skill), advanced and expert.
       a skill in advanced should also be in basic, etc.
     */
    bitfield_t skills[MSR_SKILL_RATE_MAX];

    /* special creature traits. */
    bitfield_t creature_traits;

    /* base characteris score, +advancement. every advancement is 5 extra points. */
    struct msr_char characteristic[MSR_CHAR_MAX];

    /* ai or player callback structure */
    struct monster_controller controller;

    /* current weapon selection */
    enum msr_weapon_selection wpn_sel;

    /* inventory of this monster. */
    struct inv_inventory *inventory;

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

/* create a instance of this monster template. */
struct msr_monster *msr_create(uint32_t template_id);

/* remove monster from map and global monster list. */
void msr_destroy(struct msr_monster *monster, struct dc_map *map);

/* verify monster structure. */
bool msr_verify_monster(struct msr_monster *monster);

/* assign controller structure to this monster*/
void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller);

/* free ai context and set everything to 0. */
void msr_clear_controller(struct msr_monster *monster);

/* inster monster on this map pos, set monster->pos to that pos. */
bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);

/* check if we can move to that position, then remove monster from current map pos*/
bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);

/* remove monster from map, monster->pos = {0,0} */
bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map);

/* put this item in monsters inventory. */
bool msr_give_item(struct msr_monster *monster, struct itm_item *item);

/* remove item from monsters inventory */
bool msr_remove_item(struct msr_monster *monster, struct itm_item *item);

/* roll a d100, check if it is below this characteristic */
bool msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr);

/* roll a d100, check if it is below this skill */
int msr_skill_check(struct msr_monster *monster, enum skills skill, int mod);

/* get current characteristic value, including talents. */
int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);

/* get current characteristic bonus ( normally (characteristic / 10) ), including talents */
int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr);

/* get the current worn armour on that location, or NULL */
struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl);

/* get the damage reduction of that hit location*/
int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location hitloc);

/* do damage to that hit location, including critical hits, and handle the first part of monster death. */
bool msr_do_dmg(struct msr_monster *monster, int dmg, enum msr_hit_location mhl, struct dc_map *map);

/* given a number between 0 and 99, return the monsters hit location*/
enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll);

/* get current monster energy */
int msr_get_energy(struct msr_monster *monster);

/* change the monster's energy by this much, true if succefull. */
bool msr_change_energy(struct msr_monster *monster, int energy);

/* 
   get range of the sight.
   far includes medium, medium includes near.
 */
int msr_get_near_sight_range(struct msr_monster *monster);
int msr_get_medium_sight_range(struct msr_monster *monster);
int msr_get_far_sight_range(struct msr_monster *monster);


bool msr_has_creature_trait(struct msr_monster *monster,  bitfield_t trait);
bool msr_has_talent(struct msr_monster *monster,  bitfield_t talent);
bool msr_set_talent(struct msr_monster *monster, bitfield_t talent);
enum skill_rate msr_has_skill(struct msr_monster *monster,  enum skills skill);
bool msr_set_skill(struct msr_monster *monster, enum skills skill, enum skill_rate);
uint8_t msr_get_movement_rate(struct msr_monster *monster);

/* check if the current weapon selection of this monster is correct. */
bool msr_weapons_check(struct msr_monster *monster);

/* check if the monster wields a weapon of this type */
bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type);

/* cycle weapon selection towards the next correct setting, if possible. */
bool msr_weapon_next_selection(struct msr_monster *monster);

const char *msr_ldname(struct msr_monster *monster);

/* Male / Female / It */
const char *msr_gender_string(struct msr_monster *monster);

/* he / his -- her / hers, etc.*/
const char *msr_gender_name(struct msr_monster *monster, bool possesive);

#endif /*MONSTER_H_*/
