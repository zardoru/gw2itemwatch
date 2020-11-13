#include <wx/wx.h>
#include "common.h"
#include "PricesAPI.h"
#include "FetchListingsThread.h"

wxDEFINE_EVENT(EVT_FETCH_SUCCESS, wxCommandEvent);
wxDEFINE_EVENT(EVT_FETCH_FAILURE, wxCommandEvent);

FetchListingsThread::FetchListingsThread(
        wxEvtHandler *_parent,
        std::vector<int32_t> _item_ids,
        FetchListingsThread::Mode m
) :
    wxThread(wxTHREAD_JOINABLE),
    parent(_parent),
    item_ids(std::move(_item_ids)),
    mode(m) {}

wxThread::ExitCode FetchListingsThread::Entry() {
    try {
        fetched_listings = PricesAPI::get_listings_by_id_list(item_ids);

        wxCommandEvent evt(EVT_FETCH_SUCCESS, wxID_ANY);
        evt.SetClientData(&fetched_listings);
        parent->AddPendingEvent(evt);

        return &fetched_listings;
    } catch(std::exception &e) {
        wxLogError("Failure fetching listings: %s", e.what());
    }

    wxCommandEvent evt(EVT_FETCH_FAILURE, wxID_ANY);
    evt.SetClientData(nullptr); /* just to be explicit */
    parent->AddPendingEvent(evt);
    return nullptr;
}

FetchListingsThread::Mode FetchListingsThread::get_mode() {
    return mode;
}

std::vector<ItemListings> FetchListingsThread::get_result_listings() {
    return fetched_listings;
}
