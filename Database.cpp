#include "Database.h"
#include <wx/log.h>
#include <sqlite3.h>
#include "Schema.h"
#include <stdexcept>
#include <chrono>

void Database::init() {
    groupNames = std::make_shared<std::vector<wxString>>();
    groupsDataMap = std::make_shared<std::unordered_map<std::string, GroupData>>();
}

Database::Database() : db(nullptr), debugMode(false) {
    init();
}

Database::Database(const wxString& name, bool debug) : db(nullptr), debugMode(debug) {
    dbName = std::string(name.mb_str());  // Convert wxString to std::string
    init();
}

Database::~Database() {
    close();  // Ensure the database is closed upon destruction
}

bool Database::open() {
    int result = sqlite3_open(dbName.c_str(), &db);
    if (result != SQLITE_OK) {
        wxLogError("Could not open database %s: %s", dbName, sqlite3_errmsg(db));
        return false;
    }
    return true;
}

void Database::close() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        wxLogError("SQL error: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    if (debugMode) {
        wxLogDebug("Executed SQL: %s", sql);
    }
    return true;
}

bool Database::ExecuteQuery(const wxString& query, sqlite3_stmt** stmt) {
    std::string queryStr = std::string(query.mb_str());
    int result = sqlite3_prepare_v2(db, queryStr.c_str(), -1, stmt, nullptr);

    if (result != SQLITE_OK) {
        wxLogError("Failed to prepare SQL statement: %s", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

void Database::FinalizeStatement(sqlite3_stmt* stmt) {
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
    }
}

void Database::BindAndExecute(sqlite3_stmt* stmt, const wxString& serial, const wxString& uuid, const wxString& url, const wxString& type, const wxString& status, const wxString& date) {
    sqlite3_bind_text(stmt, 1, serial.mb_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, uuid.mb_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, url.mb_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, type.mb_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, status.mb_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, date.mb_str(), -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        wxLogError("Failed to execute SQL statement: %s", sqlite3_errmsg(db));
    }

    FinalizeStatement(stmt);
}



void Database::BindAndExecute(sqlite3_stmt* stmt, const DataGrid& grid) {
    int bindIndex = 1;

    // Bind mandatory fields
    sqlite3_bind_text(stmt, bindIndex++, grid.uuid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIndex++, grid.url.c_str(), -1, SQLITE_TRANSIENT);

    // Bind optional fields
    if (grid.type.has_value()) {
        sqlite3_bind_text(stmt, bindIndex++, grid.type->c_str(), -1, SQLITE_TRANSIENT);
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    if (grid.jobid.has_value()) {
        sqlite3_bind_text(stmt, bindIndex++, grid.jobid->c_str(), -1, SQLITE_TRANSIENT);
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    if (grid.allowed_sites_id.has_value()) {
        sqlite3_bind_int(stmt, bindIndex++, grid.allowed_sites_id.value());
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    if (grid.groups_id.has_value()) {
        sqlite3_bind_int(stmt, bindIndex++, grid.groups_id.value());
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    if (grid.status.has_value()) {
        sqlite3_bind_text(stmt, bindIndex++, grid.status->c_str(), -1, SQLITE_TRANSIENT);
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    if (grid.date.has_value()) {
        sqlite3_bind_text(stmt, bindIndex++, grid.date->c_str(), -1, SQLITE_TRANSIENT);
    }
    else {
        sqlite3_bind_null(stmt, bindIndex++);
    }

    

    std::string createdAtStr = format_time(grid.created_at);
    sqlite3_bind_text(stmt, bindIndex++, createdAtStr.c_str(), -1, SQLITE_TRANSIENT);

    std::string updatedAtStr = format_time(grid.updated_at);
    sqlite3_bind_text(stmt, bindIndex++, updatedAtStr.c_str(), -1, SQLITE_TRANSIENT);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        wxLogError("Failed to execute statement: %s", wxString(sqlite3_errmsg(db)));
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt);
}


std::string Database::getName() const {
    return dbName;
}

std::string Database::getTableName(const std::string& key) const {
    auto it = tableNames.find(key);
    if (it != tableNames.end()) {
        return it->second;
    }
    return "";  // Or throw an exception if preferred
}

bool Database::loadTableNames() {
    tableNames.clear();
    const char* query = "SELECT name FROM sqlite_master WHERE type='table';";
    sqlite3_stmt* stmt;

    if (!ExecuteQuery(wxString(query), &stmt)) {
        wxLogError("Failed to prepare query to load table names.");
        return false;
    }

    int keyCounter = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string tableName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        tableNames[std::to_string(keyCounter++)] = tableName;  // Use a counter as the key
    }

    FinalizeStatement(stmt);
    return true;
}

bool Database::registerTable(const std::string& key, const std::string& tableName, const std::string& schema, bool withTrigger) {
    std::string fullTableName = tableName + "_tbl";
    std::string createTableSQL = "CREATE TABLE IF NOT EXISTS " + fullTableName + " (" + schema + ");";
    //const char* createTableSQL = "CREATE TABLE IF NOT EXISTS Groups (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE);";

    if (!executeSQL(createTableSQL)) {
        wxLogError("Failed to create table %s.", fullTableName);
        return false;
    }

    if (withTrigger) {
        std::string triggerSQL = "CREATE TRIGGER IF NOT EXISTS update_" + fullTableName + "_updated_at "
            "AFTER UPDATE ON " + fullTableName + " "
            "FOR EACH ROW "
            "BEGIN "
            "   UPDATE " + fullTableName + " SET updated_at = CURRENT_TIMESTAMP WHERE id = OLD.id; "
            "END;";
        if (!executeSQL(triggerSQL)) {
            wxLogError("Failed to create trigger for table %s.", fullTableName);
            return false;
        }
        else {
            if (debugMode) wxLogMessage("Trigger created  table : " + wxString(fullTableName));
        }
    }
    if (debugMode) wxLogMessage("Registered table : " + wxString(fullTableName));
    tableNames[key] = fullTableName;  // Register the table name with the key
    return true;
}
bool Database::AddGroupToDB(const wxString& groupName) {
    if (!db) {
        if (debugMode) wxLogMessage("Database is not open.");
        return false;
    }

    const std::string tableName = getTableName("groups");
    if (tableName.empty()) {
        if (debugMode) wxLogMessage("Table 'groups' not registered.");
        return false;
    }

    const std::string insertQuery =
        "INSERT INTO " + tableName + " (name, num_children, is_subscribed, period_days, period_months, last_active_date) "
        "VALUES (?, 0, 0, 0, 0, NULL)";

    sqlite3_stmt* stmt = nullptr;

    // Prepare the SQL insert statement
    if (sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to prepare statement: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    // Bind the group name to the prepared statement
    if (sqlite3_bind_text(stmt, 1, groupName.mb_str().data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to bind group name: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    // Execute the insert statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (debugMode) wxLogMessage("Failed to insert group: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    if (debugMode) wxLogMessage("Database Group added successfully: %s", groupName);

    // Finalize the statement
    FinalizeStatement(stmt);
    return true;
}
bool Database::RemoveGroup(const wxString& groupName) {
    if (!db) {
        if (debugMode) wxLogMessage("Database is not open.");
        return false;
    }

    const std::string groupsTable = getTableName("groups");
    const std::string dataGroupsRelTable = getTableName("data_groups_rel");

    if (groupsTable.empty() || dataGroupsRelTable.empty()) {
        if (debugMode) wxLogMessage("Required tables are not registered.");
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    // Step 1: Query group_id for the given groupName
    const std::string selectQuery = "SELECT id FROM " + groupsTable + " WHERE name = ?";
    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to prepare SELECT statement: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, groupName.mb_str().data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to bind group name for SELECT: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        if (debugMode) wxLogMessage("Group not found: %s", groupName);
        FinalizeStatement(stmt);
        return false;
    }

    int groupId = sqlite3_column_int(stmt, 0);
    FinalizeStatement(stmt);

    // Step 2: Delete rows from data_groups_rel where group_id matches
    const std::string deleteDataGroupsQuery = "DELETE FROM " + dataGroupsRelTable + " WHERE group_id = ?";
    if (sqlite3_prepare_v2(db, deleteDataGroupsQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to prepare DELETE statement for data_groups_rel: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    if (sqlite3_bind_int(stmt, 1, groupId) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to bind group_id for deletion: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (debugMode) wxLogMessage("Failed to delete related rows in data_groups_rel: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }
    FinalizeStatement(stmt);

    // Step 3: Delete the group from groups table
    const std::string deleteGroupQuery = "DELETE FROM " + groupsTable + " WHERE id = ?";
    if (sqlite3_prepare_v2(db, deleteGroupQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to prepare DELETE statement for groups: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    if (sqlite3_bind_int(stmt, 1, groupId) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to bind id for group deletion: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (debugMode) wxLogMessage("Failed to delete group: %s", wxString(sqlite3_errmsg(db)));
        FinalizeStatement(stmt);
        return false;
    }

    if (debugMode) wxLogMessage("Group and related rows removed successfully: %s", groupName);

    FinalizeStatement(stmt);
    return true;
}



std::vector<GroupData> Database::loadGroups() {
    const std::string query = "SELECT id, name, num_children, is_subscribed, period_days, "
        "period_months, last_active_date, schedule_date, created_at, updated_at "
        "FROM groups;";
    std::vector<GroupData> groups;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            GroupData group;
            group.id = sqlite3_column_int(stmt, 0);
            group.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            group.num_children = sqlite3_column_int(stmt, 2);
            group.is_subscribed = sqlite3_column_int(stmt, 3);
            group.period_days = sqlite3_column_int(stmt, 4);
            group.period_months = sqlite3_column_int(stmt, 5);
            group.last_active_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            group.schedule_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            group.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            group.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

            groups.push_back(group);
        }
    }
    sqlite3_finalize(stmt);
    return groups;
}



bool Database::saveSchedule(std::unique_ptr<GroupData> groupData)
{
    if (!groupData)
    {
        wxLogError("Invalid group data provided.");
        return false;
    }

    if (debugMode)
    {
        wxLogMessage(
            "Received Database::saveSchedule name:%s subscribed:%d period_days:%d period_months:%d schedule_date:%s",
            groupData->name, groupData->is_subscribed, groupData->period_days, groupData->period_months, groupData->schedule_date);
    }

    sqlite3_stmt* stmt = nullptr;
    std::string tablename = getTableName("groups");
    std::string query = "UPDATE " + tablename + " SET is_subscribed = ?, updated_at = ?,";

    // Prepare query based on conditions
    if (groupData->period_days > 0 || groupData->period_months > 0)
    {
        query += R"SQL(
            period_days = ?,
            period_months = ?,
            schedule_date = ""
        )SQL";
    }
    else
    {
        query += R"SQL(
            period_days = 0,
            period_months = 0,
            schedule_date = ?
        )SQL";
    }

    query += R"SQL(
        WHERE name = ?;
    )SQL";

    if (debugMode)
    {
        wxLogMessage("Query: %s", query);
    }

    // Prepare the statement
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        wxLogError("Failed to prepare statement: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    // Bind common parameters
    int bindIndex = 1;
    if (sqlite3_bind_int(stmt, bindIndex++, groupData->is_subscribed ? 1 : 0) != SQLITE_OK ||
        sqlite3_bind_text(stmt, bindIndex++, groupData->updated_at.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        wxLogError("Failed to bind common parameters: %s", wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    // Bind conditional parameters
    if (groupData->period_days > 0 || groupData->period_months > 0)
    {
        if (sqlite3_bind_int(stmt, bindIndex++, groupData->period_days) != SQLITE_OK ||
            sqlite3_bind_int(stmt, bindIndex++, groupData->period_months) != SQLITE_OK)
        {
            wxLogError("Failed to bind period parameters: %s", wxString(sqlite3_errmsg(db)));
            sqlite3_finalize(stmt);
            return false;
        }
    }
    else {
        if (sqlite3_bind_text(stmt, bindIndex++, groupData->schedule_date.c_str(), -1 , SQLITE_TRANSIENT) != SQLITE_OK) {
            wxLogError("Failed to bind schedule_date parameter: %s", wxString(sqlite3_errmsg(db)));
            sqlite3_finalize(stmt);
            return false;
        }
    }

    // Bind the name parameter
    if (sqlite3_bind_text(stmt, bindIndex++, groupData->name.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        wxLogError("Failed to bind name parameter: %s", wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    // Execute the statement
    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        wxLogError("Failed to execute statement: %s", wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    if (debugMode)
    {
        wxLogMessage("Database::saveSchedule updated successfully.");
    }

    return true;
}







std::shared_ptr<std::unordered_map<std::string, GroupData>> Database::LoadGroupsFromDB() {

    const std::string tableName = getTableName("groups");
    if (tableName.empty()) {
        if (debugMode) wxLogMessage("Table 'groups' not registered.");
        return groupsDataMap;
    }
    std::string sqlQuery = "SELECT id, name, num_children, is_subscribed, period_days, period_months, "
        "last_active_date,schedule_date, created_at, updated_at FROM " + tableName;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to prepare statement for loading groups: %s", sqlite3_errmsg(db));
        }
        return groupsDataMap;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        GroupData groupData;
        int bindIndex = 0;
        groupData.id = sqlite3_column_int(stmt, bindIndex++);//0
        const char* groupNameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        std::string groupName = groupNameText ? std::string(groupNameText) : std::string("");
        groupData.name = groupName;
        groupData.num_children = sqlite3_column_int(stmt, bindIndex++);
        groupData.is_subscribed = sqlite3_column_int(stmt, bindIndex++) == 1;
        groupData.period_days = sqlite3_column_int(stmt, bindIndex++);
        groupData.period_months = sqlite3_column_int(stmt, bindIndex++);

        const char* lastActiveDateText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData.last_active_date = lastActiveDateText ? std::string(lastActiveDateText) : std::string("");
        const char* sch_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData.schedule_date = sch_date ? std::string(sch_date) : std::string("");
        const char* createdAtText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData.created_at = createdAtText ? std::string(createdAtText) : std::string("");

        const char* updatedAtText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData.updated_at = updatedAtText ? std::string(updatedAtText) : std::string("");

        groupsDataMap->emplace(groupName, groupData);

        if (debugMode) {
            wxLogMessage("Loaded group: %s", groupName);
        }
    }

    FinalizeStatement(stmt);

    if (debugMode) {
        wxLogMessage("Finished loading all group data from the database.");
    }

    return groupsDataMap;
}

std::shared_ptr<std::vector<wxString>> Database::LoadGroupNamesFromDB() {
    
    LoadGroupsFromDB();

    if (groupsDataMap->empty()) {
        if (debugMode) {
            wxLogMessage("No groups found in the database.");
        }
        return groupNames;
    }

    for (const auto& groupPair : *groupsDataMap) {
        groupNames->emplace_back(wxString(groupPair.first));
    }

    if (debugMode) {
        wxLogMessage("Finished loading group names from the database.");
    }

    return groupNames;
}



std::optional<int> Database::isSiteAllowed(const wxString& url) {
    const std::string query = "SELECT id FROM allowed_sites_tbl WHERE url = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::optional<int> result; // Default is empty (indicating no result)

    try {
        // Prepare the SQL statement
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare SQL query");
        }

        // Bind the URL parameter
        if (sqlite3_bind_text(stmt, 1, url.ToStdString().c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
            throw std::runtime_error("Failed to bind URL parameter");
        }

        // Execute the query
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Retrieve the ID value and store it in the result
            result = sqlite3_column_int(stmt, 0);
        }

        // Finalize the statement
        sqlite3_finalize(stmt);
    }
    catch (const std::exception& e) {
        if (stmt) {
            sqlite3_finalize(stmt);
        }
        wxLogError("Database::isSiteAllowed failed: %s", e.what());
        throw; // Optionally rethrow the exception
    }

    return result;
}

bool Database::InsertDataGridItem(std::shared_ptr<DataGridItem> item)
{
    if (item == nullptr) {
        if (debugMode) wxLogMessage("Item empty in Database::InsertDataGridItem");
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    std::string tablename = getTableName("datagrid");
    const std::string query = "INSERT OR IGNORE INTO " + tablename +
        " (uuid, url, type, status, date, allowed_sites_id) VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Failed to prepare statement: %s", wxString(sqlite3_errmsg(db)));
    }
    DataGrid temp;
    temp.uuid = item->uuid;
    temp.url = item->url;
    temp.type = !item->type.empty() ? std::make_optional<std::string>(item->type.ToStdString()) : std::nullopt;
    temp.status = !item->status.empty() ? std::make_optional<std::string>(item->status.ToStdString()) :std::nullopt;
    temp.date = !item->date.empty() ? std::make_optional<std::string>(item->date.ToStdString()) : std::nullopt;
    temp.allowed_sites_id = isSiteAllowed(temp.url);
    try {
        BindAndExecute(stmt, temp);
    }
    catch (const std::runtime_error& e) {
        if(debugMode) wxLogMessage("InsertDataGridItem Error during bind and execute: %s", wxString(e.what()));
        sqlite3_finalize(stmt);
        return false;
    }
    //is this required
    int insertID = static_cast<int>(sqlite3_last_insert_rowid(db));
    InsertGroups(insertID, item->groupNames);
    item->id = insertID;
    
    
    // Finalize the prepared statement
    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to finalize statement: %s", wxString(sqlite3_errmsg(db)));
        }
        return false;
    }

    if (debugMode) {
        wxLogMessage("Successfully inserted item into the database.");
    }
    return true;
}
bool Database::InsertDataGridItemsIntoDB(std::shared_ptr<std::vector<std::shared_ptr<DataGridItem>>> items_ptr) {
    // Check for empty input
    if (items_ptr->empty()) {
        if (debugMode) {
            wxLogMessage("Items empty in Database::InsertDataGridItemsIntoDB");
        }
        return false;
    }

    // Prepare the insert query
    sqlite3_stmt* stmt = nullptr;
    std::string table = getTableName("datagrid");
    const std::string query = "INSERT OR IGNORE INTO " + table +
        " (uuid, url, type, status, date, allowed_sites_id) VALUES (?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to prepare statement: %s", wxString(sqlite3_errmsg(db)));
        }
        return false;
    }

    // Iterate over items and bind data to the prepared statement
    for (const auto& item : *items_ptr) {
        
        DataGrid temp;
        
        temp.uuid = item->uuid.ToStdString();
        temp.url = item->url;
        temp.type = !item->type.empty() ? std::make_optional(item->type.ToStdString()) : std::nullopt;
        temp.status = !item->status.empty() ? std::make_optional(item->status.ToStdString()) : std::nullopt;
        temp.date = !item->date.empty()? std::make_optional(item->date) :std::nullopt;
        temp.allowed_sites_id = isSiteAllowed(temp.url);

        try {
            BindAndExecute(stmt, temp);
             
        }
        catch (const std::runtime_error& e) {
            if (debugMode) {
                wxLogMessage("Error during bind and execute: %s", wxString(e.what()));
            }
            sqlite3_finalize(stmt);
           
            return false;
        }
        int insertID = static_cast<int>(sqlite3_last_insert_rowid(db));
        InsertGroups(insertID, item->groupNames);
        item->id = insertID;
    }

    // Finalize the prepared statement
    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to finalize statement: %s", wxString(sqlite3_errmsg(db)));
        }
        return false;
    }

    if (debugMode) {
        wxLogMessage("Successfully inserted items into the database.");
    }
    return true;
}


std::unique_ptr<GroupData> Database::LoadGroupByName(const wxString& name) {
    const std::string tableName = getTableName("groups");
    if (tableName.empty()) {
        if (debugMode) wxLogMessage("Table 'groups' not registered.");
        return nullptr;
    }

    std::string sqlQuery = "SELECT id, name, num_children, is_subscribed, period_days, period_months, "
        "last_active_date, schedule_date, created_at, updated_at "
        "FROM " + tableName + " WHERE name = ?";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to prepare statement for loading group: %s", sqlite3_errmsg(db));
        }
        return nullptr;
    }

    if (sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        if (debugMode) {
            wxLogMessage("Failed to bind name parameter: %s", sqlite3_errmsg(db));
        }
        FinalizeStatement(stmt);
        return nullptr;
    }

    std::unique_ptr<GroupData> groupData = nullptr;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        groupData = std::make_unique<GroupData>();
        int bindIndex = 0;

        groupData->id = sqlite3_column_int(stmt, bindIndex++);
        const char* groupNameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData->name = groupNameText ? std::string(groupNameText) : std::string("");
        groupData->num_children = sqlite3_column_int(stmt, bindIndex++);
        groupData->is_subscribed = sqlite3_column_int(stmt, bindIndex++) == 1;
        groupData->period_days = sqlite3_column_int(stmt, bindIndex++);
        groupData->period_months = sqlite3_column_int(stmt, bindIndex++);

        const char* lastActiveDateText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData->last_active_date = lastActiveDateText ? std::string(lastActiveDateText) : std::string("");
        const char* sch_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData->schedule_date = sch_date ? std::string(sch_date) : std::string("");
        const char* createdAtText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData->created_at = createdAtText ? std::string(createdAtText) : std::string("");
        const char* updatedAtText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, bindIndex++));
        groupData->updated_at = updatedAtText ? std::string(updatedAtText) : std::string("");

        if (debugMode) {
            wxLogMessage("Loaded group: %s", groupData->name);
        }
    }
    else {
        if (debugMode) {
            wxLogMessage("No group found with name: %s", name.c_str());
        }
    }

    FinalizeStatement(stmt);

    return groupData;
}



/*bool Database::deleteByUUID(const wxString& uuid) {
    if (debugMode) wxLogMessage("Database::deleteByUUID called %s with len:%zu", uuid , uuid.Length());
    // Prepare the DELETE query
    std::string table = getTableName("datagrid");
    const wxString query = "DELETE FROM " + table + " WHERE uuid = ?;";
    if (debugMode) wxLogMessage("query:%s|",query);
    sqlite3_stmt* stmt = nullptr;

    // Prepare the statement
    auto aa = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (debugMode) wxLogMessage("Database::deleteByUUID prepare op:%d" ,aa);
    if ( aa!= SQLITE_OK) {
        if (debugMode) {
            wxLogError("Failed to prepare statement: %s", wxString(sqlite3_errmsg(db)));
        }
        return false;
    }

    // Bind the UUID parameter
    sqlite3_bind_text(stmt, 1, uuid.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    int result = sqlite3_step(stmt);
    if (debugMode) wxLogMessage("Database::deleteByUUID step op:%d", result);
    if (result != SQLITE_DONE) {
        if (debugMode) {
            wxLogError("Failed to delete row: %s", wxString(sqlite3_errmsg(db)));
        }
        sqlite3_finalize(stmt);
        return false;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return true;
}*/

int Database::GetItemIdByUUID(wxString uuid) {
    std::string table = getTableName("datagrid");
    int id = -1;
    if (table.empty() ) {
        if (debugMode) wxLogError("Table datagrid names could not be resolved.");
        return NULL;
    }
    // Step 1: Select the 'id' using the UUID
    const wxString selectQuery = "SELECT id FROM " + table + " WHERE uuid = ?;";
    sqlite3_stmt* selectStmt = nullptr;
    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &selectStmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogError("Database::GetItemIdByUUID Failed to prepare SELECT statement: %s", wxString(sqlite3_errmsg(db)));
        return id;
    }
    if (sqlite3_bind_text(selectStmt, 1, uuid.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        if (debugMode) wxLogError("Database::GetItemIdByUUID Failed to bind UUID: %s", wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(selectStmt);
        return false;
    }
    int result = sqlite3_step(selectStmt);
    if (result == SQLITE_ROW) {
        id = sqlite3_column_int(selectStmt, 0);
    }
    else {
        if (debugMode) wxLogError("GetItemIdByUUID UUID not found or SELECT failed: %s query: %s result: %d", uuid, selectQuery, result);
        sqlite3_finalize(selectStmt);
        return id;
    }

    sqlite3_finalize(selectStmt);
    return id;
}

bool Database::deleteByUUID(const wxString& uuid) {
    if (debugMode) wxLogMessage("Database::deleteByUUID called with UUID: %s, len: %zu", uuid, uuid.Length());

    std::string table = getTableName("datagrid");
    std::string table2 = getTableName("data_groups_rel");

    if (table.empty() || table2.empty()) {
        if (debugMode) wxLogError("Table names could not be resolved.");
        return false;
    }

    int id = -1;

    // Step 1: Select the 'id' using the UUID
    const wxString selectQuery = "SELECT id FROM " + table + " WHERE uuid = ?;";
    sqlite3_stmt* selectStmt = nullptr;

    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &selectStmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogError("Failed to prepare SELECT statement: %s", wxString(sqlite3_errmsg(db)));
        return false;
    }

    if (sqlite3_bind_text(selectStmt, 1, uuid.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        if (debugMode) wxLogError("Failed to bind UUID: %s", wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(selectStmt);
        return false;
    }

    int result = sqlite3_step(selectStmt);
    if (result == SQLITE_ROW) {
        id = sqlite3_column_int(selectStmt, 0);
    }
    else {
        if (debugMode) wxLogError("UUID not found or SELECT failed: %s query: %s result: %d", uuid, selectQuery, result);
        sqlite3_finalize(selectStmt);
        return false;
    }

    sqlite3_finalize(selectStmt);

    // Step 2: Delete from the main table
    const wxString deleteQuery1 = "DELETE FROM " + table + " WHERE id = ?;";
    if (!executeDelete(deleteQuery1, id, "main table")) return false;

    // Step 3: Delete related rows
    const wxString deleteQuery2 = "DELETE FROM " + table2 + " WHERE parent_id = ?;";
    if (!executeDelete(deleteQuery2, id, "related table")) return false;

    if (debugMode) wxLogMessage("Successfully deleted UUID: %s and related data.", uuid);
    return true;
}

// Helper function to perform delete operations
bool Database::executeDelete(const wxString& query, int id, const wxString& context) {
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogError("Failed to prepare DELETE statement for %s: %s", context, wxString(sqlite3_errmsg(db)));
        return false;
    }

    if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
        if (debugMode) wxLogError("Failed to bind ID for %s: %s", context, wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        if (debugMode) wxLogError("Failed to execute DELETE for %s: %s", context, wxString(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}




std::shared_ptr<std::vector<DataGrid>> Database::loadDataGrid() {
    // Create a shared pointer to a vector that will hold the data grid items
    auto dataGridItems = std::make_shared<std::vector<DataGrid>>();

    // Prepare the SELECT query to fetch all data from the datagrid table
    std::string table = getTableName("datagrid");
    const wxString query = "SELECT id, uuid, url, type, jobid, allowed_sites_id, "
        "groups_id, status, date, created_at, updated_at FROM " + table + ";";

    sqlite3_stmt* stmt = nullptr;

    // Prepare the statement
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) {
            wxLogError("Failed to prepare SELECT statement: %s", wxString(sqlite3_errmsg(db)));
        }
        return nullptr;  // Return nullptr in case of error
    }

    // Iterate over all rows and extract data
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DataGrid item;

        // Extract each field and populate the DataGrid struct
        item.id = sqlite3_column_int(stmt, 0);
        item.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        // Check if nullable columns exist and retrieve them
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            item.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        }
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            item.jobid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        }
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            item.allowed_sites_id = sqlite3_column_int(stmt, 5);
        }
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            item.groups_id = sqlite3_column_int(stmt, 6);
        }
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            item.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        }

        // Extract timestamps
        // Retrieve Unix timestamp from SQLite
        int64_t created_at_timestamp = sqlite3_column_int64(stmt, 9);  // Unix timestamp for created_at
        int64_t updated_at_timestamp = sqlite3_column_int64(stmt, 10); // Unix timestamp for updated_at

        // Convert Unix timestamp to std::chrono::system_clock::time_point
        item.created_at = std::chrono::system_clock::from_time_t(created_at_timestamp);
        item.updated_at = std::chrono::system_clock::from_time_t(updated_at_timestamp);

        // Add the populated item to the vector
        dataGridItems->push_back(item);
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);

    return dataGridItems;  // Return the shared pointer to the vector
}


