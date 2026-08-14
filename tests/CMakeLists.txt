
if (NOT DEFINED BANDIT_DIR)

    include(ExternalProject)

    set(BANDIT_TARGET ${PROJECT_NAME}-bandit)

    externalproject_add(${BANDIT_TARGET}
            GIT_REPOSITORY https://github.com/banditcpp/bandit.git
            GIT_TAG v2.0.0
            INSTALL_COMMAND ""
            BUILD_COMMAND ""
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/bandit)

    ExternalProject_Get_property(${PROJECT_NAME}-bandit SOURCE_DIR)

    set(BANDIT_DIR ${SOURCE_DIR})
endif()

set(TEST_PROJECT_NAME "${PROJECT_NAME}_test")
set(CURL_TEST_PROJECT_NAME "${PROJECT_NAME}_test_curl")
set(LIVE_TEST_PROJECT_NAME "${PROJECT_NAME}_test_live")

# Default deterministic suite uses the socket HTTP implementation and includes
# localhost HTTP plus Telnet coverage.
add_executable(${TEST_PROJECT_NAME} main.test.cpp http_client.test.cpp telnet_socket.test.cpp)

target_include_directories(${TEST_PROJECT_NAME} SYSTEM PUBLIC ${BANDIT_DIR} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries (${TEST_PROJECT_NAME} ${PROJECT_NAME_HTTP} ${PROJECT_NAME_SYNC} ${PROJECT_NAME_ASYNC} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES} ${URIPARSER_LIBRARIES})

# Curl gets its own process so stateful fixtures are not run twice in one Bandit
# registry/process. This still exercises the same localhost HTTP contract.
if(CURL_FOUND)
    add_executable(${CURL_TEST_PROJECT_NAME} main_curl.test.cpp http_client.test.cpp)

    target_include_directories(${CURL_TEST_PROJECT_NAME} SYSTEM PUBLIC ${BANDIT_DIR} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    target_link_libraries(${CURL_TEST_PROJECT_NAME} ${PROJECT_NAME_HTTP} ${PROJECT_NAME_ASYNC} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES}
            ${OPENSSL_LIBRARIES} ${URIPARSER_LIBRARIES})
endif()

if(CODA_BUILD_LIVE_NETWORK_TESTS)
    add_executable(${LIVE_TEST_PROJECT_NAME} main.test.cpp http_live.test.cpp)

    target_include_directories(${LIVE_TEST_PROJECT_NAME} SYSTEM PUBLIC ${BANDIT_DIR} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    target_link_libraries(${LIVE_TEST_PROJECT_NAME} ${PROJECT_NAME_HTTP} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES}
            ${OPENSSL_LIBRARIES} ${URIPARSER_LIBRARIES})
endif()

if (DEFINED BANDIT_TARGET)
    add_dependencies(${TEST_PROJECT_NAME} ${BANDIT_TARGET})

    if(CURL_FOUND)
        add_dependencies(${CURL_TEST_PROJECT_NAME} ${BANDIT_TARGET})
    endif()

    if(CODA_BUILD_LIVE_NETWORK_TESTS)
        add_dependencies(${LIVE_TEST_PROJECT_NAME} ${BANDIT_TARGET})
    endif()
endif()

# add target for code coverage
if(ENABLE_COVERAGE)
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
	setup_target_for_coverage(TARGET ${PROJECT_NAME}_coverage OUTPUT ${PROJECT_BINARY_DIR}/coverage)
endif()

include(ValgrindTest)

add_valgrind_profile_test(PROFILING ${ENABLE_PROFILING} MEMCHECK ${ENABLE_MEMCHECK} TARGET ${TEST_PROJECT_NAME})

if(CURL_FOUND)
    add_valgrind_profile_test(PROFILING ${ENABLE_PROFILING} MEMCHECK ${ENABLE_MEMCHECK} TARGET ${CURL_TEST_PROJECT_NAME})
endif()

if(CODA_BUILD_LIVE_NETWORK_TESTS)
    add_valgrind_profile_test(PROFILING ${ENABLE_PROFILING} MEMCHECK ${ENABLE_MEMCHECK} TARGET ${LIVE_TEST_PROJECT_NAME})
endif()
