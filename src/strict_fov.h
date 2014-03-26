#ifndef STRICT_FOV_H
#define STRICT_FOV_H

#include "coord.h"

struct strict_fov_set {
    void *source;
    void *map;

    bool (*is_opaque)(struct strict_fov_set *set, coord_t *point, coord_t *origin);
    bool (*apply)(struct strict_fov_set *set, coord_t *point, coord_t *origin);
};

void strict_fov(struct strict_fov_set *set, coord_t *src, int radius);
bool strict_los(struct strict_fov_set *set, coord_t *src, coord_t *dst, bool apply);

#endif /* STRICT_FOV_H */
