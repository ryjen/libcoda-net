cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

# add options for testing
option(CODE_COVERAGE "Enable code coverage testing." OFF)
option(MEMORY_CHECK "Enable testing for memory leaks." OFF)

option(WITH_CURL "Compile http_client using libcurl." OFF)
option(WITH_SSL "Enable secure socket layers on sockets." OFF)

# define project name
project (arg3net VERSION 0.3.0)

# set path to custom modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package (Threads)

if (WITH_CURL)
	find_package (CURL)
endif()

if (WITH_SSL)
	find_package(OpenSSL)
endif ()

# add target for code coverage
if(CODE_COVERAGE)
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
	setup_target_for_coverage(${PROJECT_NAME}-coverage ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME}-test ${PROJECT_SOURCE_DIR}/coverage)
endif()

# create package config
include(CreatePackage)
create_package("a c++ networking library")

# add config header file
include(CreateConfigHeader)
create_config_header()

# add directories
add_subdirectory(src)
add_subdirectory(tests)

# Setup testing
enable_testing()

include(MemCheckTest)
	
add_opt_memcheck_test(MEMORY_CHECK ${PROJECT_NAME}-test ${PROJECT_BINARY_DIR}/tests/${PROJECT_NAME}-test "--suppressions=${CMAKE_SOURCE_DIR}/suppression.map")

