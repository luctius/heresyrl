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

#ifndef ANIMATE_H
#define ANIMATE_H

#include "heresyrl_def.h"
#include "coord.h"

void ui_animate_projectile(struct dm_map *map, coord_t path[], int path_len);
void ui_animate_explosion(struct dm_map *map, coord_t path[], int path_len);

#endif /* ANIMATE_H */
