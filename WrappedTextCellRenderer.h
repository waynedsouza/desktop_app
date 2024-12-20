#pragma once
//#include "F:\wxWidgets-3.2.6\include\wx\generic\gridctrl.h"
#include <wx/grid.h>
#include <wx/wx.h>
#include <wx/dc.h>
#include <wx/string.h>
#include <wx/tokenzr.h>

/*class WrappedTextCellRenderer : public wxGridCellStringRenderer {
public:
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif
    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) override {
        // Get the text to be rendered
        wxString text = grid.GetCellValue(row, col);        
        dc.SetFont(attr.GetFont());
        dc.SetTextForeground(attr.GetTextColour());
        // Set the background color if the cell is selected
        if (isSelected) {
            dc.SetBrush(*wxLIGHT_GREY_BRUSH);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(rect);
        }

        // Calculate the rectangle to draw
        wxRect textRect = rect;
        textRect.Deflate(2);  // Add some padding

        // Split the text into lines for wrapping
        wxStringTokenizer tokenizer(text, wxT("\n"));
        int lineHeight = dc.GetTextExtent(text).GetHeight();
        int currentHeight = 0;
        if (debugMode) wxLogMessage("Wrap Renderer line:height%d ", lineHeight);
        // Draw the text with wrapping
        while (tokenizer.HasMoreTokens()) {
            wxString line = tokenizer.GetNextToken();
            wxSize textSize = dc.GetTextExtent(line);
            if (currentHeight + lineHeight > rect.height) {
                break; // Stop if we exceed the cell height
            }
            dc.DrawText(line, textRect.x, textRect.y + currentHeight);
            currentHeight += lineHeight; // Move down for the next line
        }

        // Adjust the row height based on content (optional)
        if (currentHeight > rect.height) {
            grid.SetRowSize(row, currentHeight + 4); // 4 for some padding
        }
    }
};*/

class WrappedTextCellRenderer : public wxGridCellStringRenderer {
public:
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif
    WrappedTextCellRenderer(int maxWidth=200); // Constructor with width parameter
    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) override;

private:
    int m_maxWidth; // Member variable to store the maximum width
};
