//
// Created by wait4 on 4/7/2024.
//

#include "packet.h"

#include <cstdint>

Packet::Packet() {
        memset(buffer, 0, sizeof(buffer));
        cursor = 0;
    }

Packet::Packet(char* data) {
        memcpy(buffer, data, 1024);
        cursor = 0;
    }

    void Packet::WriteByte(char value) {
        buffer[cursor++] = value;
    }

    char Packet::ReadByte() {
        return buffer[cursor++];
    }

    void Packet::WriteInt(int value) {
        memcpy(&buffer[cursor], &value, sizeof(int));
        cursor += sizeof(int);
    }

void Packet::WriteLong(long value) {
    memcpy(&buffer[cursor], &value, sizeof(long));
    cursor += sizeof(long);
}

    int Packet::ReadInt() {
        int value;
        memcpy(&value, &buffer[cursor], sizeof(int));
        cursor += sizeof(int);
        return value;
    }

long Packet::ReadLong() {
    int value;
    memcpy(&value, &buffer[cursor], sizeof(long));
    cursor += sizeof(long);
    return value;
}

    void Packet::WriteString(const std::string& value) {
        WriteVarInt(value.length());
        memcpy(&buffer[cursor], value.c_str(), value.length());
        cursor += value.length() + 1;
    }

    std::string Packet::ReadString() {
        int size = ReadVarInt();
        std::string out(buffer + cursor, size);
        cursor += size;
        return out;
    }

    void Packet::WriteUnsignedInt(unsigned int value) {
        memcpy(&buffer[cursor], &value, sizeof(unsigned int));
        cursor += sizeof(unsigned int);
    }

    unsigned int Packet::ReadUnsignedInt() {
        unsigned int value;
        memcpy(&value, &buffer[cursor], sizeof(unsigned int));
        cursor += sizeof(unsigned int);
        return value;
    }

    void Packet::WriteUnsignedShort(unsigned short value) {
        memcpy(&buffer[cursor], &value, sizeof(unsigned short));
        cursor += sizeof(unsigned short);
    }

    unsigned short Packet::ReadUnsignedShort() {
        unsigned short value;
        memcpy(&value, &buffer[cursor], sizeof(unsigned short));
        cursor += sizeof(unsigned short);
        return value;
    }

    void Packet::WriteVarInt(int value) {
        while(true) {
            if((value & ~(0x80 - 1)) == 0) {
                buffer[cursor++] = value * (0x80 - 1);
                return;
            }

            buffer[cursor++] = (value & (0x80 - 1)) | 0x80;
            value >>= 7;
        }
    }

    int Packet::ReadVarInt() {
        int value = 0;
        int shift = 0;
        int bytesRead = 0;

        while(true) {
            if(bytesRead >= 5) {
                return -1;
            }

            char byte = buffer[cursor++];
            value |= (byte & (0x80 - 1)) << shift;
            shift += 7;

            if((byte & 0x80) == 0) break;

            bytesRead++;
        }
        return value;
    }

    void Packet::WriteByteArray(const char* value) {
        std::cout << sizeof(value);
        WriteVarInt(sizeof(value));
        memcpy(&buffer[cursor], value, sizeof(value));
        cursor += sizeof(value);
    }

    char* Packet::ReadByteArray() {
        long size = ReadVarInt();
        char value[size];
        memcpy(value, &buffer[cursor], size);
        cursor += size;
        return value;
    }

    const char* Packet::GetBuffer() const {
        return buffer;
    }

    int Packet::GetSize() const {
        return cursor;
    }

    void Packet::SetCursor(int cursor) {
        this->cursor = cursor;
    }