#ifndef GW2ITEMWATCH_FRAME_H
#define GW2ITEMWATCH_FRAME_H

#include <map>
#include <wx/fileconf.h>
#include <wx/dataview.h>
#include "Database.h"
#include "PricesAPI.h"
#include "ListingsModel.h"
#include "FetchListingsThread.h"
#include "UpdateTimer.h"

class MainWindow : public wxFrame {
    Database *db;

    std::vector<ItemListings> listings_storage;
    wxDataViewCtrl *listings_ctrl;
    wxObjectDataPtr<ListingsModel> listings_model;
    wxFileConfig cfg;
    FetchListingsThread *fetch_listings_thread;
    UpdateTimer fetch_update_timer;
    std::vector<wxAcceleratorEntry> shortcuts;
    long refresh_interval; /* in seconds */
public:
    MainWindow(const wxString &title, const wxPoint &pos, const wxSize &size, Database *_db);

    void RequestListingsUpdate(wxCommandEvent& evt);

private:
    void OnExit(wxCommandEvent& evt);
    void OnAddItem(wxCommandEvent& evt);
    wxDECLARE_EVENT_TABLE();

    void build_data_model();

    void make_menus();

    void FetchThreadFinished(wxCommandEvent &evt);

    void load_configuration();

    void ExpandFirstLevel();
};


#endif //GW2ITEMWATCH_FRAME_H
