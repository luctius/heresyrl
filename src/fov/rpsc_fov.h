#ifndef RPSC_FOV_H
#define RPSC_FOV_H

#include <stdbool.h>

#include "coord.h"

enum rpsc_fov_permisiveness {
    RPSC_FOV_PERMISSIVE_STRICT,
    RPSC_FOV_PERMISSIVE_NORMAL,
    RPSC_FOV_PERMISSIVE_RELAXED,
};

struct rpsc_fov_set {
    void *source;
    void *map;
    coord_t size;

    enum rpsc_fov_permisiveness permissiveness;
    bool visible_on_equal;
    bool not_visible_blocks_vision;

    bool (*is_opaque)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
    bool (*apply)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
};

void rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius);
void rpsc_cone(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, int angle, int radius);
bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst);

#endif /* RPSC_FOV_H */
