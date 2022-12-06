cmake_minimum_required(VERSION 3.23)
project(tcp-server-client-ping-pong-timing-win)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(WIN32_LEAN_AND_MEAN)

add_executable(server server.cpp)
target_link_libraries(server
        Ws2_32.lib
        )

add_executable(client client.cpp)
target_link_libraries(client
        Ws2_32.lib
        Mswsock.lib
        AdvApi32.lib
        )
