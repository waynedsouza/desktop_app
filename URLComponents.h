#pragma once
#include <string>
#include "UrlParser.h"
#include "UUID.h"
#include <filesystem>
#include <wx/wx.h>
#include <memory>
#include <fstream>




namespace fs = std::filesystem;
extern const std::string CACHE;
extern const std::filesystem::path CACHE_PATH;



extern bool debugMode;
/*#ifdef _DEBUG
bool debugMode = true;
#else
bool debugMode = false;
#endif*/

namespace fs = std::filesystem;
extern const std::string CACHE;
extern const fs::path CACHE_PATH;


#pragma once
#include <string>
#include <fstream>
#include <memory>
#include <filesystem>
#include "UrlParser.h"
#include "UUID.h"

namespace fs = std::filesystem;
extern const std::string CACHE;
extern const fs::path CACHE_PATH;

struct URLComponents {
    std::string url;
    std::string domain;
    std::string path;
    std::string query;
    std::string fragment;
    std::string uuid;
    std::string domain_sanitised;
    std::string path_sanitised;
    std::shared_ptr<std::ifstream> contentStream; // Use shared_ptr for shared ownership

    // Delete the default constructor
    URLComponents() = delete;

    // Constructor to initialize with a URL
    URLComponents(const std::string& url)
        : url(url), uuid(UUIDcr::sgetUUID()) {
        URLParser parser(url);
        domain = parser.getHost();
        path = parser.getPath();
        query = parser.getQuery();
        domain_sanitised = parser.getDomainAsName();
        path_sanitised = parser.getPathAsName(); // Use getPathAsName for path sanitization
    }

    std::string getUUID() const {
        return uuid;
    }

    // Method to create the file path and directories
    fs::path getFilePath() {
        fs::path temp = CACHE_PATH / domain_sanitised / path_sanitised;
        fs::create_directories(temp); // Create directories if they do not exist
        temp /= uuid; // Append the UUID to the path
        temp.replace_extension(".dat"); // Set the file extension to .dat
        
        return temp;
    }

    // Method to set the file stream to read from the file
    //in refactoring change name to save filename as we have both streaming and non streaing using this
    void setFileStream() {
        fs::path filePath = getFilePath();
        ;
        if (debugMode) wxLogMessage("file loc: :%s", filePath.string());
        contentStream = std::make_shared<std::ifstream>(filePath); // Open the file stream
        if (!contentStream->is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }
    }

    // Optionally, add a method to read from the stream
    std::string readContent() {
        if (!contentStream || !contentStream->is_open()) {
            throw std::runtime_error("File stream is not open.");
        }

        std::stringstream buffer;
        buffer << contentStream->rdbuf(); // Read the entire stream
        return buffer.str();
    }
};


/*Uses Unique Ptr: However TCP of wx maybe on a diff thread
struct URLComponents {
    std::string url;
    std::string domain;
    std::string path;
    std::string query;
    std::string fragment;
    std::string uuid;
    std::string domain_sanitised;
    std::string path_sanitised;
    std::unique_ptr<std::ifstream> contentStream; // Use unique_ptr for memory management

    // Delete the default constructor
    URLComponents() = delete;

    // Constructor to initialize with a URL
    URLComponents(const std::string& url)
        : url(url), uuid(UUIDcr::sgetUUID()) {
        URLParser parser(url);
        domain = parser.getHost();
        path = parser.getPath();
        query = parser.getQuery();
        domain_sanitised = parser.getDomainAsName();
        path_sanitised = parser.getPathAsName(); // Use getPathAsName for path sanitization
    }

    std::string getUUID()const {
        return uuid;
    }

    // Method to create the file path and directories
    fs::path getFilePath() {
        fs::path temp = CACHE_PATH / domain_sanitised / path_sanitised;
        fs::create_directories(temp); // Create directories if they do not exist
        temp /= uuid; // Append the UUID to the path
        temp.replace_extension(".dat"); // Set the file extension to .dat
        return temp;
    }

    // Method to set the file stream to read from the file
    void setFileStream() {
        fs::path filePath = getFilePath();
        contentStream = std::make_unique<std::ifstream>(filePath); // Open the file stream
        if (!contentStream->is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }
    }

    // Optionally, add a method to read from the stream
    std::string readContent() {
        if (!contentStream || !contentStream->is_open()) {
            throw std::runtime_error("File stream is not open.");
        }

        std::stringstream buffer;
        buffer << contentStream->rdbuf(); // Read the entire stream
        return buffer.str();
    }
};  
*/



/*struct URLComponents {
    std::string url;
    std::string domain;
    std::string path;
    std::string query;
    std::string fragment;
    std::string uuid;
    std::unique_ptr<std::string> content; // Using unique_ptr for ownership

    URLComponents() = delete;

    URLComponents(const std::string& url, std::unique_ptr<std::string> cont)
        : url(url), uuid(UUIDcr::sgetUUID()), content(std::move(cont)) {
        URLParser parser = URLParser(url);
        domain = parser.getHost();
        path = parser.getPath();
        query = parser.getQuery();
    }
};*/


/*struct URLComponents {
    std::string url;
    //std::string protocol;
    std::string domain;
    std::string path;
    std::string query;
    std::string fragment;
    std::string uuid;
    std::string* content;

    // Delete the default constructor
    URLComponents() = delete;

    // Optionally, you can provide a constructor that takes arguments
    URLComponents(const std::string& url , std::string* cont)
        : url(url), uuid(UUIDcr::sgetUUID()), content(cont){
        URLParser parser = URLParser(url);
        domain = parser.getHost();
        path = parser.getPath();
        query = parser.getQuery();
    }
};*/

