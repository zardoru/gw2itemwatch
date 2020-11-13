#include <wx/wx.h>
#include <wx/wfstream.h>
#include <nlohmann/json.hpp>
#include "common.h"
#include "frame.h"

using json = nlohmann::json;

enum {
    ID_ADD_ITEM,
    ID_REFRESH_ITEMS
};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(ID_ADD_ITEM, MainWindow::OnAddItem)
    EVT_MENU(ID_REFRESH_ITEMS, MainWindow::RequestListingsUpdate)
    EVT_MENU(wxID_EXIT, MainWindow::OnExit)
    EVT_COMMAND(wxID_ANY, EVT_FETCH_FAILURE, MainWindow::FetchThreadFinished)
    EVT_COMMAND(wxID_ANY, EVT_FETCH_SUCCESS, MainWindow::FetchThreadFinished)

wxEND_EVENT_TABLE()

MainWindow::MainWindow(
        const wxString &title,
        const wxPoint &pos,
        const wxSize &size,
        Database *_db
)
    : wxFrame(NULL, wxID_ANY, title, pos, size), db(_db),
      cfg("gw2itemwatch (by zardoru)", "wyrmin", "gw2iw.config"),
      fetch_update_timer(this),
      fetch_listings_thread(nullptr) {

    load_configuration();

    /* menu */
    make_menus();

    /* data model */
    build_data_model();

    /* refresh every so often */
    fetch_update_timer.Start(refresh_interval * 1000);

    /* add the items already in the database */
    auto items = db->get_items_in_database();
    fetch_listings_thread = new FetchListingsThread(this, items, FetchListingsThread::Mode::ADD);
    fetch_listings_thread->Run();
}

void MainWindow::make_menus() {
    auto file_menu = new wxMenu;

    file_menu->Append(ID_ADD_ITEM, _("&Add Item...\tCtrl+A"));
    file_menu->Append(ID_REFRESH_ITEMS, _("&Refresh listings\tCtrl+R"));
    file_menu->AppendSeparator();
    file_menu->Append(wxID_EXIT);

    auto menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _("&Database"));

    SetMenuBar(menu_bar);

    CreateStatusBar();
    SetStatusText(_("Ready"));

    shortcuts.resize(2);
    shortcuts[0].Set(wxACCEL_CTRL, (int) 'R', ID_REFRESH_ITEMS);
    shortcuts[1].Set(wxACCEL_CTRL, (int) 'A', ID_ADD_ITEM);
    wxAcceleratorTable tbl(shortcuts.size(), shortcuts.data());
    SetAcceleratorTable(tbl);
}

void MainWindow::OnExit(wxCommandEvent &evt) {
    wxFileOutputStream out(wxFileConfig::GetLocalFileName("gw2fw"));
    cfg.Save(out);
    Close(true);
}

void MainWindow::build_data_model() {
    listings_model = new ListingsModel;
    listings_ctrl = new wxDataViewCtrl(this, wxID_ANY);
    listings_ctrl->AssociateModel(listings_model.get());

    /* make columns -- these correspond to ListingsModel.cpp columns. */

    listings_ctrl->AppendColumn(
            new wxDataViewColumn(
                    "Group",
                    new wxDataViewTextRenderer("string"),
                    (unsigned int)ListingColumns::COLUMN_TITLE
            )
    );


    listings_ctrl->AppendColumn(
            new wxDataViewColumn(
                "Item ID",
                new wxDataViewTextRenderer("long"),
                (unsigned int)ListingColumns::COLUMN_ITEM_ID
            )
    );

    listings_ctrl->AppendColumn(
            new wxDataViewColumn(
                    "Supply",
                    new wxDataViewTextRenderer("long"),
                    (unsigned int)ListingColumns::COLUMN_SUPPLY
            )
    );

    listings_ctrl->AppendColumn(
            new wxDataViewColumn(
                    "Unit Price",
                    new wxDataViewTextRenderer("string"),
                    (unsigned int)ListingColumns::COLUMN_UNIT_PRICE
            )
    );

    listings_ctrl->AppendColumn(
            new wxDataViewColumn(
                    "Total Coin Circulation",
                    new wxDataViewTextRenderer("string"),
                    (unsigned int)ListingColumns::COLUMN_TOTAL
            )
    );
}

