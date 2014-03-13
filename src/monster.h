#ifndef MONSTER_H_
#define MONSTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "heresyrl_def.h"
#include "coord.h"

enum msr_gender {
    MSR_GENDER_MALE,
    MSR_GENDER_FEMALE,
    MSR_GENDER_IT,
    MSR_GENDER_MAX,
};

enum msr_race {
    MSR_RACE_HUMAN,
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

enum msr_skills {
    MSR_SKILL_AWARENESS,
    MSR_SKILL_BARTER,
    MSR_SKILL_CHEM_USE,
    MSR_SKILL_COMMON_LORE,
    MSR_SKILL_CONCEALMENT,
    MSR_SKILL_DEMOLITION,
    MSR_SKILL_DISGUISE,
    MSR_SKILL_DODGE,
    MSR_SKILL_EVALUATE,
    MSR_SKILL_FORBIDDEN_LORE,
    MSR_SKILL_INVOCATION,
    MSR_SKILL_LOGIC,
    MSR_SKILL_MEDICAE,
    MSR_SKILL_PSYSCIENCE,
    MSR_SKILL_SCHOLASTIC_LORE,
    MSR_SKILL_SEARCH,
    MSR_SKILL_SECURITY,
    MSR_SKILL_SILENT_MOVE,
    MSR_SKILL_SURVIVAL,
    MSR_SKILL_TECH_USE,
    MSR_SKILL_TRACKING,
    MSR_SKILL_MAX,
};

enum msr_skill_rate {
    MSR_SKILL_RATE_BASIC,
    MSR_SKILL_RATE_ADVANCED,
    MSR_SKILL_RATE_EXPERT,
    MSR_SKILL_RATE_MAX,
};

enum msr_weapon_selection {
    MSR_WEAPON_SELECT_OFF_HAND,
    MSR_WEAPON_SELECT_MAIN_HAND,
    MSR_WEAPON_SELECT_DUAL_HAND,
    MSR_WEAPON_SELECT_BOTH_HAND,
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

    uint8_t cur_wounds;
    uint8_t max_wounds;
    bool dead;
    uint8_t fatepoints;
    uint32_t energy;

    bitfield_t race_traits;
    bitfield_t combat_talents;
    bitfield_t career_talents;
    bitfield_t creature_talents;

    struct msr_char characteristic[MSR_CHAR_MAX];
    bitfield_t skills[MSR_SKILL_RATE_MAX];

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
bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map);
bool msr_give_item(struct msr_monster *monster, struct itm_item *item);
bool msr_remove_item(struct msr_monster *monster, struct itm_item *item);

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);
int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr);
int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location hitloc);
bool msr_do_dmg(struct msr_monster *monster, int dmg, enum msr_hit_location mhl, struct dc_map *map);
enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll); /* hitroll, from 0-99*/

int msr_get_near_sight_range(struct msr_monster *monster);
int msr_get_far_sight_range(struct msr_monster *monster);
char *msr_gender_string(struct msr_monster *monster);
bool msr_do_skill_check(struct msr_monster *monster, enum msr_skills skill, int modifiers);

bool msr_weapons_check(struct msr_monster *monster);
bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type);
bool msr_weapon_next_selection(struct msr_monster *monster);

struct itm_item *msr_unarmed_weapon(struct msr_monster *monster);

#endif /*MONSTER_H_*/
