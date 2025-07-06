// =====================================================
// DCFocuserScreen.cpp  
//
// DC motor focuser - Model: Moonlight CR1 Crayford with MF1 DC Servo
//
// Author: Richard Benear 
// 12/20/21
//
// The following comments are only true for OnStep, not OnStepX..TBD what to do in OnStepX
// 
// Initially, with OnStep, this Focuser Page tried to use the FocuserDC.h
// and StepperDC.h implementations done by Howard Dutton to 
// drive a DC Motor Focuser by using the local command channel (cmdX).
// The hardware used was an A4988 stepper driver. This mostly worked
// but there were problems with inconsistent moves where the DC motor would
// not respond after multiple successive moves. I suspect there are problems 
// with the length of the Enable high/low pulse width and inductive kick from 
// the coils with a long cable and coupling this noise into the Step signal
// which moves the phase to the wrong phase. Therefore, I resorted to building 
// my own A4988 driver for the DC Motor control where I could tweak the 
// parameters more directly. These functions are found near the end 
// of this file.
#include "../display/Display.h"
#include "DCFocuserScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include "../../../lib/tasks/OnTask.h"

// For IN and OUT Buttons
#define FOC_INOUT_X             206 
#define FOC_INOUT_Y             173 
#define FOC_INOUT_BOXSIZE_X     109 
#define FOC_INOUT_BOXSIZE_Y      75 
#define FOC_INOUT_X_SPACING       0 
#define FOC_INOUT_Y_SPACING      85 

// For Focuser Status
#define FOC_LABEL_X               3 
#define FOC_LABEL_Y             176 
#define FOC_LABEL_Y_SPACING      16 
#define FOC_LABEL_OFFSET_X      115
#define FOC_LABEL_OFFSET_Y        6 

// For Focuser Speed Selection
#define SPEED_X                    3 
#define SPEED_Y                  270 
#define SPEED_BOXSIZE_X          104 
#define SPEED_BOXSIZE_Y           30 

// Stop Focuser position
#define CALIB_FOC_X              215 
#define CALIB_FOC_Y              390 
#define CALIB_FOC_BOXSIZE_X       90 
#define CALIB_FOC_BOXSIZE_Y       45 

// For Focuser Middle Buttons Selection
#define MID_X                    116 
#define MID_Y                   SPEED_Y 
#define MID_BOXSIZE_X             80 
#define MID_BOXSIZE_Y             30 

#define MTR_PWR_INC_SIZE           5
#define FOC_MOVE_DISTANCE 5 // default to 5 pulses
#define FOC_SPEED_INC 100 // default to inc/dec of 100 microsec
#define EN_OFF_TIME 2000 // microseconds

// Focuser Screen Main Button object
Button focuserButton(
                0, 0, 0, 0,
                butOnBackground, 
                butBackground, 
                butOutline, 
                mainFontWidth, 
                mainFontHeight, 
                "");

// Focuser Screen Large Button object
Button focuserXLargeButton(
                0, 0, 0, 0,
                butOnBackground, 
                butBackground, 
                butOutline, 
                xlargeFontWidth, 
                xlargeFontHeight, 
                "");

// Canvas Print object Custom Font
CanvasPrint canvFocuserInsPrint(&Inconsolata_Bold8pt7b);

