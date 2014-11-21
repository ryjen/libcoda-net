
check_PROGRAMS = arg3net_test

arg3net_test_SOURCES = arg3.test.cpp http_client.test.cpp telnet_socket.test.cpp

AM_CPPFLAGS = @AM_CPPFLAGS@ -Ivendor/bandit

AM_LDFLAGS = @AM_LDFLAGS@ -luriparser

arg3net_test_LDADD = ../src/libarg3net.la

check: arg3net_test
	./arg3net_test

