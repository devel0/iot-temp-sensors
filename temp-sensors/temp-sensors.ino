#define ARDUINO 18070

#include <Arduino.h>

#include "Config.h"
#include "WiFiUtil.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mywifikey.h>
#include <PinChangeInterrupt.h>

#include <DPrint.h>
#include <Util.h>
using namespace SearchAThing::Arduino;

unsigned long lastTemperatureRead;
unsigned long lastTemperatureHistoryRecord;
int freeram_min = -1;

uint16_t temperatureHistoryFillCnt = 0;
uint16_t temperatureHistorySize = 0; // computed
int temperatureDeviceCount = 0;
float *temperatures = NULL;    // current temp
DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

// stored as signed int8 ( ie. float rounded to int8 )
int8_t **temperatureHistory = NULL;
uint16_t temperatureHistoryOff = 0;
uint16_t temperatureHistoryIntervalSec = 5 * 60; // computed

#define TEMPERATURE_ADDRESS_BYTES 8

unsigned char *header;
EthernetServer server = EthernetServer(LISTENPORT);

OneWire tempOneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&tempOneWire);

void printFreeram()
{
  DPrintF(F("Freeram : "));
  DPrintLongln(FreeMemorySum());
}

volatile byte interrupted = 0;
void handleExtInterrupt()
{
  ++interrupted;
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

#ifdef USE_EXTERNAL_INTERRUPT
  DPrintFln(F("setting interrupt handler"));
  // external interrupt
  pinMode(EXTERNAL_INTERRUPT_PIN, EXTERNAL_INTERRUPT_INPUT_MODE);
  attachPCINT(digitalPinToPCINT(EXTERNAL_INTERRUPT_PIN), handleExtInterrupt, RISING);
#endif

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

    temperatureHistory = (int8_t **)malloc(sizeof(int8_t *) * temperatureDeviceCount);

    printFreeram();
    auto freeram = FreeMemorySum();
    auto threshold = TEMPERATURE_HISTORY_FREERAM_THRESHOLD;
    auto availram = freeram - threshold;

    temperatureHistorySize = availram / temperatureDeviceCount;

    auto backloghr = (unsigned long)TEMPERATURE_HISTORY_BACKLOG_HOURS;
    temperatureHistoryIntervalSec = backloghr * 60 * 60 / temperatureHistorySize;

    DPrintF(F("avail ram = "));
    DPrintUInt16ln(availram);
    DPrintF(F("temperatureDeviceCount = "));
    DPrintUInt16ln(temperatureDeviceCount);
    DPrintF(F("temperatureHistoryIntervalSec = "));
    DPrintUInt16ln(temperatureHistoryIntervalSec);

    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
      temperatureHistory[i] = (int8_t *)malloc(sizeof(int8_t) * temperatureHistorySize);
    }

    DPrintF(F("temperature history size: "));
    DPrintUInt16(temperatureHistorySize);
    DPrintF(F(" = "));
    DPrintULong((unsigned long)temperatureHistorySize * (temperatureHistoryIntervalSec) / 60 / 60);
    DPrintFln(F(" hours"));

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

unsigned long lastInterrupted = millis();

//
// LOOP
//
void loop()
{
  size_t size;

  auto freeram = FreeMemorySum();
  if (freeram_min == -1 || freeram_min > freeram)
    freeram_min = freeram;

#ifdef USE_EXTERNAL_INTERRUPT
  if (interrupted)
  {
    DPrintFln(F("interrupted"));
    if (TimeDiff(lastInterrupted, millis()) > EXTERNAL_INTERRUPT_DEBOUNCE_MS)
    {
      DPrintFln(F("Handle external interrupt"));

      EthernetClient client;
      IPAddress addr(PUSHINGBOX_IP);
      if (client.connect(addr, 80))
      {
        client.print("GET ");
        client.print(PUSHINGBOX_QUERY);
        client.println(" HTTP/1.1");

        client.print("Host: ");
        client.println(PUSHINGBOX_HOSTNAME);
        client.println();

        client.stop();

        DPrintFln(F("===> SENT"));

        interrupted = 0;
        lastInterrupted = millis();
      }
      else
        DPrintFln(F("fail to connect pushingbox"));
    }
    else
      interrupted = 0; // reset too fast interrupts
  }
#endif

  if (EthernetClient client = server.available())
  {

    while ((size = client.available()) > 0)
    {
      header[0] = 0;
      {
        int i = 0;
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
      while (client.available())
        client.read(); // consume remaining header

      if (strlen(header) < 5 || strncmp(header, "GET /", 5) < 0)
      {
        client.stop();
        break;
      }

      //--------------------------
      // /tempdevices
      //--------------------------
      if (strncmp(header, "GET /tempdevices ", 17) == 0)
      {
        DPrintFln(F("temp devices"));
        clientOk(client, JSON);

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
          clientOk(client, TEXT);

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
      if (strncmp(header, "GET /temphistory ", 17) == 0)
      {
        clientOk(client, JSON);

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
          auto j = (temperatureHistoryFillCnt == temperatureHistorySize) ? temperatureHistoryOff : 0;
          auto size = min(temperatureHistoryFillCnt, temperatureHistorySize);
          for (int k = 0; k < size; ++k)
          {
            if (j == temperatureHistorySize)
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
      if (strncmp(header, "GET /info ", 10) == 0)
      {
        clientOk(client, JSON);

        client.print('{');

        client.print(F("\"freeram\":"));
        client.print((long)FreeMemorySum());

        client.print(F(", \"freeram_min\":"));
        client.print((long)freeram_min);

        client.print(F(", \"history_size\":"));
        client.print(temperatureHistorySize);

        client.print(F(", \"history_interval_sec\":"));
        client.print(temperatureHistoryIntervalSec);

        client.print(F(", \"history_backlog_hours\":"));
        client.print((int)TEMPERATURE_HISTORY_BACKLOG_HOURS);

        client.print('}');

        client.stop();
        break;
      }

      //--------------------------
      // /app.js
      //--------------------------
      if (strncmp(header, "GET /app.js ", 12) == 0)
      {
        DPrintFln(F("serving app.js"));

        clientOk(client, JAVASCRIPT);

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

        clientOk(client, HTML);

        client.print(
#include "index.htm.h"
        );

        client.stop();
        break;
      }
    }
  }

  if (TimeDiff(lastTemperatureRead, millis()) >= UPDATE_TEMPERATURE_MS)
  {
    printFreeram();
    ReadTemperatures();
  }

  if (temperatureHistory != NULL &&
      (TimeDiff(lastTemperatureHistoryRecord, millis()) > 1000UL * temperatureHistoryIntervalSec))
  {
    if (temperatureHistoryFillCnt < temperatureHistorySize)
      ++temperatureHistoryFillCnt;

    if (temperatureHistoryOff == temperatureHistorySize)
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
