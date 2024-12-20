#pragma once
#include <string>
#include <optional>
#include <chrono>

class SchemaCl {
public:
    static const std::string datagrid;
    static const std::string allowed_sites;
    static const std::string crawl_list;
    static const std::string groups;
    static const std::string data_groups_rel;
};


struct GroupData {
    int id;
    std::string name;
    int num_children;
    bool is_subscribed;
    int period_days;
    int period_months;
    std::string last_active_date;
    std::string schedule_date;
    std::string created_at;
    std::string updated_at;
};

struct DataGrid {
    int id;                               // Primary key
    std::string uuid;                     // Unique identifier
    std::string url;                      // URL (not nullable)
    std::optional<std::string> type;      // Type (nullable)
    std::optional<std::string> jobid;     // Job ID (nullable)
    std::optional<int> allowed_sites_id;  // Foreign key to allowed_sites (nullable)
    std::optional<int> groups_id;         // Foreign key to groups (nullable)
    std::optional<std::string> status;    // Status (nullable)
    std::optional<std::string> date;        // Timestamp for "date"
    std::chrono::system_clock::time_point created_at;  // Timestamp for "created_at"
    std::chrono::system_clock::time_point updated_at;  // Timestamp for "updated_at"
};

