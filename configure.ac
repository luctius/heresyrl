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
#LT_CONFIG_LTDL_DIR([libltdl])
#LTDL_INIT([nonrecursive])

AC_DEFINE([_GNU_SOURCE], 1, [Use GNU extensions])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_mac=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        LT_INIT([dlopen])
        ;;
    cygwin*|mingw*)
        build_windows=yes
        LT_INIT([win32-dll])
        ;;
    darwin*)
        build_mac=yes
        LT_INIT([dlopen])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

# Checks for libraries.
AC_CHECK_LIB([m], [abs])
AC_CHECK_LIB([dl], [dlopen])

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

AM_CONDITIONAL([READLINE], [test "x$enable_wizard_mode" = "xyes"])
AM_COND_IF([READLINE], [
    dnl Do the stuff needed for enabling the feature
    AX_LIB_READLINE
    AC_DEFINE([CMDLINE_PARSER_VERSION], VERSION " (" GIT_VERSION "-wizard) ", [extended version])
], [AC_DEFINE([CMDLINE_PARSER_VERSION], VERSION " (" GIT_VERSION") ", [extended version])])

AC_PATH_PROGS([GENGETOPT], [gengetopt])
AM_CONDITIONAL([GENGETOPT], [test x$GENGETOPT != x])
AC_ARG_VAR([GENGETOPT], [application to generate the commandline arguments code])

AC_ARG_ENABLE([sdl2],
    AS_HELP_STRING([--enable-sdl2], [Enable sdl2 compatibility]))
AM_CONDITIONAL([HAVE_SDL2], [test "x$enable_sdl2" = "xyes"])
AM_COND_IF([HAVE_SDL2], [
    PKG_CHECK_MODULES([sdl2], [sdl2],  [], [])
    if test "x$sdl2_LIBS" = "x"; then
        AC_MSG_ERROR([SDL2 not found!])
        fi
    PKG_CHECK_MODULES([libpng], [libpng],  [], [])
    if test "x$libpng_LIBS" = "x"; then
        AC_MSG_ERROR([libpng not found!])
        fi
], [])

AC_ARG_ENABLE([test_uncursed],
    AS_HELP_STRING([--enable-test_uncursed], [Enable the test_uncursed test application for libuncursed]))
AM_CONDITIONAL([HAVE_TEST_UNCURSED], [test "x$enable-test_uncursed" = "xyes"])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h locale.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h termios.h unistd.h wchar.h wctype.h])

# Checks for library functions.
# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
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
AC_CHECK_TYPES([ptrdiff_t])

AC_FUNC_MALLOC
AC_FUNC_MBRTOWC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit memmove memset mkdir select setlocale strchr strdup strrchr strtol wcwidth strnlen])

CC_CHECK_CFLAG_APPEND([-Wall -Wextra -Wformat=2 -Wswitch-default])
CC_CHECK_CFLAG_APPEND([-Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wwrite-strings -Wno-aggressive-loop-optimizations -Wlogical-op -Wno-uninitialized -Wshadow])
CC_CHECK_CFLAG_APPEND([-fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer])
CC_CHECK_CFLAG_APPEND([-fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=alignment -fsanitize=object-size -fsanitize=enum -fsanitize=bool])


AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES(tests/atlocal)

AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 heresyrl.ggo])
AC_OUTPUT

