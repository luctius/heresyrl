INCLUDES=-I$(top_srcdir)/utils
bin_PROGRAMS = heresyrl

heresyrl_SOURCES = main.c simple_dungeon.c map_display.c logging.c tiles.c monster.c
heresyrl_CFLAGS = -ggdb -O0 -Wall -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings  -Wunreachable-code -Wstrict-aliasing=2 -Wno-uninitialized
heresyrl_LDADD = $(top_srcdir)/utils/libutils.la $(INTI_LIBS) -lm
