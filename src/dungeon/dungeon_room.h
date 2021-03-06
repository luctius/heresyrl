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

#ifndef DUNGEON_ROOM_H
#define DUNGEON_ROOM_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"

bool dm_generate_map_room(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr);

#endif /* DUNGEON_ROOM_H */
