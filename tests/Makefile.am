
noinst_PROGRAMS = arg3net_test

arg3net_test_SOURCES = arg3.test.cpp http_client.test.cpp telnet_socket.test.cpp

AM_CPPFLAGS = @AM_CPPFLAGS@ -Ivendor/bandit

AM_LDFLAGS = @AM_LDFLAGS@

arg3net_test_LDADD = ../src/libarg3net-0.2.la

check: arg3net_test$(EXEEXT)
	./arg3net_test
