#pragma once
#include <wx/wx.h>
#include "MyFrame.h"
#include "Database.h"
#include "Schema.h"
#include "Folder.h"
#include <gtest/gtest.h>
#include <filesystem>
#include "ServerConfig.h"



const std::string CACHE = "CACHE";
const std::filesystem::path CACHE_PATH = fs::current_path() / CACHE;
// Declare the application class
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit(); // Overriding OnExit to perform cleanup
    ~MyApp(); // Destructor to properly close and delete the database
private:
    Database* db;  // Pointer to the Database object
};
#ifdef _DEBUG
bool debugMode = true;
#else
bool debugMode = false;
#endif
// Implement the application class
bool MyApp::OnInit()
{
    wxLog::SetActiveTarget(new wxLogStderr(fopen("logfile.txt", "w")));

    if(debugMode) wxLogMessage("App started");
    ServerConfig& config = ServerConfig::getInstance("config.yaml");
    wxLogMessage("Config inited with address %p", &config);
    //FolderRoutine folder;
    //folder.testNewMethods();
    // Initialize the database with a name
    db = new Database("XtrCrAi_DB", true);  // Set debug mode to true
    db->open();
    db->registerTable("datagrid", "datagrid", SchemaCl::datagrid, true);
    db->registerTable("allowed_sites", "allowed_sites", SchemaCl::allowed_sites, true);
    db->registerTable("crawl_list", "crawl_list", SchemaCl::crawl_list, true);
    db->registerTable("groups", "groups", SchemaCl::groups, true);
    db->registerTable("data_groups_rel", "data_groups_rel", SchemaCl::data_groups_rel, true);


    // Create the main application window
    MyFrame* frame = new MyFrame("wxWidgets Application", wxPoint(50, 50), wxSize(800, 600));
    frame->setUpDb(db);  // Pass the Database object to MyFrame
    frame->performInit();
    frame->Show(true);
    return true;
}

// Override OnExit to handle any additional cleanup if needed
int MyApp::OnExit()
{
    // You could add more cleanup code here if necessary
    return 0;
}

// Destructor to close and delete the Database object
MyApp::~MyApp()
{
    if (db)
    {
        db->close();  // Ensure the database is closed
        delete db;    // Free the memory allocated for the database
        db = nullptr;
    }
}

// Implement the application entry point
wxIMPLEMENT_APP(MyApp);
//wxIMPLEMENT_APP_NO_MAIN(MyApp);


