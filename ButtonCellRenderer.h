#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/button.h>

class ButtonCellRenderer : public wxGridCellRenderer
{
public:
    

     void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
        const wxRect& rect, int row, int col, bool isSelected) override
    {
        // Draw the button inside the cell
        wxRect buttonRect = rect; // Start with the cell rect
        buttonRect.Deflate(2); // Adjust for padding

        // Create the label for the button
        wxString buttonLabel = "Add to Group";

        // Draw the button using wxDC::DrawLabel for text-centered rendering
        dc.SetBrush(*wxLIGHT_GREY_BRUSH);  // Button background
        dc.SetPen(*wxBLACK_PEN);           // Button border
        dc.DrawRectangle(buttonRect);      // Draw the button rectangle

        // Draw the button label
        dc.DrawLabel(buttonLabel, buttonRect, wxALIGN_CENTER);

        // Optionally, if the cell is selected, you can highlight it differently
        if (isSelected)
        {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(*wxRED_PEN);  // Highlight selected cells with a red border
            dc.DrawRectangle(buttonRect);
        }
    }

    // You may also override other methods if necessary
    virtual wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
        int row, int col) override
    {
        // Optionally return a fixed size or a size based on the content
        return wxSize(100, 30); // Example fixed size
    }

    wxGridCellRenderer* Clone() const override {
        return new ButtonCellRenderer();
    }
};



