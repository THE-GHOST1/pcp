cmake_minimum_required(VERSION 3.5)
project(pmproxy2)

set(CMAKE_C_STANDARD 11)

add_executable(pmproxy2 pmproxy2.c)
add_executable(libuvclient client.c)
add_executable(pmproxy2-temp  temp2.c)

include_directories("${PROJECT_SOURCE_DIR}")
target_link_libraries(libuvclient -lhiredis -luv)
target_link_libraries(pmproxy2 -lhiredis -luv)
target_link_libraries(pmproxy2-temp -lhiredis -luv)