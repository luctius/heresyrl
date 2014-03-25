/*
digital FOV with recursive shadowcasting tech demo
Copyright (C) 2010 Oohara Yuuma <oohara@libra.interq.or.jp>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the author be held liable for any damages
arising from the use of this software.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software.
2. Altered versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any redistribution.

Altered by Cor Peters (2014)
*/

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "digital_fov_other.h"

static int grid_is_illegal(int x, int y, int map_size_x, int map_size_y);
static int gcd(int x, int y);
static int which_side_of_line(int ax, int ay, int bx, int by, int x, int y);
static int digital_fov_brute_body(struct digital_fov_set *set, coord_t *src, int radius, int dir);
static int digital_fov_beam_body(struct digital_fov_set *set, coord_t *src, int radius, int dir);

/* return 1 (true) or 0 (false) */
static int
grid_is_illegal(int x, int y, int map_size_x, int map_size_y)
{
  if ((x < 0) || (x >= map_size_x))
    return 1;
  if ((y < 0) || (y >= map_size_y))
    return 1;

  return 0;
}

/* Greatest Common Denominator
 * runs at O(log(N))
 */
static int
gcd(int x, int y)
{
  int t;

  while (y != 0)
  {
    t = x % y;
    x = y;
    y = t;
  }

  return x;
}

