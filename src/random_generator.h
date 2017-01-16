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

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <stdint.h>

#define RANDOM_GEN_WEIGHT_IGNORE    (INT32_MAX)
#define RANDOM_GEN_MAX          ((INT32_MAX-1))

struct random_gen_settings {
    int32_t start_idx;
    int32_t end_idx;
    int32_t roll;
    void *ctx;
    int32_t (*weight)(void *ctx, int idx);
};

int32_t random_gen_spawn(struct random_gen_settings *s);

#endif /* RANDOM_GENERATOR_H */
