# $Id$

AM_CPPFLAGS = -I$(top_srcdir)/libllcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = npp-server

npp_server_SOURCES = npp-server.c

npp_server_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libllcp/libllcp.la
