// =====================================================
// Display.cpp
//
// Display Support - Common Display functions
// 3.5" RPi Touchscreen and Display
// SPI Interface
// Author: Richard Benear 3/30/2021 
//   - refactored 6/22
//   - bug fixes 4/25, 5/25
//   - modified command error reporting and bug fixes 7/25

// #include <Arduino.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

// Fonts
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include <Fonts/FreeSansBold12pt7b.h>

// DDScope specific
#include "Display.h"
#include "../catalog/Catalog.h"
#include "../screens/AlignScreen.h"
#include "../screens/TreasureCatScreen.h"
#include "../screens/CustomCatScreen.h"
#include "../screens/SHCCatScreen.h"
#include "../screens/DCFocuserScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SettingsScreen.h"
#include "../screens/ExtStatusScreen.h"
#include "src/telescope/mount/limits/Limits.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../odriveExt/ODriveExt.h"
  #include "../screens/ODriveScreen.h"
#endif

#define TITLE_BOXSIZE_X         313
#define TITLE_BOXSIZE_Y          43
#define TITLE_BOX_X               3
#define TITLE_BOX_Y               2 

// Shared common Status 
#define COM_LABEL_Y_SPACE        17
#define COM_COL1_LABELS_X         8
#define COM_COL1_LABELS_Y       104
#define COM_COL1_DATA_X          74
#define COM_COL1_DATA_Y          COM_COL1_LABELS_Y
#define COM_COL2_LABELS_X       170
#define COM_COL2_DATA_X         235

// copied from website plugin String_en.h
// general (background) errors
#define L_GE_NONE "None"
#define L_GE_MOTOR_FAULT "Motor/driver fault"
#define L_GE_ALT_MIN "Below horizon limit" 
#define L_GE_LIMIT_SENSE "Limit sense"
#define L_GE_DEC "Dec limit exceeded"
#define L_GE_AZM "Azm limit exceeded"
#define L_GE_UNDER_POLE "Under pole limit exceeded"
#define L_GE_MERIDIAN "Meridian limit exceeded"
#define L_GE_SYNC "Sync safety limit exceeded"
#define L_GE_PARK "Park failed"
#define L_GE_GOTO_SYNC "Goto sync failed"
#define L_GE_UNSPECIFIED "Unknown error"
#define L_GE_ALT_MAX "Above overhead limit"
#define L_GE_WEATHER_INIT "Weather sensor init failed"
#define L_GE_SITE_INIT "Time or loc. not updated"
#define L_GE_NV_INIT "Init NV/EEPROM error"
#define L_GE_OTHER "Unknown Error, code"


// Menu button object
Button menuButton(MENU_X, MENU_Y, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, butOnBackground, butBackground, butOutline, largeFontWidth, largeFontHeight, "");

// Canvas Print object Custom Font
CanvasPrint canvDisplayInsPrint(&Inconsolata_Bold8pt7b);
                
ScreenEnum Display::currentScreen = HOME_SCREEN;
//bool Display::_nightMode = false;
float previousBatVoltage = 2.1;

TinyGPSPlus dgps;

uint16_t pgBackground = XDARK_MAROON;
uint16_t butBackground = BLACK;
uint16_t titleBackground = BLACK;
uint16_t butOnBackground = MAROON;
uint16_t textColor = DIM_YELLOW; 
uint16_t butOutline = ORANGE; 

Adafruit_ILI9486_Teensy tft; 
WifiDisplay wifiDisplay;

void updateScreenWrapper() { display.updateSpecificScreen(); }

// =========================================
// ========= Initialize Display ============
// =========================================
void Display::init(const char *fwName, int fwMajor, int fwMinor) {
  
  snprintf(ddscopeVersionStrBuf, sizeof(ddscopeVersionStrBuf),
           "Plugin FW Version:%s: %d.%d",
           fwName, fwMajor, fwMinor);

  V("MSG: "); VLF(ddscopeVersionStr);  // Optional debug print

  VLF("MSG: Display, started"); 
  tft.begin(); delay(1);
  sdInit(); // initialize the SD card and draw start screen

  tft.setRotation(0); // display rotation: Note it is different than touchscreen
  setColorTheme(THEME_DUSK); // always start up in Dusk mode

  // set some defaults
  VLF("MSG: Setting up Limits and Site Name");
  commandBool(":Sh-02#"); //Set horizon limit -2 deg
  Y;
  commandBool(":So88#"); // Set overhead limit 88 deg
  Y;
  commandBool(":SMHome#"); // Set Site 0 name "Home"
  Y;
  // NOTE: OnStep compile time ordering problem when using VSCode instead of Arduino.
  // In Sound.h, bool enabled = STATUS_BUZZER_DEFAULT == ON;  never evaluates to true
  // because it must be overwritten by the value in Config.defaults.h at a later time.
  commandBool(":SX97,1#"); // Turn on Buzzer

  // Start Display update task
  // Update the currently selected screen status
  //   NOTE: this task MUST be a lower priority than the TouchScreen task to prevent
  //   race conditions that result in the WiFi uncompressedBuffer being overwritten
  //   when in the TFT Screen Mirror mode
  VF("MSG: Setup, start Screen status update task (rate 1000 ms priority 5)... ");
  uint8_t us_handle = tasks.add(1000, 0, true, 5, updateScreenWrapper, "UpdateSpecificScreen");
  if (us_handle)  { VLF("success"); } else { VLF("FAILED!"); }
}

