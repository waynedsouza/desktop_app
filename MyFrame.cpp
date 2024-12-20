#include "MyFrame.h"
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/wx.h>
#include <string>
#include <queue>
#include <json.hpp> // For JSON parsing
#include <wx/spinctrl.h>  // Add this line to include wxSpinCtrl
#include <chrono>
#include <thread>
#include "UUID.h"
#include "button_renderers.h"
#include <iostream>
#include "WrappedTextCellRenderer.h"
#include "MyGridCellAutoWrapStringRenderer.h"
#include "GroupCellRenderer.h"
#include <wx/checklst.h>  // For wxCheckListBox
#include "ButtonCellRenderer.h"
#include "GroupSchedulerDialog.h"
#include "GroupManagementDialog.h"
#include <numeric>
#include <unordered_set>
#include <sstream>
#include <cassert>



// Define event table
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_BUTTON(MyFrame::LOAD_BUTTON, MyFrame::OnLoadData)
EVT_BUTTON(MyFrame::CONFIRM_BUTTON, MyFrame::OnConfirmData)
EVT_SOCKET(SERVER_ID, MyFrame::OnClientConnect)
EVT_SOCKET(SOCKET_ID, MyFrame::OnMessageReceived)
EVT_TIMER(wxID_ANY, MyFrame::OnTimer) // Bind the timer event to OnTimer
wxEND_EVENT_TABLE()
/*EVT_MENU(SERVER_START, MyFrame::StartTcpServer)
EVT_SOCKET(wxID_ANY, MyFrame::OnMessageReceived)*/

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, wxID_ANY, title, pos, size), tcpServer(nullptr), clientSocket(nullptr), db(nullptr)
{
    mainPanel = new wxPanel(this, wxID_ANY);
    mainSizer = new wxBoxSizer(wxVERTICAL); // Main vertical sizer for the panel   
    sizerCP = new wxBoxSizer(wxVERTICAL);
    initCPControls();
    mainSizer->Add(sizerCP, 0, wxEXPAND | wxALL, 4);

    // Initialize and add group controls
    sizerGroup = new wxBoxSizer(wxVERTICAL);
    initGroupControls(); // Pass the group sizer to the function
    mainSizer->Add(sizerGroup, 0, wxEXPAND | wxALL, 4); // Add group controls sizer to main sizer

    // Initialize and add URL controls
    sizerUrl = new wxBoxSizer(wxVERTICAL);
    initUrlControls(); // Create a separate function to initialize URL controls
    mainSizer->Add(sizerUrl, 1, wxEXPAND | wxALL, 4); // Add URL controls sizer to main sizer
    //mainSizer->Add(sizerUrl, 1,  wxALL, 5);

     //init manual buttons
    

    mainPanel->SetSizer(mainSizer); // Set the main sizer for the panel
    mainPanel->Layout(); // Layout the panel

    // Initialize TCP server or other initialization tasks
    // StartTcpServer();
    if (debugMode) wxLogMessage("MyFrame initialized successfully.");
}

MyFrame::~MyFrame() {
    StopTcpServer();
}



bool MyFrame::saveSchedule(std::unique_ptr<GroupData> data)
{
    wxString name = wxString(data->name);
    if (groupMap.find(name) != groupMap.end()) {
        auto groupInfo = groupMap[name];
        groupInfo->setScheduledMessage(*data);
    }
    else {
        if (debugMode) wxLogMessage("groupMap does not have %s group in MyFrame::saveSchedule:%s", name);
        return false;
    }
    bool ret = getDb()->saveSchedule(std::move(data));

    return ret;
}

void MyFrame::initGroupControls() {
    // Add a button to add groups
    /*wxButton* addGroupButton = new wxButton(mainPanel, wxID_ANY, "Add Group");
    sizerGroup->Add(addGroupButton, 0, wxALL | wxEXPAND, 2);
    Connect(addGroupButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyFrame::OnAddGroup));*/

    // Create the notebook for managing group tabs
    groupNotebook = new wxNotebook(mainPanel, wxID_ANY);
    sizerGroup->Add(groupNotebook, 1, wxEXPAND | wxALL, 8);

    // Create a grid to display the URLs and group assignment
    urlGrid = new wxGrid(mainPanel, wxID_ANY);
    urlGrid->CreateGrid(0, 4); // 5 rows, 3 columns for URL, Parameters, and Group Assignment
    urlGrid->SetColLabelValue(0, "URL");
    urlGrid->SetColLabelValue(1, "Param 1");
    urlGrid->SetColLabelValue(2, "Group");
    urlGrid->SetColLabelValue(3, "Actions");

    sizerGroup->Add(urlGrid, 1, wxEXPAND | wxALL, 8);

    // Optionally, you can call Layout here if needed
    //LoadGroupsFromDB();
}
void MyFrame::initCPControls() {
    // Create a horizontal sizer for the buttons and spin controls
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    // Add a button to add groups
    wxButton* addGroupButton = new wxButton(mainPanel, wxID_ANY, "Add Group");
    buttonSizer->Add(addGroupButton, 0, wxALL | wxEXPAND, 2);
    Connect(addGroupButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyFrame::OnAddGroup));
    //buttonLoad = new wxButton(mainPanel, LOAD_BUTTON, "Load Data", wxPoint(5, 420));
    buttonLoad = new wxButton(mainPanel, LOAD_BUTTON, "Load Data");
    buttonSizer->Add(buttonLoad, 0, wxALL, 5); // Add Load button//sizerUrl->Add(buttonLoad, 0, wxALL | wxEXPAND, 5); // Add Load button with padding
    buttonConfirm = new wxButton(mainPanel, CONFIRM_BUTTON, "Confirm Data");
    buttonSizer->Add(buttonConfirm, 0, wxALL, 5); // Add Confirm button//sizerUrl->Add(buttonConfirm, 0, wxALL, 5); // Add Confirm button with padding

    spinDays = new wxSpinCtrl(mainPanel, wxID_ANY, wxEmptyString, wxPoint(200, 420));
    buttonSizer->Add(spinDays, 0, wxALL, 5); // Add spin control for days//sizerUrl->Add(spinDays, 0, wxALL, 5); // Add spin control for days

    spinMonths = new wxSpinCtrl(mainPanel, wxID_ANY, wxEmptyString, wxPoint(300, 420));
    buttonSizer->Add(spinMonths, 0, wxALL, 5); // Add spin control for months//sizerUrl->Add(spinMonths, 0, wxALL, 5); // Add spin control for months
    
    // Optionally, you can call Layout here if needed
    wxButton* scheduleGroupButton = new wxButton(mainPanel, wxID_ANY, "Scheduler");
    buttonSizer->Add(scheduleGroupButton, 0, wxALL | wxEXPAND, 5);

    // Bind the button click to the OnScheduleGroup method
    scheduleGroupButton->Bind(wxEVT_BUTTON, &MyFrame::OnScheduleGroup, this);
    sizerCP->Add(buttonSizer, 0, wxALIGN_LEFT); // Add the horizontal sizer to the main vertical sizer
}
void MyFrame::initUrlControls() {
    dataGridView = new wxGrid(mainPanel, wxID_ANY, wxPoint(5, 5), wxSize(180, 400));
    dataGridView->CreateGrid(0, 8);
    dataGridView->SetColLabelValue(0, "Serial #");
    dataGridView->SetColLabelValue(1, "UUID");
    dataGridView->SetColLabelValue(2, "URL");
    dataGridView->SetColLabelValue(3, "Type");
    dataGridView->SetColLabelValue(4, "Status");
    dataGridView->SetColLabelValue(5, "Date");
    dataGridView->SetColLabelValue(6, "Actions");
    dataGridView->SetColLabelValue(7, "Groups");
    //dataGridView->SetCellRenderer(0 ,2, new WrappedTextCellRenderer());
    // Add the data grid to the URL controls sizer
    sizerUrl->Add(dataGridView, 0, wxALL | wxEXPAND, 1);  // Add grid to sizer    
    dataGridView->SetColSize(2, 200); 
    //grid->SetColSize(columnIndex, maxWidth);

    
}
void MyFrame::OnScheduleGroup(const wxCommandEvent& event)
{
    // Assuming you have the list of groups available
    //wxArrayString groupNames = GetGroupNames(); // You would replace this with your method for fetching group names

    // Show the scheduling dialog
    GroupSchedulerDialog dlg(this, groupNames);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString selectedGroup = dlg.GetSelectedGroup();
        if (dlg.IsScheduleByPeriod())
        {
            int months = dlg.GetMonths();
            int days = dlg.GetDays();
            wxLogMessage("Scheduled %s by period: %d months, %d days", selectedGroup, months, days);
        }
        else
        {
            wxDateTime selectedDate = dlg.GetSelectedDate();
            wxLogMessage("Scheduled %s by date: %s", selectedGroup, selectedDate.FormatISODate());
        }
    }
    checkGroupsAssert("OnScheduleGroup");
    checkGroupsLog("OnScheduleGroup");
}



void MyFrame::performInit()
{
    remote_server = std::make_unique<ServerCommunications>(this);
    remote_server->initConfig();
    StartTcpServer();
    LoadGroupsFromDB();//groupMap holds the info and groupnames the names
    performLoadData();//UrlGridItems holds the url info
    LoadGroupRelationships();//GroupGridItems holds the group info
}




