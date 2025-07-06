// =====================================================
// TreasureCat.cpp
//
// "Treasure" Catalog Screen
// Author: Richard Benear 6/22
//
// The Treasure catalog is a compilation of popular objects from :rDUINO Scope-http://www.rduinoscope.tk/index.html
// The <mod1_treasure.csv> file must be stored on the SD card for this to work
#include "../display/Display.h"
#include "MoreScreen.h"
#include "TreasureCatScreen.h"
#include "../catalog/Catalog.h"
#include "../catalog/CatalogTypes.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/telescope/mount/goto/Goto.h"
#include "src/telescope/mount/coordinates/Transform.h"

#define BACK_X              5
#define BACK_Y              445
#define BACK_W              60
#define BACK_H              35

#define NEXT_X              255
#define NEXT_Y              BACK_Y

#define RETURN_X            165
#define RETURN_Y            BACK_Y
#define RETURN_W            80

#define SAVE_LIB_X          75
#define SAVE_LIB_Y          BACK_Y
#define SAVE_LIB_W          80
#define SAVE_LIB_H          BACK_H

#define STATUS_STR_X        3
#define STATUS_STR_Y        430
#define STATUS_STR_W        150
#define STATUS_STR_H        16

#define CAT_X               3
#define CAT_Y               50
#define CAT_W               112
#define CAT_H               21
#define CAT_Y_SPACING       1

#define SUB_STR_X_OFF       2
#define FONT_Y_OFF          7


// Catalog Button object for default Arial font
Button treasureDefButton(0, 0, 0, 0, butOnBackground, butBackground, butOutline, defFontWidth, defFontHeight, "");

// Catalog Button object for custom font
Button treasureCatButton(0, 0, 0, 0, butOnBackground, butBackground, butOutline, mainFontWidth, mainFontHeight, "");

// Canvas Print object default Arial 6x9 font
CanvasPrint canvTreasureDefPrint(display.default_font);

// Canvas Print object, Inconsolata_Bold8pt7b font
CanvasPrint canvTreasureInsPrint(&Inconsolata_Bold8pt7b);

//==================================================================
void TreasureCatScreen::init() { 
  returnToPage = display.currentScreen; // save page from where this function was called so we can return
  setCurrentScreen(TREASURE_SCREEN);
  treasureDefButton.setColors(butOnBackground, butBackground, butOutline);
  treasureCatButton.setColors(butOnBackground, butBackground, butOutline);

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  moreScreen.objectSelected = false;
  tCurrentPage = 0; 
  tPrevPage = 0;
  tEndOfList = false;
  tAbsRow = 0; // initialize the absolute index pointer into total array

  // Show Page Title
  drawTitle(110, TITLE_TEXT_Y, "Treasure");
  if (!loadTreasureArray()) {
      canvTreasureInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "ERR:Loading Treasure", false);
  } else { 
      parseTcatIntoArray();
  }
  
  drawTreasureCat(); // draw first page of the selected catalog

  tft.setFont(&Inconsolata_Bold8pt7b);
  treasureCatButton.draw(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", BUT_OFF);
  treasureCatButton.draw(NEXT_X, NEXT_Y, BACK_W, BACK_H, "NEXT", BUT_OFF);
  treasureCatButton.draw(RETURN_X, RETURN_Y, RETURN_W, BACK_H, "RETURN", BUT_OFF);
  treasureCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SAVE LIB", BUT_OFF);

  updateTreasureButtons(); 
}

bool TreasureCatScreen::loadTreasureArray() {
  //============== Begin Load from File ==========================
  // Load RAM array from SD catalog file
  char rdChar;
  char rowString[SD_CARD_LINE_LEN]=""; // temp row buffer
  int rowNum=0;
  int charNum=0;
  File rdFile = SD.open("mod1_treasure.csv");

  if (rdFile) {
    // load data from SD into array
    while (rdFile.available()) {
      rdChar = rdFile.read();
      rowString[charNum] = rdChar;
      charNum++;
      
      if (rdChar == '\n') {
        strcpy(Treasure_Array[rowNum], rowString);
        rowNum++;
        memset(&rowString, 0, SD_CARD_LINE_LEN);
        charNum = 0;
      }
    }
    treRowEntries = rowNum-1;
    //VF("treEntries="); VL(treRowEntries);
  } else {
    VLF("SD Treas read error");
    return false;
  }
  rdFile.close();
  return true;
}

