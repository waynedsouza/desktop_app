#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <iostream>
#include <string>
#include <sstream>
#include "yaml-cpp/yaml.h"
#include <wx/wx.h>

class ServerConfig {
private:
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif
    YAML::Node Config; // Holds the loaded YAML configuration
    static ServerConfig* instance; // Static pointer to the singleton instance

    // Private constructor to prevent multiple instances
    explicit ServerConfig(const std::string& filePath) {
        try {
            Config = YAML::LoadFile(filePath);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to load configuration file: " << e.what() << std::endl;
            throw;
        }
    }

public:
    // Deleted copy constructor and assignment operator
    ServerConfig(const ServerConfig&) = delete;
    ServerConfig& operator=(const ServerConfig&) = delete;

    // Static method to get the singleton instance
    static ServerConfig& getInstance(const std::string& filePath = "config.yaml");

    std::unique_ptr<YAML::Node> getConfig() {
        return std::make_unique<YAML::Node>(Config);
    }
    // Method to get a string value
    std::string getString(const std::string& key) {
        if (debugMode)wxLogMessage("getString called with key:%s", key);
        try {
            return Config[key].as<std::string>();
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving key '" << key << "': " << e.what() << std::endl;
            return "";
        }
    }

    // Method to get an integer value
    int getInt(const std::string& key) {
        if (debugMode)wxLogMessage("getInt called with key:%s", key);
        try {
            return Config[key].as<int>();
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving key '" << key << "': " << e.what() << std::endl;
            return 0;
        }
    }

    // Method to dump the entire configuration as a YAML string
    std::string dumpConfig() const {
        try {
            return YAML::Dump(Config);
        }
        catch (const std::exception& e) {
            std::cerr << "Error dumping configuration: " << e.what() << std::endl;
            return "Error dumping configuration.";
        }
    }

    template <typename T>
    T get(const std::string& path) {
        if (debugMode) wxLogMessage("template<typename T> get path: %s", path.c_str());
        try {
            // Clone the entire root node to work on an isolated copy
            YAML::Node node = YAML::Clone(Config);
            if (debugMode) wxLogMessage("Cloned root config:\n%s", YAML::Dump(node).c_str());

            std::istringstream stream(path);
            std::string token;

            while (std::getline(stream, token, '.')) {
                if (debugMode) wxLogMessage("Processing token: %s", token.c_str());

                // Ensure the token exists in the current node
                if (!node[token].IsDefined()) {
                    throw std::runtime_error("Invalid path: " + path + " (Missing key: " + token + ")");
                }

                // Move to the next node
                node = node[token];

                // Debug log for the current node
                if (debugMode) {
                    wxLogMessage("Node after token '%s': %p  node:\n%s size:%zu",
                        token.c_str(), static_cast<void*>(&node),
                        YAML::Dump(node).c_str(), node.size());
                }
            }

            // Log the final value based on its type
            if (debugMode) {
                if constexpr (std::is_same<T, std::string>::value) {
                    wxLogMessage("Final value: %s", node.as<T>().c_str());
                }
                else {
                    wxLogMessage("Final value: %s", std::to_string(node.as<T>()).c_str());
                }
            }

            return node.as<T>(); // Return the final resolved value
        }
        catch (const YAML::Exception& e) {
            std::cerr << "YAML error for path '" << path << "': " << e.what() << std::endl;
            throw;
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving path '" << path << "': " << e.what() << std::endl;
            throw;
        }
    }


    // Method to get nested configuration values
    /*template<typename T>
    T get(const std::string& path) {
        if (debugMode)wxLogMessage("template<typename T> get path:%s", path);
        try {
            YAML::Node node = Config; // Fix: Corrected from 'ServerConfig' to 'Config'
            std::istringstream stream(path);
            std::string token;

            while (std::getline(stream, token, '.')) {
                if (debugMode)wxLogMessage("in while processing token:%s", token);
                node = node[token];
                if (debugMode)wxLogMessage("in while processing node:%p",static_cast<void*>(&node));
                if (!node) {
                    throw std::runtime_error("Invalid path: " + path);
                }
            }                                                      
            
            if (debugMode)wxLogMessage("in while RETURN node size:%zu", node.size());
            return node.as<T>();
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving path '" << path << "': " << e.what() << std::endl;
            throw;
        }
    }*/
};

#endif // SERVERCONFIG_H