Database* MyFrame::getDb() const {
        return db;
    }
    void MyFrame::LoadGroupRelationships() {
        if (debugMode) wxLogMessage("LoadGroupRelationships");
        bool good_run_flag = true;
        std::vector<std::tuple<int, int, int>> group_relations = getDb()->getGroupRelationships();
        std::map<int, std::vector<int>> id_group_maps;
        std::map<std::shared_ptr<DataGridItem>, std::vector<std::shared_ptr<GroupInfo>>> url_groups_item_maping;
        for (auto& tup : group_relations) {
            int parent_id = std::get<1>(tup);
            int group_id = std::get<2>(tup);
            if (id_group_maps.find(parent_id) == id_group_maps.end()) {
                id_group_maps.emplace(parent_id, std::vector<int>{group_id});//IMPORTANT
                if (debugMode) wxLogMessage("LoadGroupRelationships emplacing parent_id:%d group_id:%d", parent_id, group_id);
            }
            else {
                id_group_maps.at(parent_id).push_back(group_id);
                if (debugMode) wxLogMessage("LoadGroupRelationships pushing back parent_id:%d group_id:%d", parent_id, group_id);
            }
        }
        if (debugMode) wxLogMessage("Sucesfully created id_group_maps parents_ size:%zu", id_group_maps.size());
        for (auto& parent_group_pair : id_group_maps) {
            int parent_id = parent_group_pair.first;
            std::vector<int> groups_ids = parent_group_pair.second;
            auto it = std::find_if(UrlGridItems.begin(), UrlGridItems.end(), [&](const std::shared_ptr<DataGridItem>& pred) {
                return pred->id == parent_id;
            });
            if (debugMode) wxLogMessage("******parent_id:%d \n", parent_id);
            if (it != UrlGridItems.end()) {
                //found a url need to find the associated groups
                std::vector<std::shared_ptr<GroupInfo>> groupdata;
                for (const int group_id : groups_ids) {
                    if (debugMode) wxLogMessage("found parent:%d  in UrlGridItems searching for group:%d", parent_id, group_id);
                    auto it_group_name = std::find_if(groupMap.begin(), groupMap.end(), [&](const std::pair<const wxString, std::shared_ptr<GroupInfo>>& item) {
                        return item.second->getId() == group_id;
                    });
                    if (it_group_name != groupMap.end()) {
                        groupdata.push_back(it_group_name->second);
                        (*it)->groupNames.push_back(it_group_name->second->getName());
                        if(debugMode) wxLogMessage("Found parent:%d group:%d groupdata filled groupNames:%s", parent_id, group_id, it_group_name->second->getName());
                    }
                    else {
                        std::cout << "Not Found in GroupMap\m DEBUG INFO groupMap groups are:" << std::endl;
                        std::for_each(groupMap.begin(), groupMap.end(), [](const std::pair<wxString , std::shared_ptr<GroupInfo>>& item) {
                            std::cout << item.first << "|" << item.second->getName() << "|"<< item.second->getId()<< std::endl;
                        });
                        if (debugMode) wxLogMessage("\nNot Found in GroupMap parent:%d group:%d ", parent_id, group_id);
                        good_run_flag = false;
                        break;
                    }
                }
                url_groups_item_maping.emplace(*it , groupdata);
            }
            else {
            std::cout << "DEBUG THIS UrlGridItems" << std::endl;
                std::for_each(UrlGridItems.begin(), UrlGridItems.end(), []( std::shared_ptr<DataGridItem> item) {
                    std::cout << item->uuid << "|" << (item->id.has_value() ? std::to_string(item->id.value()) : "NO Value" )<< std::endl;
                });
                if (debugMode)wxLogMessage("parent_id%d not found in UrlGridItems ", parent_id);
                good_run_flag = false;
                break;
            }   

        }
        assert(good_run_flag == true && "NOT A GOOD RUN");
        int row = urlGrid->GetNumberRows();
        if (debugMode)wxLogMessage("Forming UrlGrid row:%d", row);
        for (const auto& pair : url_groups_item_maping) {   
            std::shared_ptr<DataGridItem> data_item = pair.first;
            GroupGridItems.push_back(data_item);
            urlGrid->AppendRows(1);            
            urlGrid->SetCellValue(row, 0, data_item->url);  
            urlGrid->SetCellValue(row, 1, data_item->uuid);
            std::ostringstream oss;
            if (!data_item->groupNames.empty()) {
                for (size_t i = 0; i < data_item->groupNames.size(); ++i) {
                    oss << data_item->groupNames[i]; // Add the current string
                    if (i < data_item->groupNames.size() - 1) {
                        oss << ','; // Add a comma if it's not the last element
                    }
                }
            }
            std::string groupNamesStr = oss.str();  
            urlGrid->SetCellValue(row, 2, wxString(groupNamesStr));
            row++;
        }
        urlGrid->AutoSize();
        urlGrid->ForceRefresh();
        checkGroupsAssert("LoadGroupRelationships");
        checkGroupsLog("LoadGroupRelationships");
        AssertUrGroupGridItemsSync();

}
   /* void MyFrame::LoadGroupRelationships() {
        bool good_run_flag = true;

        // Step 1: Populate id_group_maps
        std::vector<std::tuple<int, int, int>> group_relations = getDb()->getGroupRelationships();
        std::map<int, std::vector<int>> id_group_maps;
        for (const auto& tup : group_relations) {
            id_group_maps[std::get<1>(tup)].push_back(std::get<2>(tup));
        }

        // Step 2: Create hash maps for faster lookups
        std::unordered_map<int, std::shared_ptr<DataGridItem>> url_grid_map;
        for (const auto& item : UrlGridItems) {
            url_grid_map[item->id] = item;
        }

        std::unordered_map<int, std::shared_ptr<GroupInfo>> group_map_by_id;
        for (const auto& [key, value] : groupMap) {
            group_map_by_id[value->getId()] = value;
        }

        // Step 3: Map relationships
        std::map<std::shared_ptr<DataGridItem>, std::vector<std::shared_ptr<GroupInfo>>> url_groups_item_mapping;
        for (const auto& [parent_id, group_ids] : id_group_maps) {
            auto it = url_grid_map.find(parent_id);
            if (it != url_grid_map.end()) {
                std::vector<std::shared_ptr<GroupInfo>> groupdata;
                for (int group_id : group_ids) {
                    auto it_group_name = group_map_by_id.find(group_id);
                    if (it_group_name != group_map_by_id.end()) {
                        groupdata.push_back(it_group_name->second);
                        it->second->groupNames.push_back(it_group_name->second->getName());
                    }
                    else {
                        good_run_flag = false;
                        break;
                    }
                }
                url_groups_item_mapping.emplace(it->second, groupdata);
            }
            else {
                good_run_flag = false;
                break;
            }
        }

        assert(good_run_flag && "NOT A GOOD RUN");

        // Step 4: Populate urlGrid
        urlGrid->AppendRows(url_groups_item_mapping.size());
        int row = urlGrid->GetNumberRows() - url_groups_item_mapping.size();
        for (const auto& [data_item, groupdata] : url_groups_item_mapping) {
            GroupGridItems.push_back(data_item);
            urlGrid->SetCellValue(row, 0, data_item->url);
            urlGrid->SetCellValue(row, 1, data_item->uuid);
            urlGrid->SetCellValue(row, 2, wxString::FromUTF8(joinStrings(data_item->groupNames, ',')));
            row++;
        }

        urlGrid->AutoSize();
        urlGrid->ForceRefresh();
        checkGroupsAssert("LoadGroupRelationships");
        checkGroupsLog("LoadGroupRelationships");
        AssertUrGroupGridItemsSync();
    }*/

void MyFrame::OnLeftClickAddToGroup(wxGridEvent& event)
{
    AssertUrlGridAndGroupNames();
    int row = event.GetRow(); // Get the row where the button was clicked
    int col = event.GetCol(); // Column for the 'Add to Group' button
    if (debugMode) wxLogMessage("OnLeftClickAddToGroup row:%d column:%d" , row , col);
    // Ensure it's the 'Add to Group' column
    if (col == 7) // Assuming column 7 is for the group button
    {
        // Get the currently selected group (the active notebook tab)
        int selectedGroupIndex = groupNotebook->GetSelection();

        if (debugMode) wxLogMessage("OnLeftClickAddToGroup selectedGroupIndex:%d    ");
        // If no group is selected, prompt the user
        if (selectedGroupIndex == wxNOT_FOUND)
        {
            wxMessageBox("Please select a group before adding the URL.");
            return;
        }

        // Get the label of the selected group (the name of the notebook tab)
        wxString groupName = groupNotebook->GetPageText(selectedGroupIndex);
        wxString url = dataGridView->GetCellValue(row, 2); // Get the URL

        wxLogMessage("Added URL: %s to Group: %s", url, groupName);
        // Add the group name to the 'Group' column in the corresponding row
        if (row >= 0 && row < UrlGridItems.size()) {
            auto itemToAdd = UrlGridItems[row]; // Get item with associated metadata

            // Add item to GroupGridItems to retain all metadata
            GroupGridItems.push_back(itemToAdd);
        }
        int row_g = urlGrid->GetNumberRows();
        
        urlGrid->AppendRows(1);
        urlGrid->SetCellValue(row_g, 0, url);
        urlGrid->SetCellValue(row_g, 2, groupName); // Assuming column 6 is for groups
        urlGrid->SetCellRenderer(row_g, 3, new ButtonRenderer());
        urlGrid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDeleteGroup, this);
        urlGrid->AutoSize();
        urlGrid->ForceRefresh();
        AssertUrGroupGridItemsSync();
    }
}

void MyFrame::UpdateUrlGridRow(int row, const DataGridItem& dataItem) {
   // AssertUrlGridAndGroupNames();
    //used to display context_item in UrlGrid on deletion , This is called from UpdateUrlGridOnGroupDelete
    //we need to merge this down the line with displayGroupItems as they both do the same thing
    wxLogMessage("UpdateUrlGridRow: %s to url: %s on row:%d", dataItem.uuid, dataItem.url , row);
    // Assuming 'url' is placed in the first column
    urlGrid->SetCellValue(row, 0, dataItem.url);
    // Set UUID for "Param 1", adjust if needed based on your DataGridItem structure
    urlGrid->SetCellValue(row, 1, dataItem.uuid);  // Example placeholder; adjust based on your actual parameters
    // Concatenate group names into a single string for display in the "Group" column
    std::ostringstream oss;
    // Check if groupNames is not empty to avoid unnecessary processing
    if (!dataItem.groupNames.empty()) {
        for (size_t i = 0; i < dataItem.groupNames.size(); ++i) {
            oss << dataItem.groupNames[i]; // Add the current string
            if (i < dataItem.groupNames.size() - 1) {
                oss << ','; // Add a comma if it's not the last element
            }
        }
    }

    std::string groupNamesStr = oss.str(); // Get the concatenated result
    //std::cout << "\n\n\n****UpdateGroupName for uuid:"<<dataItem.uuid<<" in MyFrame:"<< groupNamesStr<< " bearing url"<<dataItem.url<<"\n\n\n"<<std::endl;
    // Set the concatenated group names in the grid (convert std::string to wxString)
    urlGrid->SetCellValue(row, 2, wxString(groupNamesStr));

    // For "Actions", set a placeholder action text; could also add buttons if wxGrid supports it
    urlGrid->SetCellValue(row, 3, "Delete");  // Placeholder action text
    urlGrid->SetCellRenderer(row, 3, new ButtonRenderer());
    //rebinding is unnecessary test this
    //urlGrid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDeleteGroup, this);
    urlGrid->AutoSize();
    urlGrid->ForceRefresh();
    checkGroupsAssert("UpdateUrlGridRow");
    checkGroupsLog("UpdateUrlGridRow");
    AssertUrGroupGridItemsSync();
}


/*void MyFrame::precisionUpdateGrid(const wxString& groupname, std::shared_ptr<DataGridItem> item) {
    int rowCount = urlGrid->GetNumberRows();
    int eq_row_count = static_cast<int>(GroupGridItems.size());
    std::vector<std::shared_ptr<DataGridItem>>::iterator it = std::find_if(GroupGridItems.begin(), GroupGridItems.end(),[&](std::shared_ptr<DataGridItem> group_item) {
        return *item == *group_item;
    });
    int row = std::distance(GroupGridItems.begin(), it);//represents row of data on UrlGrid which is not teh same as GroupGrid
    
    // Assertions for debug mode (only active in debug builds)
#ifdef _DEBUG
    assert(row >= 0 && eq_row_count && "Group Grid row count assert");  // Validate row index
    assert(rowCount == eq_row_count);        // Validate grid and data consistency
    assert(*(GroupGridItems[row]) == *item);     // Validate that the correct item is being processed
#endif

    // Check row index validity for safety in release mode
    if (row < 0 || row >= eq_row_count) {
        wxLogError("Invalid row index: %d (GroupGridItems size: %d)", row, eq_row_count);
        return;
    }

    // Double-check data consistency in release builds
    if (**it != *item) {
        wxLogError("Data mismatch: Grid item at row %d does not match the provided item", row);
        return;
    }

    // Locate the group name in the data item's groupNames list
    auto it_groupname = std::find((*it)->groupNames.begin(), (*it)->groupNames.end(), groupname);
    if (it_groupname != (*it)->groupNames.end()) {
        std::cout << "\nFound group name " << groupname << ", performing delete." << std::endl;

        // Erase the group name from the list
        (*it)->groupNames.erase(it_groupname);        
        if ((*it)->groupNames.empty()) {
            // If no group names remain, delete the row from the grid and GroupGridItems
            urlGrid->DeleteRows(row, 1);
            GroupGridItems.erase(GroupGridItems.begin() + row);
            AssertUrGroupGridItemsSync();
            wxLogMessage("Row %d deleted. Remaining rows: %d", row, urlGrid->GetNumberRows());
        }
        else {
            // Update the grid row with remaining group names
            UpdateUrlGridRow(row, *item);
            wxLogMessage("Updated row %d with remaining group names.", row);
        }
    }
    else {
        std::cout << "\nGroup name '" << groupname << "' not found in precision update." << std::endl;
        wxLogMessage("Group name '%s' not found in row %d.", groupname, row);
    }
}*/
bool MyFrame::precisionUpdateGrid(const wxString& groupname, std::shared_ptr<DataGridItem> item) {
    int rowCount = urlGrid->GetNumberRows();
    int eq_row_count = static_cast<int>(GroupGridItems.size());
    std::cout << "Row counts urlGrid & GroupGridItems:" << rowCount << " " << eq_row_count << std::endl;
    assert(rowCount == eq_row_count && "Row counts differ urlGrid & GroupGridItems:");
    auto it = std::find_if(GroupGridItems.begin(), GroupGridItems.end(), [&](std::shared_ptr<DataGridItem> group_item) {
        return *item == *group_item;
    });

    if (it == GroupGridItems.end()) {
        std::cout << "MyFrame::precisionUpdateGrid iterator not found for item:" << item->uuid.ToStdString() << std::endl;
        wxLogError("Item not found in GroupGridItems.");
        return false;
    }

    int row = std::distance(GroupGridItems.begin(), it);

    // Validate the iterator and row index
    if (row < 0 || row >= eq_row_count) {
        wxLogError("Invalid row index: %d (GroupGridItems size: %d)", row, eq_row_count);
        return false;
    }

    auto it_groupname = std::find((*it)->groupNames.begin(), (*it)->groupNames.end(), groupname);

    if (it_groupname != (*it)->groupNames.end()) {
        (*it)->groupNames.erase(it_groupname);
        if ((*it)->groupNames.empty()) {
            // Delete the row if no group names remain
            urlGrid->DeleteRows(row, 1);
            GroupGridItems.erase(it);  // Erase only if it's valid
            AssertUrGroupGridItemsSync();
            std::cout << "Row: " << row << " deleted in GroupGridItems and UrlGrid remaining rows : " << urlGrid->GetNumberRows() << std::endl;
            wxLogMessage("Row %d deleted. Remaining rows: %d", row, urlGrid->GetNumberRows());
            return true;  // Indicate that the item has been erased
        }
        else {
            UpdateUrlGridRow(row, *item);
            wxLogMessage("Updated row %d with remaining group names.", row);
        }
    }
    else {
        std::cout << "\n groupname:" << groupname << " not found in row:" << row << std::endl;
        wxLogMessage("Group name '%s' not found in row %d.", groupname, row);
    }

    return false;  // Item was not erased
}

