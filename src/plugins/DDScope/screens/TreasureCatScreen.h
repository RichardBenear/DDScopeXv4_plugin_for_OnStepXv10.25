
// =====================================================
// TreasureScreen.h

#ifndef TREASURE_S_H
#define TREASURE_S_H

#include <Arduino.h>
class Display;

#define NUM_CAT_ROWS_PER_SCREEN 16 //(370/CAT_H+CAT_Y_SPACING)
#define SD_CARD_LINE_LEN       110 // Length of line stored to SD card for Custom Catalog
#define MAX_TREASURE_PAGES       8 // more than needed with current settings
#define MAX_TREASURE_ROWS      129 // full number of rows in catlog; starts with 1, not 0

//====== Treasure Screen Class =========================
class TreasureCatScreen : public Display {
  public:
    void init();
    void updateTreasureButtons();
    bool touchPoll(uint16_t px, uint16_t py);
    bool trCatalogButStateChange();
    void updateTreasureStatus();

  private:  
    typedef struct {
      char* tObjName;
      char* tRAhRAm;
      char* tsDECdDECm;
      char* tCons;
      char* tObjType;
      char* tMag;
      char* tSize;
      char* tSubId;
    } treasure_t; 

    void updateScreen();
    bool loadTreasureArray();
    void parseTcatIntoArray();
    void drawTreasureCat();
    void saveTreasure();
    void writeTreasureTarget(uint16_t index);
    void showTargetCoords();

    bool delSelected = false;
    bool objSel = false;
    bool saveTouched = false;
    bool tEndOfList = false;
    bool isLastPage = false;

    // ======== Arrays ==========
    char       tRaSrCmd[MAX_TREASURE_ROWS][13]; 
    char      tDecSrCmd[MAX_TREASURE_ROWS][14];
    char      tRAhhmmss[MAX_TREASURE_ROWS][9];
    char    tDECsddmmss[MAX_TREASURE_ROWS][10];
    char Treasure_Array[MAX_TREASURE_ROWS][SD_CARD_LINE_LEN];
    uint16_t tFiltArray[MAX_TREASURE_ROWS];
    treasure_t  tArray[MAX_TREASURE_ROWS];
    double        dtAlt[MAX_TREASURE_ROWS];
    double        dtAzm[MAX_TREASURE_ROWS];
    uint16_t tPagingArrayIndex[MAX_TREASURE_PAGES];
    char   treaCustWrSD[SD_CARD_LINE_LEN];

    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};

    uint16_t trCatButDetected;
    uint16_t catButSelPos = 0;
    uint16_t tCurrentPage;
    uint16_t tPrevPage;
    uint16_t returnToPage;

    uint16_t pre_tAbsIndex;
    uint16_t pre_tRelIndex;
    uint16_t curSelTIndex;
    uint16_t tPrevRowIndex;
    
    uint16_t tAbsIndex;
    uint16_t tAbsRow;
    uint16_t tLastPage;
    uint16_t tRow;
    uint16_t treRowEntries;
    uint16_t tNumRowsLastPage;
    uint16_t tRowsPerPage = 0;
     
};

extern TreasureCatScreen treasureCatScreen;

#endif
