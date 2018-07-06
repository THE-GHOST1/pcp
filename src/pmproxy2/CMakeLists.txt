cmake_minimum_required(VERSION 3.5)
project(pmproxy2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
include_directories("${PROJECT_SOURCE_DIR}")


include_directories(../libpcp_web/src/)
include_directories(../external/)

add_executable(
        pmproxy2
        src/pmproxy2.c
        src/CommandKeys.c)

set(LIB_DIR /home/prajwal/CLionProjects/pcp/src)

#target_link_libraries(libuvclient -luv -L${LIB_DIR} -lpcp -L${LIB_DIR}/libpcp_web/src/ -lpcp_web /usr/lib/libpcp_pmda.a -lpcp_gui -lpcp_import -lpcp_mmv -lpcp_trace -pthread -ldl -lm)
target_link_libraries(pmproxy2 -luv -L${LIB_DIR} -lpcp -L${LIB_DIR}/libpcp_web/src/ -lpcp_web /usr/lib/libpcp_pmda.a -lpcp_gui -lpcp_import -lpcp_mmv -lpcp_trace -pthread -ldl -lm)