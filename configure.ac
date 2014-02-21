AC_INIT(src/main.c)

PACKAGE=helloworld
VERSION=0.1.0

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AC_SEARCH_LIBS(initscr, ncurses, [], [AC_MSG_ERROR([Unable to find ncurses])], [])

AC_PROG_CC_C99

AC_OUTPUT(Makefile src/Makefile)
