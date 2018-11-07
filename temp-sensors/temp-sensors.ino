#define ARDUINO 18070

//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//
// SECURITY WARNING : uncomment ENABLE_CORS=0 in production!
//---------------------
// I used CORS policy to allow me write index.htm and app.js outside atmega controller from pc
// using atmega webapis
//
#define ENABLE_CORS 1

#define MACADDRESS 0x33, 0xcf, 0x8d, 0x9f, 0x5b, 0x89
#define MYIPADDR 10, 10, 4, 111
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 10, 10, 0, 6
#define MYGW 10, 10, 0, 1
#define LISTENPORT 80
#define MAX_HEADER_SIZE 80
// EDIT DebugMacros to set SERIAL_SPEED and enable/disable DPRINT_SERIAL

//
//==============================================================================

char *tempDescription[][2] = {
    {"28b5742407000084", "external"},
    {"2833bf3a050000ec", "bedroom"},
    {"28cc5d3a050000e3", "bathroom"},
    {"288aef140500008d", "lab"}};

//-------------------------

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mywifikey.h>

#include <UIPEthernet.h>
// edit UIPEthernet/utility/uipethernet-conf.h to customize
// - define UIP_CONF_UDP=0 to reduce flash size

#include <DPrint.h>
#include <Util.h>
using namespace SearchAThing::Arduino;

// header generted from debug-html files using gen-h script util
/*const char index_html[] PROGMEM = {

};

const char app_js[] PROGMEM = {
#include "app.js.h"
};*/

String header;
EthernetServer server = EthernetServer(LISTENPORT);

#define TEMPERATURE_INTERVAL_MS 5000
#define TEMPERATURE_ADDRESS_BYTES 8
#define ONE_WIRE_BUS 3

OneWire tempOneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&tempOneWire);

int temperatureDeviceCount = 0;
float *temperatures = NULL;
DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

void printFreeram()
{
  DPrint(F("Freeram : "));
  DPrintln((long)FreeMemorySum());
}

//
// SETUP
//
void setup()
{
  DPrintln(F("STARTUP"));

  printFreeram();

  header.reserve(MAX_HEADER_SIZE);
  SetupTemperatureDevices();

  uint8_t mac[6] = {MACADDRESS};
  uint8_t myIP[4] = {MYIPADDR};
  uint8_t myMASK[4] = {MYIPMASK};
  uint8_t myDNS[4] = {MYDNS};
  uint8_t myGW[4] = {MYGW};

  // dhcp
  //Ethernet.begin(mac);

  // static
  Ethernet.begin(mac, myIP, myDNS, myGW, myMASK);

  DPrint("my ip : ");
  for (int i = 0; i < 4; ++i)
  {
    DPrint(Ethernet.localIP()[i]);
    if (i != 3)
      DPrint('.');
  }
  DPrintln();

  server.begin();
}

//
// TEMPERATURE SETUP
//
void SetupTemperatureDevices()
{
  DS18B20.begin();
  temperatureDeviceCount = DS18B20.getDeviceCount();
  DPrint(F("temperature device count = "));
  DPrintIntln(temperatureDeviceCount);
  if (temperatureDeviceCount > 0)
  {
    temperatures = new float[temperatureDeviceCount];
    tempDevAddress = new DeviceAddress[temperatureDeviceCount];
    tempDevHexAddress = (char **)malloc(sizeof(char *) * temperatureDeviceCount);

    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
      tempDevHexAddress[i] = (char *)malloc(sizeof(char) * (TEMPERATURE_ADDRESS_BYTES * 2 + 1));
      DS18B20.getAddress(tempDevAddress[i], i);
      sprintf(tempDevHexAddress[i], "%02x%02x%02x%02x%02x%02x%02x%02x",
              tempDevAddress[i][0],
              tempDevAddress[i][1],
              tempDevAddress[i][2],
              tempDevAddress[i][3],
              tempDevAddress[i][4],
              tempDevAddress[i][5],
              tempDevAddress[i][6],
              tempDevAddress[i][7]);

      DPrint("sensor [");
      DPrintInt(i);
      DPrint("] address = ");
      DPrint(tempDevHexAddress[i]);
      DPrintln();

      DS18B20.setResolution(12);
    }
  }
  ReadTemperatures();
}

unsigned long lastTemperatureRead;

