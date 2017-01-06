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

#ifndef CAREER_H
#define CAREER_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "enums.h"
#include "careers/careers_static.h"
#include "monster/monster.h"

#define CR_TRAPPINGS_MAX 10
struct cr_homeworld {
    const char *name;
    const char *description;

    int characteristics[MSR_CHAR_MAX];

    int fate_points;
    int fate_threshold;
    int wounds;

    bitfield32_t aptitudes;
    enum msr_talents homeworld_talent;
};

struct cr_background {
    const char *name;
    const char *description;

    bitfield32_t skills;
    bitfield64_t talents;
    bitfield32_t aptitudes;
    enum msr_talents background_talent;

    enum item_ids trappings[CR_TRAPPINGS_MAX][2];
};

struct cr_role {
    const char *name;
    const char *description;

    bitfield32_t aptitudes;
    bitfield64_t talents;
    enum msr_talents role_talent;
};

void cr_init();
void cr_exit(struct pl_player *plr);

void cr_init_career(struct pl_player *plr, enum homeworld_ids hid, enum background_ids btid, enum role_ids rtid);
struct cr_homeworld *cr_get_homeworld_by_id(enum homeworld_ids tid);
struct cr_background *cr_get_background_by_id(enum background_ids tid);
struct cr_role *cr_get_role_by_id(enum role_ids tid);

int cr_skill_cost(struct pl_player *plr, enum msr_skills skill);
int cr_talent_cost(struct pl_player *plr, enum msr_talents talent);

bool cr_has_aptitude(struct pl_player *plr, enum aptitude_enum aptitude);
void cr_set_aptitude(struct pl_player *plr, enum aptitude_enum aptitude);
const char *cr_aptitude_name(enum aptitude_enum aptitude);

void cr_add_achievement(struct pl_player *plr, int turn, const char *achievement);
void cr_print_morgue_file(struct pl_player *plr);

#endif /* CAREER_H */
