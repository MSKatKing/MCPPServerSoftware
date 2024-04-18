#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

//#include "console.h"

namespace Configurations {

    class CFGConfiguration {
    private:
        std::string path;
        std::unordered_map<std::string, std::string> config;

        void parseLine(const std::string& line) {
            std::istringstream iss(line);
            if(std::string key, value; std::getline(iss, key, '=') && std::getline(iss, value)) {
                trim(key);
                trim(value);
                config[key] = value;
            }
        }

        static void trim(std::string& str) {
            if(const size_t start = str.find_first_not_of(" \t"), end = str.find_last_not_of(" \t"); start != std::string::npos && end != std::string::npos) {
                str = str.substr(start, end - start + 1);
            } else {
                str.clear();
            }
        }

    public:
        explicit CFGConfiguration(const std::string& path) : path(path) {
            if(std::ifstream file(path); file) {
                std::string line;
                while(std::getline(file, line)) {
                    parseLine(line);
                }
            } else {
                //Console::error("Failed to open configuration files!");
            }
        }

        ~CFGConfiguration() {
            if(std::ofstream file(path); file) {
                for(const auto& pair : config) {
                    file << pair.first << " = " << pair.second << std::endl;
                }
            } else {
                //Console::error("Failed to open configuration files to save!");
            }
        }

        std::string get(const std::string& key, const std::string& def) const {
            if(const auto it = config.find(key); it != config.end()) {
                return it->second;
            }
            return def;
        }
    };

}