// The parsing of the mod1_treasure.csv file is based on these first few lines
// as an example.
// NGC189 ;00h39.6m;+61d06m;Cari;Open Clus; 8.8;5m       ;Caroline Herschel
// NGC225 ;00h43.6m;+61d46m;Cari;Open Clus; 7.0;15m      ;Caroline Herschel
// NGC281 ;00h52.8m;+56d37m;Cari;Brigt Neb; 7.8;35m-30m  ;Pacman           
// NGC288 ;00h52.8m;-26d35m;Sagi;Glob Clus; 7.9;13m      ;None             
// NGC404 ;01h09.4m;+35d43m;Andr;Galaxy   ; 9.8;6.6m     ;Mirachms Ghost   
// NGC584 ;01h31.3m;-06d52m;Ceph;Galaxy   ;10.4;3.8m-2.4m;Little Spindle   
//
// Parse the treasure.csv line data into individual fields in an array
// mod1_treasure.csv file format = ObjName;RAhRAm;SignDECdDECm;Cons;ObjType;Mag;Size;SubId\n
// mod1_treasure.csv field spacing: 7;8;7;4;9;4;9;18 = 66 total w/out semicolons, 73 with
// 7 - objName
// 8 - RA
// 7 - DEC
// 4 - Cons
// 9 - ObjType
// 4 - Mag
// 9 - Size
//18 - SubId
void TreasureCatScreen::parseTcatIntoArray() {
  for (int i=0; i<MAX_TREASURE_ROWS; i++) {
    tArray[i].tObjName      = strtok(Treasure_Array[i], ";"); //VL(tArray[i].tObjName);
    tArray[i].tRAhRAm       = strtok(NULL, ";");              //VL(tArray[i].tRAhRAm);
    tArray[i].tsDECdDECm    = strtok(NULL, ";");              //VL(tArray[i].tsDECdDECm);
    tArray[i].tCons         = strtok(NULL, ";");              //VL(tArray[i].tCons);
    tArray[i].tObjType      = strtok(NULL, ";");              //VL(tArray[i].tObjType);
    tArray[i].tMag          = strtok(NULL, ";");              //VL(tArray[i].tMag);
    tArray[i].tSize         = strtok(NULL, ";");              //VL(tArray[i].tSize);
    tArray[i].tSubId        = strtok(NULL, "\r");             //VL(tArray[i].tSubId);
  }
}

