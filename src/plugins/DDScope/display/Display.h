// =====================================================
// Display.h

#ifndef DISPLAY_H
#define DISPLAY_H

//=====================================================================================
// COMPILE-TIME SWITCH to enable capture of bitmaps written to the SD Flash of each 
// Screen for documentation purposes
//#define ENABLE_TFT_CAPTURE  // Comment this line to disable screen capture to SD card
//=====================================================================================

//=====================================================================================
// COMPILE-TIME SWITCH to enable redirect of pixel commands of each Screen to a WiFi client
#define ENABLE_TFT_MIRROR  // Comment this line to disable screen redirect
#define FRAME_TYPE_RAW  0x00 // Raw frame of 307600 bytes
#define FRAME_TYPE_RLE  0x01 // Run Length encoding
#define FRAME_TYPE_LZ4  0x02 // removed this because native JavaScript support limited
#define FRAME_TYPE_DIF  0x03 // Differential pixel encoding...removed due to complexity
#define FRAME_TYPE_DEF  0x04 // Deflate compression is a lossless data compression algorithm 
                             // that combines the LZ77 algorithm and Huffman coding to 
                             // reduce the size of data. Has Native browser support.
//=====================================================================================

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>
#include <TinyGPS++.h> // http://arduiniana.org/libraries/tinygpsplus/
#include "TimeLib.h"

// OnStepX
#include "src/telescope/mount/Mount.h"
#include "src/lib/commands/CommandErrors.h"
#include "src/telescope/mount/goto/Goto.h"
#include "src/telescope/mount/site/Site.h"
#include "src/lib/tasks/OnTask.h"

#include "UIelements.h"
#include "CmdDirect.h"

class AlignScreen;
class Catalog;
class TreasureCatScreen;
class CustomCatScreen;
class SHCCatScreen;
class DCFocuserScreen;
class GotoScreen;
class GudideScreen;
class HomeScreen;
class MoreScreen;
class PlanetsScreen;
class SettingsScreen;
class ExtStatusScreen;
class TouchScreen;

 #ifdef ODRIVE_MOTOR_PRESENT
   class ODriveExt;
   class ODriveScreen;
 #endif

#include <SD.h>
#include "src/Common.h"
#include <XPT2046_Touchscreen.h>
#include "../Adafruit_ILI9486_Teensy/Adafruit_ILI9486_Teensy.h"
#include "WifiDisplay.h"

#define TFTWIDTH 320
#define TFT_HEIGHT 480

#define C_WIDTH  80
#define C_HEIGHT 14

#define BUT_ON  true
#define BUT_OFF false

// ODrive hardwired motor numbers
#define AZM_MOTOR 1
#define ALT_MOTOR 0 

#define TITLE_TEXT_Y       22

// Emergency ABORT button
#define ABORT_X           266
#define ABORT_Y             3
#define ABORT_BOXSIZE_X    52
#define ABORT_BOXSIZE_Y    42

// Screen Selection buttons
#define MENU_X              3
#define MENU_Y             46
#define MENU_X_SPACING     81
#define MENU_Y_SPACING      0
#define MENU_BOXSIZE_X     72
#define MENU_BOXSIZE_Y     45
#define MENU_TEXT_X_OFFSET  8
#define MENU_TEXT_Y_OFFSET 28

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX           250 
#define TS_MINY           250
#define TS_MAXX          3900
#define TS_MAXY          3900
#define MINPRESSURE        80
#define MAXPRESSU        1000
#define PENRADIUS           3

// Color definitions-565 format    Red, Grn, Blu
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define DIM_YELLOW  0xFE60      /* 255, 237, 102 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255, 172 */
#define PINK        0xF81F		
#define XDARK_MAROON 0x1800     /*  99,   0,   0 */   
#define DARK_MAROON 0x3000      /* 197,   0,   0 */
#define GRAY_BLACK  0x0840
#define DARK_RED    0x8140
#define DIM_ORANGE  0xbb00     
#define DIM_MAROON  0x5000

#define THEME_DAY   0
#define THEME_DUSK  1
#define THEME_NIGHT 2

// recommended cutoff for LiPo battery is 19.2V but want some saftey margin
#define BATTERY_LOW_VOLTAGE   21.0  

