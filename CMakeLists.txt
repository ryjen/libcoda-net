cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

# add options for testing
option(ENABLE_COVERAGE "Enable code coverage testing." OFF)
option(ENABLE_MEMCHECK "Enable testing for memory leaks." OFF)
option(ENABLE_PROFILING "Enable profiling code paths." OFF)

option(WITH_CURL "Compile http client using libcurl." ON)
option(WITH_SSL "Compile sockets with OpenSSL support." ON)
option(WITH_URIPARSER "Use liburiparser for uri parsing." ON)

# define project name
project (rj_net VERSION 0.3.0)

# set path to custom modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package (Threads)
find_package (PkgConfig)

set(PROJECT_NAME_TELNET "${PROJECT_NAME}_telnet")

set(PROJECT_NAME_HTTP "${PROJECT_NAME}_http")

set(TEST_PROJECT_NAME "${PROJECT_NAME}_test")

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
	endif ()
endif()

# add target for code coverage
if(ENABLE_COVERAGE)
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
	setup_target_for_coverage(TARGET ${PROJECT_NAME}_coverage OUTPUT ${PROJECT_BINARY_DIR}/coverage)
endif()

# create package config
include(CreatePackage)
create_package("a c++ networking library")

# add directories
add_subdirectory(src)
add_subdirectory(tests)

# Setup testing
enable_testing()

include(ValgrindTest)
	
add_valgrind_profile_test(PROFILING ${ENABLE_PROFILING} MEMCHECK ${ENABLE_MEMCHECK} TARGET ${TEST_PROJECT_NAME} VALGRIND_ARGS "--suppressions=${CMAKE_SOURCE_DIR}/suppression.map")