void MyFrame::refreshGrid() {
    urlGrid->ForceRefresh();  // Refresh urlGrid to reflect changes
}
void MyFrame::UpdateUrlGridOnGroupDelete(const wxString& groupName) {
   
    int rowCount = urlGrid->GetNumberRows();
    //std::cout << "\nINAPP UpdateUrlGridOnGroupDelete groupname: "<< groupName << " rowcount:"<< rowCount<< std::endl;
    //std::cout << "GroupGridItems.size(): "<< GroupGridItems.size() << std::endl;
    wxLogMessage("UpdateUrlGridOnGroupDelete: %s numrows:%d", groupName, rowCount);
     AssertUrlGridAndGroupNames();
    for (int row = 0; row < rowCount; ++row) {
        auto& dataItem = GroupGridItems[row];  // Get DataGridItem for the row
        //std::cout << "UpdateUrlGridOnGroupDelete: now doing uuid:"<< dataItem->uuid<<" url:"<<dataItem->url  <<" in group:"<< groupName << std::endl;
        wxLogMessage("UpdateUrlGridOnGroupDelete: now doing uuid:%s url:%s in group:%s", dataItem->uuid, dataItem->url , groupName);
        // Check if groupName is in the item's groupNames
        auto it = std::find(dataItem->groupNames.begin(), dataItem->groupNames.end(), groupName);
        if (it != dataItem->groupNames.end()) {
            //std::cout << "\n found groupname "<< groupName<< "performing delete"<< std::endl;
            
            dataItem->groupNames.erase(it);  // Remove the group name from groupNames
            if (dataItem->groupNames.empty()) {
                // If no groups remain, delete the row from urlGrid and GroupGridItems
                urlGrid->DeleteRows(row, 1);
                GroupGridItems.erase(GroupGridItems.begin() + row);

                // Adjust row indexes after deletion
                --row;
                --rowCount;
            }
            else {
              //  std::cout << "\n isn else marker INAPPUpdateurlgrid groupnames not emty size: "<< dataItem->groupNames.size()<< std::endl;
                // If there are other groups, just update the grid (no row deletion needed)
                UpdateUrlGridRow(row, *dataItem);  // You may need to implement this function to refresh the row data
            }
        }else{
            //std::cout << "\n Not found groupname "<< groupName<< " Cannot perform delete\n\ncurrent groupnames size:"<<dataItem->groupNames.size() <<" groupnames are "<< std::endl;
            //std::copy(dataItem->groupNames.begin() , dataItem->groupNames.end() , std::ostream_iterator<wxString>(std::cout << "  "));
            //std::cout<< "\n---------Peek into UrlGrid----------------"<< std::endl;
                 //for (int row = 0; row < urlGrid->GetNumberRows(); ++row){
                    //wxString url = urlGrid->GetCellValue(row, 0);
                  //  std::string all_groups = urlGrid->GetCellValue(row, 2).ToStdString();
                //    std::cout << "row: "<< row << " url: "<< url << " all_groups: "<< all_groups << std::endl;
                 //}
              //  std::cout <<std::endl<<std::endl;
        }
    }
    std::cout << "\nforcing refresh\n"<< std::endl;
    urlGrid->ForceRefresh();  // Refresh urlGrid to reflect changes
    checkGroupsLog("UpdateUrlGridOnGroupDelete");
    checkGroupsAssert("UpdateUrlGridOnGroupDelete");
    
    AssertUrGroupGridItemsSync();
}
// Function to validate the consistency between urlGrid and GroupGridItems


void MyFrame::AssertUrlGridAndGroupNames() {
#ifdef _DEBUG  // Enable assert only in debug mode

    int numRows = urlGrid->GetNumberRows();

    // Assert that the number of rows in the wxGrid matches the size of GroupGridItems
    assert(numRows == static_cast<int>(GroupGridItems.size()) && "Row count mismatch between urlGrid and GroupGridItems");
    if (numRows == 0)return;
    for (int row = 0; row < numRows; ++row) {
        // Get URL from the wxGrid (column 0)
        wxString gridUrl = urlGrid->GetCellValue(row, 0);

        // Get URL from the GroupGridItems
        wxString itemUrl = GroupGridItems.at(row)->url;

        // Assert that URLs match
        assert(gridUrl == itemUrl && "URL mismatch at row");

        // Extract group names from the wxGrid (column 2)
        std::stringstream ss(urlGrid->GetCellValue(row, 2).ToStdString());
        std::string group;
        std::unordered_multiset<wxString> gridGroupNames;

        while (std::getline(ss, group, ',')) {
            gridGroupNames.insert(wxString(trim(group)));  // Convert and insert trimmed group names as wxString
        }

        // Get group names from the GroupGridItems (already wxString)
        std::unordered_multiset<wxString> itemGroupNames(GroupGridItems.at(row)->groupNames.begin(), 
                                                         GroupGridItems.at(row)->groupNames.end());
        std::cout << "\nINAPP DEBUG ASSERT OUTPUT\n";
        std::cout << "Grid URL: "<< gridUrl << "\n";
        std::cout << "Item URL: "<< itemUrl << "\n";
        std::cout<< "Grid Group Names: " << std::endl;
        std::copy(gridGroupNames.begin() , gridGroupNames.end() , std::ostream_iterator<wxString>(std::cout, " "));
        std::cout<< "Item Group Names: " << std::endl;
        std::copy(itemGroupNames.begin() , itemGroupNames.end() , std::ostream_iterator<wxString>(std::cout, " "));
        // Debug output
        wxLogMessage("Grid URL: %s", gridUrl);
        wxLogMessage("Item URL: %s", itemUrl);
        wxLogMessage("prebind Group Names: %s", ss.str());

        wxLogMessage("Grid Group Names: ");
        for (const auto& name : gridGroupNames) wxLogMessage("%s ", name);

        wxLogMessage("Item Group Names: ");
        for (const auto& name : itemGroupNames) wxLogMessage("%s ", name);
        
        // Assert that group names match as unordered multisets
        assert(gridGroupNames == itemGroupNames && "Group name mismatch at row");
    }

#endif  // _DEBUG
}



