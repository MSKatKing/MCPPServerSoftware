//
// Created by wait4 on 4/7/2024.
//

#include "minecraft_server.h"

#include <iostream>
#include <thread>
#include <valarray>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "../io/configs.h"
#include "../io/json.h"
#include "../networking/packets/packet.h"

MinecraftServer::MinecraftServer() : running(true), ticks(0), logger("THREAD-MAIN") {
    logger.info("Intitializing server...");

    Configurations::CFGConfiguration config("server-config.cfg");
    int port = std::stoi(config.get("port", "25565"));

#ifdef WIN32
    WSAData wsData{};
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if(wsOk != 0) {
        logger.error("Couldn't initialize Winsock! Cannot continue, quitting.");
        running = false;
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET) {
        logger.error("Can't create socket! Cannot continue, quitting.");
        running = false;
        WSACleanup();
        return;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        logger.error("Can't bind to port! Cannot continue, quitting.");
        running = false;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        logger.error("Can't listen on socket! Cannot continue, quitting.");
        running = false;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
#else
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1) {
        logger.error("Can't create socket! Cannot continue, quitting.");
        running = false;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        logger.error("Can't bind to port! Cannot continue, quitting.");
        running = false;
        close(serverSocket);
        return;
    }

    if(listen(serverSocket, SOMAXCONN) == -1) {
        logger.error("Can't listen on socket! Cannot continue, quitting.");
        running = false;
        close(serverSocket);
        return;
    }
#endif

    logger.info("Successfully binded! Server listening on port " + config.get("port", "25565"));
}

void MinecraftServer::listenForClients() const {
    while(running) {
#if WIN32
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if(clientSocket == INVALID_SOCKET && running) {
            logger.error("Unable to accept incoming connection.");
            continue;
        }
#else
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if(clientSocket == -1 && running) {
            logger.error("Unable to accept incoming connection.");
            continue;
        }
#endif

        Player player(clientSocket);

        logger.info("Client connected! Gathering data...");

        clientPool->enqueue([](const Player& p, const MinecraftServer* server) {
            server->handleClient(p);
        }, player, this);
    }
}

std::string threadId_to_string(const std::thread::id& id) {
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

void MinecraftServer::handleClient(Player player) const {
    Logger logger("CLIENT-POOL-" + threadId_to_string(std::this_thread::get_id()));
    bool connect = true;

#ifdef _WIN32
    u_long mode = 1;
    if(ioctlsocket(player.getSocket(), FIONBIO, &mode) == SOCKET_ERROR) {
        logger.error("Couldn't set the socket to non-blocking mode!");
        return;
    }
#else
    int flags = fcntl(player.getSocket(), F_GETFL, 0);
    if(flags == -1) {
        logger.error("Couldn't set the socket to non-blocking mode!");
        return;
    }
    flags |= O_NONBLOCK;
    if(fcntl(player.getSocket(), F_SETFL, flags) == -1) {
        logger.error("Couldn't set the socket to non-blocking mode!");
        return;
    }
#endif

    while(connect) {
        char buffer[1024];
        const int bytesRecieved = recv(player.getSocket(), buffer, sizeof(buffer), 0);
        if(bytesRecieved < 0) continue;
        if(bytesRecieved == 0) {
            connect = false;
            break;
        }

        Packet in(buffer);

        Packet out;

        int length = in.ReadVarInt();
        if(length == 0xFE) {
            out.WriteByte(0xFF);
            std::string str = "§1\0127\01.20.4\0C++ Minecraft Server\00\020";
            out.WriteUnsignedShort(str.size());
            for(char c : str) out.WriteByte(c);
            send(player.getSocket(), out.Sendable(), out.GetSize(), 0);
            break;
        }

        char id = in.ReadByte();

        switch (player.getState()) {
            case HANDSHAKE: {
                switch(id) {
                    case 0x00: {
                        int version = in.ReadVarInt();
                        std::string addr = in.ReadString();
                        unsigned short port = in.ReadUnsignedShort();
                        int state = in.ReadVarInt();

                        logger.info("handshake " + std::to_string(version) + " " + addr + " " + std::to_string(port) + " " + std::to_string(state));

                        if(state == 1)
                            player.setState(STATUS);
                        else if(state == 2)
                            player.setState(LOGIN);
                        break;
                    }
                    default: {
                        logger.warn("Error! Client sent unknown data. Disconnecting from client. (State: HANDSHAKE)");
                        connect = false;
                        break;
                    }
                }
                break;
            }
            case STATUS: {
                switch(id) {
                    case 0x00: {
                        player.sendMOTD(this->generateMOTD());
                        break;
                    }
                    case 0x01: {
                        player.sendPingResponse(in.ReadLong());
                        break;
                    }
                    default: {
                        logger.warn("Error! Client sent unknown data. Disconnecting from client. (State: STATUS)");
                        connect = false;
                        break;
                    }
                }
                break;
            }
            case LOGIN: {
                player.kick({"Not supported at this time."});
                break;
            }
            default: {
                logger.warn("Error! Client sent unknown data. Disconnecting from client.");
                connect = false;
                break;
            }
        }
    }
    logger.info("Client disconnected");

#ifdef WIN32
    closesocket(player.getSocket());
#else
    close(player.getSocket());
#endif

    logger.close();
}

void MinecraftServer::start() {
    if(!running) return;

    logger.info("Server starting...");

    listenerThread = std::thread(&MinecraftServer::listenForClients, this);

    consoleThread = std::thread(&MinecraftServer::inputThreadFunction, this);

    clientPool = new ThreadPool(25);

    constexpr std::chrono::milliseconds targetTickDuration(50);

    logger.info("Timings started!");

    logger.info("Server started!");

    while(running) {
        const auto tickStartTime = std::chrono::steady_clock::now();

        tick();

        const auto tickEndTime = std::chrono::steady_clock::now();
        const auto tickDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tickEndTime - tickStartTime);

        if(const auto remainingTime = targetTickDuration - tickDuration; remainingTime > std::chrono::milliseconds(0))
            std::this_thread::sleep_for(remainingTime);
    }
}

