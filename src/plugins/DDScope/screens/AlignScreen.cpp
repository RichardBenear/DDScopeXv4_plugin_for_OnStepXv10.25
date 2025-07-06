// =====================================================
// AlignScreen.cpp
//
// Author: Richard Benear 6/22
#include "../display/Display.h"
#include "AlignScreen.h"
#include "MoreScreen.h"
#include "SHCCatScreen.h"
#include "../catalog/Catalog.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/telescope/mount/coordinates/Transform.h"
#include "src/telescope/mount/goto/Goto.h"
#include "src/telescope/mount/guide/Guide.h"
#include "src/telescope/mount/home/Home.h"
#include "src/lib/tasks/OnTask.h"

#define BIG_BOX_W           80
#define BIG_BOX_H           40
#define A_STATUS_X          115
#define A_STATUS_Y          245
#define A_STATUS_Y_SP       19

#define STATE_UPDATE_W      220
#define STATE_UPDATE_H      15
#define STATE_LABEL_X       95
#define STATE_LABEL_Y       217

#define STATUS_LABEL_X      95
#define STATUS_LABEL_Y      172
#define STATUS_LABEL_W      319-STATUS_LABEL_X
#define STATUS_LABEL_H      15

// Go to Home position button
#define HOME_X              7
#define HOME_Y              175
#define HOME_BOXSIZE_W      BIG_BOX_W
#define HOME_BOXSIZE_H      BIG_BOX_H 

// Align Num Stars Button(s)
#define NUM_S_X             2
#define NUM_S_Y             HOME_Y+HOME_BOXSIZE_H+5
#define NUM_S_BOXSIZE_W     30
#define NUM_S_BOXSIZE_H     35
#define NUM_S_SPACING_X     NUM_S_BOXSIZE_W+3 

// Go to Catalog Page Button
#define ALIGN_CAT_X         HOME_X
#define ALIGN_CAT_Y         NUM_S_Y+NUM_S_BOXSIZE_H+5
#define CAT_BOXSIZE_W       BIG_BOX_W
#define CAT_BOXSIZE_H       BIG_BOX_H 

// GO TO Target Button
#define GOTO_X              HOME_X
#define GOTO_Y              ALIGN_CAT_Y+CAT_BOXSIZE_H+5
#define GOTO_BOXSIZE_W      BIG_BOX_W
#define GOTO_BOXSIZE_H      BIG_BOX_H 

// ALIGN/SYNC this Star Button
#define ALIGN_X             HOME_X
#define ALIGN_Y             GOTO_Y+GOTO_BOXSIZE_H+5
#define ALIGN_BOXSIZE_W     BIG_BOX_W
#define ALIGN_BOXSIZE_H     BIG_BOX_H 

// Write the Alignment Button
#define WRITE_ALIGN_X       HOME_X
#define WRITE_ALIGN_Y       ALIGN_Y+ALIGN_BOXSIZE_H+5
#define SA_BOXSIZE_W        BIG_BOX_W
#define SA_BOXSIZE_H        BIG_BOX_H 

// Start/Clear the Alignment Button
#define START_ALIGN_X       225
#define START_ALIGN_Y       WRITE_ALIGN_Y
#define ST_BOXSIZE_W        BIG_BOX_W
#define ST_BOXSIZE_H        BIG_BOX_H 

// ABORT Button
#define AS_ABORT_X          120
#define AS_ABORT_Y          WRITE_ALIGN_Y
#define AS_ABT_BOXSIZE_W    BIG_BOX_W
#define AS_ABT_BOXSIZE_H    BIG_BOX_H 

AlignStates Current_State = Idle_State;
AlignStates Next_State = Idle_State;

// Align Button object
Button alignButton(0,0,0,0, butOnBackground, butBackground, butOutline, mainFontWidth, mainFontHeight, "");

// Canvas Print object Custom Font
CanvasPrint canvAlignInsPrint(&Inconsolata_Bold8pt7b);

