// =====================================================
// TouchScreen.cpp
//
// Author: Richard Benear 2022

#include "TouchScreen.h"
#include "../display/Display.h"
#include "../../../lib/tasks/OnTask.h"
#include "../display/UsbBridge.h"
#include "../screens/AlignScreen.h"
#include "../screens/CustomCatScreen.h"
#include "../screens/DCFocuserScreen.h"
#include "../screens/ExtStatusScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SHCCatScreen.h"
#include "../screens/SettingsScreen.h"
#include "../screens/TreasureCatScreen.h"
#include "../display/WifiDisplay.h"

#ifdef ODRIVE_MOTOR_PRESENT
#include "../screens/ODriveScreen.h"
#endif

void touchWrapper() { touchScreen.touchScreenPoll(display.currentScreen); }

// ================== Initialize Touchscreen ===================
void TouchScreen::init() {
  if (!ts.begin()) {
    VLF("MSG: TouchScreen, unable to start");
  } else {
    pinMode(TS_IRQ, INPUT_PULLUP); // XPT2046 library doesn't turn on pullup
    ts.setRotation(3);             // touchscreen rotation
    VLF("MSG: TouchScreen, started");
  }

  //Start touchscreen task 
  //300 msec has a reasonable immunity from double clicking
  VF("MSG: Setup, start TouchScreen polling task (rate 300 ms priority 4)... ");
  uint8_t TShandle = tasks.add(300, 0, true, 4, touchWrapper, "TouchScreen");
  if (TShandle) {
    VLF("success");
  } else {
    VLF("FAILED!");
  }
  tasks.setTimingMode(TShandle, TM_MINIMUM);
}

bool externalTouch = false;

// ============ Poll the TouchScreen ==================
void TouchScreen::touchScreenPoll(ScreenEnum tCurScreen) {
    
//Serial.print((int)tCurScreen);

#ifdef ENABLE_TFT_MIRROR
  //wifiDisplay.take_esp_lock();
  // Check for external touch input from ESP32-S3
  if (SERIAL_ESP32S3.available() >= 5) {
    static String incoming = "";
    char c = SERIAL_ESP32S3.peek();
    if (c == 'T') {
      SERIAL_ESP32S3.read(); // read the 'T'
      uint16_t x = (SERIAL_ESP32S3.read() << 8) | SERIAL_ESP32S3.read();
      uint16_t y = (SERIAL_ESP32S3.read() << 8) | SERIAL_ESP32S3.read();
  
      if (x >= 0 && y >= 0) {
        p.x = x;
        p.y = y;
        externalTouch = true;  // Mark as external touch to skip scaling
        //SERIAL_DEBUG.printf("EX TOUCH: x=%d, y=%d\n", x, y);
       }
    } 
  }
  //wifiDisplay.give_esp_lock();
#endif

  if (externalTouch) {
    // Process WiFi external touch without scaling (already scaled)
    processTouch(tCurScreen);
  } else if (ts.touched()) {  // Scale if TFT touch
      p = ts.getPoint();
      
      // Scale from ~0->4000 to tft.width using the calibration #'s
      // VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); // for
      // calibration
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
      // VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); //for
      // calibration
     
      processTouch(tCurScreen);  // Handle the detected touch event
  }
}

