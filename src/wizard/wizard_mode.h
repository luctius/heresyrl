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

#ifndef WIZARD_MODE_H
#define WIZARD_MODE_H

#include "ui/ui_common.h"
#include "config.h"

#ifdef HAVE_LIBREADLINE
    void wz_init(void);
    void wz_exit(void);

    void wz_mode(void);
    void wz_mode_exit(void);
#else
    /* Trickery to avoid too much ifdefs */
    static void wz_init() {}
    static void wz_exit() {}
    static void wz_mode() {}
    static void wz_mode_exit() {}
#endif

#endif /* WIZARD_MODE_H */