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

#include <stdlib.h>
#include <assert.h>

#include "random_generator.h"
#include "logging.h"

int32_t random_gen_spawn(struct random_gen_settings *s) {
    if (s == NULL) return -1;
    if (s->weight == NULL) return -1;
    if (s->start_idx >= s->end_idx) return -1;

    int32_t size = s->end_idx - s->start_idx;
    int32_t roll = RANDOM_GEN_MAX;
    int32_t cumm_prob_arr[size];
    int32_t sum = 0;
    int32_t idx = -1;

    int32_t w;
    for (int32_t i = 0; i < size; i++) {
        cumm_prob_arr[i] = RANDOM_GEN_WEIGHT_IGNORE;

        w = s->weight(s->ctx, i + s->start_idx);
        if (w != RANDOM_GEN_WEIGHT_IGNORE) {
            sum += w;
            cumm_prob_arr[i] = sum;
        }
    }

    if (sum == 0) return -1;

    roll = s->roll % sum;
    for (int32_t i = size-1; i > 0; i--) {
        if (cumm_prob_arr[i] != RANDOM_GEN_WEIGHT_IGNORE) {
            if (roll < cumm_prob_arr[i]) idx = i + s->start_idx;
        }
    }

    return idx;
}

