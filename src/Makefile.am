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


-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = gtknodesocket.c \
			gtknodesocket.h \
			gtknode.c \
			gtknode.h \
			gtknodeview.c \
			gtknodeview.h

GtkNodes-0.1.gir: $(INTROSPECTION_SCANNER) libgtknodes-0.1.la Makefile

GtkNodes_0_1_gir_NAMESPACE = GtkNodes
GtkNodes_0_1_gir_INCLUDES = GObject-2.0 Gtk-3.0
GtkNodes_0_1_gir_CFLAGS = $(INCLUDES)
GtkNodes_0_1_gir_LIBS = libgtknodes-0.1.la
GtkNodes_0_1_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += GtkNodes-0.1.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
