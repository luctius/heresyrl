ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-Isrc/
bin_PROGRAMS = heresyrl

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)
AM_CFLAGS= -Wall -O0 -ggdb -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wwrite-strings -Wno-aggressive-loop-optimizations -Wlogical-op -Wno-uninitialized -fsanitize=address -fno-omit-frame-pointer -Wshadow -fsanitize=undefined -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=alignment -fsanitize=object-size -fsanitize=enum -fsanitize=bool 
#-Wpadded -Waggregate-return #this is not something you want during general development

heresyrl_SOURCES =  \
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