void MainWindow::OnAddItem(wxCommandEvent &evt) {

    if (fetch_listings_thread) {
        wxMessageBox(_("Can't add - wait until the previous operation is done."));
        return;
    }

    wxTextEntryDialog dlg(
        this,
        _("Enter an item ID."),
        _("New Item ID...")
    );

    auto res = dlg.ShowModal();
    if (res == wxID_OK) {
        auto result = strtol(dlg.GetValue(), nullptr, 10);
        if (result <= 0) {
            wxMessageBox("Please enter a positive number.");
            return;
        }

        if (fetch_listings_thread) {
            /* gotta add this in here twice just in case -
             * i'm not sure if notify is called on a separate thread
             */
            wxMessageBox(_("Can't add - wait until the previous operation is done."));
            return;
        }

        if (!db->item_exists(result)) {
            SetStatusText(_("Wait just a moment - adding a new item to your database."));
            auto json_data = PricesAPI::get_item_json_by_id(result);
            auto tree = json::parse(json_data);

            /* commit it to database */
            db->add_item(result, json_data);
        }

        /* get the listings */
        fetch_listings_thread = new FetchListingsThread(this, {result}, FetchListingsThread::Mode::ADD);
        fetch_listings_thread->Run();
    }
}

void MainWindow::RequestListingsUpdate(wxCommandEvent& evt) {
    if (fetch_listings_thread != nullptr) {
        SetStatusText(_("Tried to refresh listings, but the thread is busy."));
        return; /* still working. is not null while working */
    }

    /* store listing IDs into a vector */
    std::vector<int32_t> item_ids;
    item_ids.reserve(listings_storage.size());
    for (auto &l: listings_storage) {
        item_ids.push_back(l.item_id);
    }

    fetch_listings_thread = new FetchListingsThread(this, item_ids, FetchListingsThread::Mode::REFRESH);
    fetch_listings_thread->Run();
    SetStatusText(_("Refreshing listings data..."));
}

void MainWindow::FetchThreadFinished(wxCommandEvent &evt) {
    fetch_listings_thread->Wait(); /* let it complete, since it is joinable */

    auto vec = evt.GetClientData();
    if (vec) { // success
        if (fetch_listings_thread->get_mode() == FetchListingsThread::Mode::REFRESH) {
            listings_model->RemoveAllItemListings(); /* needs to be done before storage gets updated */
            listings_storage = fetch_listings_thread->get_result_listings();

            // copy names back into the refreshed listings
            for (auto &it: listings_storage) {
                it.item_name = db->get_item_name(it.item_id);
            }

            listings_model->copy_listings(listings_storage);

            auto new_status = wxString::Format(_("[%s] Refreshed listings data!"), wxDateTime::Now().FormatTime());
            SetStatusText(new_status);

            ExpandFirstLevel();

        } else if (fetch_listings_thread->get_mode() == FetchListingsThread::Mode::ADD) {
            auto new_data = fetch_listings_thread->get_result_listings();

            for (auto &l: new_data) {
                l.item_name = db->get_item_name(l.item_id); /* give it a name -- it not provided by listings... */
                listings_storage.push_back(l);
                listings_model->add_listing(listings_storage.back());
            }

            SetStatusText(_("Added a new item to the listings."));

            ExpandFirstLevel();
        }
    } else {
        SetStatusText(_("The thread failed to perform the requested operation."));
    }

    delete fetch_listings_thread;
    fetch_listings_thread = nullptr;
}

void MainWindow::ExpandFirstLevel() {
    wxDataViewItemArray arr;
    listings_model->GetChildren(wxDataViewItem(nullptr), arr);

    for (auto &l: arr) {
        listings_ctrl->Expand(l);
    }
}

void MainWindow::load_configuration() {
    cfg.EnableAutoSave();
    if (!cfg.Read("refresh_interval", &refresh_interval)) {
        refresh_interval = 120; // every 2 minutes
    }
}