// Draw the initial content for Focuser Page
void DCFocuserScreen::draw() {
  tasks.yield(10);
  setCurrentScreen(FOCUSER_SCREEN);
  focuserButton.setColors(butOnBackground, butBackground, butOutline);
  focuserXLargeButton.setColors(butOnBackground, butBackground, butOutline);

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  
  drawMenuButtons();
  drawTitle(110, TITLE_TEXT_Y, "Focuser");

  tft.setFont(&Inconsolata_Bold8pt7b);
  drawCommonStatusLabels();
  updateFocuserButtons();
  //showOnStepCmdErr(); // show error bar
  
  int y_offset = 0;

  // Maximum out position
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print("          Max:");
  
  // Minimum in position
  y_offset +=FOC_LABEL_Y_SPACING;
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print("          Min:");
  
  // Move Speed - actually is the width of motor enable pulse low
  y_offset +=FOC_LABEL_Y_SPACING;
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print("   Move Speed:");

  // Move Distance - actually is the number of enable pulses in a move
  y_offset +=FOC_LABEL_Y_SPACING;
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print("Move Distance:");

  // Current position
  y_offset +=FOC_LABEL_Y_SPACING;
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print("Curr Position:");

  // Delta from target position
  y_offset +=FOC_LABEL_Y_SPACING;
  tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
  tft.print(" Target Delta:");

  updateCommonStatus();
  updateFocuserStatus();

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  #endif
  #ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("Focus");
  #endif
}

// task update for this screen
void DCFocuserScreen::updateFocuserStatus() {

  int y_offset = 0;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focMaxPosition, false);

  y_offset +=FOC_LABEL_Y_SPACING;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focMinPosition, false);
     
  // Focuser Speed
  y_offset +=FOC_LABEL_Y_SPACING;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focMoveSpeed, false);
   
  // Focuser move distance
  y_offset +=FOC_LABEL_Y_SPACING;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focMoveDistance, false);
     
  // Update Current Focuser Position
  y_offset +=FOC_LABEL_Y_SPACING;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focPosition, false);

  // Update Delta Focuser Position
  y_offset +=FOC_LABEL_Y_SPACING;
  canvFocuserInsPrint.printRJ(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y+y_offset, C_WIDTH, C_HEIGHT, focDeltaMove, false);
}

bool DCFocuserScreen::focuserButStateChange() {
  bool changed = false;

  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (setMax || focReset || setZero || incSpeed || decSpeed) {
      changed = true;
    }

    if (setPoint || gotoSetpoint) {
      changed = true;
    }

    if (focGoToHalf || incMoveCt || decMoveCt) {
      changed = true;
    }
  }

  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  return changed;
}

//***** Update Focuser Buttons ******
void DCFocuserScreen::updateFocuserButtons() {  
  
  tft.setFont(&FreeSansBold12pt7b);
  if (focMovingIn && focGoToActive) {
    focuserXLargeButton.draw(FOC_INOUT_X, FOC_INOUT_Y, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, "IN", BUT_ON);
  } else {
    focuserXLargeButton.draw(FOC_INOUT_X, FOC_INOUT_Y, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, "IN", BUT_OFF);
  }

  if (!focMovingIn && focGoToActive) {
    focuserXLargeButton.draw(FOC_INOUT_X, FOC_INOUT_Y + FOC_INOUT_Y_SPACING, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, "OUT", BUT_ON);
  } else {
    focuserXLargeButton.draw(FOC_INOUT_X, FOC_INOUT_Y + FOC_INOUT_Y_SPACING, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, "OUT", BUT_OFF);
  }
  tft.setFont(&Inconsolata_Bold8pt7b);
  
  // *** Left Column Buttons ***
  // update speed selection
  int y_offset = 0;
  // Increment Speed
  if (incSpeed) {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Inc'ing", BUT_ON);
    incSpeed = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Inc Speed", BUT_OFF);
  }

  // Decrement Speed
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (decSpeed) {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Dec'ing", BUT_ON);
    decSpeed = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Dec Speed", BUT_OFF);
  }

  // Set a GoTo setpoint
  y_offset +=SPEED_BOXSIZE_Y + 2; 
  if (setPoint) {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Setting..", BUT_ON);
    setPoint = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Set Goto Pt", BUT_OFF);
  }

  // Goto the setpoint
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (gotoSetpoint) {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Going to SP", BUT_ON);
    gotoSetpoint = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "Goto the SP", BUT_OFF);
  }

  // Goto Halfway point
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (focGoToHalf) {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "GoingTo Half", BUT_ON);
    focGoToHalf = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, "GoTo Half", BUT_OFF);
  }

  // Calibrate Min And Max
  y_offset = 0;
  if (!calibActive) {
    focuserButton.draw(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, "Calibrate", BUT_OFF);
  } else {
      if (inwardCalState && calibActive) {
        focuserButton.draw(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, "Min Calib", BUT_ON);
      } else if (!inwardCalState && calibActive) {
        focuserButton.draw(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, "Max Calib", BUT_ON);
      }
  }

  // *** Center column Buttons ***
  y_offset = 0;
  // Increment Motor Power
  if (incMoveCt) {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Inc'ing", BUT_ON);
    incMoveCt = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Inc Cnt", BUT_OFF);
  }

  y_offset +=SPEED_BOXSIZE_Y + 2;
  // Decrement Motor Power
  if (decMoveCt) {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Dec'ing", BUT_ON);
    decMoveCt = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Dec Cnt", BUT_OFF);
  }

  y_offset +=SPEED_BOXSIZE_Y + 2;
  // Set Zero Position
  if (setZero) {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Setting..", BUT_ON);
    setZero = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Set Zero", BUT_OFF);
  }

  y_offset +=SPEED_BOXSIZE_Y + 2;
  // Set Maximum position
  if (setMax) {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Setting..", BUT_ON);
    setMax = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Set Max", BUT_OFF);
  }

  y_offset +=SPEED_BOXSIZE_Y + 2;
  // Reset focuser
  if (focReset) {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "Resetting", BUT_ON);
    focReset = false;
    display._redrawBut = true;
  } else {
    focuserButton.draw(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, "RESET", BUT_OFF);
  }
}

