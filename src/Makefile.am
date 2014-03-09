INCLUDES=-I$(top_srcdir)/utils
bin_PROGRAMS = heresyrl

heresyrl_SOURCES = main.c dungeon_creator.c ui.c logging.c tiles.c monster.c random.c heresyrl_priv.c dungeon_cave.c game.c player.c items.c fov.c sight.c pathfinding.c coord.c fight.c inventory.c linewrap.c input.c dowear.c load.c save.c spawn.c event_sys.c
heresyrl_CFLAGS = $(lua_CFLAGS) -ggdb -O0 -Wall -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wno-uninitialized
heresyrl_LDADD = $(top_srcdir)/utils/libutils.la $(INTI_LIBS) -lm $(lua_LIBS)
