# iot-temp-sensors

atmega328 + onewire ds18b20 + ethernet enc28j60

![img](doc/Selection_003.png)

## prerequisites

- software
  - install [vscode arduino](https://github.com/devel0/knowledge/blob/master/arduino/vscode-arduino.md)
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
