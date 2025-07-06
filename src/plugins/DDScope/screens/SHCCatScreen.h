// =====================================================
// SHCCatScreen.h

#ifndef SHC_S_H
#define SHC_S_H

#include <Arduino.h>
#include "CatalogDefs.h"

class Display;

#define NUM_CAT_ROWS_PER_SCREEN 16 //(370/CAT_H+CAT_Y_SPACING)
//#define SD_CARD_LINE_LEN       110 // Length of line stored to SD card for Custom Catalog
#define MAX_SHC_PAGES           30 // used by paging indexer affay for SHC

//===============================
class SHCCatScreen : public Display {
  public:
    void init(uint8_t);
    void updateShcButtons();
    bool touchPoll(uint16_t px, uint16_t py);
    bool shCatalogButStateChange();
    void updateShcStatus();
    
  private:
    void updateScreen();
    void drawShcCat();
    void saveSHC();
    void writeSHCTarget(uint16_t index);
    void showTargetCoords();

    bool shCatButDetected = false;
    bool delSelected = false;
    bool objSel = false;
    bool saveTouched = false;
    bool shcCatalog =false;
    bool shcEndOfList = false;
    
    // === Catalog selection & paging ===
    uint8_t  _catSelected;
    uint16_t catButSelPos = 0;
    uint16_t shcCurrentPage;
    uint16_t returnToPage;
    uint16_t shcLastPage = 0;
    uint16_t pre_shcIndex = 0;
    uint16_t curSelSIndex = 0;
    uint16_t shcPrevRowIndex = 0;
    uint16_t shcLastRow = 0;
    uint16_t shcRow = 0;
    uint16_t shcPagingArrayIndex[MAX_SHC_PAGES];
    
    // === Strings and fixed char arrays ===
    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};

    char herschObjName[7];
    char prefix[5];
    char title[14];
    char shcCustWrSD[SD_CARD_LINE_LENGTH];

    // Smart Hand Controller (4) Catalogs
    char     shcObjName[NUM_CAT_ROWS_PER_SCREEN][OBJNAME_LENGTH];
    char        shcCons[NUM_CAT_ROWS_PER_SCREEN][CONS_LENGTH];
    char          bayer[NUM_CAT_ROWS_PER_SCREEN][BAYER_LENGTH]; 
    char         shcMag[NUM_CAT_ROWS_PER_SCREEN][MAG_LENGTH];
    char       shcSubId[NUM_CAT_ROWS_PER_SCREEN][SUBID_LENGTH];
    char   truncObjType[5];
    char     objTypeStr[NUM_CAT_ROWS_PER_SCREEN][OBJTYPE_LENGTH]; 
    char  shcRACustLine[NUM_CAT_ROWS_PER_SCREEN][RA_LENGTH]; 
    char shcDECCustLine[NUM_CAT_ROWS_PER_SCREEN][DEC_LENGTH]; 
    char     shcRaSrCmd[NUM_CAT_ROWS_PER_SCREEN][17]; // has ":Sr...#" cmd channel chars
    char    shcDecSrCmd[NUM_CAT_ROWS_PER_SCREEN][18]; // has ":Sd...#" cmd channel chars

    uint8_t    shcRaHrs[NUM_CAT_ROWS_PER_SCREEN][3]; 
    uint8_t    shcRaMin[NUM_CAT_ROWS_PER_SCREEN][3];
    uint8_t    shcRaSec[NUM_CAT_ROWS_PER_SCREEN][3];
    short     shcDecDeg[NUM_CAT_ROWS_PER_SCREEN][5]; 
    uint8_t   shcDecMin[NUM_CAT_ROWS_PER_SCREEN][3];
    uint8_t   shcDecSec[NUM_CAT_ROWS_PER_SCREEN][3];
    
    double       shcAlt[NUM_CAT_ROWS_PER_SCREEN];
    double       shcAzm[NUM_CAT_ROWS_PER_SCREEN];

};

extern SHCCatScreen shcCatScreen;

#endif

