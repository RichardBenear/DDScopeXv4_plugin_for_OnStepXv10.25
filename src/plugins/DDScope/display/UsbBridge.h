#ifndef USB_BRIDGE_H
#define USB_BRIDGE_H

#ifdef SWITCH
  #undef SWITCH
#endif

#include <USBHost_t36.h>

extern USBHost myusb;
extern USBSerial userSerial;

#define SERIAL_ESP32S3 userSerial

void usbBegin(void);
void usbPollTask(void);

#endif  // USB_BRIDGE_H
