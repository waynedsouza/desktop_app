#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/image.h>
//wxString delete_button ="delete_button-20.jpg";
class ImageButtonRenderer : public wxGridCellRenderer {
public:
    wxBitmap buttonBitmap;

    ImageButtonRenderer(const wxString& imagePath) {
        // Load the image from file
        wxImage image;
        if (image.LoadFile(imagePath)) {
            buttonBitmap = wxBitmap(image.Scale(20, 20)); // Scale the image to fit in the cell
        }
    }

    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) override {
        // Draw the button bitmap in the center of the cell
        int x = rect.x + (rect.width - buttonBitmap.GetWidth()) / 2;
        int y = rect.y + (rect.height - buttonBitmap.GetHeight()) / 2;

        dc.DrawBitmap(buttonBitmap, x, y, true); // true means transparency
    }

    wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col) override {
        // Return the size of the button (which is the image size in this case)
        return wxSize(buttonBitmap.GetWidth(), buttonBitmap.GetHeight());
    }

    wxGridCellRenderer* Clone() const override {
        return new ImageButtonRenderer(*this); // Clone the renderer
    }
};
/*
class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(NULL, wxID_ANY, "wxGrid Image Button Example", wxDefaultPosition, wxSize(600, 400)) {
        wxPanel* panel = new wxPanel(this, wxID_ANY);
        wxGrid* grid = new wxGrid(panel, wxID_ANY, wxDefaultPosition, wxSize(580, 380));

        grid->CreateGrid(5, 7);
        grid->SetColLabelValue(6, "Actions");

        // Set custom renderer for the last column (with the image button)
        wxString imagePath = "delete_button-20.jpg"; // Specify the path to the image file
        grid->SetCellRenderer(0, 6, new ImageButtonRenderer(imagePath));
        grid->SetColSize(6, 50);  // Adjust column size to fit the image

        // Bind event for mouse click
        grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MyFrame::OnCellLeftClick, this);
    }

    void OnCellLeftClick(wxGridEvent& event) {
        if (event.GetCol() == 6) {  // If the clicked column is the one with the image button
            int row = event.GetRow();
            wxLogMessage("Delete image button clicked in row: %d", row);
            // Perform your delete action here
        }
        event.Skip();
    }
};

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
*/