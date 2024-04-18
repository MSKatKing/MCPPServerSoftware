#ifndef UUID_H
#define UUID_H

#include <string>
#include <regex>

class UUID {
private:
    std::string uuid;
    int version;

    bool valid;

public:
    UUID(const std::string& id) {
        std::regex pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
        
        if(!std::regex_match(id, pattern)) {
            valid = false;
            return;
        }

        uuid = id;
        version = std::stoi(std::to_string(uuid[14]));
    }

    int getVersion() const {
        return version;
    }

    std::string getUUID() const {
        return uuid;
    }
};

#define INVALID_UUID UUID("00000000-0000-0000-0000-000000000000")

#endif