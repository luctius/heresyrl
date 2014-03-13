AC_INIT(src/main.c)

PACKAGE=heresyrl
VERSION=0.1.0

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_LANG_C
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

AC_DEFINE([_GNU_SOURCE], 1, [Use GNU extensions])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
PKG_CHECK_MODULES([lua], [lua], [], [PKG_CHECK_MODULES([lua], [lua5.1] )])
AC_SEARCH_LIBS(initscr, ncurses, [], [AC_MSG_ERROR([Unable to find ncurses])], [])
AC_SEARCH_LIBS(new_panel, panel, [], [AC_MSG_ERROR([Unable to find ncurses panel])], [])
AC_SEARCH_LIBS(new_menu, menu, [], [AC_MSG_ERROR([Unable to find ncurses menu])], [])
AC_SEARCH_LIBS(new_form, form, [], [AC_MSG_ERROR([Unable to find ncurses form])], [])

AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([pthread], [sem_init])

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h stddef.h stdint.h stdlib.h string.h sys/param.h unistd.h])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset])


AC_CONFIG_FILES([Makefile
                 utils/Makefile 
                 src/Makefile 
                 heresyrl.ggo])
AC_OUTPUT
