#include "UrlParser.h"


#include <iterator>
#include <cctype>
#include <functional>

URLParser::URLParser(const std::string& url) {
    if (url.empty()) {
        throw std::invalid_argument("Empty URL is not valid");
    }
    parse(url);
}



void URLParser::parse(const std::string& url) {
    // Handling CSS-style URLs
    std::string::size_type start = 0;
    std::string url_copy = url; // Create a copy of the original URL

    if (url_copy.compare(0, 2, "//") == 0) {
        start = 2;  // Skip the leading "//"
    }

    // Find the fragment
    auto fragment_pos = url_copy.find('#', start);
    if (fragment_pos != std::string::npos) {
        fragment_ = url_copy.substr(fragment_pos + 1);
        // Remove the fragment from the URL for further processing
        url_copy = url_copy.substr(0, fragment_pos);
    }

    // Protocol
    auto protocol_end = url_copy.find("://", start);
    if (protocol_end != std::string::npos) {
        protocol_ = url_copy.substr(start, protocol_end - start);
        start = protocol_end + 3; // Move past "://"
    }

    // Host and port
    auto path_start = url_copy.find('/', start);
    auto host_end = url_copy.find(':', start);
    if (host_end == std::string::npos || (path_start != std::string::npos && host_end > path_start)) {
        host_end = path_start;
    }

    host_ = url_copy.substr(start, host_end - start);

    if (host_end != std::string::npos && url_copy[host_end] == ':') {
        auto port_end = path_start != std::string::npos ? path_start : url_copy.size();
        port_ = url_copy.substr(host_end + 1, port_end - host_end - 1);
    }

    // Path
    if (path_start != std::string::npos) {
        auto query_start = url_copy.find('?', path_start);
        path_ = url_copy.substr(path_start, query_start != std::string::npos ? query_start - path_start : url_copy.size() - path_start);

        // Query
        if (query_start != std::string::npos) {
            query_ = url_copy.substr(query_start + 1);
        }
    }
}


// Helper function to parse query parameters into a map
std::unordered_map<std::string, std::string> URLParser::parseQueryParameters(const std::string& query) const
{
    std::unordered_map<std::string, std::string> params;
    std::stringstream ss(query);
    std::string keyValuePair;

    // Split query string by '&' to extract key-value pairs
    while (std::getline(ss, keyValuePair, '&')) {
        size_t equalPos = keyValuePair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = keyValuePair.substr(0, equalPos);
            std::string value = keyValuePair.substr(equalPos + 1);
            params[key] = value;
        }
    }

    return params;
}

std::string URLParser::normalizeDomain(const std::string& domain) const
{
    std::string lowerDomain = domain;
    std::transform(lowerDomain.begin(), lowerDomain.end(), lowerDomain.begin(), ::tolower);

    // Find and remove 'www.' if it exists anywhere in the domain
    std::string www = "www.";
    size_t pos = lowerDomain.find(www);
    if (pos != std::string::npos) {
        lowerDomain.erase(pos, www.length());
    }

    return lowerDomain;
}

// Sanitize input for folder names
std::string URLParser::sanitizeForFolderName(const std::string& input) const{
    std::string sanitized = input;
    std::replace_if(sanitized.begin(), sanitized.end(),
        [](char c) { return !std::isalnum(c); }, '-'); // Replace non-alphanumeric characters
    sanitized = URLParser::trim(sanitized, '-');

    return sanitized;
   
}

// Get domain as a folder-safe name
std::string URLParser::getDomainAsName() const {
    return sanitizeForFolderName(host_);
}

// Get path as a folder-safe name
std::string URLParser::getPathAsName() const {
    return sanitizeForFolderName(path_);
}
void URLParser::extractQueryParameters()
{
    if (query_.empty()) return;
    queryParams_.clear();  // Clear any existing parameters
    std::istringstream queryStream(query_);
    std::string keyValuePair;

    while (std::getline(queryStream, keyValuePair, '&')) {
        size_t equalPos = keyValuePair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = keyValuePair.substr(0, equalPos);
            std::string value = keyValuePair.substr(equalPos + 1);
            // Decode the key and value if necessary
            queryParams_[key] = value;
        }
    }
}


bool URLParser::compareURLs(const URLParser& other) const
{
    std::string thisDomain = normalizeDomain(getHost());
    std::string otherDomain = normalizeDomain(other.getHost());

    // Compare normalized domains (ignore www. and case differences)
    if (thisDomain != otherDomain) {
        return false;
    }

    // Compare protocols, but treat http and https as the same
    if (!protocolsEqual(this->getProtocol(), other.getProtocol())) {
        return false;
    }

    // Check if both have query parameters
    if (query_.empty() && !other.query_.empty() || !query_.empty() && other.query_.empty()) {
        return false; // No query params, URLs are the same
    }
  

    // Parse and compare query parameters
    auto thisParams = parseQueryParameters(query_);
    auto otherParams = parseQueryParameters(other.query_);

    if (thisParams.size() != otherParams.size()) {
        return false; // Mismatched number of query parameters
    }

    // Compare key-value pairs in query parameters (order doesn't matter)
    for (const auto& pair : thisParams) {
        auto it = otherParams.find(pair.first);
        if (it == otherParams.end() || it->second != pair.second) {
            return false; // Key not found or value mismatch
        }
    }

    std::string thisPath = normalizePath(getPath());
    std::string otherPath = normalizePath(other.getPath());

    if (thisPath != otherPath) {
        return false;
    }

    return true; // All checks passed, URLs are considered equal
}


std::string URLParser::normalizePath(const std::string& path) const
{
    std::string normalizedPath = path;

    // Convert to lowercase (if paths should be case-insensitive)
    std::transform(normalizedPath.begin(), normalizedPath.end(), normalizedPath.begin(), ::tolower);

    // Remove trailing slash, if present (except if it's the root path "/")
    if (normalizedPath.length() > 1 && normalizedPath.back() == '/') {
        normalizedPath.pop_back();
    }

    return normalizedPath;
}
