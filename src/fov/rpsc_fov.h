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
    /* the item or actor which 'sees' the fov*/
    void *source;

    /* map structure. */
    void *map;

    /* size in the x and y direction */
    coord_t size;

    /* permission settings */
    enum rpsc_fov_permisiveness permissiveness;

    /* true if a square is visible even when an angle of it is equal to a blocking tile. */
    bool visible_on_equal;

    /* when a tile is not visible it will block further tiles. */
    bool not_visible_blocks_vision;

    /* callback, should return true if the tile does not block vision, false if it does. */
    bool (*is_opaque)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);

    /* callback, applies the line of sight. */
    bool (*apply)(struct rpsc_fov_set *set, coord_t *point, coord_t *origin);
};

/* creates a fov from src with a given radius. */
void rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius);

/* creates a cone from src with dst in the middle of angle and a given radius. */
void rpsc_cone(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, int angle, int radius);

/*
   calculates line of sight from src to dst.
   returns true if src can see dst.
   if apply within set is not NULL, it will give the path from src to dst.
   This path will be created on the go, thus when it return with false, the path should be ignored.
 */
bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst);

#endif /* RPSC_FOV_H */
