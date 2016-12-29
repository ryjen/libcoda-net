
add_executable(jest main.cpp config.cpp jester.cpp util.cpp)

target_link_libraries(jest ${PROJECT_NAME_HTTP} ${CURL_LIBRARIES})

install(TARGETS jest RUNTIME DESTINATION bin)
