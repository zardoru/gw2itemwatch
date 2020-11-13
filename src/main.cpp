#include <wx/wx.h>
#include "common.h"
#include "main.h"
#include "frame.h"

bool Application::OnInit() {
    db = std::make_unique<Database>();
    db->open("listings.db");

    auto wnd = new MainWindow("gw2itemwatch", wxDefaultPosition, wxDefaultSize, db.get());
    wnd->Show();
    return true;
}


wxIMPLEMENT_APP(Application);