void MyFrame::RemoveGroupFromNotebook(const wxString& groupName) {
    wxLogMessage("MyFrame::RemoveGroupFromNotebook: " + groupName);

    // Find the index of the page with the given group name
    int pageIndex = -1;
    for (size_t i = 0; i < groupNotebook->GetPageCount(); ++i) {
        if (groupNotebook->GetPageText(i) == groupName) {
            pageIndex = i;
            break;
        }
    }

    // Check if the group exists
    if (pageIndex == -1) {
        wxLogWarning("Group '%s' not found in notebook.", groupName);
        return;
    }

    // Remove the panel from the notebook
    groupNotebook->RemovePage(pageIndex);

    // Remove the group entry from groupMap
    auto it = groupMap.find(groupName);
    if (it != groupMap.end()) {
        groupMap.erase(it);
    }
    else {
        wxLogWarning("Group '%s' not found in groupMap.", groupName);
    }

    // Remove the group from groupNames (assuming it's a vector or similar structure)
    auto nameIt = std::find(groupNames.begin(), groupNames.end(), groupName);
    if (nameIt != groupNames.end()) {
        groupNames.erase(nameIt);
    }
    else {
        wxLogWarning("Group '%s' not found in groupNames.", groupName);
    }

    // Update the choice control or any other UI components as needed
    UpdateChoiceControl();
    groupNotebook->Layout();

    // Assert that notebook, groupMap, and groupNames are consistent in size
    wxASSERT(groupNotebook->GetPageCount() == groupMap.size() && groupMap.size() == groupNames.size());

    if (!(groupNotebook->GetPageCount() == groupMap.size() && groupMap.size() == groupNames.size())) {
        wxLogError("Inconsistent sizes after removal: Notebook pages: %d, groupMap entries: %d, groupNames size: %d",
            groupNotebook->GetPageCount(), groupMap.size(), groupNames.size());
    }
}
void MyFrame::AddGroupToNotebook(const wxString& groupName , GroupData data) {
    wxLogMessage("MyFrame::AddGroupToNotebook: " + groupName);
    // Create a new panel for the group
    wxPanel* groupPanel = new wxPanel(groupNotebook, wxID_ANY, wxDefaultPosition, wxSize(500, 700));
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);  // Use vertical to stack components
    // Add a label for the group
    wxStaticText* label = new wxStaticText(groupPanel, wxID_ANY, "This is group: " + groupName);
    panelSizer->Add(label, 0, wxALL, 5); 
    if (data.is_subscribed) {
        wxString subscription_period;
        if (data.period_days != 0 || data.period_months != 0) {
            subscription_period = wxString(std::string("Every ") + std::to_string(data.period_months) + " months and " + std::to_string(data.period_days) + " days");
        }
        else {
            subscription_period = wxString("Scheduled for " + data.schedule_date);
        }
        wxStaticText* label = new wxStaticText(groupPanel, wxID_ANY, "Supscriptions turned on : " + subscription_period);
        panelSizer->Add(label, 0, wxALL, 5);
    }

    // Add a delete button for this group
    wxButton* deleteGroupButton = new wxButton(groupPanel, wxID_ANY, "Delete Group", wxDefaultPosition, wxSize(100, 20));
    panelSizer->Add(deleteGroupButton, 0, wxALL | wxALIGN_RIGHT, 5);
    deleteGroupButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnDeleteGroup, this);
    deleteGroupButton->Show(true);
   // panelSizer->Layout();
    // Initialize or retrieve the GroupInfo and grid
    std::shared_ptr<GroupInfo> groupInfo;
    if (groupMap.find(groupName) == groupMap.end()) {
        groupInfo = std::make_shared<GroupInfo>(this, panelSizer, groupName);
        groupInfo->setGroupPanel(groupPanel);
        groupInfo->setId(data.id);
        groupInfo->setScheduledMessage(data);
        groupMap[groupName] = groupInfo;
    }
    else {
        groupInfo = groupMap[groupName];
    }

    // Create and initialize grid, add it to the panelSizer
    groupInfo->InitializeGrid(groupPanel);
    wxGrid* groupGrid = groupInfo->GetGrid();
    //wxTextCtrl* schedule_message = groupInfo->getScheduledMessage();
    //panelSizer->Add(schedule_message, 0, wxALL, 5);    
    //groupGrid->AutoSize();
    //groupGrid->Refresh();
    panelSizer->Add(groupGrid, 1, wxEXPAND | wxALL, 5);  // Add with EXPAND to fill panel
    // SetSizerAndFit(panelSizer);
    groupPanel->SetSizerAndFit(panelSizer);
    
    //// panelSizer->RecalcSizes();

    //groupPanel->SetSizer(panelSizer);
   
    groupNotebook->AddPage(groupPanel, groupName);
    //groupPanel->Layout(); // Ensure layout is updated
    
    //groupPanel->Refresh(); // Refresh the panel to update its appearance
    //groupPanel->Update();
    //groupPanel->
    UpdateChoiceControl();
    groupNotebook->Layout();
    panelSizer->Layout();
    deleteGroupButton->Refresh();
    deleteGroupButton->Update(); // Force update to ensure it draws immediately

    checkGroupsLog("AddGroupToNotebook");


}
/*void MyFrame::AddGroupToNotebook(const wxString& groupName) {
    wxLogMessage("MyFrame::AddGroupToNotebook: " + groupName);    
    // Create a new panel for the group
    wxPanel* groupPanel = new wxPanel(groupNotebook, wxID_ANY , wxDefaultPosition, wxSize(500,700));
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL );  // Use vertical to stack components
    // Add a label for the group
    wxStaticText* label = new wxStaticText(groupPanel, wxID_ANY, "This is group: " + groupName);
    panelSizer->Add(label, 0, wxALL, 5);
    // Add a delete button for this group
    wxButton* deleteGroupButton = new wxButton(groupPanel, wxID_ANY, "Delete Group", wxDefaultPosition,wxSize(100,20));
    panelSizer->Add(deleteGroupButton, 0, wxALL | wxALIGN_RIGHT, 5);
    deleteGroupButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnDeleteGroup, this);
    deleteGroupButton->Show(true);
    panelSizer->Layout();
    // Initialize or retrieve the GroupInfo and grid
    std::shared_ptr<GroupInfo> groupInfo;
    if (groupMap.find(groupName) == groupMap.end()) {
        groupInfo = std::make_shared<GroupInfo>(this, panelSizer , groupName);
        groupMap[groupName] = groupInfo;
    }
    else {
        groupInfo = groupMap[groupName];
    }

    // Create and initialize grid, add it to the panelSizer
    groupInfo->InitializeGrid(groupPanel);
    auto groupGrid = groupInfo->GetGrid();
    //groupGrid->AutoSize();
    //groupGrid->Refresh();
    panelSizer->Add(groupGrid, 1, wxEXPAND | wxALL, 5);  // Add with EXPAND to fill panel
   // SetSizerAndFit(panelSizer);
    groupPanel->SetSizerAndFit(panelSizer);
    panelSizer->Layout();
   //// panelSizer->RecalcSizes();
    
   //groupPanel->SetSizer(panelSizer);
    panelSizer->Layout();
    groupNotebook->AddPage(groupPanel, groupName);
    groupPanel->Layout(); // Ensure layout is updated
    groupPanel->Refresh(); // Refresh the panel to update its appearance
    groupPanel->Update();
    //groupPanel->
    UpdateChoiceControl();
    groupNotebook->Layout();
    deleteGroupButton->Refresh();
    deleteGroupButton->Update(); // Force update to ensure it draws immediately

    checkGroupsLog("AddGroupToNotebook");

    
}*/
void MyFrame::checkGroupsAssert(const std::string& from) {
    if (debugMode) wxLogMessage("checkGroupsAssert from:%s", from);
    // Assert that the notebook pages, groupMap, and groupNames are consistent in size
    //if(unittesting_flag) std::cout << "groupNotebook->GetPageCount():"<< groupNotebook->GetPageCount() << " groupMap.size():" <<groupMap.size()<< " groupNames.size()" << groupNames.size() << std::endl;
    wxASSERT(groupNotebook->GetPageCount() == groupMap.size() && groupMap.size() == groupNames.size());
    // Optionally, log an error if the assertion fails (for release builds)//remove true in production
    if (!(groupNotebook->GetPageCount() == groupMap.size() && groupMap.size() == groupNames.size())) {
        wxLogError("Inconsistent sizes: Notebook pages: %zu, groupMap entries: %zu, groupNames size: %zu log:%s",
            groupNotebook->GetPageCount(), groupMap.size(), groupNames.size(), from.c_str());
    }
}
void MyFrame::checkGroupsLog(const std::string& from) {
    if (debugMode) wxLogMessage("checkGroupsLog from:%s", from);
    // Assert that the notebook pages, groupMap, and groupNames are consistent in size
   //wxASSERT(groupNotebook->GetPageCount() == groupMap.size() && groupMap.size() == groupNames.size());
    // Optionally, log an error if the assertion fails (for release builds)
    if (debugMode) {
        wxLogMessage("checkGroupsLog pagecount: %zu, groupMapSize: %zu, groupNames: %zu, from: %s",
            groupNotebook->GetPageCount(),
            groupMap.size(),    // Cast size_t to int if necessary
            groupNames.size(),  // Cast size_t to int if necessary
            from.c_str());  // Convert wxString to const char*
    }
   
}

void MyFrame::setUpGroupItems(std::shared_ptr<DataGridItem> context_item) {
    //adds a context_item to a group in Notebook page
    if (!context_item) {
        if (debugMode) wxLogMessage("setUpGroupItems: context_item is null, skipping insertion.");
        return;
    }
    if (debugMode) wxLogMessage("setUpGroupItems: context_item: uuid:%s, url: %s", context_item->uuid , context_item->url);
    for (const auto& groupName : context_item->groupNames) {
        if (debugMode) wxLogMessage("setUpGroupItems: Processing group '%s' groupMap size:%zu", groupName, groupMap.size());
        // Ensure groupName exists in the map; if not, raise an exception
        auto it = groupMap.find(groupName);
        if (it == groupMap.end()) {
            wxLogError("setUpGroupItems: Group '%s' not found in groupMap. Ensure it is added via AddGroupToNotebook.", groupName);
            throw std::runtime_error("Group '" + std::string(groupName.mb_str()) + "' not found in groupMap.");
        }
        // Retrieve the existing GroupInfo if found
        std::shared_ptr<GroupInfo> groupInfo = it->second;
        if (debugMode) wxLogMessage("setUpGroupItems: Adding item to group '%s'", groupName);
        // Add the context item to the group
        groupInfo->AddItem(context_item);
        if (debugMode) wxLogMessage("setUpGroupItems: End Process group '%s' groupMap size:%zu groupNames:%zu  GroupGridItems:%zu UrlGridItems:%zu", groupName, groupMap.size(), groupNames.size(), GroupGridItems.size() , UrlGridItems.size());
    }
    checkGroupsAssert("setUpGroupItems");
    checkGroupsLog("setUpGroupItems");
}




/*void MyFrame::displayGroupItems(std::shared_ptr<DataGridItem> items) {
    //used to display context_item in the group of Notebook page on init
    if (debugMode) wxLogMessage("displayGroupItems: Processing item uuid '%s' url:%s", items->uuid , items->url);
    GroupGridItems.push_back(items);
    int row = urlGrid->GetNumberRows();
    urlGrid->AppendRows(1);
    row = urlGrid->GetNumberRows() - 1; // Set row to the last added row

    // Concatenate group names into a comma-separated string
    wxString group_names = items->groupNames.empty()
        ? wxString()
        : std::accumulate(std::next(items->groupNames.begin()), items->groupNames.end(), items->groupNames[0],
            [](const wxString& a, const wxString& b) { return a + ", " + b; });

    urlGrid->SetCellValue(row, 0, items->url);
    urlGrid->SetCellValue(row, 2, group_names); // Assuming column 2 is for group names
    urlGrid->SetCellRenderer(row, 3, new ButtonRenderer());
    urlGrid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDeleteGroup, this);
    urlGrid->AutoSize();
    urlGrid->ForceRefresh();
    checkGroupsAssert("setUpGroupItems");
    AssertUrGroupGridItemsSync();
}*/
void MyFrame::displayGroupItems(std::shared_ptr<DataGridItem> items) {
    if (debugMode) wxLogMessage("displayGroupItems: Processing item uuid '%s' url: %s", items->uuid, items->url);

    // Check if the item already exists in GroupGridItems
    auto it = std::find_if(GroupGridItems.begin(), GroupGridItems.end(),
        [&items](const std::shared_ptr<DataGridItem>& existingItem) {
        return existingItem->uuid == items->uuid;
    });

    int row;
    if (it != GroupGridItems.end()) {
        // Item already exists, get its index
        row = std::distance(GroupGridItems.begin(), it);
        if (debugMode) wxLogMessage("Item already exists at row %d, updating...", row);
    }
    else {
        // Item does not exist, add it to GroupGridItems
        GroupGridItems.push_back(items);

        // Append a new row to the grid
        urlGrid->AppendRows(1);
        row = urlGrid->GetNumberRows() - 1; // Set row to the last added row
        if (debugMode) wxLogMessage("Item added to GroupGridItems and new row created at index %d", row);
    }

    // Concatenate group names into a comma-separated string
    wxString group_names = items->groupNames.empty()
        ? wxString()
        : std::accumulate(std::next(items->groupNames.begin()), items->groupNames.end(), items->groupNames[0],
            [](const wxString& a, const wxString& b) { return a + ", " + b; });

    // Update the grid row with the item's information
    urlGrid->SetCellValue(row, 0, items->url); // Assuming column 0 is for URL
    urlGrid->SetCellValue(row, 1, items->uuid); // Assuming column 1 is for uuid
    urlGrid->SetCellValue(row, 2, group_names); // Assuming column 2 is for group names
    urlGrid->SetCellRenderer(row, 3, new ButtonRenderer());
    urlGrid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDeleteGroup, this);
    updateDbRelations(items);
    // Resize and refresh the grid
    urlGrid->AutoSize();
    urlGrid->ForceRefresh();

    checkGroupsAssert("setUpGroupItems");
    AssertUrGroupGridItemsSync();
}
void MyFrame::updateDbRelations(std::shared_ptr<DataGridItem> item) {
    int parent_id = getDb()->GetItemIdByUUID(item->uuid);
    if (debugMode) wxLogMessage("MyFrame::updateDbRelations got parent_id:%d", parent_id);
    if (parent_id == -1) {
        if (debugMode) wxLogMessage("MyFrame::updateDbRelations No Item in db :%d", parent_id);
        return;
    }
    getDb()->InsertGroupRelationship(parent_id, item->groupNames);

}



