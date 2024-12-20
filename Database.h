#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <wx/string.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "Schema.h"
#include "DataGridItem.h"
#include <optional>

class Database {
private:
    sqlite3* db;
    std::string dbName;  // Store the database name
    bool debugMode;
    std::unordered_map<std::string, std::string> tableNames;  // Key-value store for table names
    std::shared_ptr<std::unordered_map<std::string, GroupData>> groupsDataMap;
    std::shared_ptr<std::vector<wxString>> groupNames;
    // Helper function to execute SQL queries
    bool executeSQL(const std::string& sql);
    void init();
public:
   
    // Constructors
    Database();
    Database(const wxString& name, bool debug = false);

    // Destructor
    ~Database();

    // Open and close the database connection
    bool open();
    void close();

    // Execute query and handle statements
    bool ExecuteQuery(const wxString& query, sqlite3_stmt** stmt);
    void FinalizeStatement(sqlite3_stmt* stmt);
    void BindAndExecute(sqlite3_stmt* stmt, const wxString& serial, const wxString& uuid, const wxString& url, const wxString& type, const wxString& status, const wxString& date);
    void BindAndExecute(sqlite3_stmt* stmt, const DataGrid& grid);
    // Format chrono::time_point to string
    std::string format_time (std::chrono::system_clock::time_point tp) {
        std::time_t time = std::chrono::system_clock::to_time_t(tp);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    };

    // Get the database name
    std::string getName() const;
    // Get the table name by key
    std::string getTableName(const std::string& key) const;
    // Register a new table or ensure it exists
    bool registerTable(const std::string& key, const std::string& tableName, const std::string& schema, bool withTrigger = false);

    // Load table names from the database
    bool loadTableNames();
    bool AddGroupToDB(const wxString& groupName);
    bool RemoveGroup(const wxString& groupName);

    std::vector<GroupData> loadGroups();

    bool saveSchedule(std::unique_ptr<GroupData> groupData);

    std::shared_ptr<std::unordered_map<std::string, GroupData>> LoadGroupsFromDB();

    std::shared_ptr<std::vector<wxString>> LoadGroupNamesFromDB();    

    std::optional<int> isSiteAllowed(const wxString& url);

    bool  InsertDataGridItemsIntoDB(std::shared_ptr<std::vector<std::shared_ptr<DataGridItem>>> items);
    std::unique_ptr<GroupData> LoadGroupByName(const wxString& name);
    bool InsertDataGridItem(std::shared_ptr<DataGridItem> item);
    int GetItemIdByUUID(wxString uuid);
    bool deleteByUUID(const wxString& uuid);

    bool executeDelete(const wxString& query, int id, const wxString& context);

    std::shared_ptr<std::vector<DataGrid>> loadDataGrid();

    

    bool InsertGroups(int parent_id, std::vector<wxString> dataItems);

    bool InsertGroupRelationship(int parent_id, int group_id);

    std::vector<std::tuple<int, int, int>> getGroupRelationships();

    bool InsertGroupRelationship(int parent_id, const std::vector<wxString>& groupNames);

    

   

  

    
    
};

#endif
