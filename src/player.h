#pragma once
#ifndef PLAYER_H_
#define PLAYER_H_

#include "heresyrl_def.h"

struct pl_player {
    struct msr_monster *player;
    unsigned long age;
};

#endif /*PLAYER_H_*/
