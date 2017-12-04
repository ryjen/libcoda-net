
add_executable(${PROJECT_NAME}_test main.test.cpp http_client.test.cpp telnet_socket.test.cpp )

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC vendor/bandit SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries (${PROJECT_NAME}_test ${PROJECT_NAME_HTTP} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES} ${URIPARSER_LIBRARIES})
