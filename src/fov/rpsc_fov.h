#ifndef RPSC_FOV_H
#define RPSC_FOV_H

#include <stdbool.h>

#include "coord.h"

struct rpsc_fov_set {
    void *source;
    void *map;
    coord_t size;

    bool (*is_opaque)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
    bool (*apply)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
};

bool rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius);
bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, bool apply);

#endif /* RPSC_FOV_H */