void MyFrame::OnLeftClickDeleteGroup(wxGridEvent& event) {
    /*Called 
    */
    if (m_clkDeb1) return;
    m_clkDeb1 = true;

    int row = event.GetRow();
    int col = event.GetCol();

    // Check if the clicked cell is in the delete button column
    if (col == 3 && row >= 0 && row < urlGrid->GetNumberRows()) {
        if (debugMode) wxLogMessage("Delete Group button clicked in row: %d", row);

        // Retrieve the corresponding DataGridItem from GroupGridItems
        std::shared_ptr<DataGridItem> item = GroupGridItems[row];
        if (item) {
            std::vector<wxString> groupNames = item->groupNames;
            // Iterate over all group names associated with this item
            for (const wxString& groupName : groupNames) {
                auto it = groupMap.find(groupName);
                if (it != groupMap.end()) {
                    auto groupInfo = it->second;
                    if (debugMode) wxLogMessage("Checking groupinfo:");
                    if (groupInfo) {
                        if (debugMode) wxLogMessage("In  groupinfo removing Item:");
                        // Remove the item from the GroupInfo object
                        //groupInfo->RemoveItem(item);
                        groupInfo->RemoveItem(item);
                        if (debugMode) wxLogMessage("removing Item success");
                        // If the GroupInfo is now empty, clean it up and remove from the groupMap
                        if (groupInfo->GetGrid() && groupInfo->isEmpty()) {
                            if (groupInfo->getPanelSizer()) {
                                if (debugMode) wxLogMessage("resetting panelsizer");
                                groupInfo->getPanelSizer()->Detach(groupInfo->GetGrid());
                                groupInfo->GetGrid()->Show(false); // Hide grid if no items remain
                            }
                            if (debugMode) wxLogMessage("groupmap erase");
                            //NOT SUPPOSED TO TOUCH THIS //groupMap.erase(it);//only with remove groups // Remove GroupInfo from groupMap
                            if (debugMode) wxLogMessage("Group '%s' removed from groupMap.", groupName);
                        }
                    }
                }
            }
            if (debugMode) wxLogMessage("resizing the grid deleting the row");
            // Remove the row from the urlGrid
            //urlGrid->DeleteRows(row, 1); // Delete one row at the specified index
            if (debugMode) wxLogMessage("delete rows success sizining next");
            urlGrid->AutoSize();
            urlGrid->ForceRefresh();
            if (debugMode) wxLogMessage("GroupGridItems erase upcoming");
            // Remove the item from GroupGridItems
            //GroupGridItems.erase(GroupGridItems.begin() + row);
        }
        if (debugMode) wxLogMessage("all through checkGroupsAssert next");
        checkGroupsAssert("OnLeftClickDeleteGroup");
        if (debugMode) wxLogMessage("checkGroupsLog next");
        checkGroupsLog("OnLeftClickDeleteGroup");
        if (debugMode) wxLogMessage("final assert");
        AssertUrGroupGridItemsSync();
        if (debugMode) wxLogMessage("fini");
    }

    // Reset debounce flag after event handling
    CallAfter([this]() {
        m_clkDeb1 = false;
        if (debugMode) wxLogMessage("Click status reset.");
    });

    event.Skip(); // Continue event propagation if needed
}








void MyFrame::OnLoadData(wxCommandEvent& event) {
    if (debugMode) wxLogMessage("OnLoadData triggered.");
    performLoadData();
}
void MyFrame::initLoadData() {
    // Clear and prepare the wxGrid for new data
    dataGridView->ClearGrid();
    dataGridView->AppendRows(0);  // Clear any existing rows
    UrlGridItems.clear();  // Clear UrlGridItems to reload
    performLoadData();
}
void MyFrame::performLoadData() {
    if (debugMode) wxLogMessage("performLoadData triggered.");

    // Retrieve data from the database (loadDataGrid now returns a shared_ptr to a vector)
    std::shared_ptr<std::vector<DataGrid>> datagrid_initview = getDb()->loadDataGrid();

    // Initialize an unordered_set to keep track of UUIDs already present in UrlGridItems
    std::unordered_set<wxString> seenUuids;

    // Populate seenUuids with UUIDs already present in UrlGridItems 
    /*std::for_each(UrlGridItems.begin(), UrlGridItems.end(), [&](const auto& existingItem) {
        seenUuids.insert(existingItem->uuid);
    });*/
    
    std::transform(UrlGridItems.begin(), UrlGridItems.end(), std::inserter(seenUuids, seenUuids.end()), [](const auto& item) {
        return item->uuid;
    });


    int row = dataGridView->GetNumberRows();

    // Iterate over each DataGrid item and populate the wxGrid
    for (const auto& item : *datagrid_initview) {
        // Skip items that already exist in UrlGridItems based on UUID
        if (seenUuids.find(wxString::FromUTF8(item.uuid)) != seenUuids.end()) {
            continue;  // Skip this item if it's already in the grid
        }

        // Add the item to UrlGridItems and the wxGrid
        dataGridView->AppendRows(1);  // Add a new row

        // Create a new DataGridItem and set its properties
        auto gridItem = std::make_shared<DataGridItem>();        
        gridItem->serial = wxString::Format("%d", row + 1); // Serial number is next available value
        gridItem->id = item.id; 
        gridItem->uuid = wxString::FromUTF8(item.uuid);
        gridItem->url = wxString::FromUTF8(item.url);
        gridItem->type = wxString::FromUTF8(item.type.value_or(""));  // Ensure fallback for missing type
        gridItem->status = wxString::FromUTF8(item.status.value_or("")); // Ensure fallback for missing status
        gridItem->date = wxString::FromUTF8(item.date.value_or(""));     // Ensure fallback for missing date

        // Add the item to UrlGridItems
        UrlGridItems.push_back(gridItem);

        // Fill the grid with the values from the DataGrid item
        dataGridView->SetCellValue(row, 0, gridItem->serial);  // Serial
        dataGridView->SetCellValue(row, 1, gridItem->uuid);   // UUID
        dataGridView->SetCellValue(row, 2, gridItem->url);    // URL
        dataGridView->SetCellValue(row, 3, gridItem->type);   // Type
        dataGridView->SetCellValue(row, 4, gridItem->status); // Status
        dataGridView->SetCellValue(row, 5, gridItem->date);   // Date

        // Render the grid with necessary styling
        dataGridView->SetRowSize(row, 1 * dataGridView->GetDefaultRowSize());
        dataGridView->SetCellRenderer(row, 2, new wxGridCellAutoWrapStringRenderer);
        dataGridView->SetCellRenderer(row, 3, new wxGridCellAutoWrapStringRenderer);
        dataGridView->SetCellRenderer(row, 4, new wxGridCellAutoWrapStringRenderer);

        // Set the buttons for delete and add-to-group functionality
        dataGridView->SetCellRenderer(row, 6, new ButtonRenderer());
        dataGridView->SetColSize(6, 100);  // Adjust the column size for delete button
        dataGridView->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDelete, this);

        int cols = dataGridView->GetNumberCols();
        dataGridView->SetCellRenderer(row, 7, new ButtonCellRenderer());
        dataGridView->SetColSize(7, 100);  // Adjust the column size for add-to-group button
        dataGridView->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnGridCellClickAddToGroup, this);

        // Make the grid cells read-only
        for (int x = 0; x < cols; x++) {
            dataGridView->SetReadOnly(row, x, true);
        }

        row++; // Move to the next row
    }

    // Force refresh of the grid after populating the data
    dataGridView->ForceRefresh();

    // Optional: Finalize the statement if needed (not applicable since no raw SQL is used anymore)
    // getDb()->FinalizeStatement(stmt);

    AssertUrlGridItemsSync();  // Optional: Sync URL Grid Items if needed

    if (debugMode) wxLogMessage("Data loaded successfully.");
}




void MyFrame::OnTimer(wxTimerEvent& event)
{
    singlegridflag = false; // Reset the flag when the timer expires
    if (debugMode) wxLogMessage("OnTimer: singlegridflag reset to false.");
}
void MyFrame::OnGridCellClickAddToGroup(wxGridEvent& event)
{
    
    if (singlegridflag) {
        if (debugMode) wxLogMessage("OnGridCellClickAddToGroup: Click ignored (flag is true).");
        return; // Prevent further processing if the flag is set
    }

    int row = event.GetRow();
    int col = event.GetCol();

    if (debugMode) wxLogMessage("OnGridCellClickAddToGroup triggered. row:%d column:%d", row, col);

    // Check if the clicked cell is the one with the button
    if (col == 7) // Assuming 'Group' column
    {
        if (debugMode) wxLogMessage("Opening GroupManagementDialog from row:%d column:%d", row, col);

        if(!unittesting_flag)singlegridflag = true; // Set flag to prevent further clicks //upsets unitetss
        // Set context_row to point to the corresponding item in UrlGridItems
        
        if (row < UrlGridItems.size()) {
            //context_row = UrlGridItems[row];
            setContextItem(UrlGridItems[row]);
            
        }
        else {
            if (debugMode) wxLogMessage("Row %d is out of bounds for UrlGridItems.", row);
            return;
        }
        
        // Open the GroupManagementDialog
        GroupManagementDialog* groupDialog = new GroupManagementDialog(this, groupNames);
        
        if(!unittesting_flag)groupDialog->ShowModal();
        
        // After the dialog is closed, destroy it
        groupDialog->Destroy();
        
        groupDialog = nullptr;
        // Reset the flag after dialog is closed
        //singlegridflag = false;
        // Start the timer to reset the flag after 320 ms
          wxTimer*  delayTimer = new wxTimer(this);
        delayTimer->Start(320, wxTIMER_ONE_SHOT); // One-shot timer
        
    }
    if (debugMode) wxLogMessage("Wre out of here from row:%d column:%d", row, col);
     event.Skip(); // Skip to allow default handling if necessary
   // checkGroupsAssert("OnGridCellClickAddToGroup");
    checkGroupsLog("OnGridCellClickAddToGroup");
}



void MyFrame::OnConfirmData(wxCommandEvent& event) {
    if (debugMode) wxLogMessage("OnConfirmData triggered.");

    InsertDataGridItemsIntoDB();
    remote_server->processSend();

    if (debugMode) wxLogMessage("Data confirmed.");
}



void MyFrame::OnSocketEvent(wxSocketEvent& event)
{
    wxSocketBase* sock = event.GetSocket();
    wxSocketBase* socket = sock;
    // Process the event
    switch (event.GetSocketEvent())
    {
    case wxSOCKET_INPUT:
    {
        char buf[10];
        // Read the data
        sock->Read(buf, sizeof(buf));
        // Write it back
        sock->Write(buf, sizeof(buf));
        // We are done with the socket, destroy it
        sock->Destroy();
        break;
    }
    case wxSOCKET_LOST:
    {
        sock->Destroy();
        break;
    }
    }
    char buffer[4096];
    wxString message;
    size_t bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
    if (bytesRead > 0) {
        message.append(buffer, bytesRead);
        if (debugMode) wxLogMessage("Received message: %s", message);
        // Call HandleHttpRequest to process the HTTP request
        HandleHttpRequest(socket, message);
    }
}

MyFrame::StreamExitCondition MyFrame::HandleStreamingRequest(wxSocketBase* socket, const wxString& request) {
    wxString response;
    wxString body;  // Store the response body separately
    std::string uuid_str;
    bool success_ok = true;
    std::shared_ptr<URLComponents> urlptr = nullptr;

    if (debugMode) wxLogMessage("Processing streaming request.");

    if (request.StartsWith("POST stream_data")) {
        if (debugMode) wxLogMessage("Entered streaming data");

        // Extract UUID from the request
        bool is_success = ExtractUUIDFromRequest(request, uuid_str);
        if (!is_success) {
            if (debugMode) wxLogMessage("Error: stream has no UUID");
            return StreamExitCondition::MissingUUID;
        }

        if (debugMode) wxLogMessage("Got UUID: %s", uuid_str);
        urlptr = url_manager.GetURLComponentById(uuid_str);
        if (!urlptr) {
            if (debugMode) wxLogMessage("URL pointer not found for UUID: %s", uuid_str);
            return StreamExitCondition::UrlPtrNotFound;
        }

        if (debugMode) wxLogMessage("Starting trim");
        TrimStreamingPostRequest(const_cast<wxString&>(request));

        if (debugMode) wxLogMessage("Starting process");
        ProcessStreamedData(urlptr, request);  // Process content using the URLComponents

        body = "{\"ack\": \"STREAM_DATA_RECEIVED_ACK\"}"; // Acknowledge receipt
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);
    }
    else {
        if (request.Length() > 160) {
            wxString truncatedRequest = request.substr(0, 160);
            wxLogMessage("Else First 160 characters: %s", truncatedRequest);
        }
        else {
            wxLogMessage("Else Full request (less than 160 characters): %s", request);
        }

        response = "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n\r\n";
        success_ok = false;
    }

    // Write the response to the socket
    socket->Write(response.c_str(), response.length());
    if (success_ok && urlptr) {
        urlptr->setFileStream();
        //urlComponents.setFileStream();
    }
    //
    return success_ok ? StreamExitCondition::Success : StreamExitCondition::InvalidRequest;
}

