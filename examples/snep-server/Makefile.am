# $Id$

AM_CPPFLAGS = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = snep-server

snep_server_SOURCES = snep-server.c

snep_server_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
