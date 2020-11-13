#ifndef GW2ITEMWATCH_MAIN_H
#define GW2ITEMWATCH_MAIN_H

#include "Database.h"

class Application : public wxApp {
    std::unique_ptr<Database> db;
public:
    bool OnInit();
};

#endif //GW2ITEMWATCH_MAIN_H
