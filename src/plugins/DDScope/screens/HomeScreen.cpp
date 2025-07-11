// =====================================================
// HomeScreen.cpp
//
// Author: Richard Benear 3/20/21
#include "../display/Display.h"
#include "HomeScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"
#include "src/lib/axis/Axis.h"
#include "../display/UsbBridge.h"
#include "TouchScreen.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../odriveExt/ODriveExt.h"
#endif

// Column 1 Home Screen
#define COL1_LABELS_X            5
#define COL1_LABELS_Y            182
#define COL1_LABEL_SPACING       21
#define COL1_DATA_BOXSIZE_X      70
#define COL1_DATA_X              82
#define COL1_DATA_Y              COL1_LABELS_Y

// Column 2 Home Screen
#define COL2_LABELS_X            170
#define COL2_LABELS_Y            COL1_LABELS_Y+8
#define COL2_LABEL_SPACING       COL1_LABEL_SPACING
#define COL2_DATA_X              267
#define COL2_DATA_Y              COL1_DATA_Y+8
#define COL2_DATA_BOXSIZE_X      70
#define COL2_DATA_BOXSIZE_Y      COL1_LABEL_SPACING

// Buttons for actions that are not page selections
#define ACTION_BOXSIZE_X         100 
#define ACTION_BOXSIZE_Y         36 
#define ACTION_COL_1_X           6 
#define ACTION_COL_1_Y           324
#define ACTION_COL_2_X           ACTION_COL_1_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_2_Y           ACTION_COL_1_Y
#define ACTION_COL_3_X           ACTION_COL_2_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_3_Y           ACTION_COL_1_Y
#define ACTION_X_SPACING         7
#define ACTION_Y_SPACING         4

#define MOTOR_CURRENT_WARNING    2.0 // Warning when over 2 amps....coil heating occuring
#define MAX_MOTOR_TEMP           110 // Deg F

// ------------ Page Drawing Support ----------------
// Modify the following strings to customize the Home Screen
// Column 1 Status strings
#define COL_1_NUM_ROWS 7
#define COL_1_ROW_1_S_STR "Time-----:"
#define COL_1_ROW_2_S_STR "LST------:"
#define COL_1_ROW_3_S_STR "Latitude-:"
#define COL_1_ROW_4_S_STR "Longitude:"
#define COL_1_ROW_5_S_STR "Temperat-:"
#define COL_1_ROW_6_S_STR "Humidity-:"
#define COL_1_ROW_7_S_STR "Dew Point:"
//#define COL_1_ROW_8_S_STR "Altitude-:"

// Column 2 Status strings
#define COL_2_NUM_ROWS 6
#define COL_2_ROW_1_S_STR "AZM enc deg:"
#define COL_2_ROW_2_S_STR "ALT enc deg:"
#define COL_2_ROW_3_S_STR "AZM Ibus---:"
#define COL_2_ROW_4_S_STR "ALT Ibus---:"
#define COL_2_ROW_5_S_STR "AZM MotTemp:"
#define COL_2_ROW_6_S_STR "ALT MotTemp:"

// Column 1 Command strings
#define COL_1_ROW_1_C_STR ":GL#  " // Current Time
#define COL_1_ROW_2_C_STR ":GS#  " // LST
#define COL_1_ROW_3_C_STR ":Gt#  " // Latitude
#define COL_1_ROW_4_C_STR ":Gg#  " // Longitude
#define COL_1_ROW_5_C_STR ":GX9A#" // temperature deg C
#define COL_1_ROW_6_C_STR ":GX9C#" // humidity
#define COL_1_ROW_7_C_STR ":GX9E#" // dew point deg C
//#define COL_1_ROW_8_C_STR ":GX9D#" // altitiude

// Column One Status strings
//static const char colOneStatusStr[COL_1_NUM_ROWS][12] = {
const char colOneStatusStr[COL_1_NUM_ROWS][12] = {
  COL_1_ROW_1_S_STR, COL_1_ROW_2_S_STR, COL_1_ROW_3_S_STR, COL_1_ROW_4_S_STR,
  COL_1_ROW_5_S_STR, COL_1_ROW_6_S_STR, COL_1_ROW_7_S_STR};

// Column Two Status strings
//static const char colTwoStatusStr[COL_2_NUM_ROWS][14] = {
const char colTwoStatusStr[COL_2_NUM_ROWS][14] = {
  COL_2_ROW_1_S_STR, COL_2_ROW_2_S_STR, COL_2_ROW_3_S_STR, COL_2_ROW_4_S_STR,
  COL_2_ROW_5_S_STR, COL_2_ROW_6_S_STR};