// ---- Draw Alignment Page ----
void AlignScreen::draw() {
  setCurrentScreen(ALIGN_SCREEN);
  alignButton.setColors(butOnBackground, butBackground, butOutline);

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawTitle(100, TITLE_TEXT_Y, "Alignment");
  drawMenuButtons();
  tft.setFont(&Inconsolata_Bold8pt7b);
  drawCommonStatusLabels();
  updateAlignButtons(); // draw initial buttons

  canvAlignInsPrint.printLJ(242, 233, 75, 15, "GuideRate", false);
  char guideRateText[10];
  switch (guide.settings.axis1RateSelect) {
    case 0:  strcpy(guideRateText, " Quarter"); break;
    case 1:  strcpy(guideRateText, "  Half  "); break;
    case 2:  strcpy(guideRateText, "    1X  "); break;
    case 3:  strcpy(guideRateText, "    2X  "); break;
    case 4:  strcpy(guideRateText, "    4X  "); break;
    case 5:  strcpy(guideRateText, "    8X  "); break;
    case 6:  strcpy(guideRateText, "   20X  "); break;
    case 7:  strcpy(guideRateText, "   48X  "); break;
    case 8:  strcpy(guideRateText, "Half Max"); break;
    case 9:  strcpy(guideRateText, "   Max  "); break;
    case 10: strcpy(guideRateText, " Custom "); break;
    default: strcpy(guideRateText, "  Error "); break;
  }
  canvAlignInsPrint.printLJ(247, 249, 67, 15, guideRateText, false);
  showCorrections();
  updateAlignStatus();
  updateCommonStatus();

  if (moreScreen.objectSelected) restoreAlignState(); // coming back from the STARS screen

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  #endif
  #ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("Align");
  #endif
}

// task update for this screen
void AlignScreen::updateAlignStatus() {
  stateMachine();
}

// void AlignScreen::showAlignStatus() {
//   char reply[16];
//   char aStatus[30];
//   int start_y = 215;
//   int start_x = 95;
//   commandWithReply(":A?#", reply); 
//   tft.setCursor(start_x, start_y); 
//   sprintf(aStatus,  "Align Status: %s", reply);
//   canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, 200, 15, aStatus, false);
// }

// state change detection
bool AlignScreen::alignButStateChange() {
  bool changed = false;

  if (preSlewState != mount.isSlewing()) {
    preSlewState = mount.isSlewing(); 
    changed = true;
  }
  
  if (preHomeState != mount.isHome()) {
    preHomeState = mount.isHome(); 
    changed = true;
  }

  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (abortBut) {
      changed = true;
    }
    if (saveAlignBut || syncBut || gotoBut || startAlignBut) {
      changed = true;
    }
  }

  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  return changed;
}

/**************** Alignment Steps *********************
0) Press the [START] Button to begin Alignment
1) Home the Scope using [HOME] button
2) Select number of stars for Alignment, max=4
3) Press [CATALOG] button - Filter ALL_SKY_ALIGN automatically selected
4) On catalog page, select a suitable star
5) On catalog page, use [RETURN] to this page
6) Press [GOTO] Button which slews to star
7) If star not centered, use guide buttons to center
8) Press [SYNC] button to register this star for correction
9) If more stars are to be used, go to step 3, repeat
10) Press the [WRITE] button to save calculations to EEPROM
11) The [ABORT] button resets everything ready for [START] again
********************************************************/

// *********** Update Align Buttons **************
void AlignScreen::updateAlignButtons() {
  int x_offset = 0;

  // Go to Home Position
  if (mount.isSlewing()) {
    alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "Slewing", BUT_ON);                  
  } else {
    alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "Go Home ", BUT_OFF);
  }
    
  // Number of Stars for Alignment Buttons
  // Alignment become active here
  if (numAlignStars == 2) {   
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 3) {   
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 4) {   
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "4", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "4", BUT_OFF);
  } 

  // go to the Star Catalog
  if (catalogBut) {
    alignButton.draw(ALIGN_CAT_X, ALIGN_CAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "CATALOG", BUT_ON);
  } else {
    alignButton.draw(ALIGN_CAT_X, ALIGN_CAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "SEL CATLG", BUT_OFF);         
  }

  // Go To Coordinates Button
  if (gotoBut || mount.isSlewing()) {
    alignButton.draw(GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "Slewing", BUT_ON);
  } else {
    alignButton.draw(GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "GOTO", BUT_OFF);
  }
  
  // SYNC button; calculate alignment parameters
  if (syncBut) {
    alignButton.draw(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, "SYNC'd", BUT_ON);
    display._redrawBut = true;
  } else {
    alignButton.draw(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, "SYNC", BUT_OFF);         
  }

  // save the alignment calculations to EEPROM
  if (saveAlignBut) {
    alignButton.draw(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, "Saved", BUT_ON);
    display._redrawBut = true;
  } else {
    alignButton.draw(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, "SAVE", BUT_OFF);
  }

  // start alignnment
  if (startAlignBut) {
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "Running", BUT_ON);
  } else {
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "START", BUT_OFF);
  }
  
  // Stop Alignment Button
  if (abortBut) {
    alignButton.draw(AS_ABORT_X, AS_ABORT_Y, AS_ABT_BOXSIZE_W, AS_ABT_BOXSIZE_H, "Stop'd", BUT_ON);
    display._redrawBut = true;
    alignButton.draw(AS_ABORT_X, AS_ABORT_Y, AS_ABT_BOXSIZE_W, AS_ABT_BOXSIZE_H, "STOP", BUT_OFF);
  } else {
    alignButton.draw(AS_ABORT_X, AS_ABORT_Y, AS_ABT_BOXSIZE_W, AS_ABT_BOXSIZE_H, "STOP", BUT_OFF);
  }
  updateGuideButtons(); // draw the guide buttons
}

