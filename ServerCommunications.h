#pragma once    
#include <memory>
#include "MyFrame.h"
#include "json.hpp" // Use your preferred JSON library
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <string>
#include "ServerConfig.h"

    using boost::asio::ip::tcp;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;

    //using json = nlohmann::json; // Adjust as needed based on the JSON library you're using.
    class MyFrame;
    class ServerCommunications {
    public:
        explicit ServerCommunications(MyFrame* frame) : frame(frame) 
            ,config(&ServerConfig::getInstance()){
        
            wxLogMessage("Config in ServerCommunications inited %p" , static_cast<void*>(config));
        
            // Retrieve configuration values
       
        
           // int timeout = config.get<int>("server.timeout");
        }
        void initConfig() {
            if (debugMode)wxLogMessage("Initing config");
            auto conf = config->getConfig();
            
            host = (*conf)["server"]["host"].as<std::string>();
            if (debugMode)wxLogMessage("config loaded host:%s", host);
            port = (*conf)["server"]["port"].as<int>();
            if (debugMode)wxLogMessage("config port:%d", port);
            path = (*conf)["server"]["path"].as<std::string>();
            if (debugMode)wxLogMessage("config path:%s", path);

           /*try {
                host = config->get<std::string>("server.host");
                if (debugMode)wxLogMessage("config loaded host:%s", host);
                port = config->get<int>("server.port");
                if (debugMode)wxLogMessage("config port:%d", port);
                path = config->get<std::string>("server.path");
                if (debugMode)wxLogMessage("config path:%s", path);
                
            }
            catch (const std::runtime_error& err) {
                if (debugMode)wxLogMessage("configError error:%s", err.what());
            }*/
            
           
        }

        void sendDataToServer(const std::string& url);
        void processSend();
        nlohmann::json prepareData();

        bool sendJsonWithAsio(const std::string& jsonPayload);

        bool sendJsonWithBoostBeast(const std::string& jsonPayload);
        

    private:
        MyFrame* frame;
        //server api details
        std::string host;
        int port=-1;
        std::string path;
        ServerConfig* config;

        Database* getDb() const;// { return frame->getDb(); }
       // nlohmann::json toJson();

        //void sendJsonToServer(const nlohmann::json& j, const std::string& url);
       
    };
