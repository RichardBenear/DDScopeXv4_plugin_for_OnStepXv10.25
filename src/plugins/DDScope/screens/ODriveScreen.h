// =====================================================
// ODriveScreen.h

#ifndef ODRIVE_S_H
#define ODRIVE_S_H

#include <Arduino.h>
#include "../display/Display.h"
#include "../odriveExt/ODriveExt.h"

class Display;

class ODriveScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateOdriveStatus();
    void updateOdriveButtons();
    bool odriveButStateChange();
    
    
  private:
    void showODriveErrors();
    void showGains();
    uint8_t decodeODriveTopErrors(int axis, uint32_t errorCode, int y_offset);
    uint8_t decodeODriveAxisErrors(int axis, uint32_t errorCode, int y_offset);
    uint8_t decodeODriveMotorErrors(int axis, uint32_t errorCode, int y_offset);
    uint8_t decodeODriveContErrors(int axis, uint32_t errorCode, int y_offset);
    uint8_t decodeODriveEncErrors(int axis, uint32_t errorCode, int y_offset);

    static const uint8_t box_height_adj = 10;
    bool demoActive       = false;
    bool clearODriveErrs  = false;
    bool resetODriveFlag  = false;
    bool OdStopButton     = false;
    bool ODpositionUpdateEnabled = true;
    bool azGainHi = false;
    
    int preAzmState       = 0;
    int preAltState       = 0;
    int demoHandle;
};

extern ODriveScreen oDriveScreen;

#endif