// ===============================================================
// ==== Align State Machine .. updates at the status task rate ===
// ===============================================================
void AlignScreen::stateMachine() {
  Current_State = Next_State;

  // These messages are printed at the bottom of the screen, generally only used for debug.
  switch(Current_State) {
    case 0:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Waiting For Start", false);     break;
    case 1:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Home State", false);            break;
    case 2:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Waiting For Home", false);      break;
    case 3:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Select Numb of Stars", false);  break;
    case 4:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Select Star in Catlg", false);  break;
    case 5:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Wait for Catlg return", false); break;
    case 6:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Go To Star", false);            break;
    case 7:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Wait Slew to finish", false);   break;
    case 8:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Center & Sync", false);         break;
    case 9:  canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Show Coords", false);           break;
    case 10: canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Write Coords", false);          break;
    default: canvAlignInsPrint.printLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State=Waiting for Start", false);     break;
  }
  
  //if (startAlignBut) { 
    sprintf(curAlign,  "Current Star is: %d of %d", alignCurStar, numAlignStars);
    canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y+15, 220, 15, curAlign, false);
  
    // align state machine, messages printed here are near the top half of the screen
    switch(Current_State) {
      case Idle_State: {
        if (startAlignBut) {
          startAlignBut = false;
          Next_State = Home_State;
        } else {
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Press Start to Align", false);
          Next_State = Idle_State;
        }
        break;
      } 

      case Home_State: {
        if (abortBut) {
          homeBut = false;
          abortBut = false;
          Next_State = Idle_State;
        } else if (homeBut) {
          homeBut = false;
            commandBlind(":hC#"); //  go HOME
            Next_State = Wait_For_Home_State;   
        } else {
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Not Home", false);
          Next_State = Home_State; 
        }
        break;
      } 

      case Wait_For_Home_State: {
        if (abortBut) {
          abortBut = false;
          Next_State = Idle_State;
        } else if (mount.isSlewing()) { 
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Slewing", false);
          Next_State = Wait_For_Home_State;
        } else if (mount.isHome()) {
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Is Home", false);
          Next_State = Num_Stars_State; 
        } else {
          Next_State = Wait_For_Home_State;
        }
        break;
      }

      case Num_Stars_State: {
        if (abortBut) {
          abortBut = false;
          numAlignStars = 0;
          Next_State = Idle_State;
        } else if (numAlignStars > 1) {
          char s[10]; 
          sprintf (s, ":A%d#", numAlignStars); // set number of align stars
          SERIAL_DEBUG.print("numAlignStars="); SERIAL_DEBUG.println(numAlignStars);
          commandBool(s);
          if (!commandBool(s)) {
            Next_State = Select_Catalog_State;
          } else {
            canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y+30, 200, 15, "Failed: Start Align", true);
            numAlignStars = 0;
            Next_State = Idle_State;
          }
          Next_State = Select_Catalog_State; 
        } else {
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Select Number of Stars", false);
          Next_State = Num_Stars_State;
        }
        break;
      } 

      case Select_Catalog_State: {
        if (abortBut) {
          abortBut = false;
          catalogBut = false;
          Next_State = Idle_State;
        } else if (catalogBut) {
          catalogBut = false;
          //alignCurStar++;
          Next_State = Wait_Catalog_State;
          moreScreen.activeFilter = FM_ALIGN_ALL_SKY;
          cat_mgr.filterAdd(moreScreen.activeFilter); 
          saveAlignState();
          shcCatScreen.init(STARS); // draw the STARS SCREEN
          return;
        } else { // stay here
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Select Star in Catalog", false);
          Next_State = Select_Catalog_State;
        }
        break;
      }
          
      case Wait_Catalog_State: {
        if (abortBut) {
          abortBut = false;
          Next_State = Idle_State;
        } else if (display.currentScreen == ALIGN_SCREEN) { // doesn't change state until Catalog returns back to this page
          if (moreScreen.objectSelected) { // a star has been selected from the catalog
            Next_State = Goto_State;
          }
        } else {
          Next_State = Wait_Catalog_State;
        }
        break;
      }   

      char reply[13];
      case Goto_State: {
        if (abortBut) {
          abortBut = false;
          gotoBut = false;
          Next_State = Idle_State;
        } else if (gotoBut) {
          if (!commandWithReply(":MS#", reply)) {
            Next_State = Wait_For_Slewing_State;
            gotoBut = false;
          } else { // Goto fails
            Next_State = Wait_Catalog_State;
          }
        } else { // wait for GoTo button press
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Press GoTo", false);
          Next_State = Goto_State;
        }
        break;
      }

      case Wait_For_Slewing_State: {
        if (abortBut) {
          abortBut = false;
          Next_State = Idle_State;
        } else if (mount.isSlewing()) { 
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Slewing", false);
          Next_State = Wait_For_Slewing_State;
        } else { // not slewing
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "GoTo Done", false);
          Next_State = Sync_State;
        }
        break;
      }

      case Sync_State: { 
        if (abortBut) {
          abortBut = false;
          syncBut = false;
          Next_State = Idle_State;
        } else if (syncBut) {
          alignCurStar++;
          commandBool(":A+#");
          if (!commandBool(":A+#")) {
            syncBut = false;
            SERIAL_DEBUG.print("CurStar="); SERIAL_DEBUG.println(alignCurStar);
            if (alignCurStar < numAlignStars) { // more stars to align? 
              Next_State = Select_Catalog_State;
            } else {
              Next_State = Status_State;
            }
          } else {
            canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y+30, 200, 15, "Failed: Adding Star", true);;
            Next_State = Idle_State;
          }
        } else { 
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Center Star then Press Sync", false);
          Next_State = Sync_State;
        } 
        break;
      }

      case Status_State: {
        showCorrections(); // show current align corrections
        Next_State = Write_State;
        break;
      }

      case Write_State: {
        if (abortBut) {
          abortBut = false;
          saveAlignBut = false;
          Next_State = Idle_State;
        } else if (saveAlignBut) {
          if (commandBool(":AW#")) {
            canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Writing Align Data", false);
            saveAlignBut = false;
            saveAlignState();
            Next_State = Idle_State;
          } else {
            canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y+30, 200, 15, "Failed: Writing Coord's", true);;
            Next_State = Idle_State;
          }
        } else {
          canvAlignInsPrint.printLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Waiting for Save", false);
          Next_State = Write_State;
        }
        break;
      }

      default: Next_State = Idle_State; 
    } // end switch current state
} // end State Machine

