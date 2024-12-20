#ifndef MYFRAME_H
#define MYFRAME_H

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/socket.h>
#include <queue>
#include "Database.h"
#include <json.hpp>
#include "DataGridItem.h"
#include "URLComponents.h"
#include "URLManager.h"
#include <wx/notebook.h>
#include <wx/choice.h>
#include <cassert> // Include for assert
#include "GroupInfo.h"
#include <unordered_set>   // For std::unordered_multiset
#include <wx/string.h>     // For wxString
#include <wx/grid.h>       // For wxGrid
#include <wx/log.h>        // For wxLogDebug
#include "ServerCommunications.h"


//class Database;
class GroupInfo;
class ServerCommunications;
class MyFrame : public wxFrame
{
    friend class MyFrameTest;
    friend class GroupInfo;
    //const int SERVER_ID = 3000;
    //const int SOCKET_ID = 3001;
    //const int SERVER_ID = wxWindow::NewControlId();//needs to work with macro
    //const int SOCKET_ID = wxWindow::NewControlId();
    enum {
        SERVER_ID = wxID_HIGHEST + 1,
        SOCKET_ID,
        SERVER_START,
        //OTHER_ID_1,
        //OTHER_ID_2,
        // Add more IDs as needed
    };


    enum ButtonID {
        LOAD_BUTTON = 1001,
        CONFIRM_BUTTON = 1002
    };
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);    
    ~MyFrame();
    bool ForTestsOnDeleteGroup(const wxString& groupname);
    inline void setUpDb(Database* dbo) { db = dbo; }; // Method to set up the database pointer
    void performInit();
    Database* getDb() const;   // Method to get the database pointer
    void LoadGroupRelationships();
    void OnLeftClickAddToGroup(wxGridEvent& event);
    void UpdateUrlGridRow(int row, const DataGridItem& dataItem);
    void UpdateUrlGridOnGroupDelete(const wxString& groupName);
    void RemoveGroupFromNotebook(const wxString& groupName);
    
    void setUpGroupItems(std::shared_ptr<DataGridItem> context_item);
    void displayGroupItems(std::shared_ptr<DataGridItem> items);
    void updateDbRelations(std::shared_ptr<DataGridItem> item);
    void UpdateUrlGrid(); // Method to refresh the grid
    void OnLeftClickDeleteGroup(wxGridEvent& event);
    void UpdateGridFromQueue();
    //void OnDeleteButtonClicked(wxCommandEvent& event);
    void OnDeleteRow(wxCommandEvent& event);
    //void AddGroupToNotebook(const wxString& groupName);
    void AddGroupToNotebook(const wxString& groupName, GroupData data); 
    void checkGroupsAssert(const std::string& from);
    void checkGroupsLog(const std::string& from);
    inline void addGroup(const wxString& newGroup) {
        groupNames.Add(newGroup);
        getDb()->AddGroupToDB(newGroup);
    }
    inline int getIndexOfGroup(const wxString& newGroup) const {
        return groupNames.Index(newGroup);
    };
    /*//bad programming
    inline void refreshGroupDisplay() {
        static bool isRefreshing = false;
        if (isRefreshing) return;
        isRefreshing = true;
        groupPanel->Refresh();
        groupPanel->Update();
        isRefreshing = false;

    }*/
    void doDeletePage(const wxString& groupName);
    std::shared_ptr<DataGridItem> getContextItem() const{
        return context_row;
    }
    void setContextItem(std::shared_ptr<DataGridItem> item){
            context_row = item;
    }
    void setUnitTest(){
        unittesting_flag=true;
    }
    bool  getUnitTest(){
        return unittesting_flag;
    }
    bool saveSchedule(std::unique_ptr<GroupData> data);
