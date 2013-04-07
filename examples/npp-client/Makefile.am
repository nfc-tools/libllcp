# $Id$

AM_CPPFLAGS = -I$(top_srcdir)/libllcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = npp-client

npp_client_SOURCES = npp-client.c

npp_client_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libllcp/libllcp.la
