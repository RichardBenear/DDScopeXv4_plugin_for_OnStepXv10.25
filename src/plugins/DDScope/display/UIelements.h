// =====================================================
// UIelements.h
// User Interface elements (Button, CanvasPrint)
//
// Author: Richard Benear 6/22

#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#define BUTTON_RADIUS 7

//----------------------------------------------------------
// Button element
//----------------------------------------------------------
class Button {
  
	public:
    // constructor
    Button(
      int           x,
      int           y,
      uint16_t      width,
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      uint16_t      colorBorder,
      uint8_t       fontCharWidth,
      uint8_t       fontCharHeight,
      const char*   label);

    void draw     (int x, int y, uint16_t width, uint16_t height, const char* label, bool active);
    void draw     (int x, int y,                                  const char* label, bool active);
    void draw     (       int y,                                  const char* label, bool active);
    void drawLJ   (int x, int y, uint16_t width, uint16_t height, const char* label, bool active);
    void setColors(uint16_t active, uint16_t notActive, uint16_t border);

  private:
    int           b_x;
    int           b_y;
    uint16_t      b_width; 
    uint16_t      b_height;
    uint16_t      colorActive;
    uint16_t      b_colorActive;
    uint16_t      b_colorNotActive;
    uint16_t      b_colorBorder;
    uint8_t       b_fontCharWidth;
    uint8_t       b_fontCharHeight;      
    const char*   b_label;
};

//----------------------------------------------------------
// Canvas Text element
//----------------------------------------------------------
class CanvasPrint {
  
	public:
    // constructor
    CanvasPrint(const GFXfont *font);

    void  printRJ(int x, int y, uint16_t width, uint16_t height, const char* c_label, bool warning);
    void  printRJ(int x, int y, uint16_t width, uint16_t height,      double d_label, bool warning);
    void  printRJ(int x, int y, uint16_t width, uint16_t height,         int i_label, bool warning);

    void  printLJ(int x, int y, uint16_t width, uint16_t height, const char* c_label, bool warning);
    void  printLJ(int x, int y, uint16_t width, uint16_t height,         int d_label, bool warning);
   
  private:
    const GFXfont *c_font;    
};

extern Button button;
extern CanvasPrint canvasPrint;

#endif