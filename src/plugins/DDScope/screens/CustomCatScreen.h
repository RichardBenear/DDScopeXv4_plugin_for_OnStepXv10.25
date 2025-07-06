// =====================================================
// CustomCatScreen.h

#ifndef CUSTOM_S_H
#define CUSTOM_S_H

#include <Arduino.h>
#include "CatalogDefs.h"

class Display;
class MoreScreen;

typedef struct {
  char* cObjName;
  char* cMag;
  char* cCons;
  char* cObjType;
  char* cSubId;
  char* cRAhhmmss;
  char* cDECsddmmss;
} custom_t; 

class CustomCatScreen : public Display {
  public:
    void init();
    void updateCustomButtons();
    bool touchPoll(uint16_t px, uint16_t py);
    bool cusCatalogButStateChange();
    void updateCustomStatus();

  private:
    void updateScreen();
    void writeCustomTarget(uint16_t index);
    void showTargetCoords();
    bool loadCustomArray();
    void parseCcatIntoArray();
    void drawCustomCat();
    void deleteRow();
    void drawPageData(uint8_t startAbsIndex);

    bool delSelected = false;
    bool buttonDetected = false;
    bool endOfList = false;
    bool isLastPage = false;
    bool objSel = false;

    uint8_t buttonSelected = 0;
    uint8_t totalNumRows = 0;
    uint8_t rowIndex = 0;
    uint8_t relIndex = 0;
    uint8_t absIndex = 0;
    uint8_t prevAbsIndex = 0;
    uint8_t prevRelIndex = 0;

    // Paging
    uint8_t currentPageNum = 0;
    uint8_t prevPageNum = 0;
    uint8_t returnToPage = 0;
    uint8_t lastPageNum = 0;
    uint8_t numRowsLastPage = 0;
    uint8_t rowsThisPage = 0;

    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};
};

extern custom_t cArray[MAX_CUSTOM_CATALOG_ROWS];
extern CustomCatScreen customCatScreen;

#endif
