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
