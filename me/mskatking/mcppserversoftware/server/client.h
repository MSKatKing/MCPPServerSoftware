#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <utility>
#include "../networking/packets/packet.h"
#include "../util/component/text_components.h"
#include "../util/identifiers/uuid.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

enum ClientState {
    HANDSHAKE,
    STATUS,
    LOGIN
};

class Player {
private:
#ifdef WIN32
    SOCKET socket;
#else
    int socket;
#endif
    std::string playerName;
    UUID playerUUID;

    ClientState state;

    void sendPacket(const Packet& final);

public:
#ifdef WIN32
    explicit Player(SOCKET socket) : socket(socket), state(HANDSHAKE), playerUUID(INVALID_UUID) {}
#else
    explicit Player(int socket) : socket(socket), state(HANDSHAKE), playerUUID(INVALID_UUID) {}
#endif

#ifdef WIN32
    [[nodiscard]] SOCKET getSocket() const {
        return socket;
    }
#else
    [[nodiscard]] int getSocket() const {
        return socket;
    }
#endif

    [[nodiscard]] std::string getName() const {
        return playerName;
    }

    void setName(std::string newName) {
        playerName = std::move(newName);
    }

    [[nodiscard]] ClientState getState() const {
        return state;
    }

    void setState(ClientState newState) {
        state = newState;
    }

    void kick(TextComponent reason);

    // Handshake packets
    void handleHandshake(Packet in);
    void handleLegacyPing(Packet in);

    // Status packets
    void sendMOTD(JSON motd);
    void sendPingResponse(long payload);

    // Login packets
};



#endif //CLIENT_H
