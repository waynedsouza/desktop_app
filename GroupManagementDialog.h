#pragma once
#include <wx/wx.h>
#include "MyFrame.h"
class GroupManagementDialog : public wxDialog
{
    friend class MyFrameTest;
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif
public:
    GroupManagementDialog(wxWindow* parent, wxArrayString& groupNames);
    GroupManagementDialog(MyFrame* parent, wxArrayString& groupNames);

    wxCheckListBox* ForTestsGetGroupListBox()const{
        return groupListBox;
    }
    wxArrayString& ForTestsGetGroupNames()const{
        return m_groupNames;
    }
    void ForTestsOnEventGroup(wxCommandEvent& event , wxString callname){
        //std::cout << "\nGroupManagementDialog ForTestsOnEventGroup: " <<callname<< std::endl;
        if(callname=="OnAddGroup") OnAddGroup(event);
        else if(callname=="OnDeleteGroup") OnDeleteGroup(event);    
        else if(callname=="OnAssignUrls"){
             //std::cout << "In cond GroupManagementDialog ForTestsOnEventGroup: "<< std::endl;
            this->OnAssignUrls(event);
            //std::cout << "In2 cond GroupManagementDialog ForTestsOnEventGroup: "<< std::endl;
        }
        else{
            std::cout << "Else cond GroupManagementDialog ForTestsOnEventGroup: "<< std::endl;
        }
    }
    void ForTestsAddGroup(wxString groupName){
        if(!mainframe) return;
        if(!mainframe->getUnitTest()){
            std::cout << "Only for tests" << std::endl;
            return;
        }
        performAddGroup2(groupName);
    }

private:
    wxArrayString& m_groupNames;
    wxCheckListBox* groupListBox;
    MyFrame* mainframe;

    void OnAddGroup(wxCommandEvent& event);
    void OnDeleteGroup(wxCommandEvent& event);
    void OnAssignUrls(wxCommandEvent& event);
    void performAddGroup2(wxString newGroup); //split from OnAddGroup for tests

    //wxDECLARE_EVENT_TABLE();
};



