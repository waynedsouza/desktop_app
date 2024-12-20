#include "ServerConfig.h"

// Initialize the static instance pointer
ServerConfig* ServerConfig::instance = nullptr;

// Static method to get the singleton instance
ServerConfig& ServerConfig::getInstance(const std::string& filePath) {
    if (instance == nullptr) {
        instance = new ServerConfig(filePath); // Instantiate only once
        atexit([] { delete instance; });       // Ensure cleanup on program exit
    }
    return *instance;
}
