#ifndef LOAD_H
#define LOAD_H

#include <stdbool.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "heresyrl_def.h"

/** 
* Opens the configfile as set in options.
* Then evaluates the aqcuired Lua State for the possible
* settings and puts them into the options structure. When
* an setting is not found; the default is set.
*
* When no file is found or the file was not valid 
* 
* @return 0 on success or 1 on failure.
*/
int ld_read_save_file(const char *path, struct gm_game *g);

#endif /* LOAD_H */
