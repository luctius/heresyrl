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
