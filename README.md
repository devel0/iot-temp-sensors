# iot-temp-sensors

atmega328 + onewire ds18b20 + ethernet enc28j60

![img](doc/Selection_003.png)

## prerequisites

- software
  - install [vscode arduino](https://github.com/devel0/knowledge/blob/master/electronics/vscode-arduino.md)
  - install [UIPEthernet](https://github.com/UIPEthernet/UIPEthernet)
  - install [MemoryUsage](https://github.com/Locoduino/MemoryUsage)
- see [iot-atmega328p-enc28j60](https://github.com/devel0/iot-atmega328p-enc28j60/blob/master/README.md)

## temp sensor description

- after first connection to http://aaa.bbb.ccc.ddd of webserver copy and paste temp sensor addresses to `tempDescription` array givin a description

## debug

```
cd iot-temp-sensors/temp-sensors
code .
```

- choose `usbasp` programmer
- `ctrl+shift+p upload using programmer`

## notes

- current size

```
Sketch uses 28648 bytes (87%) of program storage space. Maximum is 32768 bytes.
Global variables use 1424 bytes (69%) of dynamic memory, leaving 624 bytes for local variables. Maximum is 2048 bytes.
```

- to enable serial debug uncomment [SERIAL_DEBUG](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L3)
- can work in [dhcp](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L62) ( uses some more flash )
- [temperature update interval](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L23) at [server side](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L354-L357) set to 5 sec
- [onewire bus](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L25) set to digital pin 3 ( [pin mapping](https://www.arduino.cc/en/Hacking/PinMapping168) )
- [resolution](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L110) to 12 bits
- http protocol
  - [read only first header](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L158-L176) due to constrained memory device
  - [mobile friendly](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L239)
  - [bootstrap](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L240) and [script](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L342)
  - [jquery](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L341)
  - [named and replaced page content](https://github.com/devel0/iot-temp-sensors/blob/c0d3a918df7af414b01e09c07f2cd6cc2e3d634d/temp-sensors/temp-sensors.ino#L267-L269)
  