void MyFrame::HandleHttpRequest(wxSocketBase* socket, const wxString& request) {
    wxString response;
    wxString body;  // Store the response body separately

    if (debugMode) wxLogMessage("Processing HTTP request.");

    if (request.StartsWith("OPTIONS")) {
        if (debugMode) wxLogMessage("Handling OPTIONS (CORS Preflight)");

        response = "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n\r\n";
    }
    else if (request.StartsWith("POST /ext_pulse")) {
        if (debugMode) wxLogMessage("Handling ext_pulse");

        body = "{\"ack\": \"APP_ACK\"}\r\n";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);

        if (debugMode) wxLogMessage("Sent APP_ACK to Chrome Extension. Length: %zu", body.length());
    }
    else if (request.StartsWith("POST /app_pulse")) {
        if (debugMode) wxLogMessage("Handling /app_pulse");

        body = "{\"ack\": \"EXT_ACK\"}\r\n";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);

        if (debugMode) wxLogMessage("Sent EXT_ACK to Chrome Extension. Length: %zu", body.length());
    }
    else if (request.StartsWith("GET")) {
        body = "{\"ack\": \"GET_APP_DATA_ACK\"}";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);
    }
    else if (request.StartsWith("POST /ext_data")) {
        wxString bodyContent = ExtractRequestBody(request);
        ParseAndQueueData(bodyContent);

        body = "{\"ack\": \"APP_DATA_ACK\"}";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);
    }
    else if (request.StartsWith("POST /register_url")) {
        wxString bodyContent = ExtractRequestBody(request);
        ParseAndQueueUrl(bodyContent);

        body = "{\"ack\": \"URL_REGISTERED_ACK\"}";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);

        // Update the grid after processing the URL
        //wxCallAfter(&MyFrame::UpdateGridFromQueue, this);
        CallAfter(&MyFrame::UpdateGridFromQueue);
   

    }
    else if (request.StartsWith("POST /stream_url")) {
        wxString bodyContent = ExtractRequestBody(request);
        std::shared_ptr<URLComponents> urlptr = ParseAndQueueUrlforStreaming(bodyContent);
        std::string uuid_s = urlptr->getUUID();
        url_manager.AddURLComponent(uuid_s, urlptr);
        //body = "{\"ack\": \"STREAM_READY_TOSTART_ACK\" , \"uuid\":+uiid_s+ }";    
        body = "{\"ack\": \"STREAM_READY_TOSTART_ACK\", \"uuid\": \"" + uuid_s + "\"}";
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);
        CallAfter(&MyFrame::UpdateGridFromQueue);
        
    }
    else if (request.StartsWith("POST stream_data")) {
        //POST stream_data/c36a294c-96d9-4aef-8113-e7e28e640bba/
        if (debugMode) wxLogMessage("Entered streaming data");
        //wxString bodyContent = ExtractRequestBody(request); // Extract body content
        std::string uuid_str;
        bool is_success = ExtractUUIDFromRequest(request , uuid_str); // Implement this method to extract UUID

        if (!is_success) {
            if (debugMode) wxLogMessage("Error stream has no uuid");
            return;
        }
        if (debugMode) wxLogMessage("Got uuid :%s" , uuid_str);
        //std::unique_ptr urlptr = url_manager.GetURLComponentById(uuid_str);        //
        // // Change here from unique_ptr to shared_ptr

        std::shared_ptr<URLComponents> urlptr = url_manager.GetURLComponentById(uuid_str);

        //auto urlComponents = urlMap[uuid]; // Retrieve URLComponents using UUID
         // Call method to process streamed data
        if (urlptr) {
            if (debugMode) wxLogMessage("starting trim ");
            TrimStreamingPostRequest(const_cast<wxString&>(request));
            //wxString bodyContent = ExtractRequestBody(request);            
            //ProcessStreamedData(std::move(urlptr), request);// Process content using the URLComponents
            if (debugMode) wxLogMessage("starting process");
            ProcessStreamedData(urlptr, request);// Process content using the URLComponents
        
        body = "{\"ack\": \"STREAM_DATA_RECEIVED_ACK\"}"; // Acknowledge receipt
        response = wxString::Format("HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            body.length(), body);
        }
        else {
            // Handle error: UUID not found
            if (debugMode) wxLogMessage("url ptr not found %s " , uuid_str);
        }
    }
    else {

        if (request.Length() > 160) {
            wxString truncatedRequest = request.substr(0, 160);
            wxLogMessage("Else First 160 characters: %s", truncatedRequest);
        }
        else {
            wxLogMessage("Else Full request (less than 160 characters): %s", request);
        }
        response = "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n\r\n";
    }

    // Write the response to the socket
    socket->Write(response.c_str(), response.length());
}

bool MyFrame::ExtractUUIDFromRequest(const wxString& request, std::string& uuid) {
    const std::string prefix = "POST stream_data/";
    std::string reqStr = request.ToStdString();

    // Initialize the UUID as an empty string
    uuid.clear();
    if (debugMode) wxLogMessage("ExtractUUIDFromRequest1 ");

    // Check if the request starts with the expected prefix
    if (reqStr.find(prefix) == 0) {
        if (debugMode) wxLogMessage("ExtractUUIDFromRequest2 ");

        // Extract the UUID, which follows the prefix
        std::size_t startPos = prefix.length();
        std::size_t endPos = reqStr.find('/', startPos); // Start search after the prefix

        if (endPos != std::string::npos) {
            // Extract the UUID until the next '/' or end of string
            uuid = reqStr.substr(startPos, endPos - startPos); // Fixed index for UUID extraction
            if (debugMode) wxLogMessage("ExtractUUIDFromRequest3 %s start: %zu end: %zu ", uuid.c_str(), startPos, endPos);
            return true; // Successfully extracted the UUID
        }

        if (debugMode) wxLogMessage("ExtractUUIDFromRequest4 ");
    }

    if (debugMode) wxLogMessage("ExtractUUIDFromRequest5 ");
    // If no valid UUID is found, return false
    return false;
}


void MyFrame::ProcessStreamedData(std::shared_ptr<URLComponents> urlComponentPtr , const wxString& bodyContent) {
    wxLogMessage("starting ProcessStreamedData");
    try {        
        std::ofstream tempFile(urlComponentPtr->getFilePath());
        if (!tempFile.is_open()) { 
            wxLogError("Failed to open the file for writing: %s", urlComponentPtr->getFilePath().string());
            return;  // Early exit if file couldn't be opened
        }
        if (!bodyContent.IsEmpty()) {
            std::string content = bodyContent.ToStdString();  // Convert wxString to std::string
            tempFile << content;  // Save content to the temporary file            
        }        
        tempFile.close();
        urlComponentPtr->setFileStream();
        // Open the temporary file as an input stream for further processing        
       
    }
    catch (const std::exception& e) {
        wxLogError("Error processing streamed data: %s", e.what());
    }
}



void MyFrame::TrimStreamingPostRequest(wxString& request) {
    // Find the first newline, which marks the end of the POST header
    size_t newlinePos = request.find_first_of('\n');
    if (debugMode) wxLogMessage("TrimStreamingPostRequest newline pos:%zu" , newlinePos);
    if (newlinePos != wxString::npos) {
        // Erase everything up to and including the first newline
        request.erase(0, newlinePos + 1);
        if (debugMode) wxLogMessage("TrimStreamingPostRequest erased:%s", request.substr(0,150));
    }
}





wxString MyFrame::ExtractRequestBody(const wxString& request) {
    if (debugMode) wxLogMessage("Extracting Request Body  process.");
    wxString body;
    size_t bodyStart = request.Find("\r\n\r\n");
    if (bodyStart != wxNOT_FOUND) {
        bodyStart += 4; // Skip the \r\n\r\n
        body = request.Mid(bodyStart);
    }
    return body;
}




void MyFrame::UpdateGridFromQueue() {
    if (debugMode) wxLogMessage("Updating grid from queue.");
    // Ensure thread-safety by locking or using a mutex if applicable
    // Begin batch update
    dataGridView->BeginBatch();
    // Clear any existing selection
    dataGridView->ClearSelection();
    // Disable cell editing if it's currently enabled
    if (dataGridView->IsCellEditControlEnabled()) {
        dataGridView->DisableCellEditControl();
    }
    // Check if there are items in the queue
    if (dataQueue.empty()) {
        if (debugMode) wxLogMessage("Data queue is empty.");
        dataGridView->EndBatch();
        return;
    }
    // Iterate over items in the queue and update the grid
    while (!dataQueue.empty()) {
        DataGridItem item = dataQueue.front();
        dataQueue.pop();
        //UrlGridItems.push_back(item);
        std::shared_ptr<DataGridItem> item_ptr = std::make_shared<DataGridItem>(item);
        UrlGridItems.push_back(item_ptr);
        int row = dataGridView->GetNumberRows();        
        dataGridView->AppendRows(1);
        // Set cell values for the new row
        dataGridView->SetCellValue(row, 0, item.serial);
        dataGridView->SetCellValue(row, 1, item.uuid);
        dataGridView->SetColSize(3, 100);
        dataGridView->SetRowSize(row, 4 * dataGridView->GetDefaultRowSize());
        //dataGridView->SetCellRenderer(row, 2, new wxGridCellAutoWrapStringRenderer);
        dataGridView->SetCellValue(row, 2, item.url);
        dataGridView->SetCellRenderer(row, 2, new wxGridCellAutoWrapStringRenderer);
        //dataGridView->SetCellEditor(row, 2, new wxGridCellAutoWrapStringEditor);
        dataGridView->SetColSize(2, 100);
        dataGridView->SetCellValue(row, 3, item.type);
        dataGridView->SetCellValue(row, 4, item.status);
        dataGridView->SetCellValue(row, 5, item.date);
        dataGridView->SetCellRenderer(row, 6, new ButtonRenderer());
        dataGridView->SetColSize(6, 100);  // Adjust column size
        // Bind event for mouse click
        dataGridView->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickDelete, this);
        dataGridView->SetCellRenderer(row, 7, new ButtonCellRenderer());
        //dataGridView->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnLeftClickAddToGroup, this);
        dataGridView->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnGridCellClickAddToGroup, this);
        int cols =dataGridView->GetNumberCols(); 
        for (int x = 0; x < cols; x++) {
            dataGridView->SetReadOnly(row, x, true);
        }
        getDb()->InsertDataGridItem(item_ptr);
    }

    // Auto-size columns and refresh the grid
    dataGridView->AutoSize();
    dataGridView->ForceRefresh();

    // End batch update
    dataGridView->EndBatch();
    AssertUrlGridItemsSync();
    if (debugMode) wxLogMessage("Grid updated successfully._______________");
}

/*void MyFrame::UpdateGridFromQueue() {
    dataGridView->BeginBatch();
    dataGridView->ClearSelection();

    if (dataGridView->IsCellEditControlEnabled())
    {
        dataGridView->DisableCellEditControl();
    }

    while (!dataQueue.empty()) {
        DataGridItem item = dataQueue.front();
        dataQueue.pop();

        int row = dataGridView->GetNumberRows();
        dataGridView->AppendRows(1);
        dataGridView->SetCellValue(row, 0, item.serial);
        dataGridView->SetCellValue(row, 1, item.uuid);
        dataGridView->SetCellValue(row, 2, item.url);
        //dataGridView->SetCellValue(row, 3, item.type);
        //dataGridView->SetCellValue(row, 4, item.status);
        //dataGridView->SetCellValue(row, 5, item.date);

    }
    dataGridView->AutoSize();
    dataGridView->ForceRefresh();
    dataGridView->EndBatch();
}*/
void MyFrame::InsertDataGridItemsIntoDB() {
    AssertUrlGridItemsSync();
    assert(UrlGridItems.size() == dataGridView->GetNumberRows() && "UrlGridItems size differs from dataGridView->GetNumberRows()");
    std::shared_ptr<std::vector<std::shared_ptr<DataGridItem>>> ptr = make_shared<decltype(UrlGridItems)>(UrlGridItems);
    if (debugMode) wxLogMessage("MyFrame::InsertDataGridItemsIntoDB Inserting data into the database.");
    // Check if grid is empty
    if (dataGridView->GetNumberRows() == 0) {
        wxLogWarning("No data to insert into the database.");
        return;
    }
    if (UrlGridItems.empty()) {
        wxLogWarning("UrlGridItems No data to insert into the database.");
        return;
    }
    if (debugMode && UrlGridItems.size() != dataGridView->GetNumberRows()) {
        wxLogMessage("UrlGridItems size:%zu differs from dataGridView->GetNumberRows():%d" , UrlGridItems.size() , dataGridView->GetNumberRows());
        wxLogError("UrlGridItems size:%zu differs from dataGridView->GetNumberRows():%d", UrlGridItems.size(), dataGridView->GetNumberRows());
    }
    
    getDb()->InsertDataGridItemsIntoDB(ptr);
}