// ----- Update buttons when touched -----
bool DCFocuserScreen::touchPoll(uint16_t px, uint16_t py)
{   
  // IN button
  if (py > FOC_INOUT_Y && py < (FOC_INOUT_Y + FOC_INOUT_BOXSIZE_Y) && px > FOC_INOUT_X && px < (FOC_INOUT_X + FOC_INOUT_BOXSIZE_X))
  {
    BEEP;
    soundFreq(2000, 400);
    if (!focMovingIn) { //was moving out, change direction
        focChangeDirection();
    }   
    focMove(focMoveDistance, focMoveSpeed);
    focGoToActive = false;
    return true;
  }

  int y_offset = FOC_INOUT_Y_SPACING;
  // OUT button
  if (py > FOC_INOUT_Y + y_offset && py < (FOC_INOUT_Y + y_offset + FOC_INOUT_BOXSIZE_Y) && px > FOC_INOUT_X && px < (FOC_INOUT_X + FOC_INOUT_BOXSIZE_X))
  {
    BEEP;
    soundFreq(2300, 400);
    if (focMovingIn) { //was moving in, change direction
        focChangeDirection();
    }
    focMove(focMoveDistance, focMoveSpeed);
    focGoToActive = false;
    return true;
  }

  // ========= LEFT Column Buttons ========
  // Select Focuser Speed 
  y_offset = 0;
  // Increment Speed
  if (py > SPEED_Y + y_offset && py < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && px > SPEED_X && px < (SPEED_X + SPEED_BOXSIZE_X))
  {
    BEEP;
    focMoveSpeed += FOC_SPEED_INC; // microseconds
    if (focMoveSpeed > 900) focMoveSpeed = 900;
    incSpeed = true;
    return true;
  }

  // Decrement Speed
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > SPEED_Y + y_offset && py < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && px > SPEED_X && px < (SPEED_X + SPEED_BOXSIZE_X))
  {
    BEEP;
    focMoveSpeed -= FOC_SPEED_INC; // microseconds
    if (focMoveSpeed < 100) focMoveSpeed = 100;
    decSpeed = true;
    return true;
  }

  // ======== Set GoTo points ========
  // Set GoTo point
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > SPEED_Y + y_offset && py < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && px > SPEED_X && px < (SPEED_X + SPEED_BOXSIZE_X))
  {
    BEEP;
    setPointTarget = focPosition;
    setPoint = true;
    return true;
  }

  // GoTo Setpoint
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > SPEED_Y + y_offset && py < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && px > SPEED_X && px < (SPEED_X + SPEED_BOXSIZE_X))
  {
    BEEP;
    focTarget = setPointTarget;
    gotoSetpoint = true; 
    focGoToActive = true;
    return true;
  }

  // GoTo Halfway 
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > SPEED_Y + y_offset && py < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && px > SPEED_X && px < (SPEED_X + SPEED_BOXSIZE_X))
  {
    BEEP;
    focTarget = (focMaxPosition - focMinPosition) / 2;
    focGoToHalf = true;
    focGoToActive = true;
    return true;
  }
  
  // ======== Bottom Right Corner Button ========
  // ** Calibration Button Procedure **
  // 1st button press moves focuser inward
  // 2nd button press stops inward move and sets as Minimum position
  // 3rd button press moves focuser outward
  // 4th button press stops outward move and sets as Maximum position
  if (py > CALIB_FOC_Y && py < (CALIB_FOC_Y + CALIB_FOC_BOXSIZE_Y) && px > CALIB_FOC_X && px < (CALIB_FOC_X + CALIB_FOC_BOXSIZE_X))
  {  
    BEEP;
    if (inwardCalState) {
      if (!focGoToActive) { // then we are starting calibration
        if (!focMovingIn) focChangeDirection(); // go inward
        focMovingIn = true;
        focTarget -= 12000;
        focGoToActive = true;
        calibActive = true;
      } else { // then we have been told to stop move in and are at Minimum
        focGoToActive = false;
        focMinPosition = 0;
        focPosition = 0;
        inwardCalState = false;
      }
    } else { // now go out and calibrate max position
      if (!focGoToActive) { // then we are starting OUT MAX calibration
        if (focMovingIn) focChangeDirection();
        focMovingIn = false;
        focTarget += 12000;
        focGoToActive = true;
      } else { // then we have been told to stop move OUT and are at Maximum
        focGoToActive = false;
        focMaxPosition = focPosition;
        inwardCalState = true; // reset this in case want to calibrate again
        calibActive = false;
      }
    } 
    return true;
  }

  // ****** Center Column Buttons ******
  y_offset = 0;
  // move distance increment
  if (py > MID_Y + y_offset && py < (MID_Y + y_offset + MID_BOXSIZE_Y) && px > MID_X && px < (MID_X + MID_BOXSIZE_X))
  {
    BEEP;
    focMoveDistance += MTR_PWR_INC_SIZE;
    if (focMoveDistance >= 100) focMoveDistance = 100;
    incMoveCt = true;
    decMoveCt = false;
    return true;
  }

  // move distance decrement
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > MID_Y + y_offset && py < (MID_Y + y_offset + MID_BOXSIZE_Y) && px > MID_X && px < (MID_X + MID_BOXSIZE_X))
  {
    BEEP;
    focMoveDistance -= MTR_PWR_INC_SIZE;
    if (focMoveDistance < 0) focMoveDistance = 5;
    incMoveCt = false;
    decMoveCt = true;
    return true;
  }

  // Set Zero point of Focuser
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > MID_Y + y_offset && py < (MID_Y + y_offset + MID_BOXSIZE_Y) && px > MID_X && px < (MID_X + MID_BOXSIZE_X))
  {
    BEEP;
    focMinPosition = 0;
    focPosition = 0;
    setZero = true;
    return true;
  }

  // Set Max Position of focuser
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > MID_Y + y_offset && py < (MID_Y + y_offset + MID_BOXSIZE_Y) && px > MID_X && px < (MID_X + MID_BOXSIZE_X))
  {
    BEEP;
    focMaxPosition = focPosition;
    setMax = true;
    return true;
  }

  // Reset focuser
  y_offset +=SPEED_BOXSIZE_Y + 2;
  if (py > MID_Y + y_offset && py < (MID_Y + y_offset + MID_BOXSIZE_Y) && px > MID_X && px < (MID_X + MID_BOXSIZE_X))
  {
    BEEP;
    digitalWrite(FOCUSER_SLEEP_PIN,LOW); 
    delay(1);
    digitalWrite(FOCUSER_SLEEP_PIN,HIGH); 
    focReset = true;
    return true;
  }

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);
  
  return false;
}

