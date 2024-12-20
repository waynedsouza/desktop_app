#pragma once
#include <wx/wx.h>
#include <wx/grid.h>

class MultiGroupCellRenderer : public wxGridCellStringRenderer
{
public:
    MultiGroupCellRenderer() : wxGridCellStringRenderer() {}

    virtual void Draw(wxGrid& grid,
        wxGridCellAttr& attr,
        wxDC& dc,
        const wxRect& rect,
        int row,
        int col,
        bool isSelected) override
    {
        wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

        wxString groups = grid.GetCellValue(row, col);
        if (!groups.IsEmpty()) {
            dc.SetTextForeground(*wxBLACK);  // Set text color
            dc.DrawText(groups, rect.x + 2, rect.y + 2);  // Draw comma-separated group names
        }
    }
};

