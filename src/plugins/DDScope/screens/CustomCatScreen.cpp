// =====================================================
// CustomCatScreen.cpp
// Custom User Catalog Screen
// This catalog can hold various saved objects and their coordinates from other
// catalogs
//
// Author: Richard Benear 6/22
// Refactored 5/19/25
#include <Arduino.h>
#include "../display/Display.h"
#include "CustomCatScreen.h"
#include "MoreScreen.h"
#include "../catalog/Catalog.h"
#include "../catalog/CatalogTypes.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/lib/Macros.h"
#include "src/telescope/mount/coordinates/Transform.h"
//#include "src/telescope/mount/Mount.h"
//#include "src/telescope/mount/goto/Goto.h"

#define CUS_X 3
#define CUS_Y 50
#define CUS_W 112
#define CUS_H 24
#define CUS_Y_SPACING 2

#define BACK_X 5
#define BACK_Y 445
#define BACK_W 60
#define BACK_H 35

#define NEXT_X 255
#define NEXT_Y BACK_Y

#define RETURN_X 165
#define RETURN_Y BACK_Y
#define RETURN_W 80

#define SAVE_LIB_X 75
#define SAVE_LIB_Y BACK_Y
#define SAVE_LIB_W 80
#define SAVE_LIB_H BACK_H

#define STATUS_STR_X 3
#define STATUS_STR_Y 430
#define STATUS_STR_W 150
#define STATUS_STR_H 16

#define WIDTH_OFF 40 // negative width offset
#define SUB_STR_X_OFF 2
#define FONT_Y_OFF 7

custom_t cArray[MAX_CUSTOM_CATALOG_ROWS];

EXTMEM char customArray[MAX_CUSTOM_CATALOG_ROWS][SD_CARD_LINE_LENGTH];
EXTMEM char copyCustomArray[MAX_CUSTOM_CATALOG_ROWS][SD_CARD_LINE_LENGTH]; //save a copy for row deletion purposes
EXTMEM char cRaSrCmd[MAX_CUSTOM_CATALOG_ROWS][18];
EXTMEM char cDecSrCmd[MAX_CUSTOM_CATALOG_ROWS][18];
EXTMEM uint8_t cFiltArray[MAX_CUSTOM_CATALOG_ROWS];

Coordinate cusTarget[MAX_CUSTOM_CATALOG_ROWS];
double dcAlt[MAX_CUSTOM_CATALOG_ROWS];
double dcAzm[MAX_CUSTOM_CATALOG_ROWS];

// Catalog Button object for default Arial font
Button customDefButton(0, 0, 0, 0, butOnBackground, butBackground, butOutline,
                       defFontWidth, defFontHeight, "");

// Catalog Button object for custom font
Button customCatButton(0, 0, 0, 0, butOnBackground, butBackground, butOutline,
                       mainFontWidth, mainFontHeight, "");

// Canvas Print object default Arial 6x9 font
CanvasPrint canvCustomDefPrint(display.default_font);

// Canvas Print object, Inconsolata_Bold8pt7b font
CanvasPrint canvCustomInsPrint(&Inconsolata_Bold8pt7b);

