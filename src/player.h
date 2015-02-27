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

#ifndef PLAYER_H_
#define PLAYER_H_

#include "heresyrl_def.h"
#include "fight.h"
#include "coord.h"

void plr_create(struct pl_player *plr, char *name, uint32_t template_id, enum msr_gender gender);
bool plr_init(struct pl_player *plr);

struct pf_context *plr_map(struct pl_player *plr, struct dm_map *map);

#endif /*PLAYER_H_*/