// ================= Check Align Buttons ===================
// -- return true if touched --
bool AlignScreen::touchPoll(uint16_t px, uint16_t py) { 
  char reply[2];
  // ==== ABORT GOTO  Button ====
  if (py > AS_ABORT_Y && py < (AS_ABORT_Y + AS_ABT_BOXSIZE_H) && px > AS_ABORT_X && px < (AS_ABORT_X + AS_ABT_BOXSIZE_W)) {
    BEEP;
    commandBool(":Q#"); // stops move
    digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZM LED
    axis1.enable(false);
    digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
    axis2.enable(false);
    commandBool(":Td#"); // Disable Tracking
    char numClear[10];
    sprintf (numClear, ":A%d#", 0); // set number of align stars
    commandBool(numClear);

    // clear all button states since not sure which was active
    numAlignStars = 0;
    alignCurStar = 0;
    abortBut = true; // this should trigger the State Machine to return to Idle State
    homeBut = false;
    catalogBut = false;
    gotoBut = false;
    syncBut = false;
    saveAlignBut = false;
    startAlignBut = false;
    Next_State = Idle_State;
    return true;
  }

  // Go to Home Telescope Requested
  if (px > HOME_X && px < HOME_X + HOME_BOXSIZE_W && py > HOME_Y  && py < HOME_Y + HOME_BOXSIZE_H) {
    if (Current_State==Home_State) {
      BEEP;
      homeBut = true;
      return true;
    }
  }
  
  // Number of Stars for alignment
  int x_offset = 0;
  if (Current_State==Num_Stars_State) {
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 2;
      return true;
    }
    x_offset += NUM_S_SPACING_X;
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 3;
      return true;
    }
    x_offset += NUM_S_SPACING_X;
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 4;
      return true;
    }
  }

  // Call up the Catalog Button
  if (py > ALIGN_CAT_Y && py < (ALIGN_CAT_Y + CAT_BOXSIZE_H) && px > ALIGN_CAT_X && px < (ALIGN_CAT_X + CAT_BOXSIZE_W)) {
    if (Current_State==Select_Catalog_State ) {
      BEEP;
      catalogBut = true;
      return true;
    }
  }

  // Go To Target Coordinates
  if (py > GOTO_Y && py < (GOTO_Y + GOTO_BOXSIZE_H) && px > GOTO_X && px < (GOTO_X + GOTO_BOXSIZE_W)) {
    if (Current_State==Goto_State) { 
      BEEP;
      gotoBut = true;
      return true;
    }
  }

  // SYNC - calculate alignment corrections Button
  if (py > ALIGN_Y && py < (ALIGN_Y + ALIGN_BOXSIZE_H) && px > ALIGN_X && px < (ALIGN_X + ALIGN_BOXSIZE_W)) { 
    if (Current_State==Sync_State) {
      BEEP;
      syncBut = true;
      return true;
    }
  }

  // Write Alignment Button
  if (py > WRITE_ALIGN_Y && py < (WRITE_ALIGN_Y + SA_BOXSIZE_H) && px > WRITE_ALIGN_X && px < (WRITE_ALIGN_X + SA_BOXSIZE_W)) {
    if (Current_State==Write_State) {
      BEEP;
      saveAlignBut = true;
      return true;
    }
  }  

  // START Alignment Button - clear the corrections, reset the state machine
  if (py > START_ALIGN_Y && py < (START_ALIGN_Y + ST_BOXSIZE_H) && px > START_ALIGN_X && px < (START_ALIGN_X + ST_BOXSIZE_W)) { 
    BEEP;
    startAlignBut = true;
    alignCurStar = 0;
    numAlignStars = 0; // number of selected align stars from buttons
    transform.align.modelClear();
    Current_State = Idle_State;

    // Enable the Motors
    digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
    axis1.enable(true); // AZ motor on
    
    digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
    axis2.enable(true); // ALT motor on
    return true;
  } 

  // EAST / RIGHT button
  if (py > r_y && py < (r_y + box_h) && px > r_x && px < (r_x + box_w)) {
    BEEP;
    if (!guidingEast) {
      commandWithReply(":Mw#", reply); // east west is swapped for DDScope
      guidingEast = true;
      guidingWest = false;
      guidingNorth = false;
      guidingSouth = false;
    } else {
      commandBool(":Qw#");
      guidingEast = false;
    }
    return true;
  }
                  
  // WEST / LEFT button
  if (py > l_y && py < (l_y + box_h) && px > l_x && px < (l_x + box_w)) {
    BEEP;
    if (!guidingWest) {
      commandWithReply(":Me#", reply); // east west is swapped for DDScope
      guidingEast = false;
      guidingWest = true;
      guidingNorth = false;
      guidingSouth = false;
    } else {
      commandBool(":Qe#");
      guidingWest = false;
    }
    return true;
  }
                  
  // NORTH / UP button
  if (py > u_y && py < (u_y + box_h) && px > u_x && px < (u_x + box_w)) {
    BEEP;
    if (!guidingNorth) {
      commandWithReply(":Mn#", reply);
      guidingEast = false;
      guidingWest = false;
      guidingNorth = true;
      guidingSouth = false;
    } else {
      commandBool(":Qn#");
      guidingNorth = false;
    }
    return true;
  }
                  
  // SOUTH / DOWN button
  if (py > d_y && py < (d_y + box_h) && px > d_x && px < (d_x + box_w)) {
    BEEP;
    if (!guidingSouth) {
      commandWithReply(":Ms#", reply);
      guidingEast = false;
      guidingWest = false;
      guidingNorth = false;
      guidingSouth = true;
    } else {
      commandBool(":Qs#");
      guidingSouth = false;
    }
    return true;
  }

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);
  
  return false;
}