// sound control of both duration and frequency
#define BEEP tone(STATUS_BUZZER_PIN, 2000UL, 50ULL); // both in milliseconds
#define ALERT tone(STATUS_BUZZER_PIN, 2500UL, 200ULL); // both in milliseconds

enum ScreenEnum
{
  HOME_SCREEN,     // 0
  GUIDE_SCREEN,    // 1
  FOCUSER_SCREEN,  // 2
  GOTO_SCREEN,     // 3
  MORE_SCREEN,     // 4
  ODRIVE_SCREEN,   // 5 
  SETTINGS_SCREEN, // 6
  ALIGN_SCREEN,    // 7
  PLANETS_SCREEN,  // 8
  XSTATUS_SCREEN,  // 9
  TREASURE_SCREEN, // 10
  CUSTOM_SCREEN,   // 11
  SHC_CAT_SCREEN   // 12
}; 

enum SelectedCatalog
{
  STARS,
  MESSIER,
  CALDWELL,
  HERSCHEL,
  INDEX,
  PLANETS,
  TREASURE,
  CUSTOM,
};

// Display object
extern Adafruit_ILI9486_Teensy tft;
extern WifiDisplay wifiDisplay;

extern Button menuButton;
extern CanvasPrint canvDisplayInsPrint;

static XPT2046_Touchscreen ts(TS_CS, TS_IRQ); // Use Interrupts for touchscreen
static TS_Point p;

// --- Common Globals ---
// Color Theme 
extern uint16_t pgBackground; 
extern uint16_t butBackground;
extern uint16_t titleBackground;
extern uint16_t butOnBackground;
extern uint16_t textColor; 
extern uint16_t butOutline; 

// Font sizing info
const uint8_t  defFontWidth = 8; // default Arial
const uint8_t  defFontHeight = 8; // default Arial
const uint8_t  mainFontWidth = 8; // 8pt
const uint8_t  mainFontHeight = 16; // 8pt
const uint8_t  largeFontWidth = 12; // 11pt
const uint8_t  largeFontHeight = 22; // 11pt
const uint8_t  xlargeFontWidth = 17; // 12pt
const uint8_t  xlargeFontHeight = 29; // 12pt

// =========================================
class Display {
  public:
    void init(const char *fwName, int fwMajor, int fwMinor);
    void sdInit();
    void refreshButtons();
    void setCurrentScreen(ScreenEnum);

    void updateAltAzmTarget();

    // Colors, Buttons, BitMap printing
    void drawTitle(int text_x_offset, int text_y_offset, const char* label);
    void drawMenuButtons();
    void drawCommonStatusLabels();
    void drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH);

    // Status and updates
    void updateSpecificScreen();
    void updateCommonStatus();  
    void showOnStepCmdErr();
    void showOnStepGenErr();

    #ifdef ODRIVE_MOTOR_PRESENT
      void showGpsStatus();
      void updateBatVoltage(int axis);
      void motorsOff(uint16_t px, uint16_t py);
    #endif

    // Day or Night Modes
    void setColorTheme(uint8_t index);
    uint8_t getColorThemeIndex() const;

    // frequency and duration adjustable tone
    inline void soundFreq(int freq, int duration) { tone(STATUS_BUZZER_PIN, freq, duration); }

    bool getGeneralErrorMessage(char message[], uint8_t error);
    CommandError cmdError = CE_NONE;

    static ScreenEnum currentScreen;
    uint8_t _colorThemeIndex = 1;  // 0 = Day, 1 = Dusk, 2 = Night
    bool _redrawBut = false;
    volatile bool buttonTouched = false;
    uint32_t resetHomeStartTime = 0;
  
    // Default Font Arial 6x8 is NULL
    const GFXfont *default_font = (const GFXfont *)__null;

    char ddscopeVersionStrBuf[80] = "PluginName:Unknown";
    const char* ddscopeVersionStr = ddscopeVersionStrBuf;

  private:
    char lastCmdErr[4] = "";
    bool firstGPS = true;
    bool firstRTC = true;
    bool trackLedOn = false;
    bool flash = false;
};

extern Display display;

#endif