// Column One Status commands
//static const char colOneCmdStr[COL_1_NUM_ROWS][8] = {
const char colOneCmdStr[COL_1_NUM_ROWS][8] = {
  COL_1_ROW_1_C_STR, COL_1_ROW_2_C_STR, COL_1_ROW_3_C_STR, COL_1_ROW_4_C_STR,
  COL_1_ROW_5_C_STR, COL_1_ROW_6_C_STR, COL_1_ROW_7_C_STR};

// Home Screen Button object
Button homeButton(
                ACTION_COL_1_X, ACTION_COL_1_Y, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y,
                butOnBackground, butBackground, butOutline, mainFontWidth, mainFontHeight, "");

// Canvas Print object Custom Font
CanvasPrint canvHomeInsPrint(&Inconsolata_Bold8pt7b);

uint8_t extern fan_icon[];

// ===============================================
// ======= Draw Initial content of HOME PAGE =====
// ===============================================
void HomeScreen::draw() {
  setCurrentScreen(HOME_SCREEN);
  homeButton.setColors(butOnBackground, butBackground, butOutline);

  #ifdef ENABLE_TFT_MIRROR
    wifiDisplay.enableScreenCapture(true);
  #endif
  
  tft.setTextSize(1);
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawTitle(48, TITLE_TEXT_Y, "Direct-Drive Scope");
  tft.setFont(&Inconsolata_Bold8pt7b);
  tft.drawFastVLine(TFTWIDTH/2, 172, 141, textColor);
  
  // Draw the FAN Icon bitmap
  tft.drawBitmap(7, 7, fan_icon, 30, 30,  butBackground, butOutline);

  // ====== Draw Home Screen Status Text ===========
  // Labels for Real Time data only here, no data displayed yet
  // ---- Column 1 ----
  int y_offset = 0;
  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
    tft.print(colOneStatusStr[i]);
    y_offset +=COL1_LABEL_SPACING;
  }
  
  // ---- Column 2 ----
  y_offset = 0;
  for (int i=0; i<COL_2_NUM_ROWS; i++) {
    tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
    tft.print(colTwoStatusStr[i]);
    y_offset +=COL1_LABEL_SPACING;
  }
 
  drawMenuButtons();
  
  // draw and initialize buttons, labels, and status upon entry to this screen
  updateHomeButtons();
  drawCommonStatusLabels();
  updateHomeStatus();
  updateCommonStatus();
  
  #ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false); // stop first
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
  #endif
  #ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("Home");
  #endif
}

// =================================================
// ========== Update HOME Screen Status ============
// =================================================
void HomeScreen::updateHomeStatus() {
  float currentAZEncPos     = 00.0;
  float currentALTEncPos    = 00.0;
  float currentAZMotorCur   = 00.0;
  float currentALTMotorCur  = 00.0;
  float currentALTMotorTemp = 00.0;
  float currentAZMotorTemp  = 00.0;
  char curCol1[11][8] = { {0} };
  char xchReply[13]="";
  int y_offset = 0;

  // Loop through Column 1 poll updates
  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    commandWithReply(colOneCmdStr[i], xchReply); 
    Y;
    if (i == (int)4) { // handle special case....convert C to F
      double tempF = ((atof(xchReply)*9)/5) + 32;
      sprintf(xchReply, "%3.1f F", tempF); // convert back to string to right justify
    }

    // only update screen if value is different
    if (strcmp(curCol1[i], xchReply) != 0) {
      canvHomeInsPrint.printRJ(COL1_DATA_X, COL1_DATA_Y+y_offset, C_WIDTH-5, C_HEIGHT, xchReply, false);
      strcpy(curCol1[i], xchReply);
    }
    y_offset +=COL1_LABEL_SPACING;
  }

  // Column 2 poll updates
  int bitmap_width_sub = 30;
  y_offset =0;

  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM encoder positions
    currentAZEncPos = oDriveExt.getEncoderPositionDeg(AZM_MOTOR);
  #elif
    currentAZEncPos = 0; // define this for non ODrive implementations
  #endif
  canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZEncPos, false);

  
  // ALT encoder
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT encoder positions
    currentALTEncPos = oDriveExt.getEncoderPositionDeg(ALT_MOTOR);
  #elif
    currentALTEncPos = 0; // define this for non ODrive implementations
  #endif
  canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTEncPos, false);
 

  // AZ current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM motor current
    currentAZMotorCur = oDriveExt.getMotorCurrent(AZM_MOTOR);
  #elif
    currentAZMotorCur = 0; // define this for non ODrive implementations
  #endif
  
  if (abs(currentAZMotorCur) > MOTOR_CURRENT_WARNING) { // change background color...Warning!
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur, true);
  } else {
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur, false);
  }

  // ALT current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT motor current
    currentALTMotorCur = oDriveExt.getMotorCurrent(ALT_MOTOR);
  #elif
    currentALTMotorCur = 0; // define this for non ODrive implementations
  #endif

  if (abs(currentALTMotorCur) > MOTOR_CURRENT_WARNING) {
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur, true);
  } else {
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur, false);
  }

  // AZ Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentAZMotorTemp = oDriveExt.getMotorTemp(AZM_MOTOR);
  #elif
    currentAZMotorTemp = 0; // define this for non ODrive implementations
  #endif
  
  if (currentAZMotorTemp >= MAX_MOTOR_TEMP) { // make box red
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp, true);
  } else {
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp, false);
  }

  // ALT Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentALTMotorTemp = oDriveExt.getMotorTemp(ALT_MOTOR);
    #elif
    currentALTMotorTemp = 0; // define this for non ODrive implementations
  #endif

  if (currentALTMotorTemp >= MAX_MOTOR_TEMP) { // make box red
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp, true);
  } else {
    canvHomeInsPrint.printRJ(COL2_DATA_X, COL2_DATA_Y+y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp, false);
  }
}

