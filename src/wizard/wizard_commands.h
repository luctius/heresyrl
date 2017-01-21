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

#ifndef WIZARD_COMMANDS_H
#define WIZARD_COMMANDS_H

#include <wchar.h>

struct wz_cmd {
    const char *name;
    const char *descr;
    void (*func)(char *input);
};

const struct wz_cmd *wz_command_get(unsigned int i);
int wz_command_sz(void);

#endif /* WIZARD_COMMANDS_H */
