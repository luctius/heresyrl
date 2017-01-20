ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I$(top_srcdir)/src/ -I$(top_srcdir)/libuncursed/include
bin_PROGRAMS = heresyrl 
lib_LTLIBRARIES = libuncursed.la
noinst_LTLIBRARIES =
noinst_PROGRAMS =

GIT_VERSION=$(shell cd $(top_srcdir) && git describe --abbrev=10 --dirty --always)

###

libuncursed_CFLAGS_ADD=
if WINDOWS
AM_CPPFLAGS += -I$(top_srcdir)/compat
libuncursed_CFLAGS_ADD += -D'AIMAKE_IMPORT(x)=__declspec(dllimport) x'
libuncursed_CFLAGS_ADD += -D'AIMAKE_EXPORT(x)=__declspec(dllexport) x'
libuncursed_CFLAGS_ADD += -DAIMAKE_BUILDOS_MSWin32
else
libuncursed_CFLAGS_ADD += -DAIMAKE_IMPORT(x)=
libuncursed_CFLAGS_ADD += -DAIMAKE_EXPORT(x)=

if LINUX
libuncursed_CFLAGS_ADD += -DAIMAKE_BUILDOS_linux
endif

if DARWIN
libuncursed_CFLAGS_ADD += -DAIMAKE_BUILDOS_darwin
endif

endif

libuncursed_la_SOURCES=libuncursed/src/libuncursed.c libuncursed/src/plugins.c
libuncursed_la_CFLAGS= $(libuncursed_CFLAGS_ADD)
libuncursed_la_LDFLAGS=-no-undefined
libuncursed_la_includedir=$(top_srcdir)/libuncursed/include
libuncursed_la_DEPENDENCIES=
libuncursed_la_LIBADD=

if WINDOWS
libuncursed_la_DEPENDENCIES += libuncursed_wincon.la
libuncursed_la_LIBADD += -luncursed_wincon

noinst_LTLIBRARIES += libuncursed_wincon.la
libuncursed_wincon_la_SOURCES=libuncursed/src/plugins/wincon.c libuncursed/src/plugins/wincon_cxx.cxx
libuncursed_wincon_la_CFLAGS= $(libuncursed_CFLAGS_ADD)
libuncursed_wincon_la_CXXFLAGS= $(libuncursed_CFLAGS_ADD)
libuncursed_wincon_la_LDFLAGS=-avoid-version -no-undefined
libuncursed_wincon_la_includedir=$(top_srcdir)/libuncursed/include
else
libuncursed_la_DEPENDENCIES += libuncursed_tty.la
libuncursed_la_LIBADD += -ldl -luncursed_tty

noinst_LTLIBRARIES += libuncursed_tty.la
libuncursed_tty_la_SOURCES=libuncursed/src/plugins/tty.c libuncursed/src/plugins/tty_cxx.cxx
libuncursed_tty_la_CFLAGS= $(libuncursed_CFLAGS_ADD)
libuncursed_tty_la_CXXFLAGS= $(libuncursed_CFLAGS_ADD)
libuncursed_tty_la_LDFLAGS=-avoid-version -no-undefined
libuncursed_tty_la_includedir=$(top_srcdir)/libuncursed/include
endif

if HAVE_TEST_UNCURSED
noinst_PROGRAMS += test_uncursed
test_uncursed_DEPENDENCIES=libuncursed.la
test_uncursed_SOURCES=libuncursed/src/test_uncursed.c
test_uncursed_CFLAGS=-DUNCURSED_MAIN_PROGRAM $(libuncursed_CFLAGS_ADD)
test_uncursed_LDADD=-ldl -luncursed
test_uncursed_LDFLAGS=-no-install
test_uncursed_includedir=$(top_srcdir)/libuncursed/include
endif

if HAVE_SDL2
lib_LTLIBRARIES += libuncursed_sdl.la
libuncursed_sdl_la_DEPENDENCIES=libuncursed.la
libuncursed_sdl_la_SOURCES=libuncursed/src/plugins/sdl.c libuncursed/src/plugins/sdl_cxx.cxx
libuncursed_sdl_la_CFLAGS=$(sdl2_CFLAGS) $(libpng_CFLAGS) $(libuncursed_CFLAGS_ADD)
libuncursed_sdl_la_CXXFLAGS=$(sdl2_CFLAGS) $(libpng_CFLAGS) $(libuncursed_CFLAGS_ADD)
libuncursed_sdl_la_LDFLAGS=-avoid-version -no-undefined
libuncursed_sdl_la_LIBADD=$(sdl2_LIBS) $(libpng_LIBS) -luncursed
libuncursed_sdl_la_DATA=$(top_srcdir)/data/fonts/font14.png
libuncursed_sdl_ladir=$(prefix)/@PACKAGE@/data/fonts
libuncursed_sdl_la_includedir=$(top_srcdir)/libuncursed/include
if WINDOWS
libuncursed_sdl_la_LIBADD += -lWs2_32
endif
endif

###

