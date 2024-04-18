//
// Created by wait4 on 4/7/2024.
//

#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <ostream>

struct Packet {
    int id;
    char buffer[4096];
    int cursor;

    Packet();

    Packet(char* data);

    void WriteByte(char value);

    char ReadByte();

    void WriteInt(int value);

    void WriteLong(long value);

    int ReadInt();

    long ReadLong();

    void WriteString(const std::string& value);

    std::string ReadString();

    void WriteUnsignedInt(unsigned int value);

    unsigned int ReadUnsignedInt();

    void WriteUnsignedShort(unsigned short value);

    unsigned short ReadUnsignedShort();

    void WriteVarInt(int value);

    int ReadVarInt();

    void WriteByteArray(const char* value);

    char* ReadByteArray();

    const char* GetBuffer() const;

    int GetSize() const;

    void SetCursor(int cursor);

    const Packet Finalize() const {
        Packet p;
        p.WriteVarInt(cursor);
        memcpy(&p.buffer[p.cursor], buffer, cursor);
        p.cursor += cursor;
        return p;
    }

    const char* Sendable() const {
        return buffer;
    }

    friend std::ostream& operator<<(std::ostream& os, const Packet& p) {
        for(int i = 0; i < p.GetSize(); i++) {
            std::stringstream num;
            num << std::setw(4) << std::setfill('0') << i;
            os << "@" << num.str() << " -> " << (int) p.GetBuffer()[i] << "\t(" << p.GetBuffer()[i] << ")" << std::endl;
        }
        return os;
    }
};

#endif //PACKET_H
