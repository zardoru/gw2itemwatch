#ifndef GW2ITEMWATCH_UPDATETIMER_H
#define GW2ITEMWATCH_UPDATETIMER_H


class MainWindow;

class UpdateTimer : public wxTimer {
    MainWindow* wnd;
public:
    explicit UpdateTimer(MainWindow* _wnd);
    void Notify();
};


#endif //GW2ITEMWATCH_UPDATETIMER_H