// ======= A4988 Driver Functions =======
// Initialize DC Focuser
void DCFocuserScreen::focInit() {
    // set phase power from 70% Home position backward to 100% power position
    digitalWrite(FOCUSER_DIR_PIN,LOW);
    delayMicroseconds(2);
    digitalWrite(FOCUSER_EN_PIN,LOW);
    delayMicroseconds(2);
    digitalWrite(FOCUSER_STEP_PIN,HIGH); delayMicroseconds(5); digitalWrite(FOCUSER_STEP_PIN,LOW); delayMicroseconds(5);
    digitalWrite(FOCUSER_EN_PIN,HIGH);
    delayMicroseconds(2);
    digitalWrite(FOCUSER_DIR_PIN,HIGH);
}

// step phases to opposite current direction, 4 steps required in half step mode
void DCFocuserScreen::focChangeDirection() {
    digitalWrite(FOCUSER_EN_PIN,LOW);
    delayMicroseconds(10);
    digitalWrite(FOCUSER_STEP_PIN,HIGH); delayMicroseconds(5); digitalWrite(FOCUSER_STEP_PIN,LOW); delayMicroseconds(5);
    digitalWrite(FOCUSER_STEP_PIN,HIGH); delayMicroseconds(5); digitalWrite(FOCUSER_STEP_PIN,LOW); delayMicroseconds(5);
    digitalWrite(FOCUSER_STEP_PIN,HIGH); delayMicroseconds(5); digitalWrite(FOCUSER_STEP_PIN,LOW); delayMicroseconds(5);
    digitalWrite(FOCUSER_STEP_PIN,HIGH); delayMicroseconds(5); digitalWrite(FOCUSER_STEP_PIN,LOW); delayMicroseconds(5);
    digitalWrite(FOCUSER_EN_PIN,HIGH);
    delayMicroseconds(10);
    if (focMovingIn) focMovingIn = false; else focMovingIn = true;
}

