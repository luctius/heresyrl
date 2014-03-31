#ifndef RPSC_FOV_H
#define RPSC_FOV_H

#include <stdbool.h>

#include "coord.h"

enum rpsc_fov_permisiveness {
    RPSC_FOV_PERMISSIVE_STRICT = 1,
    RPSC_FOV_PERMISSIVE_NORMAL = 2,
    RPSC_FOV_PERMISSIVE_RELAXED = 3,
};

struct rpsc_fov_set {
    void *source;
    void *map;
    coord_t size;

    enum rpsc_fov_permisiveness permissiveness;

    bool (*is_opaque)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
    bool (*apply)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
};

bool rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius);
bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, bool apply);

#endif /* RPSC_FOV_H */
