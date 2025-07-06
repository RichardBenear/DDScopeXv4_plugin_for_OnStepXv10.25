// =====================================================
// GotoScreen.h

#ifndef GOTO_S_H
#define GOTO_S_H

#include <Arduino.h>
class Display;

class GotoScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateGotoStatus();
    void updateGotoButtons();
    bool gotoButStateChange();

    float rateRatio = 1.0;
    
  private:
    void processNumPadButton();
    void setTargPolaris();
    
    char RAtext[8] = "";
    char DECtext[8] = "";
    char cRate[12]   = "";
    char bRate[12]   = "";

    int buttonPosition = 0;
    uint8_t RAtextIndex = 0;
    uint8_t DECtextIndex = 0;

    bool RAselect = true;
    bool RAclear = false;
    bool DECselect = false;
    bool DECclear = false;
    bool sendOn = false;
    bool setPolOn = false;
    bool numDetected = false;
    bool goToButton = false;
    bool abortPgBut = false;
    bool preSlewState = false;

    float cRateF;
    float bRateF;
};

extern GotoScreen gotoScreen;

#endif