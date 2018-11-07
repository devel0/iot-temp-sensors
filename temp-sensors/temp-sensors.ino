#define ARDUINO 18070

//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//
// SECURITY WARNING : uncomment ENABLE_CORS in production!
//---------------------
// I used CORS policy to allow me write index.htm and app.js outside atmega controller from pc
// using atmega webapis
//
#define ENABLE_CORS

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
  }

#ifdef ENABLE_CORS
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

      //--------------------------
      // TEMPERATURE
      //--------------------------
      if (temperatureDeviceCount > 0)
      {
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
      // HELP
      //--------------------------
      if (header.indexOf("GET /") >= 0)
      {
        clientOk(client, CCTYPE_HTML);

        client.println(F("<html>"));
        client.println(F("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>"));
        client.println(F("<link href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"));
        client.println(F("<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.4.1/css/all.css\" integrity=\"sha384-5sAR7xN1Nv6T6+dT2mhtzEpVJvfS3NScPQTrOxhwjIuvcA67KV2R5Jz6kr4abQsz\" crossorigin=\"anonymous\">"));
        client.println(F("<body>"));

        client.println(F("<div class=\"container\">"));

        // interactive
        client.println(F("<div class=\"row\"><div class=\"col-auto\"><h1>Temperature sensors</h1></div><div class=\"col\"><i class=\"fas fa-spin fa-spinner j-spin collapse\"></i></div></div>"));
        if (temperatureDeviceCount > 0)
        {
          client.println(F("<div class=\"row\">"));
          client.println(F("<div class=\"col\">"));
          client.println(F("<div class=\"table-container\">"));
          client.println(F("<div class=\"table table-striped\">"));
          client.println(F("<table class=\"table\">"));
          client.println(F("<thead><tr>"));
          client.print(F("<th scope=\"col\"><b>Address</b></th>"));
          client.print(F("<th scope=\"col\"><b>Description</b></th>"));
          client.print(F("<th scope=\"col\"><b>Value (C)</b></th>"));
          client.print(F("<th scope=\"col\"><b>Action</b></th>"));
          client.print(F("</tr></thead>"));
          client.println(F("<tbody>"));
          char tmp[20];
          for (int i = 0; i < temperatureDeviceCount; ++i)
          {
            client.print(F("<tr>"));

            // address
            client.print(F("<td><span class='tempdev'>"));
            client.print(tempDevHexAddress[i]);
            client.print(F("</span></td>"));

            // description
            client.print(F("<td>"));
            client.print(getTempDescription(tempDevHexAddress[i]));
            client.print(F("</td>"));

            // temperature
            client.print(F("<td><span id='t"));
            client.print(tempDevHexAddress[i]);
            client.print(F("'>"));
            FloatToString(tmp, temperatures[i], 6);
            client.print(tmp);
            client.print(F("</span>"));

            // action
            client.print(F("</td><td><button class=\"btn btn-primary\" onclick='reloadTemp(\""));
            client.print(tempDevHexAddress[i]);
            client.print(F("\");'>reload</button></td>"));

            client.print(F("</tr>"));
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
          client.print(F("<code>/tempdevices</code> ( retrieve list of temperature devices )<br/>"));
        }
        client.println(F("</div>")); // col
        client.println(F("</div>")); // row

        client.println(F("<div class=\"row mt-2\">"));
        client.println(F("<div class=\"col\">"));
        client.println(F("<code>Freeram : "));
        client.println(FreeMemorySum());
        client.println("</code><br/>");
        client.println(F("</div>")); // col
        client.println(F("</div>")); // row

        client.println(F("</div>")); // container

        //
        // JAVASCRIPTS
        //
        client.println(F("<script>"));

        client.println(F("function reloadTemp(addr)"));
        client.println(F("{"));
        client.println(F("    $('.j-spin').removeClass('collapse');"));
        client.println(F("    $.get('/temp/' + addr, function(data) { $('.j-spin').addClass('collapse'); $('#t' + addr)[0].innerText = data; });"));
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
      }

      client.stop();
    }
  }
  else if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)
  {
    printFreeram();

    ReadTemperatures();
  }
}