// initialize the SD card and boot screen
void Display::sdInit() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    VLF("MSG: SD Card, initialize failed");
  } else {
    VLF("MSG: SD Card, initialized");
  }

  // draw bootup screen
  File StarMaps;
  if((StarMaps = SD.open("NGC1566.bmp")) == 0) {
    VF("File Not Found");
    return;
  } 

  // Draw Start Page of NGC 1566 bitmap
  // tft.fillScreen(pgBackground); 
  // tft.setTextColor(textColor);
  // drawPic(&StarMaps, 1, 0, TFTWIDTH, TFT_HEIGHT);  
  // drawTitle(20, 30, "DIRECT-DRIVE SCOPE");
  // tft.setCursor(60, 80);
  // tft.setTextSize(2);
  // tft.print("Initializing");
  // tft.setTextSize(1);
  // tft.setCursor(120, 120);
  // tft.print("NGC 1566");
}

// Monitor any button that is waiting for a state change (other than being pressed)
// This does not include the Menu Buttons
void Display::refreshButtons() {
  bool buttonPressed = false;  // <-- signal variable

  switch (currentScreen) {
    case HOME_SCREEN:      
      if (homeScreen.homeButStateChange()) {
        homeScreen.updateHomeButtons();
        buttonPressed = true;
      }
      break;       
    case GUIDE_SCREEN:   
      if (guideScreen.guideButStateChange()) {
        guideScreen.updateGuideButtons();
        buttonPressed = true;
      }
      break;        
    case FOCUSER_SCREEN:  
      if (dcFocuserScreen.focuserButStateChange()) {
        dcFocuserScreen.updateFocuserButtons();
        buttonPressed = true;
      }
      break; 
    case GOTO_SCREEN:     
      if (gotoScreen.gotoButStateChange()) {
        gotoScreen.updateGotoButtons();
        buttonPressed = true;
      }
      break;          
    case MORE_SCREEN:     
      if (moreScreen.moreButStateChange()) {
        moreScreen.updateMoreButtons();
        buttonPressed = true;
      }
      break;          
    case SETTINGS_SCREEN:  
      if (settingsScreen.settingsButStateChange()) {
        settingsScreen.updateSettingsButtons();
        buttonPressed = true;
      }
      break;
    case ALIGN_SCREEN:     
      if (alignScreen.alignButStateChange()) { 
        alignScreen.updateAlignButtons();
        buttonPressed = true;
      }
      break;     
    case PLANETS_SCREEN:   
      if (planetsScreen.planetsButStateChange()) {
        planetsScreen.updatePlanetsButtons();
        buttonPressed = true;
      }
      break;
    case TREASURE_SCREEN:   
      if (treasureCatScreen.trCatalogButStateChange()) {
        treasureCatScreen.updateTreasureButtons();
        buttonPressed = true;
      }
      break; 
    case CUSTOM_SCREEN:   
      if (customCatScreen.cusCatalogButStateChange()) {
        customCatScreen.updateCustomButtons();
        buttonPressed = true;
      }
      break; 
    case SHC_CAT_SCREEN:   
      if (shcCatScreen.shCatalogButStateChange()) {
        shcCatScreen.updateShcButtons();
        buttonPressed = true;
      }
      break; 
    case XSTATUS_SCREEN:  
      // No buttons here
      break; 

    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN: 
      if (oDriveScreen.odriveButStateChange()) {
        oDriveScreen.updateOdriveButtons();
        buttonPressed = true;
      }
      break;
    #endif
  }

  if (buttonPressed) {
    //VLF("A button was pressed.");
    //showOnStepCmdErr();
  }
}

