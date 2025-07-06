// =====================================================
// DCFocuserScreen.h

#ifndef DCFOCUSER_S_H
#define DCFOCUSER_S_H

#include <Arduino.h>
class Display;

class DCFocuserScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateFocuserButtons();
    void updateFocuserStatus();
    bool focuserButStateChange();
    
  private:
    void focInit();
    void focChangeDirection();
    void focMove(int numPulses, int pulseWidth);
    void updateFocPosition();

    bool redrawBut = false;
    bool focMovingIn = false;
    bool gotoSetpoint = false;
    bool focGoToHalf = false;
    bool setPoint = false;
    bool decSpeed = false;
    bool incSpeed = false;
    bool incMoveCt = false;
    bool decMoveCt = false;
    bool setZero = false;
    bool setMax = false;
    bool focReset = false;
    bool revFocuser ;
    bool inwardCalState; // start with inward calibration
    bool calibActive ;
    bool focGoToActive ;
    int focMoveSpeed; // pulse width in microsec
    int focMoveDistance; // probably need to start with 30 after powering up
    int moveDistance;

    int focPosition;
    int focTarget;
    int focDeltaMove;
    int focMaxPosition;
    int focMinPosition;
    int setPointTarget;
};

extern DCFocuserScreen dcFocuserScreen;

#endif
