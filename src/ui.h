#ifndef UI_H
#define UI_H

#include "heresyrl_def.h"
#include "coord.h"

bool ui_create(int cols, int lines);
void ui_destroy(void);

void update_screen(void);

void mapwin_display_map(struct dc_map *map, coord_t *player);
void mapwin_overlay_examine_cursor(struct dc_map *map, coord_t *p_pos);
bool mapwin_overlay_fire_cursor(struct gm_game *g, struct dc_map *map, coord_t *p_pos);
bool mapwin_overlay_throw_cursor(struct gm_game *g, struct dc_map *map, coord_t *p_pos);

void msgwin_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void msgwin_log_refresh(struct logging *log, struct log_entry *le);

void charwin_refresh(void);
bool invwin_inventory(struct dc_map *map, struct pl_player *plr);

void ui_animate_projectile(struct dc_map *map, coord_t path[], int path_len);
#endif /* UI_H */