private:
    enum class StreamExitCondition {
        Success,
        MissingUUID,
        UrlPtrNotFound,
        InvalidRequest,
        Unknown_Error
    };
    void initGroupControls();
    void initCPControls();
    void initUrlControls();

    void OnScheduleGroup(const wxCommandEvent& event);
    // Event handlers
    void OnLoadData(wxCommandEvent& event);
    void initLoadData();
    void performLoadData();
    void OnTimer(wxTimerEvent& event);
    void OnGridCellClickAddToGroup(wxGridEvent& event);
    void OnConfirmData(wxCommandEvent& event);
    void OnClientConnect(wxSocketEvent& event);
    void OnMessageReceiveddep3(wxSocketEvent& event);
    void OnMessageReceivedAdvanced(wxSocketEvent& event);
    void OnMessageReceiveddeprecated2(wxSocketEvent& event);
    void OnMessageReceived(wxSocketEvent& event);
    void OnSocketEvent(wxSocketEvent& event);

    
   
    StreamExitCondition HandleStreamingRequest(wxSocketBase* socket, const wxString& request);
    //void HandleStreamingRequest(wxSocketBase* socket, const wxString& request);

    // Internal methods
    void HandleHttpRequest(wxSocketBase* socket, const wxString& request);
    bool ExtractUUIDFromRequest(const wxString& request, std::string& uuid);
    void ProcessStreamedData(std::shared_ptr<URLComponents> urlComponents, const wxString& bodyContent);
    void TrimStreamingPostRequest(wxString& request);
    wxString ExtractRequestBody(const wxString& request);
    void ParseAndQueueData(const wxString& jsonData);
    void ParseAndQueueUrl(const wxString& jsonData);
    void CleanUpGroupGridItems(const wxString& uuid);
    void OnLeftClickDelete(wxGridEvent& event);
    std::shared_ptr<URLComponents> ParseAndQueueUrlforStreaming(const wxString& jsonData);
    void InsertDataGridItemsIntoDB();
    void StartTcpServer();
    void OnMessageReceiveddeprecated(wxSocketEvent& event);
    void StopTcpServer();
   
    //GROUP related
    void OnAddGroup(wxCommandEvent& event);
    
    void OnDeleteGroup(wxCommandEvent& event);
    
    void doProcOnDeleteGroup(int  , const wxString&);
    void RemoveGroup(const wxString& groupName);
    void UpdateChoiceControl();
    
    void LoadGroupsFromDB(); // Load groups from DB
    void AddGroupToDB(const wxString& groupName); // Add group to DB
    void DeleteGroupFromDB(const wxString& groupName); // Delete group from DB

    void OnFrameClicked(wxMouseEvent& event) {
        wxLogStatus("Frame clicked at position: (%d, %d)", event.GetX(), event.GetY());

        // Optionally call the base class event handler
        event.Skip();
    }
    void AssertUrlGridItemsSync() {
        if (debugMode) wxLogMessage("AssertUrlGridItemsSync UrlGridItems:%zu dataGridView:%d", UrlGridItems.size(), dataGridView->GetNumberRows());
        // Ensure UrlGridItems and grid row count are in sync
        assert(UrlGridItems.size() == dataGridView->GetNumberRows() && "UrlGridItems and Grid rows are not synchronized.");
    }
    void AssertUrGroupGridItemsSync() {
        if (debugMode) wxLogMessage("AssertUrGroupGridItemsSync GroupGridItems:%zu urlGrid:%d", GroupGridItems.size(), urlGrid->GetNumberRows());
        // Ensure UrlGridItems and grid row count are in sync
        assert(GroupGridItems.size() == urlGrid->GetNumberRows() && "GroupGridItems and Grid rows are not synchronized. ");
        AssertUrlGridAndGroupNames();
    }
    bool precisionUpdateGrid(const wxString& groupname, std::shared_ptr<DataGridItem> item);
    void refreshGrid();
    void AssertUrlGridAndGroupNames();
    std::string trim(const std::string& s) {
    auto start = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}
    // UI elements
    
    wxButton* buttonLoad;
    wxButton* buttonConfirm;
    wxSpinCtrl* spinDays;
    wxSpinCtrl* spinMonths;
    // TCP Server and socket management
    wxSocketServer* tcpServer;
    wxSocketBase* clientSocket;
    int serial_no = 1;
    bool m_clkDeb1 = false;
    // Data handling
    std::queue<DataGridItem> dataQueue;

    // Database reference
    Database* db;    
    URLManager url_manager;    
   // void initGroupControls();
    wxNotebook* groupNotebook;

    wxGrid* dataGridView;//for uros
    wxGrid* urlGrid;//for grouping
    std::vector<std::shared_ptr<DataGridItem>> UrlGridItems;//Vector to maintain state for url grid matched against dataGridView
    std::vector<std::shared_ptr<DataGridItem>> GroupGridItems;// Vector to maintain state for group grid matched against urlGrid
    std::shared_ptr<DataGridItem> context_row; // Holds reference to selected item from UrlGridItems
    std::unique_ptr<ServerCommunications> remote_server=nullptr;

    wxArrayString groupNames; // Stores the names of groups
    wxBoxSizer* mainSizer;
    wxBoxSizer* sizerUrl;
    wxBoxSizer* sizerGroup;
    wxBoxSizer* sizerCP;
    wxPanel* mainPanel;       // Main panel
    //wxPanel* groupPanel;//bad programming
    bool singlegridflag = false;
    // Define an enum for exit conditions in streaming requests
    wxCheckListBox* groupCheckListBox; // CheckListBox for group selection
       
    std::unordered_map<wxString, std::shared_ptr<GroupInfo>> groupMap;    
    bool unittesting_flag=false;
    wxDECLARE_EVENT_TABLE();
};

#endif // MYFRAME_H


