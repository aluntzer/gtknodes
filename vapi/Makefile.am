VAPIGEN_VAPIS = libgtknodes.vapi

if ENABLE_VAPIGEN

include $(VAPIGEN_MAKEFILE)

libgtknodes.vapi: $(top_builddir)/introspection/GtkNodes-0.1.gir libgtknodes.deps

libgtknodes_vapi_METADATADIRS = $(srcdir)
libgtknodes_vapi_FILES = $(top_builddir)/introspection/GtkNodes-0.1.gir
libgtknodes_vapi_VAPIDIRS = $(builddir)
libgtknodes_vapi_DEPS = glib-2.0 gio-2.0 gtk+-3.0

vapidir = $(datadir)/vala/vapi
vapi_DATA = $(VAPIGEN_VAPIS) $(VAPIGEN_VAPIS:.vapi=.deps)

endif

CLEANFILES = $(VAPIGEN_VAPIS)
EXTRA_DIST = \
	libgtknodes.deps
