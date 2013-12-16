# $Id$

AM_CPPFLAGS = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = snep-client

snep_client_SOURCES = snep-client.c

snep_client_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