heresyrl_SOURCES =  \
				src/save.h \
				src/player.h \
				src/heresyrl_def.h \
				src/input.h \
				src/ui/ui_common.h \
				src/ui/ui.h \
				src/ui/animate.h \
				src/dowear.h \
				src/random.h \
				src/cqc.h \
				src/coord.h \
				src/enums.h \
				src/load.h \
				src/game.h \
				src/items/items_static.h \
				src/items/items_static_def.h \
				src/items/items.h \
				src/logging.h \
				src/inventory.h \
				src/turn_tick.h \
				src/random_generator.h \
				src/status_effects/ground_effects.h \
				src/status_effects/ground_effects_static.h \
				src/status_effects/ground_effects_static_def.h \
				src/status_effects/status_effects.h \
				src/status_effects/ground_effects.h \
				src/status_effects/status_effects_critical_blunt.h \
				src/status_effects/status_effects_static.h \
				src/status_effects/status_effects_static_def.h  \
				src/monster/monster_static.h \
				src/monster/monster_static_def.h    \
				src/monster/monster_action.h \
				src/monster/monster.h \
				src/dungeon/dungeon_dla.h \
				src/dungeon/dungeon_map.h \
				src/dungeon/cellular_automata.h \
				src/dungeon/diffusion_limited_aggregation.h \
				src/dungeon/spawn.h \
				src/dungeon/dungeon_plain.h \
				src/dungeon/tiles.h \
				src/dungeon/dungeon_cave.h \
				src/dungeon/dungeon_room.h \
				src/dungeon/dungeon_bsp.h \
				src/quests/quests.h \
				src/quests/quests_static_def.h \
				src/quests/quests_static.h \
				src/careers/careers.h \
				src/careers/careers_static.h \
				src/careers/careers_static_def.h    \
				src/fov/sight.h \
				src/fov/rpsc_fov.h \
				src/ai/pathfinding.h \
				src/ai/ai.h \
				src/ai/ai_utils.h \
				src/cmdline.h \
				src/fight.h \
				src/options.h \
				src/wizard/wizard_mode.h \
				src/wizard/wizard_commands.h \
				src/cmdline.c \
				src/main.c \
				src/logging.c \
				src/random.c \
				src/heresyrl_priv.c \
				src/game.c \
				src/player.c \
				src/coord.c \
				src/fight.c \
				src/inventory.c \
				src/input.c \
				src/dowear.c \
				src/load.c \
				src/save.c \
				src/options.c \
				src/turn_tick.c \
				src/random_generator.c \
				src/careers/careers.c \
				src/dungeon/tiles.c \
				src/dungeon/dungeon_cave.c \
				src/dungeon/dungeon_room.c \
				src/dungeon/dungeon_plain.c \
				src/dungeon/dungeon_bsp.c \
				src/dungeon/dungeon_dla.c \
				src/dungeon/dungeon_map.c \
				src/dungeon/spawn.c \
				src/dungeon/cellular_automata.c \
				src/dungeon/diffusion_limited_aggregation.c \
				src/quests/quests.c \
				src/ai/ai.c \
				src/ai/ai_utils.c \
				src/ai/pathfinding.c \
				src/monster/monster_action.c \
				src/monster/monster.c \
				src/status_effects/status_effects.c \
				src/status_effects/ground_effects.c \
				src/fov/sight.c \
				src/fov/rpsc_fov.c \
				src/items/items.c \
				src/ui/ui.c \
				src/ui/ui_common.c \
				src/ui/character_creation_ui.c \
				src/ui/animate.c

if READLINE
heresyrl_SOURCES += src/wizard/wizard_commands.c \
				src/wizard/wizard_mode.c
endif

heresyrl_CFLAGS = $(lua_CFLAGS)  -DGIT_VERSION=\"$(GIT_VERSION)\" $(AM_CFLAGS) $(libuncursed_CFLAGS_ADD)
heresyrl_LDFLAGS = -no-install
heresyrl_LDADD = $(INTI_LIBS) -lm $(lua_LIBS) $(LIBS) -luncursed
heresyrl_DEPENDENCIES=libuncursed.la

if GENGETOPT
src/cmdline.c src/cmdline.h: heresyrl.ggo
	$(AM_V_GEN)$(SED) -i".bkp" "s:usr_prefix:${prefix}:g" $<
	$(AM_V_GEN)$(GENGETOPT) --output-dir=src/ < $<
	$(AM_V_GEN)distcleancheck_listfiles="src/cmdline.c src/cmdline.h"
endif

#clean-local:
#-rm -f src/cmdline.c
#-rm -f src/cmdline.h

heresyrl_tmpdir=/tmp/heresyrl-dist/
heresyrl_release=$(PACKAGE)-$(VERSION)-$(shell gcc -dumpmachine)

dist-binary:
	$(AM_V_GEN)rm heresyrl_release.tar.gz || :
	$(AM_V_GEN)mkdir -p $(heresyrl_tmpdir)/$(heresyrl_release)
	$(AM_V_GEN)cp $(bin_PROGRAMS) $(heresyrl_tmpdir)/$(heresyrl_release)
	$(AM_V_GEN)tar -C $(heresyrl_tmpdir) -cvf $(heresyrl_release).tar $(heresyrl_release)/$(bin_PROGRAMS)
	$(AM_V_GEN)gzip $(heresyrl_release).tar
	$(AM_V_GEN)rm -rf $(heresyrl_tmpdir)

SUBDIRS=tests