void MyFrame::StartTcpServer() {
    if (tcpServer) {
        wxLogWarning("TCP server is already running.");
        return;
    }
    wxIPV4address addr;
    //addr.AnyAddress();
    addr.LocalHost();   // Bind to localhost (127.0.0.1)
    addr.Service(8080); // Bind to port 8080
    tcpServer = new wxSocketServer(addr);
    if (!tcpServer->IsOk()) {
        wxLogError("Failed to start TCP server.");
        return;
    }
    //tcpServer->SetEventHandler(*this, wxID_ANY);
    tcpServer->SetEventHandler(*this, SERVER_ID);
    tcpServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
    tcpServer->Notify(true);
    if (debugMode) wxLogMessage(wxString("END OF start TCP server process.") + wxString(tcpServer->Ok() ? "True" : "False"));
}

void MyFrame::OnClientConnect(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Client connected.");
    //wxSocketBase* socket = event.GetSocket();
    // Accept the new connection and get the socket pointer
    wxSocketBase* socket = tcpServer->Accept(false);
    // Tell the new socket how and where to process its events
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);
    clientSocket = socket;
    if (debugMode) wxLogMessage("Client connected.out a");
    if (debugMode) wxLogMessage(wxString("Client connected.") + wxString(std::to_string(event.GetId())));
    if (debugMode) wxLogMessage("Client connected.out b");
}


void MyFrame::OnMessageReceived(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Message received from client.");

    wxSocketBase* socket = event.GetSocket();
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    wxString message;
    char buffer[4096];
    size_t bytesRead = 0;
    bool isStreaming = false;

    // Start reading the actual data in chunks
    do {
        bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
        if (bytesRead > 0) {
            message.append(buffer, bytesRead);  // Append read data to message
            if (debugMode) {
                wxLogMessage("Received chunk of data (size: %zu)", bytesRead);
                wxString logMessage(message.substr(0, std::min<size_t>(message.size(), 140))); // Log first 140 characters
                wxLogMessage("Received Message: %s", logMessage);
            }
        }
        else {
            if (socket->Error()) {
                wxLogMessage("Socket read error occurred.");
            }
            break;  // Exit loop if no more data is available or socket disconnected
        }

        // Check if the message indicates streaming data
        if (message.StartsWith("POST stream_data")) {
            isStreaming = true;
            if (debugMode) wxLogMessage("Handling streaming data");

            // Avoid clearing message prematurely
            if (message.EndsWith("\r\n\r\n")) {  // Full message has been received
                StreamExitCondition exitCondition = HandleStreamingRequest(socket, message);
                message.Clear();  // Clear message after processing

                // Early exit based on the exit condition
                if (exitCondition != StreamExitCondition::Success) {
                    if (debugMode) wxLogMessage("Exiting processing due to exit condition: %d", exitCondition);
                    break;  // Exit loop if the handling did not succeed
                }

                if (debugMode) wxLogMessage("Completed streaming data processing.");
            }
        }
        // Handle non-streaming data
        else {
            if (debugMode) wxLogMessage("Handling non-streaming data");
            HandleHttpRequest(socket, message);
            message.Clear();  // Clear message after processing
            break;  // Exit loop after processing non-streaming data
        }
    } while (bytesRead > 0 || isStreaming);

    if (debugMode) wxLogMessage("Destroying socket");
    socket->Destroy();  // Clean up the socket after handling the message
}


/*void MyFrame::OnMessageReceived(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Message received from client.");

    wxSocketBase* socket = event.GetSocket();
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    wxString message;
    char buffer[4096];
    size_t bytesRead = 0;
    bool isStreaming = false;

    // Start reading the actual data in chunks
    do {
        bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
        if (bytesRead > 0) {
            message.append(buffer, bytesRead);  // Append read data to message
            if (debugMode) {
                wxLogMessage("Received chunk of data (size: %zu)", bytesRead);
                wxString logMessage(message.substr(0, std::min<size_t>(message.size(), 140))); // Log first 140 characters
                wxLogMessage("Received Message: %s", logMessage);
            }
        }
        else {
            if (socket->Error()) {
                wxLogMessage("Socket read error occurred.");
            }
            break;  // Exit loop if no more data is available or socket disconnected
        }

        // Check if the message indicates streaming data
        if (message.StartsWith("POST stream_data")) {
            isStreaming = true;
            if (debugMode) wxLogMessage("Handling streaming data");

            // Avoid clearing message prematurely
            if (message.EndsWith("\r\n\r\n")) {  // Full message has been received
                HandleStreamingRequest(socket, message);
                message.Clear();  // Clear message after processing
                if (debugMode) wxLogMessage("Completed streaming data processing.");
            }
        }
        // Handle non-streaming data
        else {
            if (debugMode) wxLogMessage("Handling non-streaming data");
            HandleHttpRequest(socket, message);
            message.Clear();  // Clear message after processing
            break;  // Exit loop after processing non-streaming data
        }
    } while (bytesRead > 0 || isStreaming);

    if (debugMode) wxLogMessage("Destroying socket");
    socket->Destroy();  // Clean up the socket after handling the message
}*/



void MyFrame::OnMessageReceiveddep3(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Message received from client.");

    wxSocketBase* socket = event.GetSocket();
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    wxString message;
    char buffer[4096];
    size_t bytesRead = 0;

    
    if (debugMode) {
        size_t toLog = std::min<size_t>(bytesRead, 140);  // Prevent overflowing
        wxString logMessage(buffer, toLog); // Create a wxString from the buffer up to toLog characters
        wxLogMessage("Received Message: %s", logMessage);
    }

    //start new
    do {
        bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
        if (bytesRead > 0) {
            message.append(buffer, bytesRead);  // Append read data to message
        }
        if (debugMode) wxLogMessage("Received chunk of data (size: %zu) (sizeof:%zu)", bytesRead, sizeof(buffer));

        if (message.StartsWith("POST stream_data")) {
            if (debugMode) wxLogMessage("handling streaming data");
            // Handle other types of requests or one-shot message
            //bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
            if (bytesRead > 0) {
                message.append(buffer, bytesRead);
                HandleHttpRequest(socket, message);
            }
            if (message.EndsWith("\r\n\r\n")) {
                if (debugMode) wxLogMessage("forcing  streaming data break");
                break;
            }

        }
        else {
            if (debugMode) wxLogMessage("logging message in ELSE1", message.substr(0 , 140));
            if (debugMode) wxLogMessage("handling non streaming data");
            // Handle other types of requests or one-shot message
            //bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
            if (bytesRead > 0) {
                message.append(buffer, bytesRead);
                HandleHttpRequest(socket, message);
            }
            if (debugMode) wxLogMessage("end loop -2");
            break;
        }
    } while (bytesRead > 0);
    //end new
    
    if (debugMode) wxLogMessage("destroying socket");
    socket->Destroy();  // Clean up the socket after handling the message
}

void MyFrame::OnMessageReceiveddeprecated2(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Message received from client.");
    if (debugMode) wxLogMessage("Message received from client." + event.GetId());
    // wxSocketBase* sock = tcpServer->Accept(false);
    wxSocketBase* socket = event.GetSocket();


    // addedTell the new socket how and where to process its events
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    //end added

    wxString message;
    char buffer[4096];
    size_t bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();

    //start new
    // // Begin reading the actual data in chunks
    do {
        bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
        if (bytesRead > 0) {
            message.append(buffer, bytesRead);  // Append read data to message
        }
        else{
            if (!socket->IsConnected()) {
                if (debugMode) wxLogMessage("Client disconnected.");
            }
            break; // Exit the loop if no more data is available
        }

        if (debugMode) wxLogMessage("Received chunk of data (size: %zu)", bytesRead);

        // Process the accumulated message (for large files, you may process it incrementally)
        if (!message.IsEmpty()) {
            HandleHttpRequest(socket, message);  // Process the chunked data
            message.Clear();  // Clear the message buffer after processing to avoid reprocessing the same data
        }
        if (message.EndsWith("\r\n\r\n")) { // For example, an empty line indicates end of message
            if (debugMode) wxLogMessage("End Messagew Received.");
            break;
        }

    } while (bytesRead >0);  // Continue reading while the buffer is full

    //end new
   
    if (debugMode) wxLogMessage("Destroying Socket wait");
    std::this_thread::sleep_for(std::chrono::seconds(3)); // Pauses for 3 seconds
    if (debugMode) wxLogMessage("Destroying ");
    socket->Destroy();
    if (debugMode) wxLogMessage("Socket Destroyed");
}

void MyFrame::OnMessageReceiveddeprecated(wxSocketEvent& event) {
    if (debugMode) wxLogMessage("Message received from client.");
    if (debugMode) wxLogMessage("Message received from client." + event.GetId());
    // wxSocketBase* sock = tcpServer->Accept(false);
    wxSocketBase* socket = event.GetSocket();


    // addedTell the new socket how and where to process its events
    socket->SetEventHandler(*this, SOCKET_ID);
    socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    //end added

    wxString message;
    char buffer[4096];
    size_t bytesRead = socket->Read(buffer, sizeof(buffer)).LastReadCount();
    if (bytesRead > 0) {
        message.append(buffer, bytesRead);
        if (debugMode) wxLogMessage("Received message: %s", message);
        // Call HandleHttpRequest to process the HTTP request
        HandleHttpRequest(socket, message);
    }
    if (debugMode) wxLogMessage("Destroying Socket wait");
    std::this_thread::sleep_for(std::chrono::seconds(3)); // Pauses for 3 seconds
    if (debugMode) wxLogMessage("Destroying ");
    socket->Destroy();
    if (debugMode) wxLogMessage("Socket Destroyed");
}
void MyFrame::StopTcpServer() {
    if (debugMode) wxLogMessage("Stopping TCP server process.");
    if (tcpServer) {
        tcpServer->Destroy();
        tcpServer = nullptr;
    }
}

void MyFrame::doDeletePage(const wxString& groupName) {
    int pageCount = groupNotebook->GetPageCount();

    for (int i = 0; i < pageCount; ++i) {
        if (groupNotebook->GetPageText(i) == groupName) {
            groupNotebook->DeletePage(i);   // Delete the matched page
            //groupNames.Remove(groupName);   // Remove group from the list
            UpdateChoiceControl();          // Refresh any associated controls
            break;                          // Exit after deletion
        }
    }
}

