//==================================================
// WifiDisplay.h
//
#include <Arduino.h>
#ifndef _WIFI_SCREEN
#define _WIFI_SCREEN

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define COLOR_DEPTH 2 // 2 bytes per pixel (RGB565)

#define UNCOMPRESSED_BUFFER_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT * COLOR_DEPTH))
#define COMPRESSED_BUFFER_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT * COLOR_DEPTH))

extern uint8_t compressedBuffer[COMPRESSED_BUFFER_SIZE];
extern uint8_t uncompressedBuffer[UNCOMPRESSED_BUFFER_SIZE];

//======================================================================
class WifiDisplay  
{
  public:
    void saveBufferToSD(const char* screenName);
    void enableScreenCapture(bool enable);
    void sendFrameToEsp(uint8_t frameType);
    size_t compressWithRLE();
    size_t compressWithDeflate();
    void espPoll();
    void take_esp_lock();
    void give_esp_lock();
    void captureSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    bool isScreenCaptureEnabled = false;
    bool isUpdateScreenCaptureEnabled = false;
    volatile bool espIsLocked = false;  // Simple lock for thread safety
    String wifiStaIpStr = "";

 private:
  
};

extern WifiDisplay wifiDisplay;

#endif
