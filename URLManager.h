#pragma once
#include <unordered_map>
#include <memory>
#include <iostream> // For logging
#include "URLComponents.h"

class URLManager {
public:
    // Add URLComponents to the map
    void AddURLComponent(const std::string& uuid, std::shared_ptr<URLComponents> component) {
        urlMap[uuid] = component; // Store shared_ptr directly
    }

    // Finalize a specific URLComponents instance
    bool FinalizeComponent(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        if (it != urlMap.end()) {
            // it->second->finalize(); // Call cleanup method
            urlMap.erase(it); // Remove from the map
            std::cout << "Removed URLComponents with UUID: " << uuid << std::endl;
            return true;
        }
        return false; // UUID not found
    }

    std::shared_ptr<URLComponents> GetURLComponentById(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        if (debugMode) wxLogMessage("GetURLComponentById NotFOund:%d  size:%zu", (it == urlMap.end()), urlMap.size());
        return (it != urlMap.end()) ? it->second : nullptr; // Return shared_ptr or nullptr
    }

    // Retrieve a URLComponents by UUID
    URLComponents* GetRawPointerURLComponentById(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        return (it != urlMap.end()) ? it->second.get() : nullptr; // Return raw pointer or nullptr
    }

    // Set a URLComponents by UUID
    void SetURLComponent(const std::string& uuid, std::shared_ptr<URLComponents> component) {
        urlMap[uuid] = component; // Store shared_ptr directly
    }

private:
    std::unordered_map<std::string, std::shared_ptr<URLComponents>> urlMap; // Use shared_ptr
};

/*
* USING UNIQUE PTR: howver we may be running on different threads so were implementing shared above
class URLManager {
public:
    // Add URLComponents to the map
    void AddURLComponent(const std::string& uuid, std::unique_ptr<URLComponents> component) {
        urlMap[uuid] = std::move(component);
    }

    // Finalize a specific URLComponents instance
    bool FinalizeComponent(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        if (it != urlMap.end()) {
            //it->second->finalize(); // Call cleanup method
            urlMap.erase(it); // Remove from the map
            std::cout << "Removed URLComponents with UUID: " << uuid << std::endl;
            return true;
        }
        return false; // UUID not found
    }

    std::unique_ptr<URLComponents> GetURLComponentById(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        if (it != urlMap.end()) {
            auto ptr = std::move(it->second); // Transfer ownership
            urlMap.erase(it);                 // Remove from the map after ownership is transferred
            return ptr;
        }
        return nullptr;
    }


    // Retrieve a URLComponents by UUID//deprecate
    URLComponents* GetRawPointerURLComponentById(const std::string& uuid) {
        auto it = urlMap.find(uuid);
        return (it != urlMap.end()) ? it->second.get() : nullptr; // Return pointer or nullptr
    }

    // Set a URLComponents by UUID
    void SetURLComponent(const std::string& uuid, std::unique_ptr<URLComponents> component) {
        urlMap[uuid] = std::move(component);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<URLComponents>> urlMap;
};
*/


