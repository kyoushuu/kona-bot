## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(MYSQL_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g \
	 -O2

bin_PROGRAMS = kona-bot

kona_bot_SOURCES = \
	main.c \
	io-utils.c \
	io-utils.h \
	database.c \
	database.h \
	irc.c \
	irc.h \
	genre.c \
	genre.h \
	str-utils.h

kona_bot_LDFLAGS = 

kona_bot_LDADD = \
	$(MYSQL_LIBS)