// ========== draw TREASURE page of catalog data ========
// TREASURE Catalog takes different processing than SmartHandController Catalogs
// since it is stored on the SD card in a different format
void TreasureCatScreen::drawTreasureCat() {
  tRow = 0;
  pre_tAbsIndex=0;
  char catLine[47]=""; //hold the string that is displayed beside the button on each page
  
  tAbsRow = (tPagingArrayIndex[tCurrentPage]); // array of page 1st row indexes
  tLastPage = ((MAX_TREASURE_ROWS / NUM_CAT_ROWS_PER_SCREEN)+1);
  tNumRowsLastPage = MAX_TREASURE_ROWS % NUM_CAT_ROWS_PER_SCREEN;
  if (tCurrentPage+1 == tLastPage) isLastPage = tNumRowsLastPage <= NUM_CAT_ROWS_PER_SCREEN; else isLastPage = false;

  // Show Page number and total Pages
  tft.fillRect(6, 9, 77, 32,  butBackground); // erase page numbers
  tft.fillRect(2,60,317,353, pgBackground); // clear lower screen
  tft.setFont(0); // basic Arial font
  tft.setCursor(6, 9); 
  tft.print("Page "); 
  tft.print(tCurrentPage+1);
  tft.print(" of "); 
  if (moreScreen.activeFilter == FM_ABOVE_HORIZON) 
    tft.print("??"); 
  else 
    tft.print(tLastPage); 
  tft.setCursor(6, 25); 
  tft.print(activeFilterStr[moreScreen.activeFilter]);
  
  // TO DO: add code for case when filter enabled and screen contains fewer than NUM_CAT_ROWS_PER_SCREEN
  while ((tRow < NUM_CAT_ROWS_PER_SCREEN) && (tAbsRow != MAX_TREASURE_ROWS)) {  
    // ======== convert RA/DEC ===========
    char  *_end;
    char  *nextStr;
    double raH  = 0.0;
    int    raM  = 0;
    int    raS  = 0;
    double decD = 0.0;
    int    decM = 0;
    int    decS = 0;
    double tRAdouble;
    double tDECdouble;

    // Format RA in Hrs:Min:Sec and form string to be sent as target
    raH = strtod(tArray[tAbsRow].tRAhRAm, &_end); // convert first hour chars
    nextStr = strchr(tArray[tAbsRow].tRAhRAm,'h'); // get ptr to minute chars
    if (nextStr != NULL) {
      raM = atoi(nextStr+1);
    } else {
      VLF("found RA==NULL in treasure");
    }
    snprintf(tRAhhmmss[tAbsRow], 9, "%02d:%02d:00", (int)raH, raM);
    snprintf(tRaSrCmd[tAbsRow], 13, ":Sr%02d:%02d:%02d#", (int)raH, raM, raS); //Note: this is in HOURS

    // Format DEC in Deg:Min:Sec and form string to be sent as target
    decD = strtod(tArray[tAbsRow].tsDECdDECm, &_end);
    nextStr = strchr(tArray[tAbsRow].tsDECdDECm, 'd');
    if (nextStr != NULL) {
      decM = atoi(nextStr+1);
    } else {
      VLF("found DEC==NULL in treasure");
    }
    snprintf(tDECsddmmss[tAbsRow], 10, "%+03d*%02d:00", (int)decD, decM);
    snprintf(tDecSrCmd[tAbsRow], 14, ":Sd%+03d*%02d:%02d#", (int)decD, decM, decS);
    
    // to get Altitude, first convert RAh and DEC to double
    // Note: PM_HIGH is required for the format used here
    convert.hmsToDouble(&tRAdouble, tRAhhmmss[tAbsRow], PM_HIGH);
    convert.dmsToDouble(&tDECdouble, tDECsddmmss[tAbsRow], true, PM_HIGH);

    // dtAlt[tAbsRow] is used by filter to check if above Horizon
    Coordinate treTarget;
    treTarget.r = hrsToRad(tRAdouble);
    treTarget.d = degToRad(tDECdouble);
    transform.rightAscensionToHourAngle(&treTarget, true);
    transform.equToAlt(&treTarget);
    transform.equToHor(&treTarget);
    dtAlt[tAbsRow] = radToDeg(treTarget.a);
    dtAzm[tAbsRow] = NormalizeAzimuth(radToDeg(treTarget.z));
    
    // filter out elements below 10 deg if filter enabled
    if (((moreScreen.activeFilter == FM_ABOVE_HORIZON) && (dtAlt[tAbsRow] > 10.0)) || moreScreen.activeFilter == FM_NONE) { 
     
      // Erase text background
      tft.setCursor(CAT_X+CAT_W+2, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING));
      tft.fillRect(CAT_X+CAT_W+5, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING), 197, 17,  butBackground);

      // get object names and put them on the buttons
      treasureDefButton.drawLJ(CAT_X, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING), CAT_W, CAT_H, tArray[tAbsRow].tSubId, BUT_OFF);
                  
      // format and print the text field for this row next to the button
      // FYI: mod1_treasure.csv file field order and spacing: 7;8;7;4;9;4;9;18 = 66 total w/out semicolons, 73 with ;'s
      // Char number does not include null terminator
      // 7 - objName
      // 8 - RA
      // 7 - DEC
      // 4 - Cons
      // 9 - ObjType
      // 4 - Mag
      // 9 - Size ( Not used )
      //18 - SubId
      // select some Treasure fields to show beside button
      snprintf(catLine, sizeof(catLine), "%-4s |%-4s |%-9s |%-18s",  //35 + 6 + NULL = 42
                                          tArray[tAbsRow].tMag, 
                                          tArray[tAbsRow].tCons, 
                                          tArray[tAbsRow].tObjType, 
                                          tArray[tAbsRow].tObjName);
      tft.setCursor(CAT_X+CAT_W+SUB_STR_X_OFF, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING)+FONT_Y_OFF); 
      tft.print(catLine);
      tFiltArray[tRow] = tAbsRow;
      //VF("tFiltArray[tRow]="); VL(tFiltArray[tRow]);
      tRow++; // increments only through the number of lines displayed on screen per page
      //VF("tRow="); VL(tRow);
    } 
    tPrevRowIndex = tAbsRow;
    tAbsRow++; // increments through all lines in the catalog
    
    // stop printing data if last row on the last page
    if (tAbsRow == MAX_TREASURE_ROWS) {
      tEndOfList = true; 
      if (tRow == 0) {canvTreasureInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "None above 10 deg", false);}
      return; 
    }
  }
  tPagingArrayIndex[tCurrentPage+1] = tAbsRow; // tPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
  //VF("tPagingArrayIndex+1="); VL(tPagingArrayIndex[tCurrentPage+1]);
}

