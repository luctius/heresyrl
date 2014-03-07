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
    MSR_GENDER_RANDOM,
};

enum msr_race {
    MSR_RACE_HUMAN,
};

enum msr_characteristic {
    MSR_CHAR_WEAPON_SKILL,
    MSR_CHAR_BALISTIC_SKILL,
    MSR_CHAR_STRENGTH,
    MSR_CHAR_TOUCHNESS,
    MSR_CHAR_AGILITY,
    MSR_CHAR_INTELLIGENCE,
    MSR_CHAR_PERCEPTION,
    MSR_CHAR_WILLPOWER,
    MSR_CHAR_FELLOWSHIP,
    MSR_CHAR_MAX,
};

struct msr_char {
    uint8_t base_value;
    uint8_t advancement;
};

struct msr_monster {
    uint32_t uid;
    coord_t pos;
    char icon;
    int icon_attr;
    uint8_t visibility;
    bool is_player;
    enum msr_gender gender;
    const char *sd_name;
    const char *ld_name;
    const char *description;

    enum msr_race race;

    uint8_t cur_wounds;
    uint8_t max_wounds;
    uint8_t mov_speed;
    uint8_t fatepoints;

    uint64_t race_traits;
    uint64_t combat_traits;
    uint64_t career_traits;

    struct msr_char characteristic[MSR_CHAR_MAX];

    struct inv_inventory *inventory;
};

void msrlst_monster_list_init(void);
void msrlst_monster_list_exit(void);
struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev);

struct msr_monster *msr_create(enum msr_race race);
void msr_die(struct msr_monster *monster, struct dc_map *map);
bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos);
bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map);
bool msr_give_item(struct msr_monster *monster, struct itm_item *item);
bool msr_remove_item(struct msr_monster *monster, struct itm_item *item);
bool msr_use_item(struct msr_monster *monster, struct itm_item *item);

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr);
int msr_get_near_sight_range(struct msr_monster *monster);
int msr_get_far_sight_range(struct msr_monster *monster);
char *msr_gender_string(struct msr_monster *monster);

#endif /*MONSTER_H_*/
