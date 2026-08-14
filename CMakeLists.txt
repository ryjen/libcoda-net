cmake_minimum_required(VERSION 3.21)

project(coda_net VERSION 0.3.0 LANGUAGES CXX)

# build/test options
option(CODA_BUILD_TESTS "Build libcoda-net tests." ON)
option(CODA_FETCH_DEPS "Allow libcoda-net to fetch missing third-party dependencies." OFF)
option(ENABLE_COVERAGE "Enable code coverage testing." OFF)
option(ENABLE_MEMCHECK "Enable testing for memory leaks." OFF)
option(ENABLE_PROFILING "Enable profiling code paths." OFF)

option(WITH_CURL "Compile http client using libcurl." ON)
option(WITH_SSL "Compile sockets with OpenSSL support." ON)
option(WITH_URIPARSER "Use liburiparser for uri parsing." ON)

# set path to custom modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads)
find_package(PkgConfig)
find_package(cereal CONFIG QUIET)

if (NOT cereal_FOUND AND NOT CODA_FETCH_DEPS)
	message(FATAL_ERROR
		"cereal is required. Install a cereal CMake package or configure with CODA_FETCH_DEPS=ON to allow the pinned fallback download.")
endif()

set(PROJECT_NAME_TELNET "${PROJECT_NAME}_telnet")
set(PROJECT_NAME_HTTP "${PROJECT_NAME}_http")
set(PROJECT_NAME_ASYNC "${PROJECT_NAME}_async")
set(PROJECT_NAME_SYNC "${PROJECT_NAME}_sync")

include(CheckIncludeFileCXX)
check_include_file_cxx("sys/epoll.h" EPOLL_FOUND)

if (WITH_CURL)
	find_package(CURL REQUIRED)
endif()

if (WITH_SSL)
	find_package(OpenSSL)
endif()

if (WITH_URIPARSER)
	pkg_check_modules(URIPARSER liburiparser)
	if (URIPARSER_FOUND)
		# Legacy raw-library lookup still needs the discovered library path. Moving
		# this to an imported dependency target is tracked in the next #2 slice.
		link_directories(${URIPARSER_LIBRARY_DIRS})
	endif()
endif()

# create package config
include(CreatePackages)
create_packages(DESCRIPTION "a c++ networking library")

# add directories
add_subdirectory(src)

if(CODA_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
