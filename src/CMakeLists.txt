add_library(arg3net buffered_socket.cpp socket.cpp http_client.cpp socket_server.cpp socket_factory.cpp uri.cpp telnet_socket.cpp)

include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

target_link_libraries(arg3net ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})


