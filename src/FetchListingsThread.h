//
// Created by silav on 12-11-2020.
//

#ifndef GW2ITEMWATCH_FETCHLISTINGSTHREAD_H
#define GW2ITEMWATCH_FETCHLISTINGSTHREAD_H


/**
 * A joinable thread (so must be Wait()-ed) to fetch the requested price listings.
 */
class FetchListingsThread : public wxThread {
    wxEvtHandler *parent;
    std::vector<ItemListings> fetched_listings;
    std::vector<int32_t> item_ids;
public:
    enum class Mode {
        REFRESH,
        ADD
    };

    FetchListingsThread(wxEvtHandler* _parent, std::vector<int32_t> _item_ids, Mode m);
    wxThread::ExitCode Entry();
    Mode get_mode();

    std::vector<ItemListings> get_result_listings();

private:
    Mode mode;
};

wxDECLARE_EVENT(EVT_FETCH_SUCCESS, wxCommandEvent);
wxDECLARE_EVENT(EVT_FETCH_FAILURE, wxCommandEvent);

#endif //GW2ITEMWATCH_FETCHLISTINGSTHREAD_H
