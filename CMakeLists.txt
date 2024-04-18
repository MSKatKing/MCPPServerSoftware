cmake_minimum_required(VERSION 3.22)
project(MCPPServerSoftware)

set(CMAKE_CXX_STANDARD 17)

include_directories(MCPPServerSoftware)

add_executable(MCPPServerSoftware main.cpp
        me/mskatking/mcppserversoftware/io/configs.cpp
        me/mskatking/mcppserversoftware/io/configs.h
        me/mskatking/mcppserversoftware/server/minecraft_server.cpp
        me/mskatking/mcppserversoftware/server/minecraft_server.h
        me/mskatking/mcppserversoftware/io/logger.cpp
        me/mskatking/mcppserversoftware/io/logger.h
        me/mskatking/mcppserversoftware/threads/thread_pool.cpp
        me/mskatking/mcppserversoftware/threads/thread_pool.h
        me/mskatking/mcppserversoftware/server/client.cpp
        me/mskatking/mcppserversoftware/server/client.h
        me/mskatking/mcppserversoftware/networking/packets/packet.cpp
        me/mskatking/mcppserversoftware/networking/packets/packet.h
        me/mskatking/mcppserversoftware/io/json.cpp
        me/mskatking/mcppserversoftware/io/json.h
        me/mskatking/mcppserversoftware/util/component/text_components.cpp
        me/mskatking/mcppserversoftware/util/component/text_components.h
)

#target_link_libraries(MCPPServerSoftware wsock32 ws2_32)