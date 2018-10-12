cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

# add options for testing
option(ENABLE_COVERAGE "Enable code coverage testing." OFF)
option(ENABLE_MEMCHECK "Enable testing for memory leaks." OFF)
option(ENABLE_PROFILING "Enable profiling code paths." OFF)

option(WITH_CURL "Compile http client using libcurl." ON)
option(WITH_SSL "Compile sockets with OpenSSL support." ON)
option(WITH_URIPARSER "Use liburiparser for uri parsing." ON)

# define project name
project (coda_net VERSION 0.3.0)

# set path to custom modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package (Threads)
find_package (PkgConfig)

set(PROJECT_NAME_TELNET "${PROJECT_NAME}_telnet")

set(PROJECT_NAME_HTTP "${PROJECT_NAME}_http")

set(PROJECT_NAME_ASYNC "${PROJECT_NAME}_async")

set(PROJECT_NAME_SYNC "${PROJECT_NAME}_sync")

include (CheckIncludeFiles)
check_include_files("sys/epoll.h" EPOLL_FOUND)

if (EPOLL_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEPOLL_FOUND")
endif()

if (WITH_CURL)
	find_package (CURL REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCURL_FOUND")
endif()

if (WITH_SSL)
	find_package (OpenSSL)
	if (OPENSSL_FOUND)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENSSL_FOUND")
	endif()
endif ()

if (WITH_URIPARSER)
	pkg_check_modules (URIPARSER liburiparser)
	if (URIPARSER_FOUND)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DURIPARSER_FOUND")
		link_directories(${URIPARSER_LIBRARY_DIRS})
	endif ()
endif()

# create package config
include(CreatePackages)
create_packages(DESCRIPTION "a c++ networking library")

# Setup testing
enable_testing()

# add directories
add_subdirectory(src)
add_subdirectory(tests)

