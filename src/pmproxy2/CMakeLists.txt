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
        ../libpcp_web/src/redis.c
        ../libpcp_web/src/schema.c
        ../libpcp_web/src/maps.c
        ../libpcp_web/src/net.c
        ../libpcp_web/src/load.c
        ../libpcp_web/src/libuv.c
        ../libpcp_web/src/util.c
        ../external/crc16.c
        ../libpcp_web/src/query.c

)
add_executable(libuvclient client.c)

target_link_libraries(libuvclient  -luv -lpcp -lpcp_web -lpcp_pmda -lpcp_gui -lpcp_import -lpcp_mmv -lpcp_trace -pthread -ldl -lm)
target_link_libraries(pmproxy2 -luv -lpcp -lpcp_web -lpcp_pmda -lpcp_gui -lpcp_import -lpcp_mmv -lpcp_trace -pthread -ldl -lm)