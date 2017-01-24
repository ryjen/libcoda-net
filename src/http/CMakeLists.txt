
set(SOURCE_FILES
  client.cpp
  socket_impl.cpp
  curl_impl.cpp
)

add_library(${PROJECT_NAME_HTTP} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME_HTTP} INTERFACE ${CURL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME_HTTP} INTERFACE ${PROJECT_NAME} INTERFACE ${CURL_LIBRARIES})

set(${PROJECT_NAME_HTTP}_HEADERS
  client.h
  protocol.h
)

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME_HTTP})

install(FILES ${${PROJECT_NAME_HTTP}_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME_HTTP} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_subdirectory(jest)
