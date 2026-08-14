
set(${PROJECT_NAME_HTTP}_HEADER_FILES
    client.h
    protocol.h
)

set(${PROJECT_NAME_HTTP}_SOURCE_FILES
  ${${PROJECT_NAME_HTTP}_HEADER_FILES}
  client.cpp
  socket_impl.cpp
  curl_impl.cpp
)

add_library(${PROJECT_NAME_HTTP} ${${PROJECT_NAME_HTTP}_SOURCE_FILES})
target_compile_features(${PROJECT_NAME_HTTP} PUBLIC cxx_std_17)

# CURL_FOUND conditions the installed HTTP client header, so propagate the
# feature view to consumers when curl support is enabled/discovered.
if (CURL_FOUND)
    target_compile_definitions(${PROJECT_NAME_HTTP} PUBLIC CURL_FOUND)
endif()

target_include_directories(${PROJECT_NAME_HTTP} SYSTEM PUBLIC ${CURL_INCLUDE_DIRS} SYSTEM PUBLIC ${URIPARSER_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME_HTTP} ${PROJECT_NAME} ${CURL_LIBRARIES} ${URIPARSER_LIBRARIES})

create_packages(TARGET ${PROJECT_NAME_HTTP} DESCRIPTION "A c++ http client library.")

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME_HTTP})

install(FILES ${${PROJECT_NAME_HTTP}_HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME_HTTP} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_subdirectory(crest)
