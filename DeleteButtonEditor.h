#pragma once
#include <wx/wx.h>
#include <wx/grid.h>

class DeleteButtonEditor : public wxGridCellEditor {
public:
    DeleteButtonEditor(wxWindow* parent) : wxGridCellEditor(parent) {
        Bind(wxEVT_BUTTON, &MyDeleteButtonEditor::OnButtonClick, this);
    }

    void OnButtonClick(wxCommandEvent& event) {
        wxGrid* grid = dynamic_cast<wxGrid*>(GetCellEditor());
        if (grid) {
            int row = grid->GetGridCursorRow();
            grid->DeleteRows(row);
        }
    }
};