// Insert a group into the 'groups' table and return the group_id
bool Database::InsertGroups(int parent_id, std::vector<wxString> dataItems) {
    sqlite3_stmt* stmt;
    std::string tablename = getTableName("groups");
    std::string query = "SELECT id FROM " + tablename + " WHERE name = ?;";
    const char* sql = query.c_str();
    if (debugMode) wxLogMessage("InsertGroups for parent_id: %d", parent_id);
    
    for (const auto& groupName : dataItems) {
        if (debugMode) wxLogMessage("InsertGroups found group : %s", groupName);
        // Prepare the SQL statement
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
            std::string errorMsg = "Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db));
            throw std::runtime_error(errorMsg);
        }
        if (debugMode) wxLogMessage("InsertGroups binding  group : %s", groupName);
        // Bind the groupName parameter
        sqlite3_bind_text(stmt, 1, groupName.mb_str(wxConvUTF8), -1, SQLITE_TRANSIENT);

        // Execute the query
        int result = sqlite3_step(stmt);
        int groupId = -1;

        if (result == SQLITE_ROW) {
            groupId = sqlite3_column_int(stmt, 0); // Get the `id` column value
        }
        else if (result == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            std::string errorMsg = "Group name not found: " + std::string(groupName.mb_str(wxConvUTF8));
            throw std::runtime_error(errorMsg);
        }
        else {
            std::string errorMsg = "Error executing SQL query: " + std::string(sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            throw std::runtime_error(errorMsg);
        }
        if (debugMode) wxLogMessage("InsertGroups finalizing  group : %d", groupId);
        sqlite3_finalize(stmt); // Finalize the statement after execution

        // Insert the relationship into data_groups_rel table
        if (!InsertGroupRelationship(parent_id, groupId)) {
            std::string errorMsg = "Failed to insert group relationship for group ID: " + std::to_string(groupId);
            throw std::runtime_error(errorMsg);
        }
    }

    return true;
}


