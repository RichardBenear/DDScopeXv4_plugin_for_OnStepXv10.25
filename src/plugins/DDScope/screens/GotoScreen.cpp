// =====================================================
// GotoScreen.cpp
//
// Author: Richard Benear 2021
#include "../display/Display.h"
#include "GotoScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include "src/telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"

#define NUM_BUTTON_X         2
#define NUM_BUTTON_Y         252
#define NUM_BUTTON_W         50
#define NUM_BUTTON_H         45
#define NUM_BUTTON_SPACING_X 1
#define NUM_BUTTON_SPACING_Y 1

#define TEXT_LABEL_X         5
#define TEXT_LABEL_Y         185
#define TEXT_FIELD_X         135
#define TEXT_FIELD_Y         TEXT_LABEL_Y
#define TEXT_FIELD_WIDTH     65
#define TEXT_FIELD_HEIGHT    26
#define TEXT_SPACING_X       10
#define TEXT_SPACING_Y       TEXT_FIELD_HEIGHT

#define RA_SELECT_X          205
#define RA_SELECT_Y          166
#define RA_CLEAR_X           265
#define RA_CLEAR_Y           RA_SELECT_Y
#define CO_BOXSIZE_X         50
#define CO_BOXSIZE_Y         24

#define DEC_SELECT_X         RA_SELECT_X
#define DEC_SELECT_Y         192
#define DEC_CLEAR_X          RA_CLEAR_X
#define DEC_CLEAR_Y          DEC_SELECT_Y

#define SEND_BUTTON_X        215
#define SEND_BUTTON_Y        220
#define SEND_BOXSIZE_X       70
#define SEND_BOXSIZE_Y       40

#define SLEW_R_BUTTON_X      190
#define SLEW_R_BUTTON_Y      266
#define SLEW_R_BOXSIZE_X     120
#define SLEW_R_BOXSIZE_Y     25

#define POL_BUTTON_X         40
#define POL_BUTTON_Y         219
#define POL_BOXSIZE_X        100
#define POL_BOXSIZE_Y        28

#define GOTO_BUTTON_X        195
#define GOTO_BUTTON_Y        300
#define GOTO_BOXSIZE_X       110
#define GOTO_BOXSIZE_Y       50

#define ABORT_BUTTON_X       GOTO_BUTTON_X
#define ABORT_BUTTON_Y       360

#define RA_CMD_ERR_X         5
#define RA_CMD_ERR_Y         229
#define DEC_CMD_ERR_X        5
#define DEC_CMD_ERR_Y        246
#define CMD_ERR_W            180
#define CMD_ERR_H            19

#define CUSTOM_FONT_OFFSET   -15

// Go To Screen Button object
Button gotoButton(
                0, 0, 0, 0,
                butOnBackground, 
                butBackground, 
                butOutline, 
                mainFontWidth, 
                mainFontHeight, 
                "");

                // Go To Screen Button object
Button gotoLargeButton(
                0, 0, 0, 0,
                butOnBackground, 
                butBackground, 
                butOutline, 
                largeFontWidth, 
                largeFontHeight, 
                "");

char numLabels[12][3] = {"9", "8", "7", "6", "5", "4", "3", "2", "1", "-", "0", "+"};

// Draw the Go To Page
void GotoScreen::draw() {
  setCurrentScreen(GOTO_SCREEN);
  gotoButton.setColors(butOnBackground, butBackground, butOutline);
  gotoLargeButton.setColors(butOnBackground, butBackground, butOutline);

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawTitle(120, TITLE_TEXT_Y, "Go To");
  drawMenuButtons();
  tft.setFont(&Inconsolata_Bold8pt7b);

  drawCommonStatusLabels();
  updateGotoButtons();

  RAtextIndex = 0; 
  DECtextIndex = 0; 

  // Draw RA and DEC Coordinate Labels
  tft.setCursor(160, 430);
  tft.print("Assumes Epoch J2000");
  tft.setCursor(TEXT_LABEL_X, TEXT_LABEL_Y);
  tft.print(" RA  (hhmm[ss]):");
  tft.setCursor(TEXT_LABEL_X, TEXT_LABEL_Y+TEXT_SPACING_Y);
  tft.print("DEC(sddmm[sec]):");

  // Draw Key Pad
  int z=0;
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      gotoButton.draw(NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X), 
              NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y), 
              NUM_BUTTON_W, NUM_BUTTON_H, numLabels[z], BUT_OFF);
      z++;
    }
  }

  // show number input fields
  tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9,  butBackground);
  tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9,  butBackground);
  
  updateCommonStatus();
  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  #endif
  #ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("Goto");
  #endif
} // end initialize

