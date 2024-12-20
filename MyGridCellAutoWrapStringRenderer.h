#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/grid.h>
#include <wx/tokenzr.h>
#include <wx/log.h>
class MyGridCellAutoWrapStringRenderer : public wxGridCellAutoWrapStringRenderer
{
public:
    MyGridCellAutoWrapStringRenderer(int maxWidth = 200, bool debug = false)
        : m_maxWidth(maxWidth), m_debugMode(debug)
    {}

    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
        const wxRect& rect, int row, int col, bool isSelected) override
    {
        wxString text = grid.GetCellValue(row, col);

        if (m_debugMode) {
            wxLogMessage("MyGridCellAutoWrapStringRenderer: Draw called for cell (%d, %d)", row, col);
            wxLogMessage("Text to render: %s", text);
            wxLogMessage("Begin Height: %d  Width: %d", rect.GetHeight(), rect.GetWidth());
            
        }

        // Prepare the drawing context
        dc.SetFont(attr.GetFont());
        dc.SetTextForeground(isSelected ? grid.GetSelectionBackground() : attr.GetTextColour());

        wxRect textRect = rect;
        textRect.Deflate(2);  // Add some padding

        // Split the text into lines for wrapping
        wxStringTokenizer tokenizer(text, wxT("\n"));
        int lineHeight = dc.GetTextExtent("A").GetHeight();
        int currentHeight = 0;
        int currentLineWidth = 0;

        if (m_debugMode) wxLogMessage("Line height: %d", lineHeight);

        // Draw the text with wrapping logic
        while (tokenizer.HasMoreTokens()) {
            wxString line = tokenizer.GetNextToken();
            wxString wrappedLine;
            wxStringTokenizer charTokenizer(line, wxEmptyString); // Tokenize by characters

            if (m_debugMode) wxLogMessage("Processing line: %s", line);

            while (charTokenizer.HasMoreTokens()) {
                wxString character = charTokenizer.GetNextToken();
                wxSize charSize = dc.GetTextExtent(character);
                if (currentLineWidth + charSize.GetWidth() > textRect.width) {
                    if (m_debugMode) wxLogMessage("Line wrapped at width %d", currentLineWidth);
                    dc.DrawText(wrappedLine, textRect.x, textRect.y + currentHeight);
                    currentHeight += lineHeight;
                    wrappedLine.Clear();
                    currentLineWidth = 0;
                }
                wrappedLine += character;
                currentLineWidth += charSize.GetWidth();
            }
            dc.DrawText(wrappedLine, textRect.x, textRect.y + currentHeight);
            currentHeight += lineHeight;

            if (m_debugMode) {
       
                wxLogMessage("End Height: %d  Width: %d", rect.GetHeight(), rect.GetWidth());

            }
        }

        // Adjust the row height based on content (optional)
        if (currentHeight > rect.height) {
            grid.SetRowSize(row, currentHeight + 4); // Add padding
            if (m_debugMode) wxLogMessage("Row size adjusted to: %d", currentHeight + 4);
        }

        if (m_debugMode) wxLogMessage("Rendering complete for cell (%d, %d)", row, col);
    }

private:
    int m_maxWidth;
    bool m_debugMode;
};

