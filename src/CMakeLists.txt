add_library(${PROJECT_NAME} buffered_socket.cpp socket.cpp http_client.cpp polling_socket_server.cpp socket_client.cpp socket_server.cpp socket_factory.cpp uri.cpp telnet_socket.cpp)

include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})


set(${PROJECT_NAME}_HEADERS
  buffered_socket.h
  exception.h
  http_client.h
  polling_socket_server.h
  protocol.h
  socket.h
  socket_client.h
  socket_factory.h
  socket_server.h
  telnet_socket.h
  uri.h
)
string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME})

install(FILES ${${PROJECT_NAME}_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
