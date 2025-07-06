// =====================================================
// GuideScreen.h

#ifndef GUIDE_S_H
#define GUIDE_S_H

#include <Arduino.h>
class Display;

class GuideScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateGuideStatus();
    void updateGuideButtons();
    bool guideButStateChange();
  
  private:
    bool guidingEast  = false;
    bool guidingWest  = false;
    bool guidingNorth = false;
    bool guidingSouth = false;
    bool oneXisOn     = false;
    bool eightXisOn   = false;
    bool twentyXisOn  = false;
    bool fourtyEightXisOn = true;
    bool HalfMaxisOn  = false;
    bool syncOn       = false;
    bool spiralOn     = false;
    bool stopPressed  = false;
    bool preSlewState = false;
    
};

extern GuideScreen guideScreen;

#endif