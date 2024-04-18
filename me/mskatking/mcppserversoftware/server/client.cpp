//
// Created by wait4 on 4/7/2024.
//

#include "client.h"
#include "../networking/packets/packet.h"
#include <iostream>

void Player::sendPacket(const Packet& final) {
    std::cout << final << std::endl;
    send(getSocket(), final.Sendable(), final.GetSize(), 0);
}

void Player::kick(TextComponent reason) {
    Packet out;
    switch (state) {
        case LOGIN:
            out.WriteByte(0x00);
            out.WriteString(reason.asString());
            sendPacket(out.Finalize());
            break;
        default:
            break;
    }
}

void Player::sendMOTD(JSON motd) {
    Packet out;
    out.WriteByte(0x00);
    out.WriteString(motd.asString());
    sendPacket(out.Finalize());
}

void Player::sendPingResponse(long payload) {
    Packet out;
    out.WriteByte(0x01);
    out.WriteLong(payload);
    sendPacket(out.Finalize());
}
