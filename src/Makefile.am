AM_CFLAGS := $(GMODULE_CFLAGS)
AM_CFLAGS += $(GTHREAD_CFLAGS)
AM_CFLAGS += $(GLIB_CFLAGS)
AM_CFLAGS += $(GTK3_CFLAGS)
AM_CFLAGS += $(GIO_CFLAGS)
AM_CFLAGS += $(GLADE2_CFLAGS)
AM_CFLAGS += -I$(top_srcdir)/src
AM_CFLAGS += -Wunused -Wall -pedantic


lib_LTLIBRARIES = libgtknodes-0.1.la

libgtknodes_0_1_la_SOURCES = gtknodesocket.c \
                    	     gtknode.c \
		             gtknodeview.c

libgtknodes_0_1_la_LIBADD = $(GTK3_LIBS) $(GLIB_LIBS) $(GTHREAD_LIBS) $(GIO_LIBS)


pkginclude_HEADERS = gtknodesocket.h \
                     gtknode.h \
                     gtknodeview.h

CLEANFILES= $(BUILT_SOURCES)



