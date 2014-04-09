#ifndef UI_H
#define UI_H

#include "heresyrl_def.h"
#include "coord.h"

bool ui_create(int cols, int lines);
void ui_destroy(void);

void update_screen(void);

void mapwin_display_map(struct dm_map *map, coord_t *player);
void mapwin_overlay_examine_cursor(struct dm_map *map, coord_t *p_pos);
bool mapwin_overlay_fire_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos);
bool mapwin_overlay_throw_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos);

void msgwin_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void msgwin_log_refresh(struct logging *log, struct log_entry *le);

void charwin_refresh(void);
bool invwin_inventory(struct dm_map *map, struct pl_player *plr);

void character_window(void);
void log_window(void);
bool char_creation_window(void);

#endif /* UI_H */