// Insert the relationship into the 'data_groups_rel' table
bool Database::InsertGroupRelationship(int parent_id, int group_id) {
    sqlite3_stmt* stmt;
    std::string tablename = getTableName("data_groups_rel");
    std::string sql_1 = "INSERT OR IGNORE INTO "+ tablename +" (parent_id, group_id) VALUES(? , ? ); ";
    const char* sql = sql_1.c_str();
    if (debugMode) wxLogMessage("InsertGroupRelationship start  parent : %d - group : %d", parent_id , group_id);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare SQL statement for relationship: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    if (debugMode) wxLogMessage("InsertGroupRelationship binding  parent : %d - group : %d", parent_id, group_id);
    sqlite3_bind_int(stmt, 1, parent_id);
    sqlite3_bind_int(stmt, 2, group_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to insert relationship: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt); // Finalize the prepared statement after execution
    if (debugMode) wxLogMessage("InsertGroupRelationship finalizing  parent : %d - group : %d", parent_id, group_id);
    return true;
}
std::vector<std::tuple<int, int, int>> Database::getGroupRelationships(){
    std::string tablename = getTableName("data_groups_rel");
    std::string sql = "SELECT id , parent_id , group_id FROM " + tablename + " WHERE 1";
    sqlite3_stmt* stmt = nullptr;
    std::vector<std::tuple<int, int, int>> group_rel_vec;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        if (debugMode) wxLogMessage("Database::getGroupRelationships failed to prepare statement");
        return group_rel_vec;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int parent_id = sqlite3_column_int(stmt, 1);
        int group_id = sqlite3_column_int(stmt, 2);
        if (debugMode) wxLogMessage("Database::getGroupRelationships parent_id:%d group_id:%d at primary_key:%d", parent_id , group_id , id);
        group_rel_vec.emplace_back(id, parent_id, group_id);
    }
    sqlite3_finalize(stmt);
    return group_rel_vec;
}

