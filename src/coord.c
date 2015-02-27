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

#include "coord.h"
#include "heresyrl_def.h"

const coord_t coord_nhlo_table[] = {
    {-1,-1}, {-1,0}, {-1,1}, 
    { 0,-1},         { 0,1}, 
    { 1,-1}, { 1,0}, { 1,1},
};
const uint8_t coord_nhlo_table_sz = ARRAY_SZ(coord_nhlo_table);

extern inline coord_t cd_create(short x, short y);
extern inline coord_t cd_add(const coord_t *a, const coord_t *b);
extern inline bool cd_equal(const coord_t *a, const coord_t *b);
extern inline bool cd_within_bound(const coord_t *a, const coord_t *max);
extern inline short cd_pyth(const coord_t *a, const coord_t *b);
extern inline coord_t cd_delta(const coord_t *a, const coord_t *b);
extern inline coord_t cd_delta_abs(const coord_t *a, const coord_t *b);
extern inline int cd_delta_abs_total(const coord_t *a, const coord_t *b);
extern inline bool cd_neighbour(const coord_t *a, const coord_t *b);
