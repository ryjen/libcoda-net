
set(${PROJECT_NAME_SYNC}_HEADER_FILES
        epoll_impl.h
        listener.h
        select_impl.h
        server.h
        server_impl.h
)

set(SOURCE_FILES ${${PROJECT_NAME_SYNC}_HEADER_FILES} epoll_impl.cpp select_impl.cpp server.cpp)

add_library(${PROJECT_NAME_SYNC} ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME_SYNC} PUBLIC cxx_std_17)

# EPOLL_FOUND conditions installed sync headers, so keep consumers on the same
# implementation contract as the library build.
if (EPOLL_FOUND)
    target_compile_definitions(${PROJECT_NAME_SYNC} PUBLIC EPOLL_FOUND)
endif()

target_link_libraries(${PROJECT_NAME_SYNC} INTERFACE ${PROJECT_NAME})

create_packages(TARGET ${PROJECT_NAME_SYNC} DESCRIPTION "A c++ syncronous server library.")

string(REPLACE "_" "/" INSTALL_DIRECTORY ${PROJECT_NAME_SYNC})

install(FILES ${${PROJECT_NAME_SYNC}_HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INSTALL_DIRECTORY}")

install(TARGETS ${PROJECT_NAME_SYNC} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
