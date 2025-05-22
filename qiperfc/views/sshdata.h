#ifndef SSHDATA_H
#define SSHDATA_H

#include "sshview.h"
#include "../src/wsclient.h"

struct SSHData {
    SSHView *sv;
    WSClient *ws;
};

#endif // SSHDATA_H
