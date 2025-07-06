// =====================================================
// SettingsScreen.h

#ifndef SETTINGS_S_H
#define SETTINGS_S_H

#include <Arduino.h>
class Display;

class SettingsScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateSettingsButtons();
    bool settingsButStateChange();
    void updateSettingsStatus();
  
  private:
    void setProcessNumPadButton();
  
    char sNumLabels[12][3] = {"9", "8", "7", "6", "5", "4", "3", "2", "1", "-", "0", "+"};
    char Ttext[8] = {'0','0',':','0','0',':','0','0'};
    char Dtext[8] = {'0','0','/','0','0','/','0','0'};
    char Tztext[3]= {'0','0','0'};
    char LaText[5]= {'0','0','0','0','0'};
    char LoText[6]= {'0','0','0','0','0','0'};
    char DefLat[3] = {'4', '3', '6'}; // decimal degrees, not sDD*MM
    char DefLong[4]= {'1', '1', '6', '3'}; // decimal degrees, not sDDD*MM
    char sCmd[12];

    int sButtonPosition;

    uint8_t TtextIndex;
    uint8_t DtextIndex;
    uint8_t TztextIndex;
    uint8_t LaTextIndex;
    uint8_t LoTextIndex;
    
    bool Tselect    = true;
    bool Tclear     = false;
    bool Dselect    = false;
    bool Dclear     = false;
    bool Tzselect   = false;
    bool Tzclear    = false;
    bool LaSelect   = false;
    bool LaClear    = false;
    bool LoSelect   = false;
    bool LoClear    = false;
    bool sSendOn    = false;
    bool setLatLongOn= false;
    bool siteOn     = false;
    bool sNumDetected = false;
};

extern SettingsScreen settingsScreen;

#endif