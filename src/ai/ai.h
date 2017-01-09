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

#ifndef AI_H
#define AI_H

#include "heresyrl_def.h"

/* generic ai struct. to let ai of differnt kinds see some info from each other. */
struct ai {
    uint32_t target_uid;
    uint32_t leader_uid;
    char *emo_state;
    void *ai_ctx;
    void (*free_func)(void *ai_ctx);
};

void ai_monster_init(struct msr_monster *monster, uint32_t leader_uid);
void ai_monster_free(struct msr_monster *monster);

#endif /* AI_H */