// screen selection
void Display::setCurrentScreen(ScreenEnum curScreen) {
  currentScreen = curScreen;
};

// select which screen to update at the Update task rate 
void Display::updateSpecificScreen() {
#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true); 
#endif

  display.refreshButtons();

  switch (currentScreen) {
    case HOME_SCREEN:       homeScreen.updateHomeStatus();            break;
    case GUIDE_SCREEN:      guideScreen.updateGuideStatus();          break;
    case FOCUSER_SCREEN:    dcFocuserScreen.updateFocuserStatus();    break;
    case GOTO_SCREEN:       gotoScreen.updateGotoStatus();            break;
    case MORE_SCREEN:       moreScreen.updateMoreStatus();            break;
    case SETTINGS_SCREEN:   settingsScreen.updateSettingsStatus();    break;
    case ALIGN_SCREEN:      alignScreen.updateAlignStatus();          break;
    case TREASURE_SCREEN:   treasureCatScreen.updateTreasureStatus(); break;
    case CUSTOM_SCREEN:     customCatScreen.updateCustomStatus();     break;
    case SHC_CAT_SCREEN:    shcCatScreen.updateShcStatus();           break;
    case PLANETS_SCREEN:    planetsScreen.updatePlanetsStatus();      break;
    case XSTATUS_SCREEN:    extStatusScreen.updateExStatus();         break;
    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN:     oDriveScreen.updateOdriveStatus();        break;
    #endif
    default:  break;
  }

  // don't do the following updates on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) {
    #ifdef ENABLE_TFT_MIRROR
      wifiDisplay.enableScreenCapture(false);
      wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
    #endif
    
    return;
  }

  updateCommonStatus();
  showOnStepGenErr(); 
  showOnStepCmdErr(); 
  updateBatVoltage(1);
  
#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
#endif
}

// Draw the Title block
void Display::drawTitle(int text_x, int text_y, const char* label) {
  tft.drawRect(1, 1, 319, 479, butOutline); // draw screen outline
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(textColor);
  tft.fillRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, titleBackground);
  //tft.drawRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, butOutline);
  tft.setCursor(TITLE_BOX_X + text_x, TITLE_BOX_Y + text_y);
  tft.print(label);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// 3 Color Themes are supported
void Display::setColorTheme(uint8_t index) {
  _colorThemeIndex = index;

  switch (index) {
    case 0:  // Day Mode
      pgBackground    = XDARK_MAROON;
      butBackground   = BLACK;
      butOnBackground = DIM_MAROON;
      textColor       = DIM_YELLOW;
      butOutline      = ORANGE;
      break;

    case 1:  // Dusk Mode
      pgBackground    = BLACK;
      butBackground   = XDARK_MAROON;
      butOnBackground = DIM_MAROON;
      textColor       = ORANGE;
      butOutline      = ORANGE;
      break;

    case 2:  // Night Mode
      pgBackground    = BLACK;
      butBackground   = XDARK_MAROON;
      butOnBackground = DIM_MAROON;
      textColor       = DIM_ORANGE;
      butOutline      = DIM_ORANGE;
      break;

    default:
      // Fallback to Day Mode
      setColorTheme(THEME_DUSK);
      return;
  }
  menuButton.setColors(butOnBackground, butBackground, butOutline);
}

uint8_t Display::getColorThemeIndex() const {
  return _colorThemeIndex;
}

// Update Battery Voltage
void Display::updateBatVoltage(int axis) {
  float currentBatVoltage = oDriveExt.getODriveBusVoltage(axis);
  //VF("Bat Voltage:"); SERIAL_DEBUG.print(currentBatVoltage);
    char bvolts[12]="00.0 v";
    sprintf(bvolts, "%4.1f v", currentBatVoltage);
    //if (previousBatVoltage == currentBatVoltage) return;
    if (currentBatVoltage < BATTERY_LOW_VOLTAGE) { 
      tft.fillRect(135, 29, 50, 14, butOnBackground);
    } else {
      tft.fillRect(135, 29, 50, 14, butBackground);
    }
    tft.setFont(&Inconsolata_Bold8pt7b);
    tft.setCursor(135, 40);
    tft.print(bvolts);
  previousBatVoltage = currentBatVoltage;
}

