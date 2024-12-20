#include "ServerCommunications.h"


void ServerCommunications::processSend()
{
    if (debugMode) {
        wxLogMessage("ServerCommunications::processSend start");
    }

    // Prepare JSON data
    nlohmann::json json = prepareData();
    if (debugMode) {
        wxLogMessage("ServerCommunications::processSend json: %s", json.dump().c_str());
    }

    // Send JSON using Asio
    auto op1 = sendJsonWithAsio(json.dump());
    if (debugMode) {
        wxLogMessage("ServerCommunications::processSend op1: %d", op1);
    }

    // Send JSON using Boost.Beast
    auto op2 = sendJsonWithBoostBeast(json.dump());
    if (debugMode) {
        wxLogMessage("ServerCommunications::processSend op2: %d", op2);
    }
}


nlohmann::json ServerCommunications::prepareData() {
    auto db = frame->getDb();
    std::shared_ptr<std::vector<DataGrid>> datagrid = db->loadDataGrid();
    std::vector<GroupData> groups = db->loadGroups();
    std::vector<std::tuple<int, int, int>> dataGroupsRel = db->getGroupRelationships();

    nlohmann::json json_arr;
    json_arr["data"] = nlohmann::json::array();
    json_arr["groups"] = nlohmann::json::array();
    json_arr["data_groups_rel"] = nlohmann::json::array();

    for (const DataGrid& data : *datagrid) {

        json_arr["data"].push_back({
            {"id", data.id},
            {"uuid", data.uuid},
            {"url", data.url},
            {"type", data.type.has_value() ? data.type.value() : ""},
            {"jobid", data.jobid.has_value() ? data.jobid.value() : ""},
            {"allowed_sites_id", data.allowed_sites_id.has_value() ? data.allowed_sites_id.value() : -1},
            {"groups_id", data.groups_id.has_value() ? data.groups_id.value() : -1},
            {"status", data.status.has_value() ? data.status.value() : ""},
            {"date", data.date.has_value() ? data.date.value() : ""},
            {"created_at", db->format_time(data.created_at)},
            {"updated_at", db->format_time(data.updated_at)}
            });
    }

    for (const auto& group : groups) {
        json_arr["groups"].push_back({
            {"id", group.id},
            {"name", group.name},
            {"num_children", group.num_children},
            {"is_subscribed", group.is_subscribed},
            {"period_days", group.period_days},
            {"period_months", group.period_months},
            {"last_active_date", group.last_active_date},
            {"schedule_date", group.schedule_date},
            {"created_at", group.created_at},
            {"updated_at", group.updated_at}
            });
    }
    // Data Groups Relationships
    json_arr["data_groups_rel"] = nlohmann::json::array();
    for (const auto& rel : dataGroupsRel) {
        json_arr["data_groups_rel"].push_back({
            {"id", std::get<0>(rel)},
            {"parent_id", std::get<1>(rel)},
            {"group_id", std::get<2>(rel)}
            });

       
    }
    return json_arr;
}

bool ServerCommunications::sendJsonWithAsio(const std::string& jsonPayload){
    try {
        // Create I/O context
        boost::asio::io_context io_context;

        // Resolve the server address and port
        tcp::resolver resolver(io_context);        
        boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints 
            = resolver.resolve(host, std::to_string(port));        
        // Create and connect the socket
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Create the HTTP POST request
        std::string request =
            "POST " + path + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Content-Type: application/json\r\n" +
            "Content-Length: " + std::to_string(jsonPayload.size()) + "\r\n" +
            "Connection: close\r\n\r\n" +
            jsonPayload;

        // Send the request
        boost::asio::write(socket, boost::asio::buffer(request));

        // Read the response
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Parse the response status line
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;

        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cerr << "Invalid response asio\n";
            return false;
        }

        if (status_code != 200) {
            std::cerr << "ASIO: Response returned with status code: " << status_code << "\n";
            return false;
        }

        // Read response headers
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            std::cout << header << "\n";
        }

        // Read the remaining response body
        std::ostringstream response_body;
        response_body << &response;

        std::cout << "Response body: " << response_body.str() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return true;
}





bool ServerCommunications::sendJsonWithBoostBeast(const std::string& jsonPayload) {
    try {
        net::io_context ioc;
        // Resolve the server's address
        tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(host, std::to_string(port));

        // Create a socket and connect to the server
        beast::tcp_stream stream(ioc);
        stream.connect(results);

        // Create the HTTP request
        http::request<http::string_body> req{ http::verb::post, path, 11 };
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        req.body() = jsonPayload;
        req.prepare_payload();

        // Send the request
        http::write(stream, req);

        // Receive the response
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        std::cout << "Response: " << res << std::endl;

        // Gracefully close the connection
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ ec };
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

// Implementation of getDb()
Database* ServerCommunications::getDb() const {
    return frame->getDb();
}