bool MyFrame::ForTestsOnDeleteGroup(const wxString& groupname){
    int selection=wxNOT_FOUND;
    for (int i = 0; i < groupNotebook->GetPageCount(); ++i)
    {
        if (groupNotebook->GetPageText(i) == groupname)
        {
            selection=i; // Return the index of the page with the matching name
            break;
        }      
    }
    if(selection == wxNOT_FOUND){
        std::cout << "Page for Group "<< groupname << " Not found "<< std::endl;
        std::cout << "Pages Indexed" << std::endl;
        for (int i = 0; i < groupNotebook->GetPageCount(); ++i)
    {
        std::cout << groupNotebook->GetPageText(i) << " ";
        
    }
    std::cout << std::endl;
        return false;
    }
    doProcOnDeleteGroup(selection , groupname);
    checkGroupsAssert("ForTestsOnDeleteGroup");
    checkGroupsLog("ForTestsOnDeleteGroup");
    return true;
}
    void MyFrame::doProcOnDeleteGroup(int selection , const wxString& groupName){
        // Remove the tab and delete the associated group
            UpdateUrlGridOnGroupDelete(groupName);
            // Clean up group info and its grid
            RemoveGroup(groupName);
            groupNotebook->DeletePage(selection);
            groupNames.Remove(groupName);
            getDb()->RemoveGroup(groupName);
            // Update the choice control for group assignment
            UpdateChoiceControl();    
    }
    void MyFrame::OnDeleteGroup(wxCommandEvent& event)
    {
        int selection = groupNotebook->GetSelection();
        if (selection != wxNOT_FOUND)
        {
            wxString groupName = groupNotebook->GetPageText(selection);
            doProcOnDeleteGroup(selection , groupName);

        }
        checkGroupsAssert("OnDeleteGroup");
        checkGroupsLog("OnDeleteGroup");
    }

    void MyFrame::RemoveGroup(const wxString& groupName) {
        auto it = groupMap.find(groupName);
        if (it != groupMap.end()) {
            auto groupInfo = it->second;

            // Remove the grid from the sizer and delete it
            if (groupInfo->GetGrid() && groupInfo->getPanelSizer()) {
                groupInfo->getPanelSizer()->Detach(groupInfo->GetGrid());  // Detach from sizer
                //groupInfo->GetGrid()->Destroy();           // Destroy the grid
            }

            // Remove the group from the map, releasing the shared pointer
            groupMap.erase(it);

            // Update the layout to reflect changes
            groupInfo->getPanelSizer()->Layout();
            
            if (debugMode) wxLogMessage("Group '%s' and associated grid cleaned up.", groupName);
        }
        //checkGroupsLog("RemoveGroup");this func only reoves the group from groupMap doesnt tally checkGroupsLog pagecount: 5, groupMapSize: 4, groupNames: 5, from: RemoveGroup
    }

void MyFrame::UpdateChoiceControl()
{
    for (int row = 0; row < urlGrid->GetNumberRows(); ++row)
    {
        wxArrayString choices;
        choices.Add("None");  // Default option

        // Add group names to choices
        for (const wxString& groupName : groupNames)
        {
            choices.Add(groupName);
        }

        // Create a wxChoice editor for the group assignment column
        wxGridCellChoiceEditor* choiceEditor = new wxGridCellChoiceEditor(choices);
        urlGrid->SetCellEditor(row, 2, choiceEditor);
    }
}

void MyFrame::LoadGroupsFromDB()
{
    if (debugMode) wxLogMessage("Loading groups from db");
    //std::shared_ptr<std::vector<wxString>> group_names_ptr = getDb()->LoadGroupNamesFromDB();
    std::shared_ptr<std::unordered_map<std::string,GroupData >> groups = getDb()->LoadGroupsFromDB();
    for ( std::pair<std::string, GroupData> pair : *groups) {
        wxString groupname = wxString(pair.first);
        GroupData data = pair.second;
        if (groupNames.Index(groupname) == wxNOT_FOUND) {
            if (debugMode) wxLogMessage("Group '%s' loaded..", groupname);
            AddGroupToNotebook(groupname, data);
            groupNames.Add(groupname);
        }

    }
    /*for (const wxString& name : *group_names_ptr) {
       
        if (groupNames.Index(name) == wxNOT_FOUND) {
            if (debugMode) wxLogMessage("Group '%s' loaded..", name);
            AddGroupToNotebook(name);
            groupNames.Add(name);
        }
    }*/
}






/*void MyFrame::OnAddGroup(wxCommandEvent& event) {
    wxString groupName = wxGetTextFromUser("Enter group name:", "Add Group");
    if (!groupName.IsEmpty())
    {
        groupNames.Add(groupName);

        // Add a new page in the notebook with the group name
        groupPanel = new wxPanel(groupNotebook, wxID_ANY);
        wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);

        // Add some group-related controls or a label
        wxStaticText* label = new wxStaticText(groupPanel, wxID_ANY, "This is group: " + groupName);
        panelSizer->Add(label, 1, wxALL, 5);

        // Add a delete button for this group
        wxButton* deleteGroupButton = new wxButton(groupPanel, wxID_ANY, "Delete Group");
        panelSizer->Add(deleteGroupButton, 0, wxALL, 5);

        // Connect the delete button event to delete the group tab
        deleteGroupButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnDeleteGroup, this);

        groupPanel->SetSizer(panelSizer);

        // Add the panel to the notebook as a new tab
        groupNotebook->AddPage(groupPanel, groupName);

        // Update the choice control for group assignment
        UpdateChoiceControl();
    }
}*/
void MyFrame::OnAddGroup(wxCommandEvent& event) {
    wxString groupName = wxGetTextFromUser("Enter group name:", "Add Group");
    if (!groupName.IsEmpty()) {
        // Check if the group already exists
        if (groupNames.Index(groupName) == wxNOT_FOUND) {
            groupNames.Add(groupName);
            getDb()->AddGroupToDB(groupName);
            auto grdata = getDb()->LoadGroupByName(groupName);
            AddGroupToNotebook(groupName , *grdata);  // Call to a new method to handle UI update
        }
        else {
            wxLogMessage("Group name already exists: " + groupName);
        }
    }
    checkGroupsAssert("OnAddGroup");
    checkGroupsLog("OnAddGroup");
}





void MyFrame::CleanUpGroupGridItems(const wxString& uuid) {
    // Locate the item in GroupGridItems by UUID
    auto group_it = std::find_if(GroupGridItems.begin(), GroupGridItems.end(),
        [&uuid](const std::shared_ptr<DataGridItem>& item) {
        return item->uuid == uuid;
    });

    if (group_it != GroupGridItems.end()) {
        if (debugMode) wxLogMessage("Found item in GroupGridItems with UUID: %s", uuid);

        // Check for associated group names
        std::vector<wxString> groupNames = (*group_it)->groupNames;
        for (const wxString& groupName : groupNames) {
            auto it = groupMap.find(groupName);
            if (it != groupMap.end()) {
                auto groupInfo = it->second;
                if (groupInfo) {
                    if (debugMode) wxLogMessage("Removing item from group: %s", groupName);
                    groupInfo->RemoveItem(*group_it);
                }
            }
        }

        // Remove the item from GroupGridItems
      //  GroupGridItems.erase(group_it);
        if (debugMode) wxLogMessage("Item removed from GroupGridItems.");
    }
    else {
        if (debugMode) wxLogMessage("No matching item found in GroupGridItems for UUID: %s", uuid);
    }
}

void MyFrame::OnLeftClickDelete(wxGridEvent & event){
    /*Deletes the row from datagridview 1st grid*/
    if (m_clkDeb1) return;
    m_clkDeb1 = true;
    if (event.GetCol() == 6) {  // If the clicked column is the one with the button
        int row = event.GetRow();
        if (debugMode) wxLogMessage("Delete button clicked in row: %d", row);
        // Perform your delete action here
        if (dataGridView) {
            if (row >= 0 && row < dataGridView->GetNumberRows()) {
                auto del_it = UrlGridItems.begin() + row;
                wxString uuid = (*del_it)->uuid;
                //Call the new cleanup function
                 CleanUpGroupGridItems(uuid);
                
                UrlGridItems.erase(UrlGridItems.begin() + row);
                dataGridView->DeleteRows(row, 1); // Delete one row at the specified index
                bool is_deleted = getDb()->deleteByUUID(uuid);
                if (is_deleted) wxLogMessage("successfully deleted %s len:%zu", uuid , uuid.length());
                dataGridView->AutoSize();
                dataGridView->ForceRefresh();
            }

        }
    }
    event.Skip();
    CallAfter([this]() {
        m_clkDeb1 = false; // Reset clicked status
        wxLogMessage("Click status reset.");
    });
}
std::shared_ptr<URLComponents> MyFrame::ParseAndQueueUrlforStreaming(const wxString& jsonData) {
    if (debugMode) wxLogMessage("Parsing for streaming single JSON object for URL queue.");
    
    try {
        nlohmann::json json = nlohmann::json::parse(jsonData.ToStdString());
        std::string url = wxString::FromUTF8(json["url"].get<std::string>()).ToStdString();

        // Create and return the unique_ptr to URLComponents
        std::shared_ptr<URLComponents> urlptr = std::make_shared<URLComponents>(url);
        // Create a DataGridItem from the single JSON object
        DataGridItem dataItem;
        dataItem.serial = std::to_string(serial_no++); // Assuming sequential serial numbers
        dataItem.uuid = urlptr->getUUID(); //UUIDcr::sgetUUID();
        dataItem.url = urlptr->url;

        // Handle optional fields if present
        if (json.contains("type")) dataItem.type = wxString::FromUTF8(json["type"].get<std::string>());
        if (json.contains("status")) dataItem.status = wxString::FromUTF8(json["status"].get<std::string>());
        if (json.contains("date")) dataItem.date = wxString::FromUTF8(json["date"].get<std::string>());
        if (json.contains("content")) std::string content = wxString::FromUTF8(json["content"].get<std::string>()).ToStdString();

        // Handle content
        //You can read the content if necessary
        //std::string fileContent = urlComponents.readContent();

        // Parse and store URL components before queueing the data

        // Add the item to the queue
        dataQueue.push(dataItem);
        return urlptr;
    }
    catch (const nlohmann::json::exception& e) {
        wxLogError("Failed to parse JSON1 data: %s", e.what());
        return nullptr;  // Return nullptr in case of failure
    }
}
void MyFrame::ParseAndQueueUrl(const wxString& jsonData) {
    if (debugMode) wxLogMessage("Parsing single JSON object for URL queue.");

    try {
        nlohmann::json json = nlohmann::json::parse(jsonData.ToStdString());
        std::string url = wxString::FromUTF8(json["url"].get<std::string>()).ToStdString();
        //create the struct
        URLComponents urlComponents(url);

        // once save file
            std::ofstream tempFile(urlComponents.getFilePath());
        if (json.contains("content")) {
            std::string content = wxString::FromUTF8(json["content"].get<std::string>()).ToStdString();
            tempFile << content;  // Save content to temp file
        }
        tempFile.close();

        // Open the temp file as an input stream for further processing
        //std::ifstream* contentStream = new std::ifstream("tempfile.txt");
        urlComponents.setFileStream();
        // 



        // Create a DataGridItem from the single JSON object
        DataGridItem dataItem;
        dataItem.serial = std::to_string(serial_no++); // Assuming sequential serial numbers
        dataItem.uuid = urlComponents.getUUID(); //UUIDcr::sgetUUID();
        dataItem.url = urlComponents.url;
        
        // Handle optional fields if present
        if (json.contains("type")) dataItem.type = wxString::FromUTF8(json["type"].get<std::string>());
        if (json.contains("status")) dataItem.status = wxString::FromUTF8(json["status"].get<std::string>());
        if (json.contains("date")) dataItem.date = wxString::FromUTF8(json["date"].get<std::string>());
        if (json.contains("content")) std::string content = wxString::FromUTF8(json["content"].get<std::string>()).ToStdString();

        // Handle content
        //You can read the content if necessary
        //std::string fileContent = urlComponents.readContent();

        // Parse and store URL components before queueing the data
        
        // Add the item to the queue
        dataQueue.push(dataItem);

    }
    catch (const nlohmann::json::exception& e) {
        wxLogError("Failed to parse JSON2 data: %s", e.what());
    }
}



void MyFrame::ParseAndQueueData(const wxString& jsonData) {
    if (debugMode) wxLogMessage("Parsing JSON data..%s", jsonData);

    try {
        nlohmann::json json = nlohmann::json::parse(jsonData.ToStdString());
        if (json.is_array()) {
            for (const auto& item : json) {
                DataGridItem dataItem;
                dataItem.serial = wxString::FromUTF8(item["serial"].get<std::string>());
                dataItem.uuid = wxString::FromUTF8(item["uuid"].get<std::string>());
                dataItem.url = wxString::FromUTF8(item["url"].get<std::string>());
                dataItem.type = wxString::FromUTF8(item["type"].get<std::string>());
                dataItem.status = wxString::FromUTF8(item["status"].get<std::string>());
                dataItem.date = wxString::FromUTF8(item["date"].get<std::string>());

                dataQueue.push(dataItem);
            }
        }
    }
    catch (const std::exception& e) {
        wxLogError("Failed to parse JSON3 data: %s", e.what());
    }

    
}
