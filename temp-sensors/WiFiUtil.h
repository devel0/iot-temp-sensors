#ifndef _WIFI_UTIL_H_
#define _WIFI_UTIL_H_

#include "Config.h"

enum CCTypes
{
  HTML,
  JSON,
  TEXT,
  JAVASCRIPT
};

void clientEnd(EthernetClient &client);
void clientOk(EthernetClient &client, CCTypes type);


#endif