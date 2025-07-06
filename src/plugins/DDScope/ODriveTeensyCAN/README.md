# ODriveTeensyCAN

Library for interfacing ODrive with Teensy 3/4 over CANbus. 

Requires FlexCAN_T4 (https://github.com/tonton81/FlexCAN_T4) and ODrive firmware version 0.5.4 or later.

You can review the ODrive CAN protocol, called CAN Simple, here: https://newdocs.odriverobotics.com/v/latest/can-protocol.html


## Getting Started

Your sketch needs to include both FlexCAN_T4 and ODriveTeensyCAN libraries. Be sure to include FlexCAN_T4 *before* ODriveTeensyCAN.
```
#include <FlexCAN_T4.h>
#include <ODriveTeensyCAN.h>
```
Create ODriveTeensyCAN object with `ODriveTeensyCAN odriveCAN();` where `odriveCAN` is any name you want to call your object. You can optionally change the CAN baud rate by passing a parameter to the constructor `ODriveTeensyCAN odriveCAN(500000);`. The default baud rate is 250000.