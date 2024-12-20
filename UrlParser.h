// url_parser.h
#pragma once
#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <iostream>

class URLParser {
    friend class URLParserTest;
public:
    URLParser(const std::string& url);
    
    std::string getProtocol() const { return protocol_; }
    std::string getHost() const { return host_; }
    std::string getPort() const { return port_; }
    std::string getPath() const { return path_; }
    std::string getQuery() const { return query_; }
    std::string getFragment() const { return fragment_; }
    // New methods to format the domain and path as folder-safe names
    //std::string getDomainAsName() const;
    //std::string getPathAsName() const;
   
    bool compareURLs(const URLParser& other) const;
    // New methods to format the domain and path as folder-safe names
    std::string getDomainAsName() const;
    std::string getPathAsName() const;
    std::unordered_map<std::string, std::string> getQueryParameters()  { 
        if (queryParams_.empty()) {
            extractQueryParameters();
        }
            return queryParams_;
    }
    // Helper function to trim leading and trailing characters
     std::string trim(const std::string& str, char charToTrim) const{
        size_t first = str.find_first_not_of(charToTrim);
        if (first == std::string::npos) return ""; // no content
        size_t last = str.find_last_not_of(charToTrim);
        return str.substr(first, last - first + 1);
    }
private:
    void extractQueryParameters();
    void parse(const std::string& url);
    // Helper function to parse query parameters into a map
    std::unordered_map<std::string, std::string> parseQueryParameters(const std::string& query) const;
    std::string normalizeDomain(const std::string& domain) const;
    bool protocolsEqual(const std::string& protocol1, const std::string& protocol2) const {
        return (protocol1 == protocol2) ||
            ((protocol1 == "http" || protocol1 == "https") &&
                (protocol2 == "http" || protocol2 == "https"));
    }
    //std::string protocol_, host_, port_, path_, query_, fragment_;

    std::string sanitizeForFolderName(const std::string& input)const;
    std::string normalizePath(const std::string& path) const;
    std::unordered_map<std::string, std::string> queryParams_;  // Unordered map to hold query parameters
   // std::string protocol_, host_, query_; // Other members like path, fragment, etc.
    std::string protocol_, host_, port_, path_, query_, fragment_;
};

#endif // URL_PARSER_H