// Align Screen state managment...needed for returning from STARS to ALIGN SCREEN
static uint8_t _numAlignStars = 0;
static uint8_t _alignCurStar = 0;
static bool _homeBut = false;
static bool _catalogBut = false;
static bool _gotoBut = false;
static bool _syncBut = false;
static bool _saveAlignBut = false;
static bool _startAlignBut = false;
static AlignStates _Next_State = Idle_State;
static AlignStates _Current_State = Idle_State;

void AlignScreen::restoreAlignState() {
  numAlignStars = _numAlignStars;
  alignCurStar = _alignCurStar;
  homeBut = _homeBut;
  catalogBut = _catalogBut;
  gotoBut = _gotoBut;
  syncBut = _syncBut;
  saveAlignBut = _saveAlignBut;
  startAlignBut = _startAlignBut;
  Next_State = _Next_State;
  Current_State = _Current_State;
}

void AlignScreen::saveAlignState() {
  _numAlignStars = numAlignStars;
  _alignCurStar = alignCurStar;
  _homeBut = homeBut;
  _catalogBut = catalogBut;
  _gotoBut = gotoBut;
  _syncBut = syncBut;
  _saveAlignBut = saveAlignBut;
  _startAlignBut = startAlignBut;
  _Next_State = Next_State;
  _Current_State = Current_State;
}

