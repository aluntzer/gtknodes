VAPIGEN_VAPIS = gtknodes.vapi

if ENABLE_VAPIGEN

include $(VAPIGEN_MAKEFILE)

gtknodes.vapi: $(top_builddir)/introspection/GtkNodes-0.1.gir gtknodes.deps

gtknodes_vapi_METADATADIRS = $(srcdir)
gtknodes_vapi_FILES = $(top_builddir)/introspection/GtkNodes-0.1.gir
gtknodes_vapi_VAPIDIRS = $(builddir)
gtknodes_vapi_DEPS = glib-2.0 gio-2.0 gtk+-3.0

vapidir = $(datadir)/vala/vapi
vapi_DATA = $(VAPIGEN_VAPIS) $(VAPIGEN_VAPIS:.vapi=.deps)

endif

CLEANFILES = $(VAPIGEN_VAPIS)
EXTRA_DIST = \
	gtknodes.deps