// task update for this screen
void GotoScreen::updateGotoStatus() {
  //
}

// assign label to pressed button field
void GotoScreen::processNumPadButton() {
  if (numDetected) {
    if (RAselect && (buttonPosition >= 0 && (buttonPosition < 9 || buttonPosition == 10)) && RAtextIndex < 6) {
      RAtext[RAtextIndex] = numLabels[buttonPosition][0];
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9,  butBackground);
      tft.setCursor(TEXT_FIELD_X, TEXT_FIELD_Y);
      tft.print(RAtext); 
      RAtextIndex++;
    }

    if (DECselect && (((DECtextIndex == 0 && (buttonPosition == 9 || buttonPosition == 11)) 
      || (DECtextIndex>0 && (buttonPosition!=9||buttonPosition!=11)))) && DECtextIndex < 7) {
      DECtext[DECtextIndex] = numLabels[buttonPosition][0]; 
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9,  butBackground);
      tft.setCursor(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y);
      tft.print(DECtext);
      DECtextIndex++;
    }
    numDetected = false;
  }
}

bool GotoScreen::gotoButStateChange() {
  bool changed = false;

  if (preSlewState != mount.isSlewing()) {
    preSlewState = mount.isSlewing(); 
    changed = true;
  }
  
  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (abortPgBut || setPolOn || sendOn || DECclear || RAclear) {
      changed = true;
    }

    if (RAselect || DECselect){
      changed = true;
    }
  }
  
  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }

  return changed;
}

