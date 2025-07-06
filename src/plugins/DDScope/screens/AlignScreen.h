// =====================================================
// Align Screen.h

#ifndef ALIGN_S_H
#define ALIGN_S_H

#include <Arduino.h>

class Display;

//States of the Align State machine
typedef enum {
    Idle_State,
    Home_State,
    Wait_For_Home_State,
    Num_Stars_State,
    Select_Catalog_State,
    Wait_Catalog_State,
    Goto_State,
    Wait_For_Slewing_State,
    Sync_State,
    Status_State,
    Write_State,
} AlignStates;

class AlignScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateAlignStatus();
    void updateAlignButtons();
    bool alignButStateChange();
    void stateMachine();

  private:
    void restoreAlignState();
    void saveAlignState();
    void showAlignStatus();
    void showCorrections();
    void drawGuideButtons();
    void updateGuideButtons();

    uint8_t alignCurStar = 0; // current align star number
    uint8_t numAlignStars = 2; // number of "selected" align stars from buttons 

    char numStarsCmd[3];
    char acorr[10];
    char stateError[20];
    char alignStatus[5];
    char maxAlign[30];
    char curAlign[30];
    char lastAlign[30];
    char alignErr[30];

    bool homeBut = false;
    bool catalogBut = false;
    bool gotoBut = false;
    bool aborted = false;
    bool abortBut = false;
    bool syncBut = false;
    bool saveAlignBut = false;
    bool startAlignBut = false;
    bool firstLabel = false;
    bool preHomeState = false;
    bool preSlewState = false;

    bool guidingEast = false;
    bool guidingWest = false;
    bool guidingNorth = false;
    bool guidingSouth = false;
    bool guidingDone = false;

    // Guide button placement coordinates
    int center_x = TFTWIDTH/2 + 50;
    int center_y = TFTHEIGHT/2+ 35;
    int box_w = 60;
    int box_h = 40;
    int but_spacer = 4;
    int r_x = center_x + box_w/2 + but_spacer;
    int l_x = center_x - box_w/2 - box_w - but_spacer;
    int u_x = center_x - box_w/2;
    int d_x = u_x;
    int r_y = center_y - box_h/2;
    int l_y = r_y;
    int u_y = center_y - box_h - but_spacer;
    int d_y = center_y + but_spacer;
    //int u_y = center_y - box_h/2 - box_h - but_spacer;
    //int d_y = center_y + box_h/2 + but_spacer;
};

extern AlignScreen alignScreen;

#endif