// Move focuser; numPulses equivalent to move distance; pulseWidth equivalent to move speed
void DCFocuserScreen::focMove(int numPulses, int pulseWidth) {   
    for (int i=0; i<numPulses; i++) {
        digitalWrite(FOCUSER_EN_PIN,LOW); delayMicroseconds(pulseWidth); 
        digitalWrite(FOCUSER_EN_PIN,HIGH); delayMicroseconds(EN_OFF_TIME); // arbitrarily selected 1 ms off time
    }  
    if (focMovingIn) focPosition -= numPulses; else focPosition += numPulses;
}

// Focuser GoTo a target position. Updated via main loop timer. Target set elsewhere.
void DCFocuserScreen::updateFocPosition() {
    if (!focGoToActive) return;
    focDeltaMove = focTarget - focPosition;
    if (abs(focDeltaMove) <= focMoveDistance) moveDistance = abs(focDeltaMove); else moveDistance = focMoveDistance;
    if (focDeltaMove < 0) { // negative move, go In
        if (focMovingIn) { // already moving In?
            focMove(moveDistance, focMoveSpeed);
        } else { // moving Out, so change direction
            focChangeDirection();
            focMove(moveDistance, focMoveSpeed);
        }
    } else if (focDeltaMove > 0) {  // positive move, go Out
        if (focMovingIn) { // moving In? then change direction
            focChangeDirection(); 
            focMove(moveDistance, focMoveSpeed);
        } else { 
            focMove(moveDistance, focMoveSpeed);
        }
    } else { // else positions equal, do nothing
        focGoToActive = false;
    }
}

DCFocuserScreen dcFocuserScreen;
