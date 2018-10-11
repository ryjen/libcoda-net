
add_executable(crest main.cpp config.cpp jester.cpp util.cpp)

target_include_directories(crest SYSTEM PUBLIC ${CURL_INCLUDE_DIRS} SYSTEM PUBLIC ${URIPARSER_INCLUDE_DIRS} PUBLIC ${PROJECT_SOURCE_DIR}/src  )

target_link_libraries(crest ${PROJECT_NAME_HTTP} ${CURL_LIBRARIES} ${URIPARSER_LIBRARIES})

install(TARGETS crest RUNTIME DESTINATION bin)
