AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I$(top_srcdir)/utils
bin_PROGRAMS = heresyrl

GIT_VERSION := $(shell git describe --abbrev=10 --dirty --always)

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
                    fov/digital_fov.c \
                    fov/fov.c \
                    fov/rpsc_fov.c \
                    items/items.c \
                    ui/ui.c \
                    ui/ui_common.c \
                    ui/character_creation_ui.c \
                    ui/animate.c 

heresyrl_CFLAGS = $(lua_CFLAGS) -ggdb -O0 -Wall -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wwrite-strings -Wno-aggressive-loop-optimizations -Wlogical-op -Wno-uninitialized -DVERSION=\"$(GIT_VERSION)\"
#-Wpadded  -Waggregate-return
heresyrl_LDADD = $(top_srcdir)/utils/libutils.la $(INTI_LIBS) -lm $(lua_LIBS)

cmdline.c cmdline.h: $(top_srcdir)/heresyrl.ggo
	sed -i "s:usr_prefix:${prefix}:g" $<
	gengetopt < $<
