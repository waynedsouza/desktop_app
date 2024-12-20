#include "GroupInfo.h"
#include <wx/wx.h>
#include <wx/event.h>

void GroupInfo::RemoveRow(int row) {
    if (!grid || row < 0 || row >= grid->GetNumberRows()) return;

    grid->BeginBatch();
    grid->DeleteRows(row, 1); // Remove the specified row
    grid->EndBatch();
    grid->ForceRefresh();
}

void GroupInfo::AddRow(const std::shared_ptr<DataGridItem>& item) {
    if (!grid) return;
    if (debugMode) wxLogMessage("GroupInfo:Add row grid.%s %s total:%zu", name, item->uuid ,items.size());
    grid->BeginBatch();
    int newRow = grid->GetNumberRows();
    grid->AppendRows(1); // Add one new row

    // Set the URL and renderer for the new row only
    grid->SetCellValue(newRow, 0, item->url);
    grid->SetCellRenderer(newRow, 1, new ButtonRenderer()); // Custom button renderer
    
    grid->EndBatch();
    grid->Layout();
    grid->AutoSize();
    
    grid->ForceRefresh();
}
    

void GroupInfo::RemoveItemDeprecated(std::shared_ptr<DataGridItem> item) {
            std::cout << "inAPP RemoveItem" << item->uuid << " " << item->url << std::endl;
            auto it = std::find(items.begin(), items.end(), item);
            if (it != items.end()) {
                std::cout << "inAPP Mark1 RemoveItem" << item->uuid << " " << item->url << std::endl;
                int row = std::distance(items.begin(), it);//the row from GrouPInfo represents the row on its gridtable and not the groupgrid table
               // mainframe->precisionUpdateGrid(name , item);                      
                items.erase(it);            
                std::cout << "RemoveItem: Group "<< name << "  now contains items:"  << " " << items.size() << std::endl;
                wxLogMessage("RemoveItem: Group %s now contains items: %zu", name, items.size());
                RemoveRow(row); // Only remove the specific row for the deleted item

                // Hide the grid and remove from sizer if no items remain
                if (items.empty()) {
                    grid->Show(false);
                    if (panelSizer && grid->GetParent()) {
                        panelSizer->Detach(grid.get());
                        grid->GetParent()->Layout();
                    }
                }
            }
           
            mainframe->UpdateUrlGridOnGroupDelete(name);//swapped with a precision update
        }
void GroupInfo::RemoveItem(std::shared_ptr<DataGridItem> item) {
    std::cout << "inAPP RemoveItem: " << item->uuid << " " << item->url << std::endl;

    auto it = std::find_if(items.begin(), items.end(),
        [&](const std::shared_ptr<DataGridItem>& item_in_gi) {
        return *item == *item_in_gi;
    });

    if (it != items.end()) {
        std::cout << "inAPP Mark1 RemoveItem: " << item->uuid << " " << item->url << std::endl;
        int row = std::distance(items.begin(), it);

        // Call precisionUpdateGrid and check if the item was erased
        bool erased = mainframe->precisionUpdateGrid(name, item);
        if(erased){
            if (debugMode) wxLogMessage("***deleteByUUID in RemoveItem:%s", item->uuid);
            //mainframe->getDb()->deleteByUUID(item->uuid); why should a datagrid table get deleted from here?
        }
       // if (!erased) {
            std::cout << "Reference count before erase: " << item.use_count() << std::endl;
            items.erase(it);  // Erase only if precisionUpdateGrid did not erase it
            std::cout << "Item erased from GroupInfo" << std::endl;
       // }

        wxLogMessage("RemoveItem: Group %s now contains items: %zu", name, items.size());
        RemoveRow(row);

        if (items.empty()) {
            grid->Show(false);
            if (panelSizer && grid->GetParent()) {
                panelSizer->Detach(grid.get());
                grid->GetParent()->Layout();
            }
        }
    }
    else {
        std::cout << "\nGroupInfo::RemoveItem Item:"<<item->uuid<<" not foun in Items\n";
    }
    mainframe->refreshGrid();
}