bool HomeScreen::homeButStateChange() {
  
  bool changed = false;

  if (preAzmState != axis1.isEnabled()) {
    preAzmState = axis1.isEnabled();
    changed = true;
  }

  if (preAltState != axis2.isEnabled()) {
    preAltState = axis2.isEnabled();
    changed = true;
  }

  if (preTrackState != mount.isTracking()) {
    preTrackState = mount.isTracking();
    changed = true;
  }

  if (preSlewState != mount.isSlewing()) {
    preSlewState = mount.isSlewing();
    changed = true;
  }

  if (preHomeState != mount.isHome()) {
    preHomeState = mount.isHome();
    changed = true;
  }

  if (preParkState != park.state) {
    preParkState = park.state;
    changed = true;
  }
  
  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (resetHome || stopButton || parkWasSet) {
      changed = true;
    }
  }

  // forces one more redraw of buttons if true
  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  
  return changed;
}

// ===============================================
// ============ Update Home Buttons ==============
// ===============================================
// This function gets called on inital draw and only after a Touchscreen event with state change
// to prevent TFT display "flickering"
void HomeScreen::updateHomeButtons() {
  
  int y_offset = 0;
  //VLF("updating Home Buttons");
  // ============== Column 1 ===============
  // Draw Enable / Disable Azimuth Motor
  if (axis1.isEnabled()) {
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "AZM Enabled", BUT_ON);
    digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
  } else { //motor off
    digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "AZM Motor", BUT_OFF);
  }

  // Enable / Disable Altitude Motor
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (axis2.isEnabled()) {
    digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "ALT Enabled", BUT_ON);
  } else { //motor off
    digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "ALT Motor", BUT_OFF);
  }

  // Stop all movement
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (stopButton) {
    stopButton = false;
    display._redrawBut = true;
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "All Stopped", BUT_ON);
  } else { 
    homeButton.draw(ACTION_COL_1_X, ACTION_COL_1_Y + y_offset, "Motors Off", BUT_OFF);
  }

  // ============== Column 2 ===============
  y_offset = 0;
  // Start / Stop Tracking
  if (!mount.isTracking()) { 
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Start Track", BUT_OFF);
  } else if(mount.isTracking()) { 
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Tracking", BUT_ON);
  }
  
  // Reset Home Telescope
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (resetHome) {
     resetHome = false;
     display._redrawBut = true;
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Resetting", BUT_ON);     
  } else {
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Reset Home", BUT_OFF);
  }  
  
  // Find Home Telescope
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (mount.isHome()) {
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "At Home", BUT_ON);
    gotoHome = false;             
  } else if (mount.isSlewing()) {
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Slewing", BUT_ON);
  } else {
    homeButton.draw(ACTION_COL_2_X, ACTION_COL_2_Y + y_offset, "Go Home", BUT_OFF);
  }  

  y_offset = 0;
  // ============== Column 3 ===============
  // Night / Dusk / Day  - Theme Modes
  const char* modeLabels[] = { "Day Mode", "Dusk Mode", "Night Mode" };
  homeButton.draw(ACTION_COL_3_X, ACTION_COL_2_Y + y_offset,
                modeLabels[getColorThemeIndex()], BUT_OFF);


  // Park / unPark Telescope
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // park states: PS_UNPARKED, PS_PARKING, PS_PARKED, PS_PARK_FAILED, PS_UNPARKING
  if (park.state == PS_PARKED) {
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "Parked", BUT_ON); 
  } else if (park.state == PS_UNPARKED) { 
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "UnParked", BUT_OFF);
  } else if (park.state == PS_PARKING) {
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "Parking", BUT_OFF); 
  } else if (park.state == PS_UNPARKING) { 
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "UnParking", BUT_OFF);
  } else {
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "ParkFail", BUT_OFF);
  }

  // Set Park Position
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (parkWasSet) {
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "ParkIsSet", true);
    parkWasSet = false;
    display._redrawBut = true;
  } else {
    homeButton.draw(ACTION_COL_3_X, ACTION_COL_3_Y + y_offset, "Set Park", BUT_OFF);
  }
}

