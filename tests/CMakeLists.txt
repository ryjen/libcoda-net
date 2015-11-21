
add_executable(arg3net_test arg3.test.cpp http_client.test.cpp telnet_socket.test.cpp )

include_directories(SYSTEM vendor/bandit ${CMAKE_SOURCE_DIR}/src ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})

target_link_libraries (arg3net_test arg3net ${CMAKE_THREAD_LIBS_INIT})