bool Database::InsertGroupRelationship(int parent_id, const std::vector<wxString>& groupNames) {
    if (!db) {
        wxLogError("Database is not open.");
        return false;
    }

    // Begin transaction
    std::string beginTransaction = "BEGIN TRANSACTION;";
    if (!executeSQL(beginTransaction)) {
        wxLogError("Failed to begin transaction.");
        return false;
    }

    try {
        // Step 1: Fetch existing relationships for the parent_id
        std::unordered_set<int> existingGroupIds;
        std::string selectQuery = "SELECT group_id FROM data_groups_rel_tbl WHERE parent_id = ?;";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            wxLogError("Failed to prepare SELECT query: %s", sqlite3_errmsg(db));
            return false;
        }

        sqlite3_bind_int(stmt, 1, parent_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            existingGroupIds.insert(sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);

        // Step 2: Get IDs for the provided group names
        std::unordered_set<int> newGroupIds;
        for (const auto& groupName : groupNames) {
            std::string groupSelect = "SELECT id FROM groups_tbl WHERE name = ?;";
            if (sqlite3_prepare_v2(db, groupSelect.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                wxLogError("Failed to prepare group SELECT query: %s", sqlite3_errmsg(db));
                return false;
            }

            sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int groupId = sqlite3_column_int(stmt, 0);
                newGroupIds.insert(groupId);

                // Step 3: Insert or ignore the new relationship
                std::string insertQuery = "INSERT OR IGNORE INTO data_groups_rel_tbl (parent_id, group_id) VALUES (?, ?);";
                sqlite3_stmt* insertStmt = nullptr;
                if (sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &insertStmt, nullptr) != SQLITE_OK) {
                    wxLogError("Failed to prepare INSERT query: %s", sqlite3_errmsg(db));
                    sqlite3_finalize(stmt);
                    return false;
                }

                sqlite3_bind_int(insertStmt, 1, parent_id);
                sqlite3_bind_int(insertStmt, 2, groupId);
                if (sqlite3_step(insertStmt) != SQLITE_DONE) {
                    wxLogError("Failed to execute INSERT query: %s", sqlite3_errmsg(db));
                }
                sqlite3_finalize(insertStmt);
            }
            sqlite3_finalize(stmt);
        }

        // Step 4: Remove outdated relationships
        for (int existingId : existingGroupIds) {
            if (newGroupIds.find(existingId) == newGroupIds.end()) {
                std::string deleteQuery = "DELETE FROM data_groups_rel_tbl WHERE parent_id = ? AND group_id = ?;";
                if (sqlite3_prepare_v2(db, deleteQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                    wxLogError("Failed to prepare DELETE query: %s", sqlite3_errmsg(db));
                    return false;
                }

                sqlite3_bind_int(stmt, 1, parent_id);
                sqlite3_bind_int(stmt, 2, existingId);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    wxLogError("Failed to execute DELETE query: %s", sqlite3_errmsg(db));
                }
                sqlite3_finalize(stmt);
            }
        }

        // Commit transaction
        std::string commitTransaction = "COMMIT;";
        if (!executeSQL(commitTransaction)) {
            wxLogError("Failed to commit transaction.");
            return false;
        }

        return true;
    }
    catch (...) {
        // Rollback transaction on error
        std::string rollbackTransaction = "ROLLBACK;";
        executeSQL(rollbackTransaction);
        wxLogError("An error occurred while updating group relationships.");
        return false;
    }
}

 
