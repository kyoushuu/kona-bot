dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_CONFIG_MACRO_DIR([m4])

AC_INIT(kona-bot, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC

AC_CHECK_TOOL([MYSQL_CONFIG], [mysql_configa], [AC_MSG_ERROR(mysql_config is required to build)])


dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=kona-bot
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT


AC_ARG_WITH(libdir-name,
    AS_HELP_STRING([--with-libdir-name[=@<:@<dir. name>@:>@]],
                   [Speficy the library name used to locate libraries, relative to the prefix of each install (for example lib64)]),
                   [lib=$withval],
		   [lib=""])

dnl Test for MySQL
MYSQL_CHECK($lib)


AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
