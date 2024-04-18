#include <iostream>
#include <thread>

#include "me/mskatking/mcppserversoftware/io/json.h"
#include "me/mskatking/mcppserversoftware/server/minecraft_server.h"

int main() {
    MinecraftServer server;
    server.start();
    return 0;
}
