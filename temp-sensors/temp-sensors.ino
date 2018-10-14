#define ARDUINO 18070

#define SERIAL_DEBUG
#define MACADDRESS 0x30, 0xcf, 0x8d, 0x9f, 0x5b, 0x89
#define MYIPADDR 10, 10, 2, 10
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 10, 10, 0, 6
#define MYGW 10, 10, 0, 1
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

#define TEMPERATURE_INTERVAL_MS 10000
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
  DEBUG_PRINTLN("STARTUP");
#endif

  SetupTemperatureDevices();

  FREERAM_PRINT;

  uint8_t mac[6] = {MACADDRESS};
  uint8_t myIP[4] = {MYIPADDR};
  uint8_t myMASK[4] = {MYIPMASK};
  uint8_t myDNS[4] = {MYDNS};
  uint8_t myGW[4] = {MYGW};

  // dhcp
  Ethernet.begin(mac);

  // static
  //Ethernet.begin(mac, myIP, myDNS, myGW, myMASK);

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
  DEBUG_PRINTLN("reading temperatures");
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

        client.println(F("<html>"));
        client.println(F("<link href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"));
        client.println(F("<body>"));

        client.println(F("<div class=\"container\">"));

        // interactive
        client.println(F("<h1>Temperature sensors</h1>"));
        if (temperatureDeviceCount > 0)
        {
          client.println(F("<div class=\"row\">"));
          client.println(F("<div class=\"col\">"));
          client.println(F("<div class=\"table-container\">"));
          client.println(F("<div class=\"table table-striped\">"));
          client.println(F("<table>"));
          client.println(F("<thead><tr><th scope=\"col\"><b>Temp Sensor</b></th><th scope=\"col\"><b>Value (C)</b></th><th scope=\"col\"><b>Action</b></th></tr></thead>"));
          client.println(F("<tbody>"));
          char tmp[20];
          for (int i = 0; i < temperatureDeviceCount; ++i)
          {
            client.print(F("<tr><td><span class='tempdev'>"));
            client.print(tempDevHexAddress[i]);
            client.print(F("</span></td><td><span id='t"));
            client.print(tempDevHexAddress[i]);
            client.print(F("'>"));
            dtostrf(temperatures[i], 3, 6, tmp);
            client.print(tmp);
            client.print(F("</span>"));
            client.print(F("</td><td><button class=\"btn btn-primary\" onclick='reloadTemp(\""));
            client.print(tempDevHexAddress[i]);
            client.print(F("\");'>reload</button></td></tr>"));
          }
          client.println(F("</tbody>"));
          client.println(F("</table>"));
          client.println(F("</div>")); // table table-striped
          client.println(F("</div>")); // table-container
          client.println(F("</div>")); // col
          client.println(F("</div>")); // row

          // autoreload function
          client.println(F("<div class=\"row\">"));
          client.println(F("<div class=\"col\">"));
          client.print(F("</td><td><button class=\"btn btn-default\" onclick='reload_enabled=true;'>autoreload</button></td></tr>"));
          client.println(F("</div>")); // col
          client.println(F("</div>")); // row
        }

        // api
        client.println(F("<h3 class=\"mt-3\">Api</h3>"));
        client.println(F("<div class=\"row\">"));
        client.println(F("<div class=\"col\">"));
        if (temperatureDeviceCount > 0)
        {
          client.print(F("<code>/temp/address</code> ( read temperature of sensor by given 8 hex address )<br/>"));
        }
        client.println(F("</div>")); // col
        client.println(F("</div>")); // row

        client.println(F("</div>")); // container

        //
        // JAVASCRIPTS
        //
        client.println(F("<script>"));        

        client.println(F("function reloadTemp(addr)"));
        client.println(F("{"));
        client.println(F("    $.get('/temp/' + addr, function(data) { $('#t' + addr)[0].innerText = data; });"));
        client.println(F("}"));

        client.println(F("var reload_enabled = false;"));
        client.println(F("setInterval(autoreload, 3000);"));
        client.println(F("function autoreload()"));
        client.println(F("{"));
        client.println(F("    if (!reload_enabled) return;"));
        client.println(F("    $('.tempdev').each(function (idx) {"));
        client.println(F("      let v=this.innerText; console.log('addr=[' + v + ']'); reloadTemp(v);"));
        client.println(F("    });"));
        client.println(F("}"));

        client.println(F("</script>"));

        client.println(F("<script src=\"https://code.jquery.com/jquery-3.3.1.min.js\" integrity=\"sha256-FgpCb/KJQlLNfOu91ta32o/NMZxltwRo8QtmkMRdAu8=\" crossorigin=\"anonymous\"></script>"));
        client.println(F("<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js\"></script>"));

        client.println(F("</body></html>"));

        FREERAM_PRINT;
      }

      client.stop();
    }
  }
  else if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)
  {
    ReadTemperatures();
  }
}