// ========== Initialize and draw Custom Catalog ===============
void CustomCatScreen::init() {
  returnToPage = display.currentScreen; // save page from where this function
                                        // was called so we can return
  display.setCurrentScreen(CUSTOM_SCREEN);
  customDefButton.setColors(butOnBackground, butBackground, butOutline);
  customCatButton.setColors(butOnBackground, butBackground, butOutline);

#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
#endif

  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  moreScreen.objectSelected = false;
  currentPageNum = 0;
  prevPageNum = 0;
  endOfList = false;
  totalNumRows = 0; // total number of rows

  drawTitle(86, TITLE_TEXT_Y, "User Catalog");

  // Draw the Trash can/Delete Icon bitmap; used to delete an entry in only the
  // CUSTOM USER catalog
  uint8_t extern trash_icon[];
  tft.drawBitmap(100, 445, trash_icon, 28, 32, butBackground, butOutline);

  tft.setFont(&Inconsolata_Bold8pt7b);
  customCatButton.draw(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", BUT_OFF);
  customCatButton.draw(NEXT_X, NEXT_Y, BACK_W, BACK_H, "NEXT", BUT_OFF);
  customCatButton.draw(RETURN_X, RETURN_Y, RETURN_W, BACK_H, "RETURN", BUT_OFF);

  // load the array from SD and parse it
  if (loadCustomArray()) {
    parseCcatIntoArray();
  } else {
    canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W,
                               STATUS_STR_H, "ERR:Loading Custom", true);
    moreScreen.draw(); // go back to the More screen
    return;
  }
  drawCustomCat(); // draw first page of the selected catalog
}