// Define Hidden Motors OFF button
// Hidden button is GPS ICON area and will turn off current to both Motors
// This hidden area is on ALL screens for Saftey in case of mount collision
void Display::motorsOff(uint16_t px, uint16_t py) {
  if (py > ABORT_Y && py < (ABORT_Y + ABORT_BOXSIZE_Y) && px > ABORT_X && px < (ABORT_X + ABORT_BOXSIZE_X)) {
    ALERT;
    soundFreq(1500, 200);
    commandBlind(":Q#"); // stops move
    axis1.enable(false); // turn off Motor1
    axis2.enable(false); // turn off Motor2
    commandBool(":Td#"); // Disable Tracking
  }
}

// Show GPS Status ICON
void Display::showGpsStatus() {
  // not on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) return;
  uint8_t extern gps_icon[];
  if (site.isDateTimeReady()) {
  //if (commandBool(":GX89#")) { // return 1 = NOT READY, NOTE: it's irritating that it shows Error Unknown
    firstGPS = true; // turn on One-shot trigger
    if (!flash) {
      flash = true;
      tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, butBackground);
    } else {
      flash = false;
      tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, butOutline);
    }   
  } else { // TLS Ready
      double f=0;
      char reply[12];

      // Set LST for the cat_mgr 
      commandWithReply(":GS#", reply);
      convert.hmsToDouble(&f, reply);
      cat_mgr.setLstT0(f);
      tasks.yield(70);
      // Set Latitude for cat_mgr
      commandWithReply(":Gt#", reply);
      convert.dmsToDouble(&f, reply, true);
      cat_mgr.setLat(f);
      
    tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, butOutline);
  }
}

bool Display::getGeneralErrorMessage(char message[], uint8_t error) {
  uint8_t lastError = error;
  enum GeneralErrors: uint8_t {
  ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT_MIN, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM,
  ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC, ERR_PARK, ERR_GOTO_SYNC, ERR_UNSPECIFIED,
  ERR_ALT_MAX, ERR_WEATHER_INIT, ERR_SITE_INIT, ERR_NV_INIT};
  strcpy(message,"");

  if (lastError == ERR_NONE) strcpy(message, L_GE_NONE); else
  if (lastError == ERR_MOTOR_FAULT) strcpy(message, L_GE_MOTOR_FAULT); else
  if (lastError == ERR_ALT_MIN) strcpy(message, L_GE_ALT_MIN); else
  if (lastError == ERR_LIMIT_SENSE) strcpy(message, L_GE_LIMIT_SENSE); else
  if (lastError == ERR_DEC) strcpy(message, L_GE_DEC); else
  if (lastError == ERR_AZM) strcpy(message, L_GE_AZM); else
  if (lastError == ERR_UNDER_POLE) strcpy(message, L_GE_UNDER_POLE); else
  if (lastError == ERR_MERIDIAN) strcpy(message, L_GE_MERIDIAN); else
  if (lastError == ERR_SYNC) strcpy(message, L_GE_SYNC); else
  if (lastError == ERR_PARK) strcpy(message, L_GE_PARK); else
  if (lastError == ERR_GOTO_SYNC) strcpy(message, L_GE_GOTO_SYNC); else
  if (lastError == ERR_UNSPECIFIED) strcpy(message, L_GE_UNSPECIFIED); else
  if (lastError == ERR_ALT_MAX) strcpy(message, L_GE_ALT_MAX); else
  if (lastError == ERR_WEATHER_INIT) strcpy(message, L_GE_WEATHER_INIT); else
  if (lastError == ERR_SITE_INIT) strcpy(message, L_GE_SITE_INIT); else
  if (lastError == ERR_NV_INIT) strcpy(message, L_GE_NV_INIT); else
  sprintf(message, L_GE_OTHER " %d", (int)lastError);
  return message[0];
}

void Display::showOnStepCmdErr() {
  // Skip on these screens
  if (currentScreen == CUSTOM_SCREEN || 
      currentScreen == SHC_CAT_SCREEN ||
      currentScreen == PLANETS_SCREEN ||
      currentScreen == XSTATUS_SCREEN ||
      currentScreen == TREASURE_SCREEN) return;

  static char displayedErr[80] = "None";
  static char lastSeenErr[80] = "";
  static unsigned long displayStartTime = 0;
  const unsigned long displayDuration = 5000;

  const char* currentError = cmdDirect.getLastCommandErrorString();

  // Show new/different non-"None" error for 5 seconds
  if (strcmp(currentError, "None") != 0 && strcmp(currentError, lastSeenErr) != 0) {
    strncpy(displayedErr, currentError, sizeof(displayedErr));
    displayedErr[sizeof(displayedErr) - 1] = '\0';

    strncpy(lastSeenErr, currentError, sizeof(lastSeenErr));
    lastSeenErr[sizeof(lastSeenErr) - 1] = '\0';

    displayStartTime = millis();
  }

  // After 5 sec, clear display to "None"
  if (strcmp(displayedErr, "None") != 0 &&
      millis() - displayStartTime >= displayDuration) {
    strncpy(displayedErr, "None", sizeof(displayedErr));
    displayedErr[sizeof(displayedErr) - 1] = '\0';
  }

  // Display message
  char errStr[128];
  snprintf(errStr, sizeof(errStr), "Command Error: %s", displayedErr);
  canvDisplayInsPrint.printLJ(3, 453, 314, C_HEIGHT + 2, errStr, false);
}

