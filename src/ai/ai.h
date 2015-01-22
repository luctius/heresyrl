#ifndef AI_H
#define AI_H

#include "heresyrl_def.h"

/* generic ai struct. to let ai of differnt kinds see some info from each other. */
struct ai {
    uint32_t target_uid;
    uint32_t leader_uid;
    void *ai_ctx;
};

void ai_monster_init(struct msr_monster *monster, uint32_t leader_uid);
void ai_monster_free(struct msr_monster *monster);

#endif /* AI_H */
