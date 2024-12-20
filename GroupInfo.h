#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <wx/grid.h>
#include <wx/wx.h>
#include "DataGridItem.h"
#include "button_renderers.h"
#include "MyFrame.h"
class MyFrame;  // Forward declaration of MyFrame
class GroupInfo {

public:
    GroupInfo(MyFrame* parent, wxBoxSizer* panel_sizer, wxString grp) : 
        mainframe(parent),
        panelSizer(panel_sizer), 
        grid(nullptr), 
        name(grp), 
        scheduled_message(nullptr),
        groupPanel(nullptr),
        id(-1)
    {
        if (debugMode) wxLogMessage("GroupInfo:constructing group..%s", name);
    }
    ~GroupInfo() {
        if (grid) {
            // Unbind the event to avoid any lingering connections
            grid->Unbind(wxEVT_GRID_CELL_LEFT_CLICK, &GroupInfo::OnLeftClickDeleteGroup, this);

            // Detach the grid from the sizer if it is part of one
            if (panelSizer) {
                panelSizer->Detach(grid.get());
                grid->Show(false); // Optionally hide the grid as it�s about to be destroyed
            }

            if (debugMode) wxLogMessage("GroupInfo:destructing group..%s", name);
        }
    }



    void InitializeGrid(wxWindow* parent);

    void AddItem(std::shared_ptr<DataGridItem> item);

    void RemoveItemDeprecated(std::shared_ptr<DataGridItem> item);
    void RemoveItem(std::shared_ptr<DataGridItem> item);
    void setGroupPanel(wxPanel* panel) {
        groupPanel = panel;
    }
    void setScheduledMessage(GroupData data);
    wxTextCtrl* getScheduledMessage() { return scheduled_message; }
    wxString getName()const {
        return name;
    }
    wxGrid* GetGrid() {
        return grid.get();
    }
    wxBoxSizer* getPanelSizer()const {
        return panelSizer;
    }
    bool isEmpty()const {
        return items.empty();
    }
    std::vector<std::shared_ptr<DataGridItem>> GetItems()const{
        return items;
    }
    void OnLeftClickDeleteGroupFORTESTS(wxGridEvent& event);
    // Getter for ID
    int getId() const {
        return id;
    }

    // Setter for ID
    void setId(int newId) {
        id = newId;
    }

    private:
    #ifdef _DEBUG
        bool debugMode = true;
    #else
        bool debugMode = false;
    #endif

        void RemoveRow(int row);
        void AddRow(const std::shared_ptr<DataGridItem>& item);
        void OnLeftClickDeleteGroup(wxGridEvent& event);

        std::unique_ptr<wxGrid> grid;
        std::vector<std::shared_ptr<DataGridItem>> items;
        bool m_clkDeb1 = false;
        wxString name;
        MyFrame* mainframe;//thi is it
        wxBoxSizer* panelSizer;
        wxTextCtrl* scheduled_message;
        wxPanel* groupPanel;
        int id;
        friend class GroupInfoTest;  // Granting access to GroupInfoTest
        friend class MyFrameTest;
        friend class GTEST_TEST;
};