// ==== Update any changing Status for GO TO Page ====
void GotoScreen::updateGotoButtons() {
    
  // Get button and print label
  switch (buttonPosition) {
    case 0:  processNumPadButton(); break;
    case 1:  processNumPadButton(); break;
    case 2:  processNumPadButton(); break;
    case 3:  processNumPadButton(); break;
    case 4:  processNumPadButton(); break;
    case 5:  processNumPadButton(); break;
    case 6:  processNumPadButton(); break;
    case 7:  processNumPadButton(); break;
    case 8:  processNumPadButton(); break;
    case 9:  processNumPadButton(); break;
    case 10: processNumPadButton(); break;
    case 11: processNumPadButton(); break;
    display._redrawBut = true;
    default: break;
  }

  // RA Select Button
  if (RAselect) {
    gotoButton.draw(RA_SELECT_X,  RA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "RaSel", BUT_ON);
  } else {
    gotoButton.draw(RA_SELECT_X,  RA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "RaSel", BUT_OFF);
  }

  // RA Clear button
  if (RAclear) {
    gotoButton.draw(RA_CLEAR_X,   RA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "RaClr", BUT_ON);
    tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, butBackground);
    memset(RAtext,0,sizeof(RAtext)); // clear RA buffer
    tft.fillRect(RA_CMD_ERR_X, RA_CMD_ERR_Y+CUSTOM_FONT_OFFSET, CMD_ERR_W, CMD_ERR_H, pgBackground);
    RAtextIndex = 0;
    buttonPosition = 12;
    RAclear = false;
    display._redrawBut = true;
  } else {
    gotoButton.draw(RA_CLEAR_X,   RA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "RaClr", BUT_OFF);
  }
  
  // DEC Select button
  if (DECselect) {
    gotoButton.draw(DEC_SELECT_X, DEC_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "DeSel", BUT_ON);
  } else {
    gotoButton.draw(DEC_SELECT_X, DEC_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "DeSel", BUT_OFF); 
  }

  // DEC Clear Button
  if (DECclear) {
    gotoButton.draw(DEC_CLEAR_X,  DEC_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "DeClr", BUT_ON);
    tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET+TEXT_FIELD_HEIGHT, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, butBackground);
    memset(DECtext,0,sizeof(DECtext)); // clear DEC buffer
    tft.fillRect(DEC_CMD_ERR_X, DEC_CMD_ERR_Y+CUSTOM_FONT_OFFSET, CMD_ERR_W, CMD_ERR_H, pgBackground);
    DECtextIndex = 0;
    buttonPosition = 12;
    DECclear = false;
    display._redrawBut = true;
  } else {
    gotoButton.draw(DEC_CLEAR_X,  DEC_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, "DeClr", BUT_OFF);
  }
  
  // Send Coordinates Button
  if (sendOn) {
    gotoButton.draw(SEND_BUTTON_X, SEND_BUTTON_Y, SEND_BOXSIZE_X, SEND_BOXSIZE_Y, "Sent", BUT_ON);
    sendOn = false;
    display._redrawBut = true; 
  } else {
    gotoButton.draw(SEND_BUTTON_X, SEND_BUTTON_Y, SEND_BOXSIZE_X, SEND_BOXSIZE_Y, "Send", BUT_OFF);
  }

  // Slew Rate Button
  char temp[16]="";
  commandWithReply(":GX92#", cRate); // get current rate 
  cRateF = atol(cRate);
  commandWithReply(":GX93#", bRate); // get base rate
  bRateF = atol(bRate);
  rateRatio = bRateF/cRateF;
  rateRatio = roundf(rateRatio * 100) / 100; // get rid of extra digits
  sprintf(temp, "Slew Rate %0.2f", rateRatio);
  gotoButton.draw(SLEW_R_BUTTON_X, SLEW_R_BUTTON_Y, SLEW_R_BOXSIZE_X, SLEW_R_BOXSIZE_Y, temp, BUT_OFF); 

  // Quick Set Polaris Target Button
  if (setPolOn) {
    gotoButton.draw(POL_BUTTON_X, POL_BUTTON_Y, POL_BOXSIZE_X, POL_BOXSIZE_Y, "Setting", BUT_ON);
    setPolOn = false;
    display._redrawBut = true; 
  } else {
    gotoButton.draw(POL_BUTTON_X, POL_BUTTON_Y, POL_BOXSIZE_X, POL_BOXSIZE_Y, "Set Polaris", BUT_OFF);
  }

  tft.setFont(&UbuntuMono_Bold11pt7b);  
  // Go To Coordinates Button
  if (goToButton) {
    gotoLargeButton.draw(GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, "Slewing", BUT_ON);
    goToButton = false;
  } else if (mount.isSlewing()) { 
    gotoLargeButton.draw(GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, "Slewing", BUT_ON);
  } else {
    gotoLargeButton.draw(GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, "Go To", BUT_OFF);
  }
  
  // Stop move GoTo Button
  if (abortPgBut) {
    gotoLargeButton.draw(ABORT_BUTTON_X, ABORT_BUTTON_Y, GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, "Stopping", BUT_ON);
    abortPgBut = false;
    display._redrawBut = true;
  } else {
    gotoLargeButton.draw(ABORT_BUTTON_X, ABORT_BUTTON_Y, GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, "Stop", BUT_OFF);
  }
  tft.setFont(&Inconsolata_Bold8pt7b); 
}

