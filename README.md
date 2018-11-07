# iot-temp-sensors

atmega328 + onewire ds18b20 + ethernet enc28j60

![img](doc/Selection_003.png)

## features

- easy customization through direct index.htm and app.js editing and debugging locally
- automatic compilatio of index.htm.h and app.js.h to be included as flash string

## prerequisites

- [iot-atmega328p-enc28j60](https://github.com/devel0/iot-atmega328p-enc28j60/blob/master/README.md)

## config

- ( see prerequisites prj )
- disable [ENABLE_CORS](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/temp-sensors.ino#L12) in production
- disable `debug` mode and set temperature device id to add a description (optional) in [app.js](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/app.js#L1-L17)

## debugging

- start `code .` from sketch folder
- program the chip `ctrl+shift+p` through usbasp programmer
- drag `index.htm` and `app.js` into code
- config `app.js`
  - enable [debug](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/app.js#L15) mode
  - tune [baseurl](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/app.js#L20)
- double click `index.htm` from file manager to open in a browser
- now webpage talk to atmega webapi through enc28j60 ( can use F12 and set some breakpoints from browser, may need useful to click on browser pretty button to format code because its minified )
- note : modified app.js and index.htm will compiled into flash automatically through [prebuild](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/.vscode/arduino.json#L6) utility [gen-h](https://github.com/devel0/iot-temp-sensors/blob/432999ee57a9892da64955140d62139edcb04fc9/temp-sensors/gen-h)

## sketch size

- serial debug included, `UIP_CONF_UDP=0` in `UIPEthernet/utility/uipethernet-conf.h`

```
Sketch uses 24534 bytes (74%) of program storage space. Maximum is 32768 bytes.
Global variables use 1169 bytes (57%) of dynamic memory, leaving 879 bytes for local variables. Maximum is 2048 bytes.
```