// show status changes on tasks timer tick
void TreasureCatScreen::updateTreasureStatus() {
  // no status, just button updates
}

// redraw screen to show state change
bool TreasureCatScreen::trCatalogButStateChange() {
  bool changed = false;

  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (saveTouched || trCatButDetected) {
      changed = true;
    }
  }

  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  return changed;
}

// =========  Update Screen buttons  ===========
void TreasureCatScreen::updateTreasureButtons() { 
  if (trCatButDetected) {
    trCatButDetected = false;
    updateScreen();
  }
  
  tft.setFont(&Inconsolata_Bold8pt7b);
  if (saveTouched) {
    treasureCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, " Saving", BUT_ON);
    saveTouched = false;
    display._redrawBut = true;
  } else { 
    treasureCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SaveToCat", BUT_OFF);
  }
  tft.setFont(0);
}

//==================================================
// =====  Update Screen buttons and text ===========
//==================================================
void TreasureCatScreen::updateScreen() {
  tft.setFont(0);
  uint16_t tRelIndex = catButSelPos; // save the relative-to-this-screen-index of button pressed
  tAbsIndex = tFiltArray[catButSelPos]; // this is absolute full array index

  // Toggle off previous selected button and toggle on current selected button
  if (tPrevPage == tCurrentPage) { //erase previous selection
    treasureDefButton.drawLJ(CAT_X, CAT_Y+pre_tRelIndex*(CAT_H+CAT_Y_SPACING), 
      CAT_W, CAT_H, tArray[pre_tAbsIndex].tSubId, BUT_OFF); 
  }
  // highlight selected by settting background ON color 
  treasureDefButton.drawLJ(CAT_X, CAT_Y+tRelIndex*(CAT_H+CAT_Y_SPACING), 
      CAT_W, CAT_H, tArray[tAbsIndex].tSubId, BUT_ON); 
  
  // the following 5 lines are displayed on the Catalog/More page
  // Note: ObjName and SubId are swapped here relative to other catalogs since the Treasure catalog is formatted differently
  snprintf(moreScreen.catSelectionStr1, 26, "Name:%-18s", tArray[tAbsIndex].tSubId);   //VF("t_subID=");   VL(tArray[tAbsIndex].tSubId);
  snprintf(moreScreen.catSelectionStr2, 26, "Mag-:%-4s",  tArray[tAbsIndex].tMag);     //VF("t_mag=");     VL(tArray[tAbsIndex].tMag);
  snprintf(moreScreen.catSelectionStr3, 26, "Cons:%-4s",  tArray[tAbsIndex].tCons);    //VF("t_constel="); VL(tArray[tAbsIndex].tCons);
  snprintf(moreScreen.catSelectionStr4, 26, "Type:%-9s",  tArray[tAbsIndex].tObjType); //VF("t_objType="); VL(tArray[tAbsIndex].tObjType);
  snprintf(moreScreen.catSelectionStr5, 26, "Id--:%-7s",  tArray[tAbsIndex].tObjName); //VF("t_objName="); VL(tArray[tAbsIndex].tObjName);
  
  // show if we are above and below visible limits
  tft.setFont(&Inconsolata_Bold8pt7b); 
  if (dtAlt[tAbsIndex] > 10.0) {   // show minimum 10 degrees altitude, use dtAlt[tAbsIndex] previously calculated 
      canvTreasureInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Above +10 deg", false);
  } else {
      canvTreasureInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Below +10 deg", true);
  }
  tft.setFont(0);

  writeTreasureTarget(tAbsIndex); // write RA and DEC as target for GoTo
  showTargetCoords(); // display the target coordinates that were just written

  pre_tRelIndex = tRelIndex;
  pre_tAbsIndex = tAbsIndex;
  curSelTIndex = tAbsIndex;
  tPrevPage = tCurrentPage;
}

