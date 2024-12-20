#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/event.h>
#include "Schema.h"
#include "MyFrame.h"
class GroupSchedulerDialog : public wxDialog
{
public:
    GroupSchedulerDialog(MyFrame* parent, const wxArrayString& groupNames)
        : wxDialog(dynamic_cast<wxWindow*>(parent), wxID_ANY, "Schedule Group", wxDefaultPosition, wxSize(400, 350)),mainframe(parent)
    {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Group Selection
        wxStaticText* groupLabel = new wxStaticText(this, wxID_ANY, "Select Group:");
        groupChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, groupNames);
        mainSizer->Add(groupLabel, 0, wxALL, 5);
        mainSizer->Add(groupChoice, 0, wxALL | wxEXPAND, 5);

        // Radio buttons for choosing between period and date scheduling
        scheduleType = new wxRadioBox(this, wxID_ANY, "Schedule Type", wxDefaultPosition, wxDefaultSize,
            { "By Period", "By Date" }, 1, wxRA_SPECIFY_ROWS);
        mainSizer->Add(scheduleType, 0, wxALL | wxEXPAND, 5);

        // Period Scheduling (Spinners for months and days)
        wxStaticText* periodLabel = new wxStaticText(this, wxID_ANY, "Schedule for a period:");
        wxStaticText* monthLabel = new wxStaticText(this, wxID_ANY, "Months:");
        wxStaticText* dayLabel = new wxStaticText(this, wxID_ANY, "Days:");
        monthSpinner = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 12);
        daySpinner = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31);
        mainSizer->Add(periodLabel, 0, wxALL, 5);

        wxBoxSizer* periodSizer = new wxBoxSizer(wxHORIZONTAL);
        periodSizer->Add(monthLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        periodSizer->Add(monthSpinner, 1, wxALL, 5);
        periodSizer->Add(dayLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        periodSizer->Add(daySpinner, 1, wxALL, 5);
        mainSizer->Add(periodSizer, 0, wxALL | wxEXPAND, 5);

        // Date Scheduling (Date picker)
        wxStaticText* dateLabel = new wxStaticText(this, wxID_ANY, "Pick a date:");
        datePicker = new wxDatePickerCtrl(this, wxID_ANY);
        mainSizer->Add(dateLabel, 0, wxALL, 5);
        mainSizer->Add(datePicker, 0, wxALL | wxEXPAND, 5);

        // OK and Cancel buttons
        mainSizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_RIGHT, 10);

        SetSizer(mainSizer);

        // Bind events for mutual exclusivity logic
        Bind(wxEVT_RADIOBOX, &GroupSchedulerDialog::OnScheduleTypeChanged, this);
        Bind(wxEVT_BUTTON, &GroupSchedulerDialog::OnSave, this, wxID_OK);
       auto evt = wxCommandEvent();
        OnScheduleTypeChanged(evt);
    }

    // Retrieve group name
    wxString GetSelectedGroup() const { return groupChoice->GetStringSelection(); }

    // Retrieve period (if scheduling by period)
    int GetMonths() const { return monthSpinner->GetValue(); }
    int GetDays() const { return daySpinner->GetValue(); }

    // Retrieve selected date (if scheduling by date)
    wxDateTime GetSelectedDate() const { return datePicker->GetValue(); }

    // Check if scheduling is by period or by date
    bool IsScheduleByPeriod() const { return scheduleType->GetSelection() == 0; }

private:
    // Widgets
    wxChoice* groupChoice;
    wxRadioBox* scheduleType;
    wxSpinCtrl* monthSpinner;
    wxSpinCtrl* daySpinner;
    wxDatePickerCtrl* datePicker;
    MyFrame* mainframe;

    // Event handler to toggle between period and date scheduling
    
    void OnScheduleTypeChanged(wxCommandEvent& event)
    {
        bool isPeriod = (scheduleType->GetSelection() == 0);
        monthSpinner->Enable(isPeriod);
        daySpinner->Enable(isPeriod);
        datePicker->Enable(!isPeriod);
    }
    // Event handler for Save button
    
    void OnSave(wxCommandEvent & event)
    {
        // Check if the selected group is empty
        if (GetSelectedGroup().empty())
        {
            wxLogError("GroupSchedulerDialog::OnSave No group selected. Save operation aborted.");
            return;
        }

        // Check if scheduling by period but both months and days are zero
        if (IsScheduleByPeriod() && GetMonths() == 0 && GetDays() == 0)
        {
            wxLogError("GroupSchedulerDialog::OnSave Invalid period: Both months and days cannot be zero.");
            return;
        }

        // Check if scheduling by date but the date is not valid
        if (!IsScheduleByPeriod() && GetSelectedDate().IsValid() && GetSelectedDate().FormatISODate().empty())
        {
            wxLogError("GroupSchedulerDialog::OnSave Invalid date: No schedule date selected.");
            return;
        }
        wxString message;
        if (IsScheduleByPeriod())
        {
            message = wxString::Format("Group: %s\nSchedule By Period\nMonths: %d\nDays: %d",
                GetSelectedGroup(), GetMonths(), GetDays());
        }
        else
        {
            message = wxString::Format("Group: %s\nSchedule By Date\nDate: %s",
                GetSelectedGroup(), GetSelectedDate().FormatISODate());
        }

        // Use wxMessageDialog to allow "Confirm" and "Cancel"
        wxMessageDialog dialog(this, message, "Schedule Details", wxYES_NO  | wxICON_INFORMATION);
        dialog.SetYesNoLabels("Confirm", "Cancel");

        int response = dialog.ShowModal();
        if (response == wxID_YES)
        {
            std::unique_ptr<GroupData> groupData = std::make_unique<GroupData>();

            // Populate the groupData struct from the dialog controls
            groupData->name = GetSelectedGroup().ToStdString();
            groupData->is_subscribed = true;

            if (IsScheduleByPeriod())
            {
                groupData->period_days = GetDays();
                groupData->period_months = GetMonths();
                groupData->schedule_date = "";
            }
            else
            {
                groupData->period_days = 0;
                groupData->period_months = 0;
                groupData->schedule_date = GetSelectedDate().FormatISODate().ToStdString();
            }

           //groupData->created_at = wxDateTime::Now().FormatISOCombined().ToStdString();
            groupData->updated_at = wxDateTime::Now().FormatISOCombined().ToStdString();

            if (!mainframe->saveSchedule(std::move(groupData)))
            {
                wxLogError("Failed to save schedule.");
            }
            else
            {
                wxLogMessage("Schedule saved successfully.");
            }
        }
        else if (response == wxID_NO || response == wxID_CANCEL)
        {
            wxLogMessage("Schedule save operation was canceled.");
        }
    }

};


