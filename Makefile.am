ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I$(top_srcdir)/src/
bin_PROGRAMS = heresyrl

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)
AM_CFLAGS= -Wall -O0 -ggdb -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wwrite-strings -Wno-aggressive-loop-optimizations -Wlogical-op -Wno-uninitialized -fsanitize=address -fno-omit-frame-pointer -Wshadow -fsanitize=undefined -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=alignment -fsanitize=object-size -fsanitize=enum -fsanitize=bool 
#-Wpadded -Waggregate-return #this is not something you want during general development

heresyrl_SOURCES =  \
				src/save.h	\
				src/player.h	\
				src/heresyrl_def.h	\
				src/input.h	\
				src/ui/ui_common.h	\
				src/ui/ui.h	\
				src/ui/animate.h	\
				src/dowear.h	\
				src/random.h	\
				src/cqc.h	\
				src/coord.h	\
				src/enums.h	\
				src/load.h	\
				src/game.h	\
				src/items/items_static.h	\
                src/items/items_static_def.h \
				src/items/items.h	\
				src/logging.h	\
				src/inventory.h	\
				src/turn_tick.h	\
				src/status_effects/status_effects.h	\
				src/status_effects/ground_effects.h	\
				src/status_effects/status_effects_critical_blunt.h	\
				src/status_effects/status_effects_static.h	\
                src/status_effects/status_effects_static_def.h  \
				src/monster/monster_static.h	\
                src/monster/monster_static_def.h    \
				src/monster/monster_action.h	\
				src/monster/monster.h	\
				src/dungeon/dungeon_dla.h	\
				src/dungeon/dungeon_map.h	\
				src/dungeon/cellular_automata.h	\
				src/dungeon/diffusion_limited_aggregation.h	\
				src/dungeon/spawn.h	\
				src/dungeon/dungeon_plain.h	\
				src/dungeon/tiles.h	\
				src/dungeon/dungeon_cave.h	\
				src/dungeon/dungeon_room.h	\
				src/careers/careers.h	\
				src/careers/careers_static.h	\
                src/careers/careers_static_def.h    \
				src/fov/sight.h	\
				src/fov/rpsc_fov.h	\
				src/ai/pathfinding.h	\
				src/ai/ai.h	\
				src/ai/ai_utils.h	\
				src/cmdline.h	\
				src/fight.h	\
				src/options.h \
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
				src/careers/careers.c \
				src/dungeon/tiles.c \
				src/dungeon/dungeon_cave.c \
				src/dungeon/dungeon_room.c \
				src/dungeon/dungeon_plain.c \
				src/dungeon/dungeon_dla.c \
				src/dungeon/dungeon_map.c \
				src/dungeon/spawn.c \
				src/dungeon/cellular_automata.c \
				src/dungeon/diffusion_limited_aggregation.c \
				src/ai/ai.c \
				src/ai/ai_utils.c \
				src/ai/pathfinding.c \
				src/monster/monster_action.c \
				src/monster/monster.c \
				src/status_effects/status_effects.c \
				src/fov/sight.c \
				src/fov/rpsc_fov.c \
				src/items/items.c \
				src/ui/ui.c \
				src/ui/ui_common.c \
				src/ui/character_creation_ui.c \
				src/ui/animate.c 

heresyrl_CFLAGS = $(lua_CFLAGS)  -DGIT_VERSION=\"$(GIT_VERSION)\"
heresyrl_LDADD = $(INTI_LIBS) -lm $(lua_LIBS)

src/cmdline.c src/cmdline.h: heresyrl.ggo
	$(SED) -i "s:usr_prefix:${prefix}:g" $<
	$(GENGETOPT) --output-dir=src/ < $<

clean-local:
	-rm -f src/cmdline.c
	-rm -f src/cmdline.h

heresyrl_tmpdir=/tmp/heresyrl-dist/
heresyrl_release=$(PACKAGE)-$(VERSION)-$(shell gcc -dumpmachine)
dist-binary:
	rm heresyrl_release.tar.gz || :
	mkdir -p $(heresyrl_tmpdir)/$(heresyrl_release)
	cp $(bin_PROGRAMS) $(heresyrl_tmpdir)/$(heresyrl_release)
	tar -C $(heresyrl_tmpdir) -cvf $(heresyrl_release).tar $(heresyrl_release)/$(bin_PROGRAMS)
	gzip $(heresyrl_release).tar
	rm -rf $(heresyrl_tmpdir)
