# $Id$

AM_CPPFLAGS = -I$(top_srcdir)/libllcp
LIBS = -lrt
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = llcp-test-server

llcp_test_server_SOURCES = llcp-test-server.c \
			   connected-echo-server.c \
			   connectionless-echo-server.c

llcp_test_server_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libllcp/libllcp.la

EXTRA_DIST = connected-echo-server.h \
	     connectionless-echo-server.h