// Save selected object data to custom catalog
void TreasureCatScreen::saveTreasure() {
// Custom Catalog Format: SubID or ObjName, Mag, Cons, ObjType, ObjName or SubID, RA, DEC
  snprintf(treaCustWrSD, sizeof(treaCustWrSD), "%-18s;%-4s;%-4s;%-14s;%-7s;%8s;%9s\n", 
                                                  tArray[curSelTIndex].tSubId,
                                                  tArray[curSelTIndex].tMag, 
                                                  tArray[curSelTIndex].tCons, 
                                                  tArray[curSelTIndex].tObjType,
                                                  tArray[curSelTIndex].tObjName, 
                                                  tRAhhmmss[curSelTIndex],
                                                  tDECsddmmss[curSelTIndex]);
  // write string to the SD card
  File wrFile = SD.open("custom.csv", FILE_WRITE);
  if (wrFile) {
    wrFile.print(treaCustWrSD);
    } else {
      canvTreasureInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W,
        STATUS_STR_H, "SD open ERROR", true);
      return;
    }
    //VF("twrFileSize="); VL(wrFile.size());
  wrFile.close();
  }

//=====================================================
// **** Handle any buttons that have been pressed *****
//=====================================================
bool TreasureCatScreen::touchPoll(uint16_t px, uint16_t py) {
  // TO DO: add code for case when filter enabled and screen contains fewer than NUM_CAT_ROWS_PER_SCREEN
  if (isLastPage) tRowsPerPage = tNumRowsLastPage; else tRowsPerPage = NUM_CAT_ROWS_PER_SCREEN;
  for (int i=0; i < tRowsPerPage; i++) {
    if (py > CAT_Y+(i*(CAT_H+CAT_Y_SPACING)) && py < (CAT_Y+(i*(CAT_H+CAT_Y_SPACING))) + CAT_H 
          && px > CAT_X && px < (CAT_X+CAT_W)) {
      BEEP;
      if (tAbsRow <= 1 || (tAbsRow > MAX_TREASURE_ROWS+1)) return false; 
      catButSelPos = i;
      trCatButDetected = true;
      return true;
    }
  }
 
  // BACK button
  if (py > BACK_Y && py < (BACK_Y + BACK_H) && px > BACK_X && px < (BACK_X + BACK_W)) {
    BEEP;
     if (tCurrentPage > 0) {
      tPrevPage = tCurrentPage;
      tEndOfList = false;
      tCurrentPage--;
      drawTreasureCat();
    }
    return false;
  }

  // NEXT page button - reuse BACK button box size
  if (py > NEXT_Y && py < (NEXT_Y + BACK_H) && px > NEXT_X && px < (NEXT_X + BACK_W)) {
    BEEP;
    if (!tEndOfList) {
      tPrevPage = tCurrentPage;
      tCurrentPage++;
      drawTreasureCat();
    }
    return false;
  }

  // RETURN page button - reuse BACK button box size
  if (py > RETURN_Y && py < (RETURN_Y + BACK_H) && px > RETURN_X && px < (RETURN_X + RETURN_W)) {
    BEEP;
    moreScreen.objectSelected = objSel; 
    updateAltAzmTarget();
    moreScreen.draw();
    return false; // don't update this screen since returning to MORE
  }

  // SAVE page to custom library button
  if (py > SAVE_LIB_Y && py < (SAVE_LIB_Y + SAVE_LIB_H) && px > SAVE_LIB_X && px < (SAVE_LIB_X + SAVE_LIB_W)) {
    BEEP;
    saveTreasure();
    saveTouched = true;
    return true;
  }
  
  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);
  
  return false; 
}

