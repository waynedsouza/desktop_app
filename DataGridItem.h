#pragma once

#include <wx/string.h>
#include <wx/wx.h>
#include <wx/grid.h>
#include <optional>
#include <vector>

struct DataGridItem
{
    std::optional<int> id;
    wxString serial;
    wxString uuid;
    wxString url;
    wxString type;
    wxString status;
    wxString date;
    std::vector<wxString> groupNames; // List of groups for each item

     // Equality operator for unordered_set
    bool operator==(const DataGridItem& other) const{
        return uuid == other.uuid; // Compare based on UUID
    }
};
