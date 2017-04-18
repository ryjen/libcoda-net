
set(SOURCE_FILES
  client.cpp
  socket_impl.cpp
  curl_impl.cpp
)

add_library(${PROJECT_NAME_HTTP} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME_HTTP} INTERFACE ${CURL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME_HTTP} INTERFACE ${PROJECT_NAME} INTERFACE ${CURL_LIBRARIES})

set(HEADER_FILES
  client.h
  protocol.h
)

create_package(TARGET ${PROJECT_NAME_HTTP} DESCRIPTION "A c++ http client library.")

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME_HTTP})

install(FILES ${HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME_HTTP} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_subdirectory(jest)