// ========== OnStep General Errors =============
void Display::showOnStepGenErr() {
  // not on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) return;

  
  char temp[80] = "";
  char temp1[80] = "General Error: ";
  char genError[80] = "";
  
  commandWithReply(":GU#", genError);
  int e = genError[strlen(genError) - 1] - '0';
  getGeneralErrorMessage(temp, e);
  //getGeneralErrorMessage(temp, limits.errorCode());
  strcat(temp1, temp);
  canvDisplayInsPrint.printLJ(3, 470, 314, C_HEIGHT+2, temp1, false);
}

// Draw the Menu buttons
void Display::drawMenuButtons() {
  int y_offset = 0;
  int x_offset = 0;

  tft.setTextColor(textColor);
  tft.setFont(&UbuntuMono_Bold11pt7b); 
  
  // *************** MENU MAP ****************
  // Current Screen   |Cur |Col1|Col2|Col3|Col4|
  // Home-----------| Ho | Gu | Fo | GT | Mo |
  // Guide----------| Gu | Ho | Fo | Al | Mo |
  // Focuser--------| Fo | Ho | Gu | GT | Mo |
  // GoTo-----------| GT | Ho | Fo | Gu | Mo |

  // if ODRIVE_PRESENT then use this menu structure
  //  More & (CATs)--| Mo | GT | Se | Od | Al |
  //  ODrive---------| Od | Ho | Se | Al | Xs |
  //  Extended Status| Xs | Ho | Se | Al | Od |
  //  Settings-------| Se | Ho | Xs | Al | Od |
  //  Alignment------| Al | Ho | Fo | Gu | Od |
  // else if not ODRIVE_PRESENT use this menu structure
  //  More & (CATs)--| Mo | GT | Se | Gu | Al |
  //  Extended Status| Xs | Ho | Se | Al | Mo |
  //  Settings-------| Se | Ho | Xs | Al | Mo |
  //  Alignment------| Al | Ho | Fo | Gu | Mo |

  switch(Display::currentScreen) {
    case HOME_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CTLGS", BUT_OFF);
      break;

   case GUIDE_SCREEN:
      x_offset = 0;
      y_offset = 0;
       menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      break;

   case FOCUSER_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
    
   case GOTO_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      break;
      
   case MORE_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;

      #ifdef ODRIVE_MOTOR_PRESENT
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      #endif

      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      break;

    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "xSTAT", BUT_OFF);
      break;
    #endif
    
    case SETTINGS_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "XSTAT", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
       
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "MORE..", BUT_OFF);
      #endif  
      break;

      case XSTATUS_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
       
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      #endif  
      break;

    case ALIGN_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      #endif  
      break;

   default: // HOME Screen
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
       
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
    
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
     
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
     
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
  }  
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ==============================================
// ====== Draw multi-screen status labels =========
// ==============================================
// These particular status labels are placed near the top of most Screens.
void Display::drawCommonStatusLabels() {
  tft.setFont(&Inconsolata_Bold8pt7b);
  int y_offset = 0;

  // Column 1
  // Display RA Current
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA-----:");

  // Display RA Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA tgt-:");

  // Display DEC Current
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC----:");

  // Display DEC Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC tgt:");

  // Column 2
  // Display Current Azimuth
  y_offset =0;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ-----:");

  // Display Target Azimuth
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ  tgt:");
  
  // Display Current ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT----:"); 

  // Display Target ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT tgt:"); 
  
  tft.drawFastHLine(1, COM_COL1_LABELS_Y+y_offset+6, TFTWIDTH-1, textColor);
  tft.drawFastVLine(TFTWIDTH/2, 96, 60, textColor);
}

