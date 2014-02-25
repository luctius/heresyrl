INCLUDES=-I$(top_srcdir)/utils
bin_PROGRAMS = heresyrl

heresyrl_SOURCES = main.c simple_dungeon.c map_display.c logging.c
heresyrl_CFLAGS = -ggdb -O0
heresyrl_LDADD = $(top_srcdir)/utils/libutils.la $(INTI_LIBS) -lm
