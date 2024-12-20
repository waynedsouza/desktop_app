#pragma once

#include <wx/wx.h>
#include <wx/grid.h>

#include <wx/wx.h>
#include <wx/grid.h>

class ButtonRenderer : public wxGridCellRenderer {
public:
    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) override {
        dc.SetBrush(*wxLIGHT_GREY_BRUSH);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rect);

        wxString label = wxT("Delete");
        wxSize textSize = dc.GetTextExtent(label);

        // Draw the "button" text centered
        dc.DrawText(label, rect.x + (rect.width - textSize.GetWidth()) / 2, rect.y + (rect.height - textSize.GetHeight()) / 2);
    }

    wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col) override {
        wxString label = wxT("Delete");
        wxSize textSize = dc.GetTextExtent(label);
        return wxSize(textSize.GetWidth() + 10, textSize.GetHeight() + 10);  // Add some padding around the text
    }

    wxGridCellRenderer* Clone() const override {
        return new ButtonRenderer();
    }
};

/*class ButtonCellRenderer : public wxGridCellRenderer {
public:
    ButtonCellRenderer() = default;
  
    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected)  override{
        wxRect buttonRect = rect;
        buttonRect.Inflate(-5);
        dc.SetBrush(*wxBLUE_BRUSH);
        dc.DrawRectangle(buttonRect);
        dc.SetTextForeground(*wxWHITE);
        dc.DrawText("Delete", buttonRect.GetTopLeft() + wxPoint(5, 5));
    }

    wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col)   {
        return wxSize(80, 30);
    }

    wxGridCellRenderer* Clone() const override {
        return new ButtonCellRenderer(*this);
    }
};
class ButtonCellEditor : public wxGridCellEditor {
public:
    ButtonCellEditor() : wxGridCellEditor(), button(nullptr) {}
   
    
    void Create(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler) override {
        button = new wxButton(parent, id, "Delete");
        button->Bind(wxEVT_BUTTON, &ButtonCellEditor::OnButtonClick, this);
      
    }

    void SetSize(const wxRect& rect) override {
        if (button) {
            button->SetSize(rect);
        }
    }

    wxString GetValue() const override {
        return wxEmptyString;
    }

    void SetValue(const wxString& value) {
        // No value to handle in this case
    }
    void Reset() override {
        // End editing
    }
    void ApplyEdit(int row, int col, wxGrid* grid) override {
        // End editing
    }

    // You need to implement this pure virtual function
    bool EndEdit(int row, int col, const wxGrid* grid, const wxString& oldval, wxString* newval)  {
        // Since it's a button editor, there may be no actual value to edit.
        // You can handle the cleanup after editing ends here.
        *newval = oldval;  // No changes for the button editor, so return the old value.
        return false;
    }

    
    

    void BeginEdit(int row, int col, wxGrid* grid) override {
        // Start editing
        this->grid = grid;  // Store grid reference
    }

    wxGridCellEditor* Clone() const override {
        return new ButtonCellEditor(*this);
    }

private:
    // This method will be called when the button is clicked
    void OnButtonClick(wxCommandEvent& event) {
        if (grid) {
            int row = grid->GetGridCursorRow();
            int col = grid->GetGridCursorCol();

            wxMessageBox(wxString::Format("Delete button clicked in row %d, column %d", row, col));
        }
    }

    wxButton* button;
    wxGrid* grid;  // Store the reference to the grid
};*/

