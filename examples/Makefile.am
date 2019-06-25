AM_CFLAGS := $(GMODULE_CFLAGS)
AM_CFLAGS += $(GTHREAD_CFLAGS)
AM_CFLAGS += $(GLIB_CFLAGS)
AM_CFLAGS += $(GTK3_CFLAGS)
AM_CFLAGS += $(GIO_CFLAGS)
AM_CFLAGS += $(GLADE2_CFLAGS)
AM_CFLAGS += -I$(top_srcdir)/src
AM_CFLAGS += -I$(top_srcdir)/examples/nodes/include
AM_CFLAGS += -Wunused -Wall -pedantic

libtool_opts = -export-dynamic

LDADD := $(top_builddir)/src/libgtknodes-0.1.la
LDADD += libnodes.la
LDADD += $(GTK3_LIBS) $(GLIB_LIBS) $(GTHREAD_LIBS) $(GIO_LIBS)


lib_LTLIBRARIES = libnodes.la

libnodes_la_SOURCES = nodes/node_pulse.c \
		      nodes/node_step.c  \
		      nodes/node_binary_decode.c \
		      nodes/node_binary_encode.c \
		      nodes/node_convert_number.c \
		      nodes/node_show_number.c \
		      nodes/node_bitwise_and.c \
		      nodes/node_bitwise_or.c \
		      nodes/node_bitwise_xor.c \
		      nodes/node_bitwise_not.c

demo_SOURCES = demo.c

noinst_PROGRAMS = demo
