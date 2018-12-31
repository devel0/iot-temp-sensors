#include "WiFiUtil.h"

void clientEnd(EthernetClient &client)
{
#if ENABLE_CORS == 1
  client.println("Access-Control-Allow-Origin: *");
#endif

  client.println("Connection: close");
  client.println();
}

void clientOk(EthernetClient &client, CCTypes type)
{
  client.println("HTTP/1.1 200 OK");
  switch (type)
  {
  case HTML:
    client.println("Content-Type: text/html");
    break;

  case JSON:
    client.println("Content-Type: application/json");
    break;

  case TEXT:
    client.println("Content-Type: text/plain");
    break;

  case JAVASCRIPT:
    client.println("Content-Type: text/javascript");
    break;
  }

  clientEnd(client);
}
