//==================================================
// WifiDisplay.h
//
#include <Arduino.h>
#ifndef _LX200_HANDLER
#define _LX200_HANDLER

#define SERIAL_ESP32C3 Serial8

//======================================================================
class LX200Handler  {
  public:
    void init();
    void lxPoll();
    //void take_esp_lock();
    //void give_esp_lock();
   
    //volatile bool espIsLocked = false;  // Simple lock for thread safety

  private:
  
};

extern LX200Handler lx200Handler;

#endif