static int
digital_fov_brute_body(struct digital_fov_set *set, coord_t *src, 
                        int radius, int dir)
{
  /* summary:
   * Generate all digital straight segments of length (radius)
   * and see which grid each of them lights.
   */
  int map_size_x = set->size.x;
  int map_size_y = set->size.y;
  int center_x = src->x;
  int center_y = src->y;
  int p;
  int q;
  int u;
  int v;
  int temp;
  int x0;
  int y0;
  int x1;
  int y1;
  int grid0_is_illegal;
  int grid1_is_illegal;
  int r;
  int s_min;
  int s_max;
  coord_t point;

  if (set == NULL)
    return 1;
  if (radius < 0)
    return 1;

  /* it is known that all digital straight segments of length (radius)
   * can be written in this form
   */
  for (p = 1; p <= radius; p++)
  {
    for (q = 0; q <= p; q++)
    {
      if ((p >= 2) && (gcd(p, q) >= 2))
        continue;

      s_min = 0;
      s_max = p - 1;
      v = 0;
      r = 0;
      for (u = 1; u <= radius; u++)
      {
        /* v = (u * q) / p;
         * r = (u * q) % p;
         */
        r += q;
        if (r >= p)
        {
          v++;
          r -= p;
        }

        x0 = u;
        y0 = v;
        x1 = u;
        y1 = v + 1;

        if ((dir & 1) == 1)
        {
          temp = x0;
          x0 = y0;
          y0 = temp;

          temp = x1;
          x1 = y1;
          y1 = temp;
        }
        if ((dir & 2) == 2)
        {
          temp = x0;
          x0 = -y0;
          y0 = temp;

          temp = x1;
          x1 = -y1;
          y1 = temp;
        }
        if ((dir & 4) == 4)
        {
          x0 = -x0;
          y0 = -y0;

          x1 = -x1;
          y1 = -y1;
        }

        x0 += center_x;
        y0 += center_y;
        x1 += center_x;
        y1 += center_y;

        grid0_is_illegal= grid_is_illegal(x0, y0, map_size_x, map_size_y);
        grid1_is_illegal= grid_is_illegal(x1, y1, map_size_x, map_size_y);

        if (r + s_min < p)
        {
          if (!grid0_is_illegal)
          {
            //map_fov[x0 - center_x + radius][y0 - center_y + radius] = 1;
            point.x = x0;
            point.y = y0;
            set->apply(set, &point, src);
          }
        }
        if (r + s_max >= p)
        {
          if (!grid1_is_illegal)
            //map_fov[x1 - center_x + radius][y1 - center_y + radius] = 1;
            point.x = x1;
            point.y = y1;
            set->apply(set, &point, src);
        }

        if (r == 0)
        {
          point.x = x0;
          point.y = y0;
          if ((grid0_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
            break;
        }
        else
        {
          point.x = x0;
          point.y = y0;
          if ((grid0_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
          {
            if (p - r > s_min)
              s_min = p - r;
          }

          point.x = x1;
          point.y = y1;
          if ((grid1_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
          {
            if (p - r  - 1 < s_max)
              s_max = p - r - 1;
          }

          if (s_min > s_max)
            break;
        }
      }
    }
  }

  return 0;
}

bool digital_fov_brute(struct digital_fov_set *set, coord_t *src, int radius)
{
  int map_size_x = set->size.x;
  int map_size_y = set->size.y;
  int center_x = src->x;
  int center_y = src->y;
  int x;
  int y;
  int dir;
  bool error_found;

  if (set == NULL)
    return false;
  if (radius < 0)
    return false;

  if (grid_is_illegal(center_x, center_y, map_size_x, map_size_y))
    return false;

  set->apply(set, src, src);

  error_found = true;
  for (dir = 0; dir < 8; dir++)
  {
    if (digital_fov_brute_body(set, src, radius, dir) != 0)
      error_found = false;
  }

  return error_found;
}

/* Suppose that:
 * * there are 4 points (ax, ay), (bx, by), (x, y) and (x, Y)
 * * ax < bx
 * * the 3 points (ax, ay), (bx, by) and (x, Y) are on the same line
 * The return value of this function has same sign as y - Y, that is,
 * this function returns a positive value if and only if (x, y) is
 * above the line which passes (ax, bx) and (bx, by).
 * The caller of this function must ensure that ax < bx.
 */
static int
which_side_of_line(int ax, int ay, int bx, int by,
                   int x, int y)
{
  return (y - ay) * (bx - ax)
    - (by - ay) * (x - ax);
}

static int
digital_fov_beam_body(struct digital_fov_set *set, coord_t *src,
                      int radius, int dir)
{
  /* summary:
   * Divide all rays into (radius + 1) groups: the group N is the set of
   * rays which pass (0, 0) and (radius, N).  A ray in the group N
   * passes no grid other than (x, (x * N) / radius) and
   * (x, (x * N) / radius + 1).
   */
  int map_size_x = set->size.x;
  int map_size_y = set->size.y;
  int center_x = src->x;
  int center_y = src->y;
  int u;
  int v;
  int temp;
  int x0;
  int y0;
  int x1;
  int y1;
  int grid0_is_illegal;
  int grid1_is_illegal;
  int this_ray_lights_grid0;
  int this_ray_lights_grid1;
  int n;
  int r;
  coord_t point;

  int bottom_ray_touch_top_wall_u;
  int bottom_ray_touch_top_wall_v;
  int bottom_ray_touch_bottom_wall_u;
  int bottom_ray_touch_bottom_wall_v;

  int top_ray_touch_bottom_wall_u;
  int top_ray_touch_bottom_wall_v;
  int top_ray_touch_top_wall_u;
  int top_ray_touch_top_wall_v;

  /* the bottom ray touches the top wall at
   * (top_wall_array_u[b_ray_t], top_wall_array_v[b_ray_t])
   * when it is updated
   * note that top_wall_array_u[b_ray_t] <= bottom_ray_touch_top_wall_u
   * even if those 2 numbers may not be equal
   */
  int b_ray_t;
  /* the top ray touches the bottom wall at
   * (bottom_wall_array_u[t_ray_b], bottom_wall_array_v[t_ray_b])
   * when it is updated
   * note that bottom_wall_array_u[t_ray_b] <= top_ray_touch_bottom_wall_u
   * even if those 2 numbers may not be equal
   */
  int t_ray_b;

  /* remember only relevant walls */
  int top_wall_num;
  int bottom_wall_num;
  int *top_wall_array_u = NULL;
  int *top_wall_array_v = NULL;
  int *bottom_wall_array_u = NULL;
  int *bottom_wall_array_v = NULL;

  if (set == NULL)
    return 1;
  if (radius < 0)
    return 1;

  top_wall_array_u = (int *) malloc(sizeof(int) * (radius + 1));
  if (top_wall_array_u == NULL)
  {
    return 1;
  }
  top_wall_array_v = (int *) malloc(sizeof(int) * (radius + 1));
  if (top_wall_array_v == NULL)
  {
    free(top_wall_array_u);
    top_wall_array_u = NULL;
    return 1;
  }
  bottom_wall_array_u = (int *) malloc(sizeof(int) * (radius + 1));
  if (bottom_wall_array_u == NULL)
  {
    free(top_wall_array_u);
    top_wall_array_u = NULL;
    free(top_wall_array_v);
    top_wall_array_v = NULL;
    return 1;
  }
  bottom_wall_array_v = (int *) malloc(sizeof(int) * (radius + 1));
  if (bottom_wall_array_v == NULL)
  {
    free(top_wall_array_u);
    top_wall_array_u = NULL;
    free(top_wall_array_v);
    top_wall_array_v = NULL;
    free(bottom_wall_array_u);
    bottom_wall_array_u = NULL;
    return 1;
  }

  for (n = 0; n < radius + 1; n++)
  {
    bottom_ray_touch_top_wall_u = 0;
    bottom_ray_touch_top_wall_v = 1;
    bottom_ray_touch_bottom_wall_u = 1;
    bottom_ray_touch_bottom_wall_v = -1;

    top_ray_touch_bottom_wall_u = 0;
    top_ray_touch_bottom_wall_v = 0;
    top_ray_touch_top_wall_u = 1;
    top_ray_touch_top_wall_v = 2;

    top_wall_array_u[0] = 0;
    top_wall_array_v[0] = 1;
    top_wall_num = 1;

    bottom_wall_array_u[0] = 0;
    bottom_wall_array_v[0] = 0;
    bottom_wall_num = 1;

    b_ray_t = 0;
    t_ray_b = 0;

    v = 0;
    r = 0;
    for (u = 1; u <= radius; u++)
    {
      /* v = (u * n) / radius;
       * r = (u * n) % radius;
       */
      r += n;
      if (r >= radius)
      {
        v++;
        r -= radius;
      }

      x0 = u;
      y0 = v;
      x1 = u;
      y1 = v + 1;

      if ((dir & 1) == 1)
      {
        temp = x0;
        x0 = y0;
        y0 = temp;

        temp = x1;
        x1 = y1;
        y1 = temp;
      }
      if ((dir & 2) == 2)
      {
        temp = x0;
        x0 = -y0;
        y0 = temp;

        temp = x1;
        x1 = -y1;
        y1 = temp;
      }
      if ((dir & 4) == 4)
      {
        x0 = -x0;
        y0 = -y0;

        x1 = -x1;
        y1 = -y1;
      }

      x0 += center_x;
      y0 += center_y;
      x1 += center_x;
      y1 += center_y;

      grid0_is_illegal = grid_is_illegal(x0, y0, map_size_x, map_size_y);
      grid1_is_illegal = grid_is_illegal(x1, y1, map_size_x, map_size_y);

      this_ray_lights_grid0 = 0;
      this_ray_lights_grid1 = 0;

      if (r == 0)
      {
        /* mark visible */
        if ((!grid0_is_illegal)
            && (which_side_of_line(bottom_ray_touch_top_wall_u,
                                bottom_ray_touch_top_wall_v,
                                bottom_ray_touch_bottom_wall_u,
                                bottom_ray_touch_bottom_wall_v,
                                u, v + 1) > 0)
            && (which_side_of_line(top_ray_touch_bottom_wall_u,
                                   top_ray_touch_bottom_wall_v,
                                   top_ray_touch_top_wall_u,
                                   top_ray_touch_top_wall_v,
                                   u, v) < 0)
            )
        {
          //map_fov[x0 - center_x + radius][y0 - center_y + radius] = 1;
          point.x = x0;
          point.y = y0;
          set->apply(set, &point, src);
          this_ray_lights_grid0 = 1;
        }

        /* check if some ray is still available
         * checking map_fov is not enough because a ray in another group
         * may have already set map_fov to non-zero
         */
        if (!this_ray_lights_grid0)
          break;

        /* check wall (no need to update top/bottom ray or remember) */
        point.x = x0;
        point.y = y0;
        if ((grid0_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
          break;
      }
      else
      {
        /* mark visible */
        if ((!grid0_is_illegal)
            && (which_side_of_line(bottom_ray_touch_top_wall_u,
                                bottom_ray_touch_top_wall_v,
                                bottom_ray_touch_bottom_wall_u,
                                bottom_ray_touch_bottom_wall_v,
                                u, v + 1) > 0)
            && (which_side_of_line(top_ray_touch_bottom_wall_u,
                                   top_ray_touch_bottom_wall_v,
                                   top_ray_touch_top_wall_u,
                                   top_ray_touch_top_wall_v,
                                   u, v) < 0))
        {
          point.x = x0;
          point.y = y0;
          set->apply(set, &point, src);
          //map_fov[x0 - center_x + radius][y0 - center_y + radius] = 1;
          this_ray_lights_grid0 = 1;
        }
        if ((!grid1_is_illegal)
            && (which_side_of_line(bottom_ray_touch_top_wall_u,
                                bottom_ray_touch_top_wall_v,
                                bottom_ray_touch_bottom_wall_u,
                                bottom_ray_touch_bottom_wall_v,
                                u, v + 2) > 0)
            && (which_side_of_line(top_ray_touch_bottom_wall_u,
                                   top_ray_touch_bottom_wall_v,
                                   top_ray_touch_top_wall_u,
                                   top_ray_touch_top_wall_v,
                                   u, v + 1) < 0))
        {
          point.x = x1;
          point.y = y1;
          set->apply(set, &point, src);
          //map_fov[x1 - center_x + radius][y1 - center_y + radius] = 1;
          this_ray_lights_grid1 = 1;
        }

        /* check if some ray is still available
         * checking map_fov is not enough because a ray in another group
         * may have already set map_fov to non-zero
         */
        if ((!this_ray_lights_grid0) && (!this_ray_lights_grid1))
          break;

        point.x = x0;
        point.y = y0;
        /* update top and bottom ray */
        if ((grid0_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
        {
          if (which_side_of_line(bottom_ray_touch_top_wall_u,
                                 bottom_ray_touch_top_wall_v,
                                 bottom_ray_touch_bottom_wall_u,
                                 bottom_ray_touch_bottom_wall_v,
                                 u, v + 1) > 0)
          {
            bottom_ray_touch_bottom_wall_u = u;
            bottom_ray_touch_bottom_wall_v = v + 1;
            while (b_ray_t + 1 < top_wall_num)
            {
              /* this loop is called at most radius times for each n
               * because each call increase bottom_ray_touch_top_wall_u,
               * which starts at 0 and can't be greater than radius
               */
              if (which_side_of_line(bottom_ray_touch_top_wall_u,
                                     bottom_ray_touch_top_wall_v,
                                     bottom_ray_touch_bottom_wall_u,
                                     bottom_ray_touch_bottom_wall_v,
                                     top_wall_array_u[b_ray_t + 1],
                                     top_wall_array_v[b_ray_t + 1]) >= 0)
                break;
              bottom_ray_touch_top_wall_u = top_wall_array_u[b_ray_t + 1];
              bottom_ray_touch_top_wall_v = top_wall_array_v[b_ray_t + 1];
              b_ray_t++;
            }
          }
        }

        point.x = x1;
        point.y =y1;
        if ((grid1_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
        {
          if (which_side_of_line(top_ray_touch_bottom_wall_u,
                                 top_ray_touch_bottom_wall_v,
                                 top_ray_touch_top_wall_u,
                                 top_ray_touch_top_wall_v,
                                 u, v + 1) < 0)
          {
            top_ray_touch_top_wall_u = u;
            top_ray_touch_top_wall_v = v + 1;
            while (t_ray_b + 1 < bottom_wall_num)
            {
              /* this loop is called at most radius times for each n
               * because each call increase top_ray_touch_bottom_wall_u,
               * which starts at 0 and can't be greater than radius
               */
              if (which_side_of_line(top_ray_touch_bottom_wall_u,
                                     top_ray_touch_bottom_wall_v,
                                     top_ray_touch_top_wall_u,
                                     top_ray_touch_top_wall_v,
                                     bottom_wall_array_u[t_ray_b + 1],
                                     bottom_wall_array_v[t_ray_b + 1]) <= 0)
                break;
              top_ray_touch_bottom_wall_u = bottom_wall_array_u[t_ray_b + 1];
              top_ray_touch_bottom_wall_v = bottom_wall_array_v[t_ray_b + 1];
              t_ray_b++;
            }
          }
        }

        point.x = x0;
        point.y = y0;
        /* remember wall */
        if ((grid0_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
        {
          if (which_side_of_line(top_ray_touch_bottom_wall_u,
                                 top_ray_touch_bottom_wall_v,
                                 top_ray_touch_top_wall_u,
                                 top_ray_touch_top_wall_v,
                                 u, v + 1) >= 0)
          {
            /* the new bottom wall blocks all rays */
            break;
          }

          bottom_wall_array_u[bottom_wall_num] = u;
          bottom_wall_array_v[bottom_wall_num] = v + 1;
          bottom_wall_num++;

          while (bottom_wall_num >= 3)
          {
            /* this loop is called at most (2 * radius) times for each n
             * because for each u, each call after the first removes
             * a wall from bottom_wall_array and at most radius walls
             * can be added to bottom_wall_array
             */
            if (which_side_of_line(bottom_wall_array_u[bottom_wall_num - 3],
                                   bottom_wall_array_v[bottom_wall_num - 3],
                                   bottom_wall_array_u[bottom_wall_num - 1],
                                   bottom_wall_array_v[bottom_wall_num - 1],
                                   bottom_wall_array_u[bottom_wall_num - 2],
                                   bottom_wall_array_v[bottom_wall_num - 2])
                > 0)
              break;

            /* bottom_wall_array[bottom_wall_num - 2] is no longer relevant */
            bottom_wall_array_u[bottom_wall_num - 2]
              = bottom_wall_array_u[bottom_wall_num - 1];
            bottom_wall_array_v[bottom_wall_num - 2]
              = bottom_wall_array_v[bottom_wall_num - 1];
            if (t_ray_b == bottom_wall_num - 2)
              t_ray_b--;
            bottom_wall_num--;
          }
        }

        point.x = x1;
        point.y = y1;
        if ((grid1_is_illegal)
              || (set->is_opaque(set, &point, src) == false))
        {
          if (which_side_of_line(bottom_ray_touch_top_wall_u,
                                 bottom_ray_touch_top_wall_v,
                                 bottom_ray_touch_bottom_wall_u,
                                 bottom_ray_touch_bottom_wall_v,
                                 u, v + 1) <= 0)
          {
            /* the new top wall blocks all rays */
            break;
          }

          top_wall_array_u[top_wall_num] = u;
          top_wall_array_v[top_wall_num] = v + 1;
          top_wall_num++;

          while (top_wall_num >= 3)
          {
            /* this loop is called at most (2 * radius) times for each n
             * because for each u, each call after the first removes
             * a wall from top_wall_array and at most radius walls
             * can be added to top_wall_array
             */
            if (which_side_of_line(top_wall_array_u[top_wall_num - 3],
                                   top_wall_array_v[top_wall_num - 3],
                                   top_wall_array_u[top_wall_num - 1],
                                   top_wall_array_v[top_wall_num - 1],
                                   top_wall_array_u[top_wall_num - 2],
                                   top_wall_array_v[top_wall_num - 2])
                < 0)
              break;

            /* top_wall_array[top_wall_num - 2] is no longer relevant */
            top_wall_array_u[top_wall_num - 2]
              = top_wall_array_u[top_wall_num - 1];
            top_wall_array_v[top_wall_num - 2]
              = top_wall_array_v[top_wall_num - 1];
            if (b_ray_t == top_wall_num - 2)
              b_ray_t--;
            top_wall_num--;
          }
        }
      }
    }
  }

  free(top_wall_array_u);
  top_wall_array_u = NULL;
  free(top_wall_array_v);
  top_wall_array_v = NULL;
  free(bottom_wall_array_u);
  bottom_wall_array_u = NULL;
  free(bottom_wall_array_v);
  bottom_wall_array_v = NULL;

  return 0;
}

bool digital_fov_beam(struct digital_fov_set *set, coord_t *src, int radius)
{
  int map_size_x = set->size.x;
  int map_size_y = set->size.y;
  int center_x = src->x;
  int center_y = src->y;
  int x;
  int y;
  int dir;
  bool error_found;

  if (set == NULL)
    return false;
  if (radius < 0)
    return false;

  if (grid_is_illegal(center_x, center_y, map_size_x, map_size_y))
    return false;

  set->apply(set, src, src);

  error_found = true;
  for (dir = 0; dir < 8; dir++)
  {
    if (digital_fov_beam_body(set, src , radius, dir) != 0)
      error_found = false;
  }

  return error_found;
}
