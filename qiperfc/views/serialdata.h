#ifndef SERIALDATA_H
#define SERIALDATA_H

#include "serialview.h"
#include "../src/wsclient.h"

struct SerialData {
    SerialView *sv;
    WSClient *ws;
};

#endif // SERIALDATA_H