void GroupInfo::setScheduledMessage(GroupData data) {
    if (!groupPanel || !panelSizer) {
        wxLogError("GroupInfo::setScheduledMessage - Invalid groupPanel or panelSizer");
        return;
    }

    wxString subscription_period;
    if (data.is_subscribed) {
        if (data.period_days != 0 || data.period_months != 0) {
            subscription_period = wxString::Format("Every %d months and %d days", data.period_months, data.period_days);
        }
        else {
            subscription_period = wxString("Scheduled for ") + data.schedule_date;
        }

        if (!scheduled_message) {
            scheduled_message = new wxTextCtrl(groupPanel, wxID_ANY, subscription_period,
                wxDefaultPosition, wxSize(300, 50),
                wxTE_MULTILINE | wxTE_READONLY);
            panelSizer->Add(scheduled_message, 0, wxALL, 5);
        }
        else {
            scheduled_message->SetValue(subscription_period);
        }
    }
    else {
        wxLogMessage("GroupData is not subscribed; no message to schedule.");
    }
}



void GroupInfo::InitializeGrid(wxWindow* element) {
        if (debugMode) wxLogMessage("GroupInfo:ready to init grid.%s", name);
        if (!grid) {
            grid = std::make_unique<wxGrid>(element, wxID_ANY);
            grid->CreateGrid(0, 2); // Create grid with 2 columns (URL and Button)
            grid->SetColLabelValue(0, "URL");
            grid->SetColLabelValue(1, "Delete");
            grid->SetColSize(1, 100); // Set width for the button column
            // Initially hide the grid
            grid->Show(false);
            // Bind delete button click event to handler
            grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &GroupInfo::OnLeftClickDeleteGroup, this);

            if (debugMode) wxLogMessage("GroupInfo:grid initialized.%s", name);
        }
    }

void GroupInfo::AddItem(std::shared_ptr<DataGridItem> item) {
        if (debugMode) wxLogMessage("GroupInfo:addItem to grid.%s %s", name, item->uuid);
        if (!grid) {
            InitializeGrid(panelSizer->GetContainingWindow());
        }
        if (item) {
            // Check if the item already exists in the items vector
            auto it = std::find_if(items.begin(), items.end(), [&](const std::shared_ptr<DataGridItem>& obj_item) {
                return *obj_item == *item;
            });
            if (it == items.end()) { // If item is not found, add it
                items.push_back(item);
                wxLogMessage("AddItem: Group %s now contains items: %zu" , name , items.size());
                AddRow(item);  // Only add a new row for the added item
                // Show the grid if it has members
                if (items.size() > 0) {
                    grid->Show(true);
                    grid->Refresh();  // Refresh to update the display
                }
            }
            else if (debugMode) {
                wxLogMessage("AddItem: Item already exists, skipping addition.");
            }
        }
    }
void GroupInfo::OnLeftClickDeleteGroupFORTESTS(wxGridEvent& event) {
     if (m_clkDeb1) return;
    m_clkDeb1 = true;

    int col = event.GetCol();
    int row = event.GetRow();
std::cout << "____________**____GroupInfo::OnLeftClickDeleteGroup called_________row: " << row << " col: " << col << std::endl;

    if (col == 1) {  // Check if the clicked column is the delete button column
        if (debugMode) wxLogMessage("Delete Group button clicked in row: %d", row);

        if (row >= 0 && row < static_cast<int>(items.size())) {
            RemoveItem(items[row]);
        }
        //mainframe->UpdateUrlGridOnGroupDelete(name);
    }

    if (grid && grid->GetParent()) {
        grid->GetParent()->CallAfter([this]() {
            m_clkDeb1 = false; // Reset click debounce flag
            if (debugMode) wxLogMessage("Click status reset.");
        });
    }

    event.Skip();
    
}
void GroupInfo::OnLeftClickDeleteGroup(wxGridEvent& event) {
    if (m_clkDeb1) return;
    m_clkDeb1 = true;

    int col = event.GetCol();
    int row = event.GetRow();
std::cout << "________________GroupInfo::OnLeftClickDeleteGroup called_________row: " << row << " col: " << col << std::endl;

    if (col == 1) {  // Check if the clicked column is the delete button column
        if (debugMode) wxLogMessage("Delete Group button clicked in row: %d", row);

        if (row >= 0 && row < static_cast<int>(items.size())) {
            RemoveItem(items[row]);
        }
        //  
    }

    if (grid && grid->GetParent()) {
        grid->GetParent()->CallAfter([this]() {
            m_clkDeb1 = false; // Reset click debounce flag
            if (debugMode) wxLogMessage("Click status reset.");
        });
    }

    event.Skip();
}
