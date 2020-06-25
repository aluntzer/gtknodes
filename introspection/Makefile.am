
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = \
			     --c-include=gtknode.h \
			     --add-include-path=$(top_srcdir)/src \
			     --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_srcdir)/src

if HAVE_INTROSPECTION
include $(INTROSPECTION_MAKEFILE)
introspection_sources = $(top_srcdir)/src/gtknodesocket.c \
			$(top_srcdir)/src/gtknodesocket.h \
			$(top_srcdir)/src/gtknode.c \
			$(top_srcdir)/src/gtknode.h \
			$(top_srcdir)/src/gtknodeview.c \
			$(top_srcdir)/src/gtknodeview.h

GtkNodes-0.1.gir: $(INTROSPECTION_SCANNER) $(top_srcdir)/src/libgtknodes-0.1.la Makefile

GtkNodes_0_1_gir_NAMESPACE = GtkNodes
GtkNodes_0_1_gir_INCLUDES = GObject-2.0 Gtk-3.0
GtkNodes_0_1_gir_CFLAGS = $(INCLUDES) -I$(top_srcdir)/src
GtkNodes_0_1_gir_LIBS = $(top_srcdir)/src/libgtknodes-0.1.la
GtkNodes_0_1_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += GtkNodes-0.1.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)

endif
