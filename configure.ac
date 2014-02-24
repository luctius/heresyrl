AC_INIT(src/main.c)

PACKAGE=heresyrl
VERSION=0.1.0

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AC_SEARCH_LIBS(initscr, ncurses, [], [AC_MSG_ERROR([Unable to find ncurses])], [])
AC_SEARCH_LIBS(new_panel, panel, [], [AC_MSG_ERROR([Unable to find ncurses panel])], [])
AC_SEARCH_LIBS(new_menu, menu, [], [AC_MSG_ERROR([Unable to find ncurses menu])], [])
AC_SEARCH_LIBS(new_form, form, [], [AC_MSG_ERROR([Unable to find ncurses form])], [])

AC_PROG_CC_C99

AC_OUTPUT(Makefile src/Makefile)
