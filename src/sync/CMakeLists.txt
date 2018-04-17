
set(${PROJECT_NAME_SYNC}_HEADER_FILES
        epoll_impl.h
        listener.h
        select_impl.h
        server.h
        server_impl.h
)

set(SOURCE_FILES ${${PROJECT_NAME_SYNC}_HEADER_FILES} epoll_impl.cpp select_impl.cpp server.cpp)

add_library(${PROJECT_NAME_SYNC} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME_SYNC} INTERFACE ${PROJECT_NAME})

create_package(TARGET ${PROJECT_NAME_SYNC} DESCRIPTION "A c++ syncronous server library.")

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME_SYNC})

install(FILES ${${PROJECT_NAME_SYNC}_HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME_SYNC} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
