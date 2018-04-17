
if (NOT DEFINED BANDIT_DIR)

    include(ExternalProject)

    set(BANDIT_TARGET ${PROJECT_NAME}-bandit)

    externalproject_add(${BANDIT_TARGET}
            GIT_REPOSITORY https://github.com/banditcpp/bandit.git
            GIT_TAG v2.0.0
            INSTALL_COMMAND ""
            BUILD_COMMAND ""
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/bandit)

    ExternalProject_Get_property(${BANDIT_TARGET} SOURCE_DIR)

    set(BANDIT_DIR ${SOURCE_DIR})
endif()

add_executable(${TEST_PROJECT_NAME} main.test.cpp http_client.test.cpp telnet_socket.test.cpp )

target_include_directories(${TEST_PROJECT_NAME} SYSTEM PUBLIC ${BANDIT_DIR} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries (${TEST_PROJECT_NAME} ${PROJECT_NAME_HTTP} ${PROJECT_NAME_SYNC} ${PROJECT_NAME_ASYNC} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES} ${URIPARSER_LIBRARIES})

if (DEFINED BANDIT_TARGET)
    add_dependencies(${TEST_PROJECT_NAME} ${BANDIT_TARGET})
endif()