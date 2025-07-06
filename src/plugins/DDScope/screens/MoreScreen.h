// =====================================================
// MoreScreen.h

#ifndef MORE_S_H
#define MORE_S_H

#include <Arduino.h>
class Display;

class MoreScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateMoreStatus();
    void updateMoreButtons();
    bool moreButStateChange();

    bool objectSelected = false;
    unsigned int activeFilter;
    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};
    
    static char catSelectionStr1[28];
    static char catSelectionStr2[28];
    static char catSelectionStr3[28];
    static char catSelectionStr4[28];
    static char catSelectionStr5[28];

  private:
    //char _sideRate[9];
    char preFilterState[20]="";

    bool catalogsActive = false;
    bool soundEnabled = true;
    bool goToButton = false;
    bool stopBut = false;
    bool abortPgBut = false;
    bool clrCustom = false;
    bool sidereal = true;
    bool lunarRate = false;
    bool kingRate = false;
    bool incTrackRate = false;
    bool decTrackRate = false;
    bool rstTrackRate = false;
    bool filterBut = false;
    bool yesBut = false; 
    bool cancelBut = false;
    bool yesCancelActive = false;
    bool preSlewState = false;
    
};

extern MoreScreen moreScreen;

#endif