void MinecraftServer::stop() {
    if(running) {
        logger.info("Server stopping...");

        running = false;

        consoleThread.detach();

        listenerThread.detach();

        delete clientPool;

#ifdef WIN32
        closesocket(serverSocket);
        WSACleanup();
#else
        close(serverSocket);
#endif

        logger.info("Server stopped!");
        logger.close();
    }
}

void MinecraftServer::tick() {
    // Gather any console input
    if(const std::string consoleInput = checkConsoleCommand(); !consoleInput.empty()) {
        if(consoleInput == "stop" || consoleInput == "s" || consoleInput == "quit") stop();
        else if(consoleInput == "help" || consoleInput == "?") {
            logger.info("Command list:");
            logger.info("stop:");
            logger.info("   aliases: s, quit");
            logger.info("   description: Stops the server");
            logger.info("");
            logger.info("help:");
            logger.info("   aliases: ?");
            logger.info("   description: shows this list");
        }
        else logger.info("Unknown command '" + consoleInput + "'! Type 'help' for a list of commands.");
        command.clear();
    }
}

std::string MinecraftServer::checkConsoleCommand() {
    std::unique_lock lock(consoleMutex);
    consoleCV.wait_for(lock, std::chrono::milliseconds(10), [this] { return !command.empty(); });

    return command;
}


void MinecraftServer::inputThreadFunction() {
    while(running) {
        std::string input;
        std::getline(std::cin, input);

        std::unique_lock lock(consoleMutex);

        command = input;

        consoleCV.notify_one();
    }
}

const Logger* MinecraftServer::getLogger() const {
    return &logger;
}

JSON MinecraftServer::generateMOTD() const {
    JSON output;
    JSON version;
    version.writeString("name", "1.20.4");
    version.writeInt("protocol", 765);
    output.writeJson("version", version);
    JSON players;
    players.writeInt("max", 100);
    players.writeInt("online", 0);
    output.writeJson("players", players);
    JSON desc;
    desc.writeString("text", "Hello world!");
    output.writeJson("description", desc);
    output.writeBool("enforcesSecureChat", true);
    output.writeBool("previewsChat", true);
    return output;
}