// =================================================
// =========== Check for Button Press ==============
// =================================================
bool HomeScreen::touchPoll(int16_t px, int16_t py) {
  // return true forces a refresh of all buttons
  int y_offset = 0;
  
  // ======= Column 1 - Leftmost =======
  // Enable Azimuth motor
  if (px > ACTION_COL_1_X && px < ACTION_COL_1_X + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!axis1.isEnabled()) { // if not On, toggle ON
      axis1.enable(true);
    } else { // since already ON, toggle OFF
      axis1.enable(false);
    }
    return true;
  }
            
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // Enable Altitude motor
  if (px > ACTION_COL_1_X && px < ACTION_COL_1_X + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!axis2.isEnabled()) { // toggle ON 
      axis2.enable(true);
    } else { // toggle OFF
      axis2.enable(false); // Idle the ODrive motor
    }
    return true;
  }

  // STOP everthing requested
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_1_X && px < ACTION_COL_1_X + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    ALERT;
    if (!stopButton) {
      commandBlind(":Q#");
      stopButton = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZM LED
      axis1.enable(false);
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
      axis2.enable(false);
      commandBool(":Td#"); // Disable Tracking
    }
    return true;
  }

  // ======= COLUMN 2 of Buttons - Middle =========
  // Start/Stop Tracking Toggle
  y_offset = 0;
  if (px > ACTION_COL_2_X && px < ACTION_COL_2_X + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!mount.isTracking()) {
      commandBool(":Te#"); // Enable Tracking
    } else {
      // disabling Tracking does not disable motors so leave motor power flags ON
      commandBool(":Td#"); // Disable Tracking
    }
    return true; 
  }

  // Reset Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X && px < ACTION_COL_2_X + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    _oDriveDriver->SetPosition(0, 0.0);
    _oDriveDriver->SetPosition(1, 0.0);
    commandBlind(":hF#"); // home Reset
    resetHome = true;
    return true;
  }
  
  // Find Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X && px < ACTION_COL_2_X + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    commandBlind(":hC#"); // go home
    gotoHome = true;
    return true;
  }
  
  y_offset = 0;
  // ======== COLUMN 3 of Buttons - Leftmost ========
  // Set Night, Dusk, Day  Theme Modes
  if (px > ACTION_COL_3_X && px < ACTION_COL_3_X + ACTION_BOXSIZE_X &&
      py > ACTION_COL_3_Y + y_offset && py < ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;

    uint8_t newIndex = (getColorThemeIndex() + 1) % 3;  // cycle through 0 → 1 → 2 → 0
    setColorTheme(newIndex);
    homeButton.setColors(butOnBackground, butBackground, butOutline);
    menuButton.setColors(butOnBackground, butBackground, butOutline);

    drawTitle(25, TITLE_TEXT_Y, "DIRECT-DRIVE SCOPE");
    draw(); // redraw screen with new colors
    return true;
  }

  // Park and UnPark Telescope 
  // Note: if Time/Date not set then you can't unpark
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X && px < ACTION_COL_3_X + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    // park states: PS_UNPARKED, PS_PARKING, PS_PARKED, PS_PARK_FAILED, PS_UNPARKING}
    if (park.state == PS_UNPARKED) {
      commandBool(":hP#"); // go Park
    } else if (park.state == PS_PARKED) { // only unpark if already parked
      commandBool(":hR#"); // Un park position
    }
    return true;
  }

  // Set Park Position to Current
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X && px < ACTION_COL_3_X + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    commandBool(":hQ#"); // Set Park Position
    parkWasSet = true;
    return true;
  }

  // Fan Control ICON button
  if (px > 0 && px < 45 && py > 0 && py < 45) {
    BEEP;
    if (!fanOn) {
      digitalWrite(FAN_ON_PIN, HIGH);
      fanOn = true;
      tft.drawBitmap(7, 7, fan_icon, 30, 30,  butBackground, GREENYELLOW);
    } else {
      digitalWrite(FAN_ON_PIN, LOW);
      fanOn = false;
      tft.drawBitmap(7, 7, fan_icon, 30, 30,  butBackground, butOutline);
    }
    return false;
  }

  // Check emergency ABORT button area in upper right of screen
  display.motorsOff(px, py);
  return false;
}

HomeScreen homeScreen; 