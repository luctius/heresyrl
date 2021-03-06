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

#ifndef DOWEAR_H
#define DOWEAR_H

#include "heresyrl_def.h"

bool dw_wear_item(struct msr_monster *monster, struct itm_item *item);
bool dw_remove_item(struct msr_monster *monster, struct itm_item *item);
bool dw_use_item(struct msr_monster *monster, struct itm_item *item);

bool dw_can_wear_item(struct msr_monster *monster, struct itm_item *item);
bool dw_can_remove_item(struct msr_monster *monster, struct itm_item *item);
#endif /* DOWEAR_H */
