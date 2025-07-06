// ==============================================
// ExtStatusScreen.cpp
//
// Extended Status Screen
// Author: Richard Benear
// 8/30/2021
#include "../display/Display.h"
#include "ExtStatusScreen.h"
#include "src/telescope/mount/site/Site.h"
#include "./src/lib/tasks/OnTask.h"

#define STATUS_BOXSIZE_X         53 
#define STATUS_BOXSIZE_Y         27 
#define STATUS_BOX_X             40 
#define STATUS_BOX_Y            150 
#define STATUS_X                 10 
#define STATUS_Y                104 
#define STATUS_SPACING           13 

// ========== Draw the Extended Status Screen ==========
void ExtStatusScreen::draw() {
  setCurrentScreen(XSTATUS_SCREEN);
  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(68, TITLE_TEXT_Y, "Extended Status");

  // :GVD#      Get OnStepX Firmware Date
  //            Returns: MTH DD YYYY#
  // :GVM#      General Message
  //            Returns: s# (where s is a string up to 16 chars)
  // :GVN#      Get OnStepX Firmware Number
  //            Returns: M.mp#
  // :GVP#      Get OnStepX Product Name
  //            Returns: s#
  // :GVT#      Get OnStepX Firmware Time
  //            Returns: HH:MM:SS#
  tft.setCursor(STATUS_X, STATUS_Y); 
  commandWithReply(":GVN#", exReply); // Get OnStep FW Version
  tft.print("OnStep FW Version: "); tft.print(exReply);
  tft.setCursor(STATUS_X, STATUS_Y+STATUS_SPACING); 
  tft.print(display.ddscopeVersionStr);
  mountStatus();
  tlsStatus();
  limitsStatus();

  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  #endif
  #ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("xStatus");
  #endif
}
  
  // status update for this screen
void ExtStatusScreen::updateExStatus() {
  // erase screen...
  //tft.setCursor(STATUS_X, STATUS_Y); 
  //tft.fillRect(STATUS_X, STATUS_Y, 250, TFT_HEIGHT-150, pgBackground);
  //tlsStatus();
  //limitsStatus();
}

void ExtStatusScreen::mountStatus() {
  int y_offset = STATUS_Y+STATUS_SPACING;
  
  // Begin parsing :GU# status data by getting the status string via local command channel
  commandWithReply(":GU#", exReply); // Get OnStep Status
  // process the status string
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  tft.print("String="); tft.print(exReply);

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"n")) tft.print("Not Tracking"); else tft.print("Tracking   ");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"N")) tft.print("Not Slewing"); else tft.print("Slewing    ");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if      (strstr(exReply,"p")) tft.print("Not Parked         ");
  else if (strstr(exReply,"I")) tft.print("Parking in process ");
  else if (strstr(exReply,"P")) tft.print("Parked             ");
  else if (strstr(exReply,"F")) tft.print("Parking Failed     ");
  else                             tft.print("Park Status Unknown");
  
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"H")) tft.print("Homed    "); else tft.print("Not Homed");

  #if TIME_LOCATION_PPS_SENSE != OFF
    y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
    if (strstr(exReply,"S")) tft.print("PPS Synched    "); else tft.print("PPS Not Synched");
  #endif

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"G")) tft.print("Pulse Guide Active  "); else tft.print("Pulse Guide Inactive");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"g")) tft.print("Guide Active    "); else tft.print("Guiding Inactive");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"r")) tft.print("Refraction Enabled Dual Axis");
  if (strstr(exReply,"rs")) tft.print("Refraction Enabled Single Axis");
  if (strstr(exReply,"t")) tft.print("OnTrack Enabled Dual Axis");
  if (strstr(exReply,"ts")) tft.print("OnTrack Enabled Single Axis");
  else tft.print("Rate Compensation None");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  tft.print("Tracking Rate = "); 
  if      (strstr(exReply,"(")) tft.print("Lunar");
  else if (strstr(exReply,"O")) tft.print("Solar");
  else if (strstr(exReply,"k")) tft.print("King");
  else tft.print("Sidereal");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"w")) tft.print("Waiting At Home    "); else tft.print("Not Waiting At Home");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"u")) tft.print("Pause At Home Enabled      "); else tft.print("Pausing-At-Home Not Enabled");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"z")) tft.print("Buzzer Enabled "); else tft.print("Buzzer Disabled");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"a")) tft.print("Auto Meridian Flip: Enabled"); else tft.print("Auto Meridian Flip: Disabled");

  #if AXIS1_PEC == ON
    y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
    if (strstr(exReply,"R")) tft.print("PEC was recorded");
    if (transform.mountType != ALTAZM) {
      tft.print("PEC state = ");
      if      (strstr(exReply,"/")) tft.print("PEC Ignored");
      else if (strstr(exReply,",")) tft.print("PEC ready lay");
      else if (strstr(exReply,"~")) tft.print("PEC laying");
      else if (strstr(exReply,";")) tft.print("PEC ready record");
      else if (strstr(exReply,"^")) tft.print("PEC recording");
    }
  #endif

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"A")) tft.print("ALTAZM Mount"); 
  else if (strstr(exReply,"K")) tft.print("FORK Mount"); 
  else if (strstr(exReply,"E")) tft.print("GEM Mount"); 
  else tft.print("No Mount ");

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  if (strstr(exReply,"o")) tft.print("pier side NONE"); 
  else if (strstr(exReply,"T")) tft.print("pier side EAST"); 
  else if (strstr(exReply,"W")) tft.print("pier side WEST");
  else tft.print("pier side N/A") ;

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  tft.print("Pulse Guide Rate = "); tft.print(exReply[strlen(exReply)-3]);

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);
  tft.print("Guide Rate = "); tft.print(exReply[strlen(exReply)-2]);
  // end :GU# data string parsing
}

void ExtStatusScreen::tlsStatus() {
  int y_offset = 386;
  
  // Other status information not from :GU# command
  // Get and show the Time and Location status
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Local Time 24 Hr format
  commandWithReply(":GL#", exReply); 
  tft.print("Local Time = "); tft.print(exReply);

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Current Date
  commandWithReply(":GC#", exReply);
  tft.print("Date = "); tft.print(exReply); 

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show TZ Offset
  commandWithReply(":GG#", exReply);   
  tft.print("Time Zone = "); tft.print(exReply);
  
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Latitude
  commandWithReply(":Gt#", exReply); 
  tft.print("Latitude = "); tft.print(exReply);

  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Longitude
  commandWithReply(":Gg#", exReply); 
  tft.print("Longitude = "); tft.print(exReply);
    
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  commandWithReply(":GX80#", exReply); 
  tft.print("UTC Time and Date = "); tft.print(exReply);
  commandWithReply(":GX81#", exReply); 
  tft.print(":"); tft.print(exReply);
}

void ExtStatusScreen::limitsStatus() {
  int y_offset = 351;
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Longitude
  commandWithReply(":Gh#", exReply); 
  tft.print("Horizon Limit = "); tft.print(exReply);
    
  y_offset +=STATUS_SPACING; tft.setCursor(STATUS_X, y_offset);  
  // show Longitude
  commandWithReply(":Go#", exReply); 
  tft.print("Overhead Limit = "); tft.print(exReply);  
}

ExtStatusScreen extStatusScreen;
