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

// choose one of follow two interface
#define USE_ENC28J60
//#define USE_W5500

#define MACADDRESS 0x33, 0xcf, 0x8d, 0x9f, 0x5b, 0x89
#define MYIPADDR 10, 10, 4, 111
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 10, 10, 0, 6
#define MYGW 10, 10, 0, 1
#define LISTENPORT 80
#define MAX_HEADER_SIZE 80
#define ONE_WIRE_BUS 3
// EDIT DebugMacros to set SERIAL_SPEED and enable/disable DPRINT_SERIAL

// tune follow watching at /info api "history_size"
// example
// - history_size=132 if want to keep last 36 hours then
// - TEMPERATURE_HISTORY_INTERVAL_SEC = 36 * 60 * 60 / 132 = 982
// this mean that a sample will be recorded each 982 seconds = 16min ( foreach temp device )
#define TEMPERATURE_HISTORY_INTERVAL_SEC 982

//
//==============================================================================

#include <Arduino.h>

unsigned long lastTemperatureHistoryRecord;
uint16_t temperatureHistoryFillCnt = 0;
#define TEMPERATURE_HISTORY_FREERAM_THRESHOLD 200
uint16_t TEMPERATURE_HISTORY_SIZE = 0;

#define TEMPERATURE_INTERVAL_MS 5000
unsigned long lastTemperatureRead;

//-------------------------

#include <OneWire.h>
#include <DallasTemperature.h>
#include <mywifikey.h>

#ifdef USE_ENC28J60
#include <UIPEthernet.h>
// edit UIPEthernet/utility/uipethernet-conf.h to customize
// - define UIP_CONF_UDP=0 to reduce flash size
#endif

#ifdef USE_W5500
#include <Ethernet.h>
#endif

#include <DPrint.h>
#include <Util.h>
using namespace SearchAThing::Arduino;

unsigned char *header;
EthernetServer server = EthernetServer(LISTENPORT);

#define TEMPERATURE_ADDRESS_BYTES 8

OneWire tempOneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&tempOneWire);

int temperatureDeviceCount = 0;
float *temperatures = NULL;    // current temp
DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

// stored as signed int8 ( ie. float rounded to int8 )
int8_t **temperatureHistory = NULL;
uint16_t temperatureHistoryOff = 0;

void printFreeram()
{
  DPrintF(F("Freeram : "));
  DPrintLongln(FreeMemorySum());
}

//
// SETUP
//
void setup()
{
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  delay(1);

  header = (unsigned char *)malloc(MAX_HEADER_SIZE + 1);

  DPrintFln(F("STARTUP"));

  lastTemperatureRead = lastTemperatureHistoryRecord = millis();

  printFreeram();

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

  DPrintF(F("my ip : "));
  for (int i = 0; i < 4; ++i)
  {
    DPrintInt16(Ethernet.localIP()[i]);
    if (i != 3)
      DPrintChar('.');
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
  DPrintF(F("temperature device count = "));
  DPrintInt16ln(temperatureDeviceCount);
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

      DPrintF(F("sensor ["));
      DPrintInt16(i);
      DPrintF(F("] address = "));
      DPrintStr(tempDevHexAddress[i]);
      DPrintln();

      DS18B20.setResolution(12);
    }

    printFreeram();
    auto freeram = FreeMemorySum();
    TEMPERATURE_HISTORY_SIZE = (freeram - TEMPERATURE_HISTORY_FREERAM_THRESHOLD) / temperatureDeviceCount;

    auto required_size = sizeof(int8_t) * temperatureDeviceCount * TEMPERATURE_HISTORY_SIZE;

    temperatureHistory = (int8_t **)malloc(sizeof(int8_t *) * temperatureDeviceCount);

    DPrintF(F("temperature history size: "));
    DPrintUInt16(TEMPERATURE_HISTORY_SIZE);
    DPrintF(F(" = "));
    DPrintULong((unsigned long)TEMPERATURE_HISTORY_SIZE * (TEMPERATURE_HISTORY_INTERVAL_SEC) / 60 / 60);
    DPrintFln(F(" hours"));
    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
      temperatureHistory[i] = (int8_t *)malloc(sizeof(int8_t) * TEMPERATURE_HISTORY_SIZE);
    }
    printFreeram();
  }
  ReadTemperatures();
}