// ==== TouchScreen was touched, determine which button ====
bool GotoScreen::touchPoll(uint16_t px, uint16_t py) {
  char temp[14] = "";

  //were number Pad buttons pressed?
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      if (py >   NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y) 
       && py <  (NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y)) + NUM_BUTTON_H 
       && px >   NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X) 
       && px <  (NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X) + NUM_BUTTON_W)) {
        BEEP;
        buttonPosition=row*3+col;
        //VF("buttonPosition="); VL(buttonPosition);
        numDetected = true;
        return true;
      }
    }
  }

  // Select RA field
  if (py > RA_SELECT_Y && py < (RA_SELECT_Y + CO_BOXSIZE_Y) && px > RA_SELECT_X && px < (RA_SELECT_X + CO_BOXSIZE_X)) {
    BEEP;
    RAselect = true; 
    DECselect = false;
    return true;
  }

  // Clear RA field
  if (py > RA_CLEAR_Y && py < (RA_CLEAR_Y + CO_BOXSIZE_Y) && px > RA_CLEAR_X && px < (RA_CLEAR_X + CO_BOXSIZE_X)) {
    BEEP;
    RAclear = true; 
    RAtextIndex = 0;
    buttonPosition = 12;
    return true; 
  }

  // Select DEC field
  if (py > DEC_SELECT_Y && py < (DEC_SELECT_Y + CO_BOXSIZE_Y) && px > DEC_SELECT_X && px < (DEC_SELECT_X + CO_BOXSIZE_X)) {
    BEEP;
    DECselect = true;
    RAselect = false;
    return true;
  }

  // Clear DEC field
  if (py > DEC_CLEAR_Y && py < (DEC_CLEAR_Y + CO_BOXSIZE_Y) && px > DEC_CLEAR_X && px < (DEC_CLEAR_X + CO_BOXSIZE_X)) {
    BEEP;
    DECclear = true; 
    DECtextIndex = 0;
    buttonPosition = 12;
    return true; 
  }

  // SEND Coordinates
  if (py > SEND_BUTTON_Y && py < (SEND_BUTTON_Y + SEND_BOXSIZE_Y) && px > SEND_BUTTON_X && px < (SEND_BUTTON_X + SEND_BOXSIZE_X)) {
    BEEP;
    sendOn = true; 
    RAtextIndex = 0;
    DECtextIndex = 0;
    buttonPosition = 12; 
    
    if (RAselect) {
      //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
      sprintf(temp, ":Sr%c%c:%c%c:%c%c#", RAtext[0], RAtext[1], RAtext[2], RAtext[3], RAtext[4], RAtext[5]);
      commandBool(temp);
    } else if (DECselect) {
      //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]# 
      sprintf(temp, ":Sd%c%c%c:%c%c:%c%c#", DECtext[0], DECtext[1], DECtext[2], DECtext[3], DECtext[4], DECtext[5], DECtext[6]);
      commandBool(temp);
    }
    updateAltAzmTarget();
    return true;
  }

  // Quick set Polaris Target
  if (py > POL_BUTTON_Y && py < (POL_BUTTON_Y + POL_BOXSIZE_Y) && px > POL_BUTTON_X && px < (POL_BUTTON_X + POL_BOXSIZE_X)) {
    BEEP;
    setPolOn = true;
    gotoScreen.setTargPolaris(); 
    return true;
  }

  // ==== Go To Target Coordinates ====
  if (py > GOTO_BUTTON_Y && py < (GOTO_BUTTON_Y + GOTO_BOXSIZE_Y) && px > GOTO_BUTTON_X && px < (GOTO_BUTTON_X + GOTO_BOXSIZE_X)) {
    BEEP;
    goToButton = true;
    commandBool(":Te#"); // Enable Tracking
    commandWithReply(":MS#", temp);
    return true;
  }

  // ==== ABORT GOTO ====
  // If tracking, will continue to track
  if (py > ABORT_BUTTON_Y && py < (ABORT_BUTTON_Y + GOTO_BOXSIZE_Y) && px > ABORT_BUTTON_X && px < (ABORT_BUTTON_X + GOTO_BOXSIZE_X)) {
    ALERT;
    abortPgBut = true;
    commandBool(":Q#"); // stops move
    digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZM LED
    digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
    return true;
  }

  // SLew Rate Button, circular selection each button press (.5X, 1X, 2X) - default 1X
  if (py > SLEW_R_BUTTON_Y && py < (SLEW_R_BUTTON_Y + SLEW_R_BOXSIZE_Y) && px > SLEW_R_BUTTON_X && px < (SLEW_R_BUTTON_X + SLEW_R_BOXSIZE_X)) {
    BEEP;
    if      (rateRatio >= 0.86F && rateRatio <= 1.35F) {commandBool(":SX93,2#"); return true;} // if 1 then 1.5
    else if (rateRatio >= 1.36F && rateRatio <= 1.75F) {commandBool(":SX93,1#"); return true;} // if 1.5 then 2.0
    else if (rateRatio >= 1.76F && rateRatio <= 2.35F) {commandBool(":SX93,5#"); return true;} // if 2.0 then 0.5
    else if (rateRatio >= 0.30F && rateRatio <= 0.65F) {commandBool(":SX93,4#"); return true;} // if 0.5 then 0.75
    else if (rateRatio >= 0.66F && rateRatio <= 0.85F) {commandBool(":SX93,3#"); return true;} // if 0.75 then 1.00
    else return false; 
  }

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);

  return false;
}

 // Quick set the target to Polaris
void GotoScreen::setTargPolaris() {
  // Polaris location RA=02:31:49.09, Dec=+89:15:50.8 (2.5303, 89.2641)
  commandBool(":Sr02:31:49#");
  commandBool(":Sd+89:15:50#"); 
  updateAltAzmTarget();
}

GotoScreen gotoScreen;