// ======= write Target Coordinates to controller =========
void TreasureCatScreen::writeTreasureTarget(uint16_t index) {
  //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
  commandBool(tRaSrCmd[index]);
 
  //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
  commandBool(tDecSrCmd[index]);
  objSel = true;
}

// Show target coordinates RA/DEC and ALT/AZM
void TreasureCatScreen::showTargetCoords() {
  char _reply[15]   = "";
  uint16_t radec_x  = 155;
  uint16_t ra_y     = 405;
  uint16_t dec_y    = 418;
  uint16_t altazm_x = 246;
  uint16_t width    = 83;
  uint16_t height   = 12;

  // The following code used the local command channel
  // Decided to change to direct access of previously calculated array values
  //double tAzm_d     = 0.0;
  //double tAlt_d     = 0.0;
  //char reply[15]    = "";
  
  // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  //commandWithReply(":Gr#", reply);

  // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
  //commandWithReply(":Gd#", reply); 

  // Get Target ALT and AZ and display them as Double
  //commandWithReply(":Gz#", reply); // DDD*MM'SS# 
  //convert.dmsToDouble(&tAzm_d, reply, false, PM_LOW);

  //commandWithReply(":Gal#", reply);	// sDD*MM'SS#
  //convert.dmsToDouble(&tAlt_d, reply, true, PM_LOW);
  
  //sprintf(_reply, "AZM: %6.1f", tAzm_d); 
  //customDefPrint(altazm_x, ra_y, width-10, height, _reply, false);    

  //sprintf(_reply, "ALT: %6.1f", tAlt_d);
  //customDefPrint(altazm_x, dec_y, width-10, height, _reply, false); 

  // Using the values stored in master array during drawCustomCat()
  // RA and DEC settings
  sprintf(_reply, "RA : %s", tArray[tAbsIndex].tRAhRAm);
  //sprintf(_reply, "RA: %6.1f", cusTarget[cAbsIndex].r);
  canvTreasureDefPrint.printRJ(radec_x, ra_y, width, height, _reply, false);
  sprintf(_reply, "DEC: %s", tArray[tAbsIndex].tsDECdDECm);
  //sprintf(_reply, "DEC: %6.1f", cusTarget[cAbsIndex].d);
  canvTreasureDefPrint.printRJ(radec_x, dec_y, width, height, _reply, false);
  
  // Alt Azm settings
  sprintf(_reply, "AZM: %6.1f", dtAzm[tAbsIndex]);
  canvTreasureDefPrint.printRJ(altazm_x, ra_y, width - 10, height, _reply, false);
  sprintf(_reply, "ALT: %6.1f", dtAlt[tAbsIndex]);
  canvTreasureDefPrint.printRJ(altazm_x, dec_y, width - 10, height, _reply, false);
}

TreasureCatScreen treasureCatScreen;