// Check for touchscreen button "action" on the selected Screen
// If any button touched then update button to display button pressed
// UpdateXXXXXButtons(bool): bool=true indicates to call this again to flash
// button on then off Does not include the Menu Buttons
void TouchScreen::processTouch(ScreenEnum tCurScreen) {
  if (externalTouch) {
    wifiDisplay.enableScreenCapture(true);
  }

  switch (tCurScreen) {
  
  case HOME_SCREEN:
    if (homeScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on HOME_SCREEN");
    break;
  case GUIDE_SCREEN:
    if (guideScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on GUIDE_SCREEN");
    break;
  case FOCUSER_SCREEN:
    if (dcFocuserScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on FOCUSER_SCREEN");
    break;
  case GOTO_SCREEN:
    if (gotoScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on GOTO_SCREEN");
    break;
  case MORE_SCREEN:
    if (moreScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on MORE_SCREEN");
    break;
  case SETTINGS_SCREEN:
    if (settingsScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on SETTINGS_SCREEN");
    break;
  case ALIGN_SCREEN:
    if (alignScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on ALIGN_SCREEN");
    break;
  case PLANETS_SCREEN:
    if (planetsScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on PLANETS_SCREEN");
    break;
  case TREASURE_SCREEN:
    if (treasureCatScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on TREASURE_SCREEN");
    break;
  case CUSTOM_SCREEN:
    if (customCatScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on CUSTOM_SCREEN");
    break;
  case SHC_CAT_SCREEN:
    if (shcCatScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on SHC_CAT_SCREEN");
    break;
  case XSTATUS_SCREEN:
    break;

#ifdef ODRIVE_MOTOR_PRESENT
  case ODRIVE_SCREEN:
    if (oDriveScreen.touchPoll(p.x, p.y)) {
      display.buttonTouched = true;
    }
      //Serial.println("Touch on ODRIVE_SCREEN");
    break;
#endif

  default:
    Serial.printf("Touch on UNKNOWN_SCREEN=%d", tCurScreen);
    break;
  }

  if (externalTouch) {
    externalTouch = false;
    wifiDisplay.enableScreenCapture(false);
    wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  }
    
  // *************** MENU MAP ****************
  // Current Screen |Cur |Col1|Col2|Col3|Col4|
  // Home-----------| Ho | Gu | Fo | GT | Mo |
  // Guide----------| Gu | Ho | Fo | Al | Mo |
  // Focuser--------| Fo | Ho | Gu | GT | Mo |
  // GoTo-----------| GT | Ho | Fo | Gu | Mo |

  // if ODRIVE_PRESENT then use this menu structure
  //   Current Screen |Cur |Col1|Col2|Col3|Col4|
  //   More & (CATs)--| Mo | GT | Se | Od | Al |
  //   ODrive---------| Od | Ho | Se | Al | Xs |
  //   Extended Status| Xs | Ho | Se | Al | Od |
  //   Settings-------| Se | Ho | Xs | Al | Od |
  //   Alignment------| Al | Ho | Fo | Gu | Od |
  // else if not ODRIVE_PRESENT use this menu structure
  //   More & (CATs)--| Mo | GT | Se | Gu | Al |
  //   Extended Status| Xs | Ho | Se | Al | Mo |
  //   Settings-------| Se | Ho | Xs | Al | Mo |
  //   Alignment------| Al | Ho | Fo | Gu | Mo |

  // Detect which Screen is requested by Menu buttons
  // skip checking these page menus since they don't have this menu setup
  if ((tCurScreen == TREASURE_SCREEN) || (tCurScreen == CUSTOM_SCREEN) ||
      (tCurScreen == SHC_CAT_SCREEN) || (tCurScreen == PLANETS_SCREEN))
    return;

  // Check for any Menu buttons pressed
  // == LeftMost Menu Button ==
  if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X) &&
      p.x < (MENU_X + MENU_BOXSIZE_X)) {
    BEEP;
    switch (tCurScreen) {
    case HOME_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
    case GUIDE_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case FOCUSER_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case GOTO_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
      //==============================================
#ifdef ODRIVE_MOTOR_PRESENT
    case MORE_SCREEN:
      display.setCurrentScreen(GOTO_SCREEN);
      gotoScreen.draw();
      break;
    case ODRIVE_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
#else
    case MORE_SCREEN:
      display.setCurrentScreen(GOTO_SCREEN);
      gotoScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
      break;
#endif
    default:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
    }
  }
  // == Center Left Menu - Column 2 ==
  if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) &&
      p.x > (MENU_X + MENU_X_SPACING) &&
      p.x < (MENU_X + MENU_X_SPACING + MENU_BOXSIZE_X)) {
    BEEP;
    switch (tCurScreen) {
    case HOME_SCREEN:
      display.setCurrentScreen(FOCUSER_SCREEN);
      dcFocuserScreen.draw();
      break;
    case GUIDE_SCREEN:
      display.setCurrentScreen(FOCUSER_SCREEN);
      dcFocuserScreen.draw();
      break;
    case FOCUSER_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
    case GOTO_SCREEN:
      display.setCurrentScreen(FOCUSER_SCREEN);
      dcFocuserScreen.draw();
      break;
      //==================================================
#ifdef ODRIVE_MOTOR_PRESENT
    case MORE_SCREEN:
      display.setCurrentScreen(SETTINGS_SCREEN);
      settingsScreen.draw();
      break;
    case ODRIVE_SCREEN:
      display.setCurrentScreen(SETTINGS_SCREEN);
      settingsScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(SETTINGS_SCREEN);
      settingsScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(XSTATUS_SCREEN);
      extStatusScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(FOCUSER_SCREEN);
      dcFocuserScreen.draw();
      break;
#else
    case MORE_SCREEN:
      display.setCurrentScreen(SETTINGS_SCREEN);
      settingsScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(SETTINGS_SCREEN);
      settingsScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(XSTATUS_SCREEN);
      extStatusScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(FOCUSER_SCREEN);
      dcFocuserScreen.draw();
      break;
#endif
    default:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
    }
  }
  // == Center Right Menu - Column 3 ==
  if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) &&
      p.x > (MENU_X + 2 * MENU_X_SPACING) &&
      p.x < (MENU_X + 2 * MENU_X_SPACING + MENU_BOXSIZE_X)) {
    BEEP;
    switch (tCurScreen) {
    case HOME_SCREEN:
      display.setCurrentScreen(GOTO_SCREEN);
      gotoScreen.draw();
      break;
    case GUIDE_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case FOCUSER_SCREEN:
      display.setCurrentScreen(GOTO_SCREEN);
      gotoScreen.draw();
      break;
    case GOTO_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
      //==============================================
#ifdef ODRIVE_MOTOR_PRESENT
    case MORE_SCREEN:
      display.setCurrentScreen(ODRIVE_SCREEN);
      oDriveScreen.draw();
      break;
    case ODRIVE_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
#else
    case MORE_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(GUIDE_SCREEN);
      guideScreen.draw();
      break;
#endif
    default:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
    }
  }
  // == Right Menu - Column 4 ==
  if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) &&
      p.x > (MENU_X + 3 * MENU_X_SPACING) &&
      p.x < (MENU_X + 3 * MENU_X_SPACING + MENU_BOXSIZE_X)) {
    BEEP;
    switch (tCurScreen) {
    case HOME_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
    case GUIDE_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
    case FOCUSER_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
    case GOTO_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
      //==============================================
#ifdef ODRIVE_MOTOR_PRESENT
    case MORE_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case ODRIVE_SCREEN:
      display.setCurrentScreen(XSTATUS_SCREEN);
      extStatusScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(ODRIVE_SCREEN);
      oDriveScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(ODRIVE_SCREEN);
      oDriveScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(ODRIVE_SCREEN);
      oDriveScreen.draw();
      break;
#else
    case MORE_SCREEN:
      display.setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      break;
    case XSTATUS_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
    case SETTINGS_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
    case ALIGN_SCREEN:
      display.setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      break;
#endif
    default:
      display.setCurrentScreen(HOME_SCREEN);
      homeScreen.draw();
    }
  }
}

TouchScreen touchScreen;