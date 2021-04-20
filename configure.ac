AC_INIT([gtknodes], [0.1], [armin.luntzer@univie.ac.at], [])
AM_INIT_AUTOMAKE([subdir-objects foreign dist-xz])

AC_PROG_CC
AC_CONFIG_MACRO_DIRS([m4])
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

AC_PREFIX_DEFAULT([/usr])

LT_INIT([dlopen])

GOBJECT_INTROSPECTION_CHECK([1.40.0])

m4_ifdef([VAPIGEN_CHECK], [
VAPIGEN_CHECK([0.48])
],[
AM_CONDITIONAL([ENABLE_VAPIGEN], false)
])


m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.31],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])


PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GIO],  [gio-2.0])
PKG_CHECK_MODULES([GTK3], [gtk+-3.0 >= 3.24.4])
PKG_CHECK_MODULES([GLADE2], [gladeui-2.0],
		  [ac_gladeui_catdir=`$PKG_CONFIG --variable=catalogdir gladeui-2.0`],
		  [ac_gladeui_catdir=;])


if test "x$ac_gladeui_catdir" = x ; then
  AC_MSG_ERROR([Couldn't find glade-2.0 catalog directory])
fi

AC_SUBST(GLADEUI_CATDIR, $ac_gladeui_catdir)



AC_CONFIG_FILES([Makefile
		 src/Makefile
		 introspection/Makefile
		 vapi/Makefile
		 examples/Makefile
		 glade/Makefile
		 docs/reference/gtknodes/Makefile
		 gtknodes.pc
		])
AC_OUTPUT
