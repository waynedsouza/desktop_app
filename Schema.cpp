#include "Schema.h"
const std::string SchemaCl::data_groups_rel =
"id INTEGER PRIMARY KEY AUTOINCREMENT,"
"parent_id INTEGER,"
"group_id INTEGER,"
"UNIQUE(parent_id, group_id)";

/*const std::string SchemaCl::groups =
"id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name TEXT UNIQUE  NOT NULL ,"
"num_children INTEGER DEFAULT 0, " //--Number of child elements(e.g., URLs or items in the group)
"is_subscribed INTEGER DEFAULT 0," //--Subscription status : 1 for subscribed, 0 for not subscribed
"period_days INTEGER DEFAULT 0, "  // --Subscription period in days
"period_months INTEGER DEFAULT 0, " // --Subscription period in months
"last_active_date TEXT, " //--Last date the group was active(YYYY - MM - DD format)
"created_at DATETIME DEFAULT(datetime('now', 'localtime')), " //--Group creation date(auto - set)
"updated_at DATETIME DEFAULT(datetime('now', 'localtime'))  "; // --Last update date(auto - set)
*/
const std::string SchemaCl::groups =
"id INTEGER PRIMARY KEY AUTOINCREMENT, "
"name TEXT UNIQUE NOT NULL, "
"num_children INTEGER DEFAULT 0, " //--Number of child elements (e.g., URLs or items in the group)
"is_subscribed INTEGER DEFAULT 0, " //--Subscription status: 1 for subscribed, 0 for not subscribed
"period_days INTEGER DEFAULT 0, " //--Subscription period in days
"period_months INTEGER DEFAULT 0, " //--Subscription period in months
"last_active_date TEXT, " //--Last date the group was active (YYYY-MM-DD format)
"schedule_date TEXT, " //--New field to store the date from wxDatePickerCtrl (YYYY-MM-DD format)
"created_at DATETIME DEFAULT(datetime('now', 'localtime')), " //--Group creation date (auto-set)
"updated_at DATETIME DEFAULT(datetime('now', 'localtime')) ";

const std::string SchemaCl::datagrid =
"id INTEGER PRIMARY KEY AUTOINCREMENT, "
"uuid TEXT UNIQUE, "
"url TEXT NOT NULL, "
"type TEXT, "
"jobid TEXT, "  // Use TEXT for variable-length strings in SQLite
"allowed_sites_id INTEGER, "
"groups_id INTEGER NULL, "
"status TEXT, "
"date DATETIME DEFAULT CURRENT_TIMESTAMP, "
"created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
"updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
"FOREIGN KEY(allowed_sites_id) REFERENCES allowed_sites_tbl(id)"
"FOREIGN KEY(groups_id) REFERENCES data_groups_rel_tbl(id)";

const std::string SchemaCl::allowed_sites =
"id INTEGER PRIMARY KEY AUTOINCREMENT, "
"url TEXT UNIQUE NOT NULL, "
"name TEXT, "
"is_approved INTEGER, "  // Use INTEGER for boolean values
"is_local INTEGER, "     // Use INTEGER for boolean values
"status TEXT, "
"date DATETIME DEFAULT CURRENT_TIMESTAMP, "
"created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
"updated_at DATETIME DEFAULT CURRENT_TIMESTAMP";

const std::string SchemaCl::crawl_list =
"id INTEGER PRIMARY KEY AUTOINCREMENT, "
"parentid INTEGER, "
"allowed_sites_id INTEGER, "  // Added missing column
"url TEXT UNIQUE NOT NULL, "
"is_crawled INTEGER, "  // Use INTEGER for boolean values
"is_uploaded INTEGER, " // Use INTEGER for boolean values
"filename TEXT, "  // Use TEXT for variable-length strings in SQLite
"status TEXT, "
"date DATETIME DEFAULT CURRENT_TIMESTAMP, "
"created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
"updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
"FOREIGN KEY(parentid) REFERENCES datagrid_tbl(id), "
"FOREIGN KEY(allowed_sites_id) REFERENCES allowed_sites_tbl(id)";
