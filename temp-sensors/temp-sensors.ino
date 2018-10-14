#define ARDUINO 18070

#define SERIAL_DEBUG
#define MACADDRESS 0x30, 0xcf, 0x8d, 0x9f, 0x5b, 0x89
#define LISTENPORT 80

#include <Arduino.h>
//#include <pins_arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mywifikey.h>
#include <arduino-utils.h>
#include <UIPEthernet.h>
#include <MemoryUsage.h>

EthernetServer server = EthernetServer(LISTENPORT);

#define TEMPERATURE_INTERVAL_MS 1000
#define TEMPERATURE_ADDRESS_BYTES 8
#define ONE_WIRE_BUS D3

OneWire tempOneWire(3);
DallasTemperature DS18B20(&tempOneWire);

int temperatureDeviceCount = 0;
float *temperatures = NULL;
DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

//
// SETUP
//
void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif

  SetupTemperatureDevices();

  uint8_t mac[6] = {MACADDRESS};

  // dhcp
  Ethernet.begin(mac);

#ifdef SERIAL_DEBUG
  DEBUG_PRINT("my ip : ");
  Ethernet.localIP().printTo(Serial);
  DEBUG_PRINTLN();
#endif

  server.begin();

  FREERAM_PRINT;
}

//
// TEMPERATURE SETUP
//
void SetupTemperatureDevices()
{
  DS18B20.begin();
  temperatureDeviceCount = DS18B20.getDeviceCount();
  DEBUG_PRINTF("temperature device count = %d\n", temperatureDeviceCount);
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

      DEBUG_PRINTF("sensor [%d] address = %s\n", i, tempDevHexAddress[i]);

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
  //DEBUG_PRINTLN("reading temperatures");
  DS18B20.requestTemperatures();
  for (int i = 0; i < temperatureDeviceCount; ++i)
  {
    auto temp = DS18B20.getTempC(tempDevAddress[i]);
    //DEBUG_PRINTF("temperature sensor %d = %f\n", i, temp);
    temperatures[i] = temp;
  }
  lastTemperatureRead = millis();
}

//
// LOOP
//
void loop()
{
  size_t size;

  if (EthernetClient client = server.available())
  {
    DEBUG_PRINT("received message : [");

    String header;

    while ((size = client.available()) > 0)
    {
      DEBUG_PRINTF("size=%d\n", size);
      bool foundcmd = false;

      for (int i = 0; i < 80; ++i)
      {
        char c = (char)client.read();
        DEBUG_PRINT("char = [");
        DEBUG_PRINTLN(c);
        if (c == '\r')
        {
          DEBUG_PRINTLN("found newline");
          break;
        }
        header.concat(c);
      }      

      DEBUG_PRINT("header=[");
      DEBUG_PRINTLN(header.c_str());
      DEBUG_PRINTLN("]");     

      FREERAM_PRINT; 

      client.println(F("(HTTP/1.1 200 OK"));
      client.println(F("Content-type:text/html"));
      client.println(F("Connection: close"));
      client.println();

      foundcmd = false;

      //--------------------------
      // TEMPERATURE
      //--------------------------
      if (temperatureDeviceCount > 0)
      {
        String q = String("GET /temp/");
        if (header.indexOf(q) >= 0)
        {
          bool found = false;
          if (header.length() - q.length() >= 8)
          {
            for (int i = 0; i < temperatureDeviceCount; ++i)
            {
              if (strncmp(header.c_str() + q.length(), tempDevHexAddress[i],
                          2 * TEMPERATURE_ADDRESS_BYTES) == 0)
              {
                char tmp[20];
                dtostrf(temperatures[i], 3, 6, tmp);
                                
                client.print(tmp);
                DEBUG_PRINT("found temp device query, res = [");
                DEBUG_PRINT(tmp);
                DEBUG_PRINTLN("]");                
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
      // HELP
      //--------------------------
      if (header.indexOf("GET /") >= 0)
      {
        FREERAM_PRINT;

        client.println(F("<script>function httpGet(theUrl)"));
        client.println(F("{"));
        client.println(F("    var xmlHttp = new XMLHttpRequest();"));
        client.println(F("    xmlHttp.open( \"GET\", theUrl, false );"));
        client.println(F("    xmlHttp.send( null );"));
        client.println(F("    return xmlHttp.responseText;"));
        client.println(F("}</script>"));
        client.println(F("<html><body>"));

        // interactive
        client.println(F("<h1>Temperature sensors</h1>"));
        if (temperatureDeviceCount > 0)
        {
          client.println(F("<table><thead><tr><td><b>Temp Sensor</b></td><td><b>Value (C)</b></td><td><b>Action</b></td></tr></thead>"));
          client.println(F("<tbody>"));
          char tmp[20];
          for (int i = 0; i < temperatureDeviceCount; ++i)
          {
            client.print(F("<tr><td>"));
            client.print(tempDevHexAddress[i]);
            client.print(F("</td><td>"));
            dtostrf(temperatures[i], 3, 6, tmp);
            client.print(tmp);
            client.print(F("</td><td><button onclick='location.reload();'>reload</button></td></tr>"));
          }
          client.println(F("</tbody></table>"));
        }

        // api
        client.println(F("<h3>Api</h3>"));
        if (temperatureDeviceCount > 0)
        {
          client.print(F("<code>/temp/address</code> ( read temperature of sensor by given 8 hex address )<br/>"));
        }

        client.println(F("</body></html>"));

        FREERAM_PRINT;
      }

      client.stop();
    }
  }
}
