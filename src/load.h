/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

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
bool ld_read_save_file(const char *path, struct gm_game *g);

#endif /* LOAD_H */
