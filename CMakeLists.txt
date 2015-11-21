cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

# set path to custom modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# add options for testing
option(CODE_COVERAGE "Enable code coverage testing." OFF)
option(MEMORY_CHECK "Enable testing for memory leaks." OFF)

option(WITH_CURL "Compile http_client using libcurl." OFF)
option(WITH_SSL "Enable secure socket layers on sockets." ON)

if(CODE_COVERAGE)
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
endif()

# define project name
project (arg3net)

find_package (Threads)

if (WITH_CURL)
	find_package (CURL)
endif()

if (WITH_SSL)
	find_package(OpenSSL)
endif ()

# add config header file
configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in ${CMAKE_SOURCE_DIR}/src/config.h)
add_definitions(-DHAVE_CONFIG_H)

# add directories
add_subdirectory(src)
add_subdirectory(tests)

# Setup testing
enable_testing()

if (MEMORY_CHECK)
	include(MemCheckTest)
	add_memcheck_test(net-test ${CMAKE_BINARY_DIR}/tests/arg3net_test)
else ()
	add_test(net-test ${CMAKE_BINARY_DIR}/tests/arg3net_test)
endif()

# add target for code coverage
if(CODE_COVERAGE)
	setup_target_for_coverage(net-coverage ${CMAKE_BINARY_DIR}/tests/arg3net_test ${CMAKE_SOURCE_DIR}/coverage)
endif()