// ========== Update Guide Page Buttons ==========
void AlignScreen::updateGuideButtons() {
  if (guidingEast && mount.isSlewing()) { 
    alignButton.draw(r_x, r_y, box_w, box_h, "EAST", BUT_ON);
  } else {
    alignButton.draw(r_x, r_y, box_w, box_h, "EAST", BUT_OFF);
  }

  if (guidingWest && mount.isSlewing()) { 
    alignButton.draw(l_x, l_y, box_w, box_h, "WEST", BUT_ON);
  } else {
    alignButton.draw(l_x, l_y, box_w, box_h, "WEST", BUT_OFF);
  }

 if (guidingNorth && mount.isSlewing()) { 
    alignButton.draw(u_x, u_y, box_w, box_h, "NORTH", BUT_ON);
  } else {
    alignButton.draw(u_x, u_y, box_w, box_h, "NORTH", BUT_OFF);
  }

  if (guidingSouth && mount.isSlewing()) { 
    alignButton.draw(d_x, d_y, box_w, box_h, "SOUTH", BUT_ON);
  } else {
    alignButton.draw(d_x, d_y, box_w, box_h, "SOUTH", BUT_OFF);
  }
}

// ***** Show Calculated Corrections ******
// ax1Cor: align internal index for Axis1
// ax2Cor: align internal index for Axis2
// altCor: for geometric coordinate correction/align, - is below the pole, + above
// azmCor: - is right of the pole, + is left
// doCor: declination/optics orthogonal correction
// pdCor: declination/polar orthogonal correction
// dfCor: fork or declination axis flex
// tfCor: tube flex
void AlignScreen::showCorrections() {

  int x_off = 0;
  int y_off = 0;
  int acorr_x = 105;
  int acorr_y = 330;
  int acorr_w = 100;
  int acorr_h = 15;
  char _reply[10];
  char acorr[20];

  tft.drawRect(acorr_x, acorr_y, acorr_w, 5*acorr_h, pgBackground); // clear background
  tft.setCursor(acorr_x+5, acorr_y); tft.print("Corrections (arcSecs)");

  y_off += 14;
  commandWithReply(":GX00#", _reply); 
  sprintf(acorr,"ax1Cor=%s", _reply); // ax1Cor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX01#", _reply); 
  sprintf(acorr,"ax2Cor=%s", _reply); // ax2Cor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX02#", _reply); 
  sprintf(acorr,"altCor=%s", _reply); // altCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX03#", _reply); 
  sprintf(acorr,"azmCor=%s", _reply); // azmCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  x_off = 105;
  y_off = 14;
  commandWithReply(":GX04#", _reply); 
  sprintf(acorr," doCor=%s", _reply);  // doCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX05#", _reply); 
  sprintf(acorr," pdCor=%s", _reply);  // pdCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX06#", _reply); 
  sprintf(acorr," ffCor=%s", _reply);  // ffCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  commandWithReply(":GX07#", _reply); 
  sprintf(acorr," dfCor=%s", _reply);  // dfCor
  canvAlignInsPrint.printLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);
}

AlignScreen alignScreen;