// The Custom catalog is a selection of User objects that have been saved on the SD card.
//      When using any of the catalogs, the SaveToCat button will store the
//      objects info in the Custom Catalog in the SD Flash.
// Load the custom.csv file from SD into an RAM array of text lines
bool CustomCatScreen::loadCustomArray() {
  File rdFile = SD.open("custom.csv");
  if (!rdFile) {
    SERIAL_DEBUG.println("SD Cust read error: File open failed");
    return false;
  }

  char rowString[SD_CARD_LINE_LENGTH] = "";
  int rowNum = 0;
  int charNum = 0;

  while (rdFile.available() && rowNum < MAX_CUSTOM_CATALOG_ROWS) {
    char rdChar = rdFile.read();

    // Prevent charNum overflow
    if (charNum >= SD_CARD_LINE_LENGTH - 1) {
      rowString[SD_CARD_LINE_LENGTH - 1] = '\0'; // Null-terminate just in case
      SERIAL_DEBUG.println("Warning: Line too long, truncating");
      // Save truncated line anyway
      strncpy(customArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
      strncpy(copyCustomArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
      rowNum++;
      charNum = 0;
      memset(rowString, 0, SD_CARD_LINE_LENGTH);
      continue;
    }

    // Append character to current row
    rowString[charNum++] = rdChar;

    // On newline, save the full row
    if (rdChar == '\n') {
      rowString[charNum] = '\0'; // Null-terminate line
      strncpy(customArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
      strncpy(copyCustomArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
      //SERIAL_DEBUG.print("Row "); SERIAL_DEBUG.print(rowNum); SERIAL_DEBUG.print(": "); SERIAL_DEBUG.println(rowString);
      rowNum++;
      charNum = 0;
      memset(rowString, 0, SD_CARD_LINE_LENGTH);
    }
  }

  // If last line didn't end with newline, still save it
  if (charNum > 0 && rowNum < MAX_CUSTOM_CATALOG_ROWS) {
    rowString[charNum] = '\0';
    strncpy(customArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
    strncpy(copyCustomArray[rowNum], rowString, SD_CARD_LINE_LENGTH);
    SERIAL_DEBUG.print("Row "); SERIAL_DEBUG.print(rowNum); SERIAL_DEBUG.print(" (no newline): ");
    SERIAL_DEBUG.println(rowString);
    rowNum++;
  }
  rdFile.close();

  // Final count: rows actually loaded
  // 1st ROW is row 0
  totalNumRows = rowNum;
  if (totalNumRows < 0) {
    totalNumRows = 0;
  }
  return true;
}

// Parse the custom.csv line data into individual fields in an array
// custom.csv file format = ObjName;Mag;Cons;ObjType;SubId;cRahhmmss;cDecsddmmss\n
void CustomCatScreen::parseCcatIntoArray() {
  for (int i = 0; i < totalNumRows; i++) {
    cArray[i].cObjName      = strtok(customArray[i], ";"); //VL(cArray[i].cObjName);
    cArray[i].cMag          = strtok(NULL, ";");           //VL(cArray[i].cMag);
    cArray[i].cCons         = strtok(NULL, ";");           //VL(cArray[i].cCons);
    cArray[i].cObjType      = strtok(NULL, ";");           //VL(cArray[i].cObjType);
    cArray[i].cSubId        = strtok(NULL, ";");           //VL(cArray[i].cSubId);
    cArray[i].cRAhhmmss     = strtok(NULL, ";");           //VL(cArray[i].cRAhhmmss);
    cArray[i].cDECsddmmss   = strtok(NULL, "\n");          //VL(cArray[i].cDECsddmmss);
  }
  // SERIAL_DEBUG.print("Final totalNumRows = ");
  // SERIAL_DEBUG.println(totalNumRows);

  // for (int i = 0; i < totalNumRows; i++) {
  //   SERIAL_DEBUG.print("cArray["); SERIAL_DEBUG.print(i); SERIAL_DEBUG.print("] = ");
  //   SERIAL_DEBUG.println(cArray[i].cObjName);
  // }
}

// ========== draw CUSTOM Screen of catalog data ========
void CustomCatScreen::drawCustomCat() {
  // write the top and bottom areas of screen
  tft.fillRect(6, 9, 77, 32, butBackground);   // erase page numbers
  tft.fillRect(2, 60, 317, 353, pgBackground); // clear lower screen
  tft.setFont(0);                              // revert to basic Arial font
  prevAbsIndex = 0;

  lastPageNum = max(1, (totalNumRows + NUM_CATALOG_ROWS_PER_SCREEN - 1) / NUM_CATALOG_ROWS_PER_SCREEN);
  numRowsLastPage = (totalNumRows % NUM_CATALOG_ROWS_PER_SCREEN);
  if (numRowsLastPage == 0 && totalNumRows > 0) {
    numRowsLastPage = NUM_CATALOG_ROWS_PER_SCREEN;
  }

  //SERIAL_DEBUG.println("drawCustomCat()");
  SERIAL_DEBUG.print("totalNumRows="); SERIAL_DEBUG.println(totalNumRows);
  SERIAL_DEBUG.print("lastPageNum="); SERIAL_DEBUG.println(lastPageNum);
  SERIAL_DEBUG.print("currentPageNum="); SERIAL_DEBUG.println(currentPageNum);
  SERIAL_DEBUG.print("numRowsLastPage="); SERIAL_DEBUG.println(numRowsLastPage);
  SERIAL_DEBUG.println(" ");

  
  tft.setCursor(6, 9);
  tft.print("Page ");
  tft.print(currentPageNum + 1);
  tft.print(" of ");
  if (moreScreen.activeFilter == FM_ABOVE_HORIZON) {
    tft.print("??");
  } else {
    tft.print(lastPageNum);
    tft.setCursor(6, 25);
    tft.print(activeFilterStr[moreScreen.activeFilter]);
  }

  // Compute how many rows to draw on this page
  rowsThisPage = NUM_CATALOG_ROWS_PER_SCREEN;
  if (currentPageNum + 1 == lastPageNum) {
    rowsThisPage = numRowsLastPage;
  }

  // Start drawing from absolute row index based on page
  uint8_t startAbsIndex = currentPageNum * NUM_CATALOG_ROWS_PER_SCREEN;

  SERIAL_DEBUG.print("startAbsIndex="); SERIAL_DEBUG.println(startAbsIndex);
  SERIAL_DEBUG.print("rowsThisPage="); SERIAL_DEBUG.println(rowsThisPage);

  drawPageData(startAbsIndex);
}

void CustomCatScreen::drawPageData(uint8_t startAbsIndex) {
  char catLine[50] = "";

  for (uint8_t i = 0; i < rowsThisPage; ++i) {
    uint8_t absIndex = startAbsIndex + i;
    if (absIndex >= totalNumRows) break;
    rowIndex = i;

    //SERIAL_DEBUG.print("rowIndex="); SERIAL_DEBUG.println(rowIndex);
    // ======== process RA/DEC ===========
    double cRAdouble;
    double cDECdouble;
    // to get Altitude, first convert RAh and DEC to double
    // Note: PM_HIGH is required for the format used here
    convert.hmsToDouble(&cRAdouble, cArray[absIndex].cRAhhmmss, PM_HIGH);
    convert.dmsToDouble(&cDECdouble, cArray[absIndex].cDECsddmmss, true, PM_HIGH);

    // dcAlt[absIndex] is used by filter to check if above Horizon
    // Coordinate calculation using OnStep transforms
    // First, convert RA hours and DEC degrees to Radians
    cusTarget[absIndex].r = hrsToRad(cRAdouble);
    cusTarget[absIndex].d = degToRad(cDECdouble);

    // Then, transform RA to hour angle and equ to Altitude in radians
    transform.rightAscensionToHourAngle(&cusTarget[absIndex], true);
    transform.equToAlt(&cusTarget[absIndex]);
    transform.equToHor(&cusTarget[absIndex]);

    // Then, convert back to AZM and ALT degrees
    dcAlt[absIndex] = radToDeg(cusTarget[absIndex].a);
    dcAzm[absIndex] = NormalizeAzimuth(radToDeg(cusTarget[absIndex].z));

    // Generate the Command strings for later use
    snprintf(cRaSrCmd[absIndex], sizeof(cRaSrCmd[absIndex]), ":Sr%11s#", cArray[absIndex].cRAhhmmss);
    snprintf(cDecSrCmd[absIndex], sizeof(cDecSrCmd[absIndex]), ":Sd%12s#", cArray[absIndex].cDECsddmmss);

    // filter out elements below 10 deg if filter enabled
    //SERIAL_DEBUG.print("activeFilter="); SERIAL_DEBUG.println(moreScreen.activeFilter);
    if ((moreScreen.activeFilter == FM_ABOVE_HORIZON && dcAlt[absIndex] <= 10.0) &&
        moreScreen.activeFilter != FM_NONE) {
      continue;
    }

    // Draw row content
    uint16_t y = CUS_Y + (rowIndex * (CUS_H + CUS_Y_SPACING));
    //SERIAL_DEBUG.print("y="); SERIAL_DEBUG.println(y);
    tft.fillRect(CUS_X + CUS_W + 2, y, 197, 17, butBackground);
    //tft.setCursor(CUS_X + CUS_W + 2, y);
    customDefButton.drawLJ(CUS_X, y, CUS_W, CUS_H, cArray[absIndex].cObjName, BUT_OFF);

    snprintf(catLine, sizeof(catLine), "%-4s|%-4s|%-14s|%-7s",
             cArray[absIndex].cMag,
             cArray[absIndex].cCons,
             cArray[absIndex].cObjType,
             cArray[absIndex].cSubId);

    tft.setCursor(CUS_X + CUS_W + SUB_STR_X_OFF + 2, y + FONT_Y_OFF);
    tft.print(catLine);
    //SERIAL_DEBUG.print("catLine["); SERIAL_DEBUG.print(rowIndex); SERIAL_DEBUG.print("] = "); SERIAL_DEBUG.println(catLine);

    // Store mapping of screen row to absolute index
    cFiltArray[rowIndex] = absIndex;
    rowIndex++;
  }

  if (rowIndex == 0) {
    canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "None above 10 deg", true);
  }

  // Mark end of list if no further rows
  endOfList = ((startAbsIndex + rowsThisPage) >= totalNumRows);
}
    
// show status changes on tasks timer tick
void CustomCatScreen::updateCustomStatus() {
  //updateScreen();
}

// redraw screen to show state change
bool CustomCatScreen::cusCatalogButStateChange() {
  bool changed = false;

  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (buttonDetected) {
      changed = true;
    }

    if (delSelected) {
      changed = true;
    }
  }

  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  return changed;
}

void CustomCatScreen::updateCustomButtons() { 
  if (buttonDetected) {
    buttonDetected = false;
    updateScreen();
  }
}

//==================================================
// =====  Update Screen buttons and text ===========
//==================================================
void CustomCatScreen::updateScreen() {
  tft.setFont(0);

  relIndex = buttonSelected; // save the relative-to-this "screen/page" index of button pressed
  absIndex = cFiltArray[buttonSelected]; // this is absolute full array index

  //SERIAL_DEBUG.println("updateScreen()");
  //SERIAL_DEBUG.print("ButSelPos="); SERIAL_DEBUG.println(buttonSelected);
  //SERIAL_DEBUG.print("absIndex="); SERIAL_DEBUG.println(absIndex);
  //SERIAL_DEBUG.println(" ");

  if (prevPageNum == currentPageNum) { // erase previous selection
    customDefButton.drawLJ(CUS_X, CUS_Y + prevRelIndex * (CUS_H + CUS_Y_SPACING), CUS_W, CUS_H,
                          cArray[prevAbsIndex].cObjName, BUT_OFF);
  }
  // highlight selected by settting background ON color
  customDefButton.drawLJ(CUS_X, CUS_Y + relIndex * (CUS_H + CUS_Y_SPACING),
                         CUS_W, CUS_H, cArray[absIndex].cObjName, BUT_ON);

  snprintf(moreScreen.catSelectionStr1, sizeof(moreScreen.catSelectionStr1),
           "Name-:%-18s", cArray[absIndex].cObjName);
  // SERIAL_DEBUG.print("c_objName="); //SERIAL_DEBUG.println(cArray[absIndex].cObjName);
  snprintf(moreScreen.catSelectionStr2, sizeof(moreScreen.catSelectionStr2),
           "Mag--:%-4s", cArray[absIndex].cMag);
  // SERIAL_DEBUG.print("c_Mag=");     //SERIAL_DEBUG.println(cArray[absIndex].cMag);
  snprintf(moreScreen.catSelectionStr3, sizeof(moreScreen.catSelectionStr3),
           "Const:%-4s", cArray[absIndex].cCons);
  // SERIAL_DEBUG.print("c_constel="); //SERIAL_DEBUG.println(cArray[absIndex].cCons);
  snprintf(moreScreen.catSelectionStr4, sizeof(moreScreen.catSelectionStr4),
           "Type-:%-14s", cArray[absIndex].cObjType);
  // SERIAL_DEBUG.print("c_objType="); //SERIAL_DEBUG.println(cArray[absIndex].cObjType);
  snprintf(moreScreen.catSelectionStr5, sizeof(moreScreen.catSelectionStr5),
           "Id---:%-6s", cArray[absIndex].cSubId);
  // SERIAL_DEBUG.print("c_subID=");   //SERIAL_DEBUG.println(cArray[absIndex].cSubId);

  // show if we are above and below visible limits
  tft.setFont(&Inconsolata_Bold8pt7b);
  if (dcAlt[absIndex] > 10.0) { // minimum 10 degrees altitude
    canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Above +10 deg", false);
  } else {
    canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Below +10 deg", true);
  }
  tft.setFont(0);

  writeCustomTarget(absIndex); // write RA and DEC as target for GoTo
  showTargetCoords(); // get and display the target coordinates that were just
                      // written

  // Support for deleting a object row from the Custom library screen
  prevRelIndex = relIndex;
  prevAbsIndex = absIndex;
  prevPageNum = currentPageNum;
}

// Delete a Row button by clicking the Trashcan ICON
void CustomCatScreen::deleteRow() {
  if (delSelected) {
    SERIAL_DEBUG.print("Deleting Row=");
    SERIAL_DEBUG.println(buttonSelected);
    delSelected = false;

    // Absolute index of the row to delete
    absIndex = cFiltArray[buttonSelected];
    uint8_t delIndex = absIndex;

    if (totalNumRows == 0) { // first entry is number 0
      // Only one row in the file — just delete the whole file
      File rmFile = SD.open("/custom.csv");
      SD.remove("/custom.csv");
      rmFile.close();
      return;
    } else {
      if (delIndex >= totalNumRows) {
        SERIAL_DEBUG.println("Invalid deletion index — skipping.");
        return;
      }

      // Shift remaining rows up one position
      memmove(&copyCustomArray[delIndex], &copyCustomArray[delIndex + 1], (totalNumRows - delIndex) * SD_CARD_LINE_LENGTH);

      // Clear last row
      memset(copyCustomArray[totalNumRows], '\0', SD_CARD_LINE_LENGTH);

      // Decrease total count of entries
      totalNumRows--;
    }

    // Delete existing file
    File rmFile = SD.open("/custom.csv");
    if (rmFile) {
      SD.remove("/custom.csv");
    }
    rmFile.close();

    // Rewrite trimmed array to SD
    File cWrFile = SD.open("custom.csv", FILE_WRITE);
    if (!cWrFile) {
      canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "SD open ERROR", true);
      return;
    }

    for (uint8_t i = 0; i < totalNumRows; i++) {
      cWrFile.print(copyCustomArray[i]);
    }
    cWrFile.close();
  }
  prevRelIndex = 0;
  prevAbsIndex = 0;
  buttonSelected = 0;
}

//=====================================================
// **** Handle any buttons that have been pressed *****
//=====================================================
bool CustomCatScreen::touchPoll(uint16_t px, uint16_t py) {
  // SERIAL_DEBUG.println("touchPoll()");
  // SERIAL_DEBUG.print("numRowsLastpage=");
  // SERIAL_DEBUG.println(numRowsLastPage);
  // SERIAL_DEBUG.print("numRowsLastPage=");
  // SERIAL_DEBUG.println(numRowsLastPage);
  // SERIAL_DEBUG.print("totalNumRows=");
  // SERIAL_DEBUG.println(totalNumRows);
  // SERIAL_DEBUG.print("rowIndex=");
  // SERIAL_DEBUG.println(rowIndex);
  // SERIAL_DEBUG.println(" ");
  for (int i = 0; i <= rowsThisPage; i++) {
    if (py > CUS_Y + (i * (CUS_H + CUS_Y_SPACING)) &&
        py < (CUS_Y + (i * (CUS_H + CUS_Y_SPACING))) + CUS_H && px > CUS_X &&
        px < (CUS_X + CUS_W)) {
      BEEP;
      buttonSelected = i;
      buttonDetected = true;
      if (buttonSelected >= rowIndex) {
        SERIAL_DEBUG.print("Invalid buttonSelected=");
        SERIAL_DEBUG.println(buttonSelected);
        return false;
      } else {
        return true;
      }
    }
  }

  // BACK button
  if (py > BACK_Y && py < (BACK_Y + BACK_H) && px > BACK_X &&
      px < (BACK_X + BACK_W)) {
    BEEP;
    if (currentPageNum > 0) {
      prevPageNum = currentPageNum;
      endOfList = false;
      currentPageNum--;
      drawCustomCat();
      buttonDetected = true;
    }
    return false;
  }

  // NEXT page button - reuse BACK button box size
  if (py > NEXT_Y && py < (NEXT_Y + BACK_H) && px > NEXT_X &&
      px < (NEXT_X + BACK_W)) {
    BEEP;
    if (!endOfList) {
      prevPageNum = currentPageNum;
      currentPageNum++;
      drawCustomCat();
      buttonDetected = true;
    }
    return false;
  }

  // RETURN page button - reuse BACK button box size
  if (py > RETURN_Y && py < (RETURN_Y + BACK_H) && px > RETURN_X &&
      px < (RETURN_X + RETURN_W)) {
    BEEP;
    moreScreen.objectSelected = objSel;
    updateAltAzmTarget();
    moreScreen.draw();
    return false;
  }

  // Trash Can pressed, Delete custom library item that is selected
  if (py > 445 && py < 465 && px > 100 && px < 125) {
    BEEP;
    delSelected = true;
    buttonDetected = true;
    deleteRow();
    // load the array from SD and parse it
    if (loadCustomArray()) {
      parseCcatIntoArray();
    } else {
    canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W,
                               STATUS_STR_H, "ERR:Loading Custom", true);
    moreScreen.draw(); // go back to the More screen
    return false;
    }
    drawCustomCat();
    return false;
  }

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);

  return false;
}