// UpdateCommon Status - Real time data update for the particular labels printed above
// This Common Status is found at the top of most pages.
void Display::updateCommonStatus() { 
  //VLF("updating common status");
  showGpsStatus();

  // Flash tracking LED if mount is tracking
  if (mount.isTracking()) {
    if (trackLedOn) {
      digitalWrite(STATUS_TRACK_LED_PIN, HIGH); // LED OFF, active low
      tft.setFont(&Inconsolata_Bold8pt7b);
      tft.fillRect(50, 28, 72, 14, BLACK); 
      tft.setCursor(50, 38);
      tft.print("        ");
      trackLedOn = false;
    } else {
      digitalWrite(STATUS_TRACK_LED_PIN, LOW); // LED ON
      tft.setFont(&Inconsolata_Bold8pt7b);
      tft.setCursor(50, 38);
      tft.print("Tracking");
      trackLedOn = true;
    }
  #ifdef ODRIVE_MOTOR_PRESENT
  // frequency varying alarm if Motor and Encoders positions are too far apart indicating unbalanced loading or hitting obstruction
    oDriveExt.MotorEncoderDelta();
  #endif
  } else { // not tracking 
    digitalWrite(STATUS_TRACK_LED_PIN, HIGH); // LED OFF
    tft.setFont(&Inconsolata_Bold8pt7b);
    tft.fillRect(50, 28, 72, 14, BLACK); 
    tft.setCursor(50, 38);
    tft.print("        ");
    trackLedOn = false;
  }

  if (currentScreen == CUSTOM_SCREEN || 
      currentScreen == SHC_CAT_SCREEN ||
      currentScreen == PLANETS_SCREEN ||
      currentScreen == TREASURE_SCREEN) return;

  char ra_hms[10]   = ""; 
  char dec_dms[11]  = "";
  char tra_hms[10]  = "";
  char tdec_dms[11] = "";
  
  int y_offset = 0;
  // ----- Column 1 -----
  // Current RA, Returns: HH:MM.T# or HH:MM:SS# (based on precision setting)
  commandWithReply(":GR#", ra_hms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y, C_WIDTH, C_HEIGHT, ra_hms, false);
  Y;
  // Target RA, Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  y_offset +=COM_LABEL_Y_SPACE; 
  commandWithReply(":Gr#", tra_hms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tra_hms, false);
  Y;
  // Current DEC
   y_offset +=COM_LABEL_Y_SPACE; 
  commandWithReply(":GD#", dec_dms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, dec_dms, false);
  Y;
  // Target DEC
  y_offset +=COM_LABEL_Y_SPACE;  
  commandWithReply(":Gd#", tdec_dms); 
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tdec_dms, false);
  Y;
  //VLF("common column 1 check point complete");

  // Select format of Common Display for ALT/AZM 
  #define SHOW_ALT_AZM_IN_DMS   OFF
  #if SHOW_ALT_AZM_IN_DMS == ON
  // NOTE: When using LX200 commands to get current and target Alt/Azm in degrees, minutes, seconds,
  // the target LX200 command will never update unless the target coordinates for Alt/Azm are
  // written using LX200 :Sz[DDD*MM'SS]# for Azimuth target; :Sa[sDD*MM'SS]# for Altitude Target.
  // Rather than going through the transformation (which I started a method for), I prefer to see the 
  // degrees "double" representation.
  // ----- Column 2 -----
  y_offset =0;

  char cAzmStr[20] = "";
  char tAzmStr[20] = "";
  char cAltStr[20] = "";
  char tAltStr[20] = "";

  // Get CURRENT AZM
  commandWithReply(":GZ#", cAzmStr); // DDD*MM'SS# 
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, cAzmStr, false);
  Y;

  // Get TARGET AZM
  y_offset +=COM_LABEL_Y_SPACE;  
  commandWithReply(":Gz#", tAzmStr); // DDD*MM'SS# 
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tAzmStr, false);
  Y;

  // Get CURRENT ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  commandWithReply(":GA#", cAltStr);	// sDD*MM'SS#
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, cAltStr, false);
  Y;
  // Get TARGET ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  commandWithReply(":Gal#", tAltStr);	// sDD*MM'SS#
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tAltStr, false);
  Y;

  #elif SHOW_ALT_AZM_IN_DMS == OFF
  y_offset =0;
  Coordinate dispTarget = goTo.getGotoTarget();
  transform.rightAscensionToHourAngle(&dispTarget, true);
  transform.equToHor(&dispTarget);
  
  // Get CURRENT AZM
  double cAzm_d = NormalizeAzimuth(radToDeg(mount.getPosition(CR_MOUNT_HOR).z));
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, cAzm_d, false);
  Y;

  // Get TARGET AZM
  y_offset +=COM_LABEL_Y_SPACE;  
  double tAzm_d = NormalizeAzimuth(radToDeg(dispTarget.z));
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tAzm_d, false);
  Y;

  // Get CURRENT ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  double cAlt_d = radToDeg(mount.getPosition(CR_MOUNT_ALT).a);
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, cAlt_d, false);
  Y;
  // Get TARGET ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH-20, C_HEIGHT, radToDeg(dispTarget.a), false);
  Y;

  #endif
  //VLF("column 2 complete");
}

