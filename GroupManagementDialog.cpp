#include "GroupManagementDialog.h"


GroupManagementDialog::GroupManagementDialog(MyFrame* parent, wxArrayString& groupNames)
    : wxDialog(dynamic_cast<wxWindow*>(parent), wxID_ANY, "Manage Groups", wxDefaultPosition, wxSize(400, 300)),
    m_groupNames(groupNames), mainframe(parent)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    // Show current groups in a checklist
    groupListBox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, groupNames);
    sizer->Add(groupListBox, 1, wxALL | wxEXPAND, 5);
    //if (debugMode) wxLogMessage("GroupManagementDialog constructor ");
    // Buttons for adding or deleting groups
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addGroupBtn = new wxButton(this, wxID_ANY, "Add Group");
    wxButton* deleteGroupBtn = new wxButton(this, wxID_ANY, "Delete Group");
    wxButton* assignGroupBtn = new wxButton(this, wxID_ANY, "Assign to Group");
    btnSizer->Add(addGroupBtn, 1, wxALL, 5);
    btnSizer->Add(deleteGroupBtn, 1, wxALL, 5);
    btnSizer->Add(assignGroupBtn, 1, wxALL, 5);

    sizer->Add(btnSizer, 0, wxCENTER);

    // Bind the events for buttons
    addGroupBtn->Bind(wxEVT_BUTTON, &GroupManagementDialog::OnAddGroup, this);
    deleteGroupBtn->Bind(wxEVT_BUTTON, &GroupManagementDialog::OnDeleteGroup, this);
    assignGroupBtn->Bind(wxEVT_BUTTON, &GroupManagementDialog::OnAssignUrls, this);
    SetSizerAndFit(sizer);
   // if (debugMode) wxLogMessage("GroupManagementDialog out constructor ");
}

// Event handlers
void GroupManagementDialog::performAddGroup2(wxString newGroup){
     if (mainframe && mainframe->getIndexOfGroup(newGroup) == wxNOT_FOUND) {
            m_groupNames.Add(newGroup);
            groupListBox->Append(newGroup);  // Update the listbox
            mainframe->getDb()->AddGroupToDB(newGroup);
            wxLogMessage("Main frame pointer IN addinggroup: " + newGroup);
            //mainframe->addGroup(newGroup);
            wxLogMessage("Main frame pointer IN addinggroup to notebook: " + newGroup);
            auto grdata = mainframe->getDb()->LoadGroupByName(newGroup);
            mainframe->AddGroupToNotebook(newGroup , *grdata);
            wxLogMessage("Main frame pointer IN addinggroup refresh: " + newGroup);
            //mainframe->refreshGroupDisplay();
        }
        else {
            wxLogError("Main frame pointer is null.Group name already exists: " + newGroup);
        }

}
void GroupManagementDialog::OnAddGroup(wxCommandEvent& event)
{
    if (debugMode) wxLogMessage("GroupManagementDialog OnAddGroup  ");
    wxString newGroup = wxGetTextFromUser("Enter new group name:", "Add Group", "", this);
    if (debugMode) wxLogMessage("GroupManagementDialog OnAddGroup  got text "+ newGroup);
    if (!newGroup.IsEmpty())
    {       
        performAddGroup2(newGroup);
        /*if (mainframe && mainframe->getIndexOfGroup(newGroup) == wxNOT_FOUND) {
            m_groupNames.Add(newGroup);
            groupListBox->Append(newGroup);  // Update the listbox
            wxLogMessage("Main frame pointer IN addinggroup: " + newGroup);
            //mainframe->addGroup(newGroup);
            wxLogMessage("Main frame pointer IN addinggroup to notebook: " + newGroup);
            mainframe->AddGroupToNotebook(newGroup);
            wxLogMessage("Main frame pointer IN addinggroup refresh: " + newGroup);
            mainframe->refreshGroupDisplay();
        }
        else {
            wxLogError("Main frame pointer is null.Group name already exists: " + newGroup);
        }*/
    }
}

void GroupManagementDialog::OnDeleteGroup(wxCommandEvent& event)
{
    if (debugMode) wxLogMessage("GroupManagementDialog OnDeleteGroup  ");
    

    // Loop through each item in the checklist in reverse order to avoid index shifting
    for (int i = groupListBox->GetCount() - 1; i >= 0; --i) {
        if (groupListBox->IsChecked(i)) {
            if (mainframe) {
                //mainframe->doDeletePage(m_groupNames[i]);
                wxString groupname = m_groupNames[i];
                
                mainframe->UpdateUrlGridOnGroupDelete(groupname);
                mainframe->RemoveGroupFromNotebook(groupname);
                mainframe->getDb()->RemoveGroup(groupname);
                            
            }
            //m_groupNames.RemoveAt(i);
            groupListBox->Delete(i);  // Remove from listbox
        }
    }
}



void GroupManagementDialog::OnAssignUrls(wxCommandEvent& event) {    
//std::cout << "\nGroupManagementDialog onAssignUrls: INSIDE core" << std::endl;

    std::shared_ptr<DataGridItem> context_item = mainframe->getContextItem();
    context_item->groupNames.clear();
    if (context_item == nullptr) {
        std::cout << "\ncontext_item was null" << std::endl;
        if (debugMode) wxLogMessage("GroupManagementDialog onAssignUrls: context_item was null");
        return;
    }
    //std::cout << "GroupManagementDialog onAssignUrls: " <<context_item->uuid <<" "<< context_item->url <<std::endl;
    if (debugMode) wxLogMessage("GroupManagementDialog onAssignUrls uuid:%s url:%s", context_item->uuid, context_item->url);
    for (int i = groupListBox->GetCount() - 1; i >= 0; --i) {
       // std::cout << "GroupManagementDialog onAssignUrls: " <<i << std::endl;
        if (groupListBox->IsChecked(i)) {
            //std::cout << "GroupManagementDialog onAssignUrls:is checked " << std::endl;
            // Add group name only if it does not already exist in groupNames
            if (std::find(context_item->groupNames.begin(), context_item->groupNames.end(), m_groupNames[i]) == context_item->groupNames.end()) {
                context_item->groupNames.push_back(m_groupNames[i]);
               // std::cout << "GroupManagementDialog onAssignUrls: " << m_groupNames[i] << std::endl;
                if (debugMode) wxLogMessage("Assigned group: %s to context_item %s", m_groupNames[i] , context_item->uuid);
            }
        }
    }
    if (context_item->groupNames.empty()) {
        return;
    }
    mainframe->setUpGroupItems(context_item);
    mainframe->displayGroupItems(context_item);
}



