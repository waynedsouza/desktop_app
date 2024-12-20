#include "WrappedTextCellRenderer.h"


WrappedTextCellRenderer::WrappedTextCellRenderer(int maxWidth) : m_maxWidth(maxWidth) {
    // Additional initialization if needed
}
void WrappedTextCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
    // Calculate the rectangle to draw
    wxRect textRect = rect;
    textRect.Deflate(2);  // Add some padding

    // Split the text into lines for wrapping
    wxString text = grid.GetCellValue(row, col); // Get the text for the cell
    wxSize charSize = dc.GetTextExtent("A"); // Size of a typical character
    int currentHeight = 0;

    // Accumulate text line by line
    wxString currentLine;
    int currentLineWidth = 0;

    // Loop through each character instead of tokenizing by words
    for (size_t i = 0; i < text.Length(); ++i) {
        wxString currentChar = text.Mid(i, 1);  // Get each character
        wxSize charSize = dc.GetTextExtent(currentChar);  // Get width of the character

        // Check if adding this character exceeds the specified maximum width
        if (currentLineWidth + charSize.GetWidth() > m_maxWidth) {
            // Draw the current line since the character doesn't fit
            dc.DrawText(currentLine, textRect.x, textRect.y + currentHeight);
            currentHeight += charSize.GetHeight(); // Move down for the next line

            // Reset the current line and width
            currentLine = currentChar;
            currentLineWidth = charSize.GetWidth();
        }
        else {
            // Add the character to the current line
            currentLine += currentChar;
            currentLineWidth += charSize.GetWidth();
        }

        // Check if we've reached the maximum cell height
        if (currentHeight + charSize.GetHeight() > rect.height) {
            break;  // Stop if the cell height is exceeded
        }
    }

    // Draw the last line if there's any remaining text
    if (!currentLine.IsEmpty()) {
        dc.DrawText(currentLine, textRect.x, textRect.y + currentHeight);
    }

    // Optionally, adjust the row height based on content
    if (currentHeight + charSize.GetHeight() > rect.height) {
        grid.SetRowSize(row, currentHeight + 4);  // Add some padding to the row height
    }
}