// NOTE!: Still under development - bugs
// This is only used if someone wants to use :Gal# or :Gz# to get the
//     ALT/AZM target coordinates in DD:MM:SS. I think(maybe), that :Gal# and :Gz#
//     will not be updated unless  targets :Sz and :Sa are set. Probably other ways to do this.
//
// Call this function anytime RA/DEC is set by :SR or :SD to update
//    the ALT and AZM target specifically if the Degree, Minute, Seconds
//    format is being displayed.
void Display::updateAltAzmTarget() {
  char cRaHms[11] = ""; 
  char cDecDms[11] = "";
  char cLstHms[11] = "";
  char cLatDm[11] = "";

  double tRaDeg = 0.0;
  double tDecDeg = 0.0;
  double tLstDeg = 0.0;
  double tLatDeg = 0.0;

  // Get values via LX200 commands
  commandWithReply(":Gr#", cRaHms);    // Target RA HH:MM:SS
  commandWithReply(":Gd#", cDecDms);   // Target Dec ±DD*MM:SS
  commandWithReply(":GL#", cLstHms);   // Local Sidereal Time HH:MM:SS
  commandWithReply(":Gt#", cLatDm);    // Latitude ±DD*MM

  tRaDeg *= 15.0;  // RA: hours to degrees
  tLstDeg *= 15.0; // LST: hours to degrees

  // Convert to degrees, assumes PM_HIGH
  convert.hmsToDouble(&tRaDeg,  cRaHms);         // RA  → degrees (×15)
  convert.dmsToDouble(&tDecDeg, cDecDms, true);  // Dec → degrees
  convert.hmsToDouble(&tLstDeg, cLstHms);        // LST → degrees
  convert.dmsToDouble(&tLatDeg, cLatDm, true);   // Lat → degrees  ← FIXED BUG!

  // Compute Hour Angle
  double HA = tLstDeg - tRaDeg;
  if (HA < 0) HA += 360.0;

  // Convert all to radians
  double haRad  = radians(HA);
  double decRad = radians(tDecDeg);
  double latRad = radians(tLatDeg);

  // Compute Altitude
  double sinAlt = sin(decRad) * sin(latRad) + cos(decRad) * cos(latRad) * cos(haRad);
  double altRad = asin(sinAlt);
  double altDeg = degrees(altRad);

  // Compute Azimuth
  double cosAz = (sin(decRad) - sin(altRad) * sin(latRad)) / (cos(altRad) * cos(latRad));
  cosAz = constrain(cosAz, -1.0, 1.0);  // Prevent domain errors
  double azRad = acos(cosAz);
  double azDeg = degrees(azRad);
  if (sin(haRad) > 0) azDeg = 360.0 - azDeg;

  // VF("RA: "); VL(tRaDeg); 
  // VF("Dec: "); VL(tDecDeg); 
  // VF("LST: "); VL(tLstDeg); 
  // VF("Lat: "); VL(tLatDeg); 
  // VF("HA: "); VL(HA); 
  // VF("Alt: "); VL(altDeg); 
  // VF("Az: "); VL(azDeg);

  // Convert degrees back to LX200 format
  char targAzm[11] = "";
  char targAlt[11] = "";
  convert.doubleToDms(targAzm, azDeg, true, false, PM_HIGH);   // unsigned
  convert.doubleToDms(targAlt, altDeg, true, true, PM_HIGH);   // signed

  // Reformat altitude string to strip leading zero if present after sign because
  // the :Sa command requires only 2 digits for degrees with a sign in front.
  // I believe this is a bug in doubleToDms that currently uses:
  // char form[]="%s%02d*%02d:%02d.%03d";
  // but should instead have something like this:
  // if (signPresent && p == PM_HIGH)
  //     strcpy(form, "%s%d*%02d:%02d"); // remove leading zero for degrees
  //     sprintf(reply, form, sign, ideg, imin, isec);
  //   else
  //     strcpy(form, "%03d*%02d:%02d");
  //     sprintf(reply, form, ideg, imin, isec);
  // }
  if (targAlt[0] == '+' || targAlt[0] == '-') {
    if (targAlt[1] == '0') {
      // Shift digits left to remove leading zero from degree field
      memmove(&targAlt[1], &targAlt[2], strlen(&targAlt[2]) + 1);  // includes null terminator
    }
  }

  // Create and send LX200 set target commands
  char tempAzm[16], tempAlt[16];  // Buffers for full commands
  snprintf(tempAzm, sizeof(tempAzm), ":Sz%s#", targAzm);
  snprintf(tempAlt, sizeof(tempAlt), ":Sa%s#", targAlt);
  commandBool(tempAzm);
  commandBool(tempAlt);
}

