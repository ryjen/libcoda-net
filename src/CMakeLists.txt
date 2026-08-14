
set(${PROJECT_NAME}_HEADERS
        buffered_socket.h
        encoders.h
        exception.h
        secure_layer.h
        socket.h
        socket_factory.h
        socket_server.h
        socket_server_listener.h
        uri.h
)

set(${PROJECT_NAME}_SOURCE_FILES
  ${${PROJECT_NAME}_HEADERS}
  buffered_socket.cpp 
  socket.cpp
  secure_layer.cpp
  socket_factory.cpp
  socket_server.cpp
  uri.cpp 
  telnet/socket.cpp 
  encoders.cpp
)

set(CEREAL_INCLUDE_DIRS)
set(CEREAL_DEPENDENCY_TARGET)

if (cereal_FOUND)
    set(CEREAL_DEPENDENCY_TARGET cereal::cereal)
elseif(CODA_FETCH_DEPS)
    include(ExternalProject)

    externalproject_add(${PROJECT_NAME}-cereal
            GIT_REPOSITORY https://github.com/USCiLab/cereal.git
            GIT_TAG v1.2.2
            INSTALL_COMMAND ""
            BUILD_COMMAND ""
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/cereal)

    ExternalProject_Get_property(${PROJECT_NAME}-cereal SOURCE_DIR)
    set(CEREAL_INCLUDE_DIRS ${SOURCE_DIR}/include)
endif()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
        SYSTEM PUBLIC ${OPENSSL_INCLUDE_DIR}
        SYSTEM PUBLIC ${URIPARSER_INCLUDE_DIRS}
        SYSTEM PUBLIC ${CEREAL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${URIPARSER_LIBRARIES}
        ${CEREAL_DEPENDENCY_TARGET})

if (CODA_FETCH_DEPS AND NOT cereal_FOUND)
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-cereal)
endif()

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME})

install(FILES ${${PROJECT_NAME}_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_subdirectory(async)
add_subdirectory(http)
add_subdirectory(sync)
add_subdirectory(telnet)

