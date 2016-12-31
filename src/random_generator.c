#include <stdlib.h>
#include <assert.h>

#include "random_generator.h"
#include "logging.h"

int32_t random_gen_spawn(struct random_gen_settings *s) {
    if (s == NULL) return -1;
    if (s->weight == NULL) return -1;
    if (s->start_idx >= s->end_idx) return -1;

    int32_t roll = RANDOM_GEN_MAX;
    int32_t cumm_prob_arr[s->end_idx - s->start_idx];
    int32_t sum = 0;
    int32_t idx = -1;

    int32_t w;
    for (int32_t i = s->start_idx; i < s->end_idx; i++) {
        cumm_prob_arr[i] = RANDOM_GEN_WEIGHT_IGNORE;

        w = s->weight(s->ctx, i);
        if (w != RANDOM_GEN_WEIGHT_IGNORE) {
            sum += w;
            cumm_prob_arr[i] = sum;
        }
    }

    assert(sum != 0);

    roll = s->roll % sum;
    for (int32_t i = s->end_idx; i > s->start_idx; i--) {
        if (cumm_prob_arr[i] != RANDOM_GEN_WEIGHT_IGNORE) {
            if (roll < cumm_prob_arr[i]) idx = i;
        }
    }

    return idx;
}