// draw a picture -This member function is a copy from rDUINOScope but with 
//    pushColors() changed to drawPixel() with a loop
// rDUINOScope - Arduino based telescope control system (GOTO).
//    Copyright (C) 2016 Dessislav Gouzgounov (Desso)
//    PROJECT Website: http://rduinoscope.byethost24.com
void Display::drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH){
  uint8_t header[14 + 124]; // maximum length of bmp file header
  uint16_t color[320];  
  uint16_t num;   
  uint8_t color_l, color_h;
  uint32_t i,j,k;
  uint32_t width;
  uint32_t height;
  uint16_t bits;
  uint32_t compression;
  uint32_t alpha_mask = 0;
  uint32_t pic_offset;
  char temp[20]="";

  /** read header of the bmp file */
  i=0;
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==14){
      break;
    }
  }

  pic_offset = (((uint32_t)header[0x0A+3])<<24) + (((uint32_t)header[0x0A+2])<<16) + (((uint32_t)header[0x0A+1])<<8)+(uint32_t)header[0x0A];
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==pic_offset){
      break;
    }
  }
 
  /** calculate picture width ,length and bit numbers of color */
  width = (((uint32_t)header[0x12+3])<<24) + (((uint32_t)header[0x12+2])<<16) + (((uint32_t)header[0x12+1])<<8)+(uint32_t)header[0x12];
  height = (((uint32_t)header[0x16+3])<<24) + (((uint32_t)header[0x16+2])<<16) + (((uint32_t)header[0x16+1])<<8)+(uint32_t)header[0x16];
  compression = (((uint32_t)header[0x1E + 3])<<24) + (((uint32_t)header[0x1E + 2])<<16) + (((uint32_t)header[0x1E + 1])<<8)+(uint32_t)header[0x1E];
  bits = (((uint16_t)header[0x1C+1])<<8) + (uint16_t)header[0x1C];
  if(pic_offset>0x42){
    alpha_mask = (((uint32_t)header[0x42 + 3])<<24) + (((uint32_t)header[0x42 + 2])<<16) + (((uint32_t)header[0x42 + 1])<<8)+(uint32_t)header[0x42];
  }
  sprintf(temp, "%lu", pic_offset);  //VF("pic_offset=");  VL(temp);
  sprintf(temp, "%lu", width);       //VF("width=");       VL(temp);
  sprintf(temp, "%lu", height);      //VF("height=");      VL(temp);
  sprintf(temp, "%lu", compression); //VF("compression="); VL(temp);
  sprintf(temp, "%d",  bits);        //VF("bits=");        VL(temp);
  sprintf(temp, "%lu", alpha_mask);  //VF("alpha_mask=");  VL(temp);

  /** set position to pixel table */
  StarMaps->seek(pic_offset);
  /** check picture format */
  if(pic_offset == 138 && alpha_mask == 0){
    /** 565 format */
    tft.setRotation(0);
    /** read from SD card, write to TFT LCD */
    for(j=0; j<HH; j++){ // read all lines
      for(k=0; k<WW; k++){ // read two bytes and pack them in int16, continue for a row
          color_l = StarMaps->read();
          color_h = StarMaps->read();
          color[k]=0;
          color[k] += color_h;
          color[k] <<= 8;
          color[k] += color_l;
      }
      num = 0;
    
      while (num < x + width - 1){  //implementation for DDScope
      //if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
        //setAddrWindow(x, y, x + 1, y + 1);
        //pushColor(uint16_t color)

        //while (num < x+width-1){
        tft.drawPixel(x+num, y+j, color[num]); //implementation for DDScope
        num++;
      }
      // dummy read twice to align for 4 
      if(width%2){
        StarMaps->read();StarMaps->read();
      }
    }
  }
}

Display display;