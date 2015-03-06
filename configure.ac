AC_INIT(heresyrl, development)
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_MAKE_SET
#AC_PROG_CC
AC_PROG_CC_C99
#AM_PROG_CC_C_O
#AC_PROG_CXX
#AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
#AC_PROG_RANLIB
AC_PROG_MKDIR_P

AC_DEFINE([_GNU_SOURCE], 1, [Use GNU extensions])

AC_DEFINE([CMDLINE_PARSER_VERSION], VERSION " (" GIT_VERSION ") <" __DATE__ " " __TIME__">", [extended version])

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

AC_SEARCH_LIBS(initscr, ncursesw, [], [AC_MSG_ERROR([Unable to find ncursesw])], [])
#AC_SEARCH_LIBS(new_panel, panel, [], [AC_MSG_ERROR([Unable to find ncurses panel])], [])
#AC_SEARCH_LIBS(new_menu, menu,   [], [AC_MSG_ERROR([Unable to find ncurses menu])], [])
#AC_SEARCH_LIBS(new_form, form,   [], [AC_MSG_ERROR([Unable to find ncurses form])], [])
AC_PATH_PROGS(GENGETOPT, gengetopt)

# Checks for header files.
AC_CHECK_HEADERS([float.h inttypes.h limits.h stddef.h stdint.h stdlib.h string.h sys/param.h unistd.h])

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

AC_CONFIG_FILES([Makefile
                 heresyrl.ggo])
AC_OUTPUT

