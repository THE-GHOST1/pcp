cmake_minimum_required(VERSION 3.5)
project(pmproxy2)

set(CMAKE_C_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}")

#set(SOURCES pmproxy2.c client.c ${PROJECT_SOURCE_DIR}/CommandKeys.h)

add_executable(
        pmproxy2
        pmproxy2.c
        CommandKeys.c
        ../libpcp_web/src/slots.c
        )
add_executable(libuvclient client.c)

target_link_libraries(libuvclient -luv)
target_link_libraries(pmproxy2 -luv)