//
// TEMPERATURE READ
//
void ReadTemperatures()
{
  DS18B20.requestTemperatures();
  for (int i = 0; i < temperatureDeviceCount; ++i)
  {
    auto temp = DS18B20.getTempC(tempDevAddress[i]);
    DPrintF(F("temperature sensor ["));
    DPrintInt16(i);
    DPrintF(F("] = "));
    DPrintFloatln(temp, 4);
    temperatures[i] = temp;
  }

  lastTemperatureRead = millis();
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
#endif

  client.println();
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
      header[0] = 0;
      {
        int i = 0;
        DPrintF(F("SZ:"));
        DPrintInt16ln(size);
        auto lim = min(MAX_HEADER_SIZE, size);
        while (i < lim)
        {
          char c = (char)client.read();

          if (c == '\r')
          {
            break;
          }
          header[i++] = c;
        }
        header[i] = 0;
      }    
      while (client.available()) client.read(); // consume remaining header  
      

      if (strlen(header) < 5 || strncmp(header, "GET /", 5) < 0)
      {
        client.stop();
        break;
      }

      //--------------------------
      // /tempdevices
      //--------------------------
      if (strncmp(header, "GET /tempdevices", 16) == 0)
      {
        DPrintFln(F("temp devices"));
        clientOk(client, CCTYPE_JSON);

        client.print(F("{\"tempdevices\":["));
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
      if (strncmp(header, "GET /temp/", 10) == 0)
      {
        auto hbasesize = 10; // "GET /temp/"
        bool found = false;

        if (strlen(header) - hbasesize >= 8)
        {
          clientOk(client, CCTYPE_TEXT);

          for (int i = 0; i < temperatureDeviceCount; ++i)
          {
            if (strncmp(header + hbasesize, tempDevHexAddress[i],
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

      //--------------------------
      // /temphistory
      //--------------------------
      if (strncmp(header, "GET /temphistory", 16) == 0)
      {
        clientOk(client, CCTYPE_JSON);

        DPrintF(F("temperatureHistoryFillCnt:"));
        DPrintInt16ln(temperatureHistoryFillCnt);
        DPrintF(F("temperatureHistoryOff:"));
        DPrintUInt16ln(temperatureHistoryOff);

        client.print('[');
        for (int i = 0; i < temperatureDeviceCount; ++i)
        {
          client.print(F("{\""));
          client.print(tempDevHexAddress[i]);
          client.print(F("\":["));
          auto j = (temperatureHistoryFillCnt == TEMPERATURE_HISTORY_SIZE) ? temperatureHistoryOff : 0;
          auto size = min(temperatureHistoryFillCnt, TEMPERATURE_HISTORY_SIZE);
          for (int k = 0; k < size; ++k)
          {
            if (j == TEMPERATURE_HISTORY_SIZE)
              j = 0;
            client.print(temperatureHistory[i][j++]);
            if (k < size - 1)
              client.print(',');
          }
          client.print(F("]}"));
          if (i != temperatureDeviceCount - 1)
            client.print(',');
        }
        client.print(']');

        client.stop();
        break;
      }

      //--------------------------
      // /info
      //--------------------------
      if (strncmp(header, "GET /info", 9) == 0)
      {
        clientOk(client, CCTYPE_JSON);

        client.print('{');

        client.print(F("\"freeram\":"));
        client.print((long)FreeMemorySum());

        client.print(F(", \"history_size\":"));
        client.print(TEMPERATURE_HISTORY_SIZE);

        client.print(F(", \"history_interval_sec\":"));
        client.print(TEMPERATURE_HISTORY_INTERVAL_SEC);

        client.print('}');

        client.stop();
        break;
      }

      //--------------------------
      // /app.js
      //--------------------------
      if (strncmp(header, "GET /app.js", 11) == 0)
      {
        DPrintFln(F("serving app.js"));

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
      if (strncmp(header, "GET / ", 6) == 0 || strncmp(header, "GET /index.htm", 14) == 0)
      {
        DPrintFln(F("serving index.htm"));

        clientOk(client, CCTYPE_HTML);

        client.print(
#include "index.htm.h"
        );

        client.stop();
        break;
      }
    }
  }

  if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)
  {
    printFreeram();
    ReadTemperatures();
  }

  if (temperatureHistory != NULL &&
      (TimeDiff(lastTemperatureHistoryRecord, millis()) > 1000UL * TEMPERATURE_HISTORY_INTERVAL_SEC))
  {
    if (temperatureHistoryFillCnt < TEMPERATURE_HISTORY_SIZE)
      ++temperatureHistoryFillCnt;

    if (temperatureHistoryOff == TEMPERATURE_HISTORY_SIZE)
      temperatureHistoryOff = 0;

    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
      int8_t t = trunc(round(temperatures[i]));
      temperatureHistory[i][temperatureHistoryOff] = t;
    }
    ++temperatureHistoryOff;
    lastTemperatureHistoryRecord = millis();
  }
}
