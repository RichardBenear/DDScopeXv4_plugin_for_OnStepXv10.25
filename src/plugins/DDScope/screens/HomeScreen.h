// =====================================================
// HomeScreen.h
// =====================================================

#ifndef HOME_S_H
#define HOME_S_H

#include "Arduino.h"
#include "src/telescope/mount/park/Park.h"
class Display;

class HomeScreen : public Display {
  public:
    void draw();
    void updateHomeStatus();
    void updateHomeButtons();
    bool touchPoll(int16_t px, int16_t py);
    bool homeButStateChange();
    //bool resetHomeChanged = false;
    
  private:
    bool parkWasSet = false;
    bool stopButton = true;
    bool resetHome = false;
    
    bool gotoHome =false;
    bool fanOn = false;
    int preAzmState;
    int curAzmState = 0;
    int preAltState = 0;
    int curAltState = 0;
   
    bool preTrackState = false;
    bool preHomeState = false;
    bool preSlewState = false;
    uint8_t preParkState = park.state;
    bool refresh = false;
    bool preRefresh = true;
};

extern HomeScreen homeScreen;

#endif