// =====================================================
// TouchScreen.h

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "../display/Display.h" 

class TouchScreen {
  public:
    void init();
    void touchScreenPoll(ScreenEnum tCurScreen);
    void processTouch(ScreenEnum tCurScreen);
    
  private:
    ScreenEnum tCurScreen = HOME_SCREEN;
};

extern TouchScreen touchScreen;

#endif
