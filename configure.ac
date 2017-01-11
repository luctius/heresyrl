AC_INIT(heresyrl, development, luctius@gmail.com, heresyrl)
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE([1.11 foreign silent-rules subdir-objects -Wall])
AM_MAINTAINER_MODE

AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_MAKE_SET
#AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
#AC_PROG_CXX
#AC_PROG_AWK
AC_PROG_SED
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P

AC_PROG_CXX
AM_PROG_AR
AC_PROG_LIBTOOL
LT_INIT([dlopen])
#LT_CONFIG_LTDL_DIR([libltdl])
#LTDL_INIT([nonrecursive])

AC_DEFINE([_GNU_SOURCE], 1, [Use GNU extensions])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

PKG_CHECK_MODULES([lua], [lua],  [], [
    PKG_CHECK_MODULES([lua], [lua5.1], [],  [
        PKG_CHECK_MODULES([lua], [lua5.2], [],[]  )
    ] )
] )
if test "x$lua_LIBS" = "x"; then
    AC_MSG_ERROR([Lua not found!])
fi

#AC_SEARCH_LIBS(initscr, ncurses, [], [AC_MSG_ERROR([Unable to find ncurses])], [])

dnl Example of default-disabled feature
AC_ARG_ENABLE([wizard_mode],
    AS_HELP_STRING([--enable-wizard_mode], [Enable feature wizard mode]))

AS_IF([test "x$enable_wizard_mode" = "xyes"], [
    dnl Do the stuff needed for enabling the feature
    AX_LIB_READLINE
    AC_DEFINE([CMDLINE_PARSER_VERSION], VERSION " (" GIT_VERSION "-wizard) ", [extended version])
], [AC_DEFINE([CMDLINE_PARSER_VERSION], VERSION " (" GIT_VERSION") ", [extended version])])


AC_PATH_PROGS([GENGETOPT], [gengetopt])
AM_CONDITIONAL([GENGETOPT], [test x$GENGETOPT != x])
AC_ARG_VAR([GENGETOPT], [application to generate the commandline arguments code])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h limits.h stddef.h stdint.h stdlib.h string.h sys/param.h unistd.h])

# Checks for library functions.
# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_HEADER_ASSERT
AC_LANG_C
AC_C_INLINE
AC_C_CONST
AC_STRUCT_TM
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_FUNCS([memmove memset mkdir strdup strtol])


AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES(tests/atlocal)

AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 heresyrl.ggo])
AC_OUTPUT

