#include <wx/wx.h>
#include "common.h"
#include "frame.h"
#include "UpdateTimer.h"

UpdateTimer::UpdateTimer(MainWindow *_wnd) : wnd(_wnd) {}

void UpdateTimer::Notify() {
    wxTimer::Notify();

    wxCommandEvent evt;
    wnd->RequestListingsUpdate(evt);
}
