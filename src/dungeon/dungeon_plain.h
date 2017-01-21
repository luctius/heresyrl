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

#ifndef DUNGEON_PLAIN_H
#define DUNGEON_PLAIN_H

#include <stdint.h>
#include <stdbool.h>

#include "dungeon/dungeon_helpers.h"

struct dungeon_features_done *dm_generate_map_plain(struct dm_map *map, struct random *r, coord_t *ul, coord_t *dr);

#endif /* DUNGEON_PLAIN_H */
