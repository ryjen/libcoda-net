

lib_LTLIBRARIES = libarg3net.la

libarg3net_la_LDFLAGS = -version-info 0:0:0

libarg3net_la_SOURCES = buffered_socket.cpp http_client.cpp socket.cpp socket_factory.cpp socket_server.cpp telnet_socket.cpp

libarg3net_la_CXXFLAGS = $(LIBARG3NET_LA_CXXFLAGS) $(COVERAGE_CFLAGS)

pkginclude_HEADERS = buffered_socket.h exception.h protocol.h http_client.h socket.h socket_factory.h socket_server.h telnet_socket.h