//
// TEMPERATURE READ
//
void ReadTemperatures()
{
  DS18B20.requestTemperatures();
  for (int i = 0; i < temperatureDeviceCount; ++i)
  {
    auto temp = DS18B20.getTempC(tempDevAddress[i]);
    DPrint(F("temperature sensor ["));
    DPrintInt(i);
    DPrint(F("] = "));
    DPrintln(temp, 4);
    temperatures[i] = temp;
  }
  lastTemperatureRead = millis();
}

const char *getTempDescription(const char *addr)
{
  auto k = sizeof(tempDescription) / (2 * sizeof(char *));
  for (int i = 0; i < k; ++i)
  {
    if (strcmp(tempDescription[i][0], addr) == 0)
    {
      return tempDescription[i][1];
    }
  }
  return "not defined";
}

#define CCTYPE_HTML 0
#define CCTYPE_JSON 1
#define CCTYPE_TEXT 2
#define CCTYPE_JS 3

void clientOk(EthernetClient &client, int type)
{
  client.println("HTTP/1.1 200 OK");
  switch (type)
  {
  case CCTYPE_HTML:
    client.println("Content-Type: text/html");
    break;

  case CCTYPE_JSON:
    client.println("Content-Type: application/json");
    break;

  case CCTYPE_TEXT:
    client.println("Content-Type: text/plain");
    break;

  case CCTYPE_JS:
    client.println("Content-Type: text/javascript");
    break;
  }

#if ENABLE_CORS == 1
  client.println("Access-Control-Allow-Origin: *");
  client.println();
#endif
}

//
// LOOP
//
void loop()
{
  size_t size;

  if (EthernetClient client = server.available())
  {

    while ((size = client.available()) > 0)
    {
      bool foundcmd = false;

      header = "";

      for (int i = 0; i < min(MAX_HEADER_SIZE, size); ++i)
      {
        char c = (char)client.read();

        if (c == '\r')
        {
          break;
        }
        header.concat(c);
      }

      foundcmd = false;

      if (temperatureDeviceCount > 0)
      {

        //--------------------------
        // /tempdevices
        //--------------------------
        String q = String("GET /tempdevices");
        if (header.indexOf(q) >= 0)
        {
          clientOk(client, CCTYPE_JSON);

          client.print("{\"tempdevices\":[");
          for (int i = 0; i < temperatureDeviceCount; ++i)
          {
            client.print('"');
            client.print(tempDevHexAddress[i]);
            client.print('"');

            if (i != temperatureDeviceCount - 1)
              client.print(',');
          }
          client.print("]}");

          client.stop();
          break;
        }

        //--------------------------
        // /temp/{id}
        //--------------------------
        q = String("GET /temp/");
        if (header.indexOf(q) >= 0)
        {
          bool found = false;
          if (header.length() - q.length() >= 8)
          {
            clientOk(client, CCTYPE_TEXT);

            for (int i = 0; i < temperatureDeviceCount; ++i)
            {
              if (strncmp(header.c_str() + q.length(), tempDevHexAddress[i],
                          2 * TEMPERATURE_ADDRESS_BYTES) == 0)
              {
                char tmp[20];
                FloatToString(tmp, temperatures[i], 6);

                client.print(tmp);

                found = true;
                break;
              }
            }
          }
          if (!found)
            client.print(F("not found"));

          client.stop();
          break;
        }
      }

      //--------------------------
      // /freeram
      //--------------------------      
      if (header.indexOf("GET /freeram") >= 0)
      {
        clientOk(client, CCTYPE_TEXT);

        client.print((long)FreeMemorySum());                
        client.stop();
        break;
      }

      //--------------------------
      // /app.js
      //--------------------------
      if (header.indexOf("GET /app.js") >= 0)
      {
        DPrintln(F("serving app.js"));

        clientOk(client, CCTYPE_JS);

        client.print(
#include "app.js.h"
        );

        client.stop();
        break;
      }

      //--------------------------
      // /
      //--------------------------
      if (header.indexOf("GET /") >= 0)
      {
        DPrintln(F("serving index.htm"));

        clientOk(client, CCTYPE_HTML);

        client.print(
#include "index.htm.h"
        );

        client.stop();
        break;
      }
    }
  }
  else if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)
  {
    printFreeram();

    ReadTemperatures();
  }
}
