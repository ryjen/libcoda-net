add_library(${PROJECT_NAME} buffered_socket.cpp socket.cpp http_client.cpp socket_server.cpp socket_factory.cpp uri.cpp telnet_socket.cpp)

include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})


