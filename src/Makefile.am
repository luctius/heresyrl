AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I$(top_srcdir)/utils -I.
bin_PROGRAMS = heresyrl

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)
AM_CFLAGS= -ggdb -O0 -Wall -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wwrite-strings -Wno-aggressive-loop-optimizations -Wlogical-op -Wno-uninitialized -fsanitize=address -fno-omit-frame-pointer -Wshadow

heresyrl_SOURCES =  \
                    cmdline.c \
                    main.c \
                    logging.c \
                    tiles.c \
                    random.c \
                    heresyrl_priv.c \
                    game.c \
                    player.c \
                    coord.c \
                    fight.c \
                    inventory.c \
                    linewrap.c \
                    input.c \
                    dowear.c \
                    load.c \
                    save.c \
                    options.c \
                    spawn.c \
                    turn_tick.c \
                    dungeon/dungeon_cave.c \
                    dungeon/dungeon_map.c \
                    ai/ai.c \
                    ai/ai_utils.c \
                    ai/pathfinding.c \
                    monster/monster_action.c \
                    monster/monster.c \
                    monster/conditions.c \
                    fov/sight.c \
                    fov/rpsc_fov.c \
                    items/items.c \
                    ui/ui.c \
                    ui/ui_common.c \
                    ui/character_creation_ui.c \
                    ui/animate.c 

heresyrl_CFLAGS = $(lua_CFLAGS)  -DGIT_VERSION=\"$(GIT_VERSION)\"
#-Wpadded -Waggregate-return #this is not something you want during general development
heresyrl_LDADD = ../utils/libutils.la $(INTI_LIBS) -lm $(lua_LIBS)

cmdline.c cmdline.h: ../heresyrl.ggo
	sed -i "s:usr_prefix:${prefix}:g" $<
	gengetopt < $<
