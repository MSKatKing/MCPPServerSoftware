#ifndef MINECRAFT_SERVER_H
#define MINECRAFT_SERVER_H
#include <condition_variable>
#include <thread>
#include <mutex>
#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "client.h"
#include "../io/logger.h"
#include "../threads/thread_pool.h"

class MinecraftServer {
private:
    std::mutex mutex;
    std::condition_variable cv;
    std::thread listenerThread;
    std::thread consoleThread;
    ThreadPool* clientPool{};
    bool running;
    int ticks;
    Logger logger;

    std::mutex consoleMutex;
    std::condition_variable consoleCV;
    std::string command;

#ifdef WIN32
    SOCKET serverSocket;
#else
    int serverSocket;
#endif

public:
    MinecraftServer();

    void start();
    void stop();

    [[nodiscard]] const Logger* getLogger() const;
    JSON generateMOTD() const;
    void handleClient(Player player) const;

private:
    void listenForClients() const;
    void inputThreadFunction();

    void tick();

    std::string checkConsoleCommand();
};


#endif //MINECRAFT_SERVER_H
