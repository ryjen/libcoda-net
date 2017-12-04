
add_executable(jest main.cpp config.cpp jester.cpp util.cpp)

target_include_directories(jest SYSTEM PUBLIC ${CURL_INCLUDE_DIRS} SYSTEM PUBLIC ${URIPARSER_INCLUDE_DIRS} )

target_link_libraries(jest ${PROJECT_NAME_HTTP} ${CURL_LIBRARIES} ${URIPARSER_LIBRARIES})

install(TARGETS jest RUNTIME DESTINATION bin)
