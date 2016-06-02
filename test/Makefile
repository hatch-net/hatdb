#-------------------------------------------------------------------------
#
# Makefile for src/bin/ux_ctl
#
# Portions Copyright (c) 1996-2015, UxsinoDB Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/bin/ux_ctl/Makefile
#
#-------------------------------------------------------------------------

UXFILEDESC = "ux_ctl - starts/stops/restarts the Uxsino Database SQL server"
UXAPPICON=win32

subdir = src/test/test
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := -I$(libuxsql_srcdir) $(CPPFLAGS)

OBJS=	test.o $(WIN32RES)
LIBS+= -lrt
all: test
CFLAGS += -g
test: $(OBJS) | submake-libuxsql submake-libuxport
	$(CC) $(CFLAGS) $(OBJS) $(libuxsql_uxport) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)


clean:
	rm *.o test