// ======= write Target Coordinates to controller =========
void CustomCatScreen::writeCustomTarget(uint16_t index) {

  //: Sr[HH:MM.T]# or :Sr[HH:MM:SS]#
  //SERIAL_DEBUG.print("index=");
  //SERIAL_DEBUG.println(index);
  commandBool(cRaSrCmd[index]);
  //SERIAL_DEBUG.println(cRaSrCmd[index]);

  //: Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
  commandBool(cDecSrCmd[index]);
  //SERIAL_DEBUG.println(cDecSrCmd[index]);
  objSel = true;
}

// Show target coordinates RA/DEC and ALT/AZM
void CustomCatScreen::showTargetCoords() {
  char _reply[15] = "";
  uint16_t radec_x = 155;
  uint16_t ra_y = 405;
  uint16_t dec_y = 418;
  uint16_t altazm_x = 246;
  uint16_t width = 83;
  uint16_t height = 12;

  // The following used the local command channel but decided
  // to just access the current calculated array values
  // double cAzm_d     = 0.0;
  // double cAlt_d     = 0.0;
  // char reply[15]    = "";

  // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  // commandWithReply(":Gr#", reply);

  // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
  // commandWithReply(":Gd#", reply);

  // Get Target ALT and AZ and display them as Double
  // commandWithReply(":Gz#", reply); // DDD*MM'SS#
  // convert.dmsToDouble(&tAzm_d, reply, false, PM_LOW);

  // commandWithReply(":Gal#", reply);	// sDD*MM'SS#
  // convert.dmsToDouble(&tAlt_d, reply, true, PM_LOW);

  // sprintf(_reply, "AZM: %6.1f", cAzm_d);
  // canvCustomDefPrint(altazm_x, ra_y, width-10, height, _reply, false);

  // sprintf(_reply, "ALT: %6.1f", cAlt_d);
  // canvCustomDefPrint(altazm_x, dec_y, width-10, height, _reply, false);

  // Using the values stored in master array during drawCustomCat()
  // RA and DEC settings
  sprintf(_reply, "RA : %s", cArray[absIndex].cRAhhmmss);
  //sprintf(_reply, "RA: %6.1f", cusTarget[absIndex].r);
  canvCustomDefPrint.printRJ(radec_x, ra_y, width, height, _reply, false);
  sprintf(_reply, "DEC: %s", cArray[absIndex].cDECsddmmss);
  //sprintf(_reply, "DEC: %6.1f", cusTarget[absIndex].d);
  canvCustomDefPrint.printRJ(radec_x, dec_y, width, height, _reply, false);
  
  // Alt Azm settings
  sprintf(_reply, "AZM: %6.1f", dcAzm[absIndex]);
  canvCustomDefPrint.printRJ(altazm_x, ra_y, width - 10, height, _reply, false);
  sprintf(_reply, "ALT: %6.1f", dcAlt[absIndex]);
  canvCustomDefPrint.printRJ(altazm_x, dec_y, width - 10, height, _reply, false);
}

CustomCatScreen customCatScreen;
