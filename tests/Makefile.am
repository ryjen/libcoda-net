
noinst_PROGRAMS = arg3net_test

arg3net_test_SOURCES = arg3.test.cpp rest_client.test.cpp telnet_client.test.cpp

AM_CPPFLAGS = -std=c++11 -stdlib=libc++ -Ivendor -Wall -Werror

arg3net_test_LDADD = ../src/libarg3net-0.2.la

all-local: arg3net_test$(EXEEXT)
	./arg3net_test
