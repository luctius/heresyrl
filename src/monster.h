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
    MSR_SIZE_SCRAWY,
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

enum monster_energy {
    MSR_ENERGY_TICK = 10,
    MSR_ENERGY_TURN = 100,
    MSR_ENERGY_FULL = 1000,
};

struct monster_controller {
    bool interruptable;
    bool interrupted;
    void *controller_ctx;
    bool (*controller_cb)(struct msr_monster *monster, void *controller_ctx);
};

struct msr_monster {
    uint32_t monster_pre;

    uint32_t uid;
    uint32_t template_id;
    coord_t pos;
    char icon;
    int icon_attr;
    bool is_player;
    enum msr_gender gender;
    char *unique_name;
    const char *sd_name;
    const char *ld_name;
    const char *description;
    int faction;

    enum msr_race race;
    enum msr_size size;

    int8_t cur_wounds;
    int8_t max_wounds;
    uint8_t fatepoints;
    bool dead;
    int energy;

    bitfield_t talents[((TALENTS_MAX >> 27) & 0x0F)+1];
    bitfield_t skills[MSR_SKILL_RATE_MAX];
    bitfield_t creature_traits;
    struct msr_char characteristic[MSR_CHAR_MAX];

    struct monster_controller controller;

    enum msr_weapon_selection wpn_sel;
    struct inv_inventory *inventory;

    uint32_t monster_post;
};

void msrlst_monster_list_init(void);
void msrlst_monster_list_exit(void);
struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev);

struct msr_monster *msr_create(uint32_t template_id);
void msr_destroy(struct msr_monster *monster, struct dc_map *map);
bool msr_verify_monster(struct msr_monster *monster);

void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller);
void msr_clear_controller(struct msr_monster *monster);
bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map);
bool msr_give_item(struct msr_monster *monster, struct itm_item *item);
bool msr_remove_item(struct msr_monster *monster, struct itm_item *item);

bool msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr);
int msr_skill_check(struct msr_monster *monster, enum skills skill, int mod);

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);
int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr);
struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl);
int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location hitloc);
bool msr_do_dmg(struct msr_monster *monster, int dmg, enum msr_hit_location mhl, struct dc_map *map);
enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll); /* hitroll, from 0-99*/

int msr_get_energy(struct msr_monster *monster);
bool msr_change_energy(struct msr_monster *monster, int energy);

int msr_get_near_sight_range(struct msr_monster *monster);
int msr_get_medium_sight_range(struct msr_monster *monster);
int msr_get_far_sight_range(struct msr_monster *monster);
char *msr_gender_string(struct msr_monster *monster);
bool msr_do_skill_check(struct msr_monster *monster, enum skills skill, int modifiers);

bool msr_has_creature_trait(struct msr_monster *monster,  bitfield_t trait);
bool msr_has_talent(struct msr_monster *monster,  bitfield_t talent);
bool msr_set_talent(struct msr_monster *monster, bitfield_t talent);
enum skill_rate msr_has_skill(struct msr_monster *monster,  enum skills skill);
bool msr_set_skill(struct msr_monster *monster, enum skills skill, enum skill_rate);
uint8_t msr_get_movement_rate(struct msr_monster *monster);

bool msr_weapons_check(struct msr_monster *monster);
bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type);
bool msr_weapon_next_selection(struct msr_monster *monster);

const char *msr_ldname(struct msr_monster *monster);
const char *msr_gender_name(struct msr_monster *monster, bool possesive);

#endif /*MONSTER_H_*/
