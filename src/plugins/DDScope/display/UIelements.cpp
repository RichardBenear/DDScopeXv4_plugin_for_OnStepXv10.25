// =====================================================
// UIelements.cpp
//
// User Interface elements (Buttons, CanvasPrint)
// Author: Richard Benear 6/22

#include "Display.h"
#include "UIelements.h"
#include "Adafruit_GFX.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"

// =======================================================================
// ======================= Button UI elements ============================
// =======================================================================
// Button constructor
Button::Button(
      int           x,
      int           y,
      uint16_t      width,
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      uint16_t      colorBorder,
      uint8_t       fontCharWidth,
      uint8_t       fontCharHeight,
      const char*   label)
  {                
    b_x               = x;
    b_y               = y;
    b_width           = width;
    b_height          = height;
    b_colorActive     = colorActive;
    b_colorNotActive  = colorNotActive;
    b_colorBorder     = colorBorder;
    b_fontCharWidth   = fontCharWidth;
    b_fontCharHeight  = fontCharHeight;      
    b_label           = label;
  }

// =====================================================================
// Draw Button, Center Text, Custom Font
// Draw a single button, assume constructor called to set colors and font size
// Center text in button both x and y
void Button::draw(int x, int y, uint16_t width, uint16_t height, const char* label, bool active) {
  int buttonRadius = BUTTON_RADIUS;
  int str_len = strlen(label);

  // font width is a value selected for alphabetic characters only, no others
  uint16_t xTextOffset = (width  - b_fontCharWidth*str_len)/2;

  // The y text offset is not an obvious calculation....explanation follows:
  // The GFX default font uses the lower left corner for the origin.
  // But, for Custom fonts, that are used here, the upper left is the origin.
  // Adafruit_GFX::setFont(const GFXfont *f) does an offset of +/- 6 pixels based on default vs. custom font.
  // Hence, that is why this calculation looks weird, standards would be nice :-/
  uint16_t yTextOffset = ((height - b_fontCharHeight)/2) + b_fontCharHeight-4;
  if (active) { // show active background
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorActive);
  } else {
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorNotActive);
  }
  tft.drawRoundRect(x, y, width, height, buttonRadius, b_colorBorder);
  tft.setCursor(x+xTextOffset, y+yTextOffset);
  tft.print(label);
}

// Draw a single button, overload, no font changes from constructor
void Button::draw(int x, int y, const char* label, bool active) {
  draw(x, y, b_width, b_height, label, active);
}

// Draw a single button, overload, no x axis changes involved
void Button::draw(int y, const char* label, bool active) {
  draw(b_x, y, b_width, b_height, label, active);
}

// =====================================================================
// Draw Button, Left Justify Text, Default GFX Font Arial
// Draw a single button, assume constructor called to set colors and font size
// Center y axis only, no centering for x axis
void Button::drawLJ(int x, int y, uint16_t width, uint16_t height, const char* label, bool active) {
  int buttonRadius = BUTTON_RADIUS;
  uint16_t yTextOffset = ((height - b_fontCharHeight)/2) + b_fontCharHeight-6;
  if (active) { // show active background
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorActive);
  } else {
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorNotActive);
  }
  tft.drawRoundRect(x, y, width, height, buttonRadius, b_colorBorder);
  tft.setCursor(x+2, y+yTextOffset);
  tft.print(label);
}

void Button::setColors(uint16_t active, uint16_t notActive, uint16_t border) {
  b_colorActive = active;
  b_colorNotActive = notActive;
  b_colorBorder = border;
}

// =======================================================================
// ================= Canvas print UI elements ============================
// =======================================================================
// Canvas Print constructor
CanvasPrint::CanvasPrint(const GFXfont *font) { c_font = (GFXfont *)font; }

// ===================== Canvas Print ==============================
// Right Justified, vertically centered
void CanvasPrint::printRJ(int x, int y, uint16_t width, uint16_t height, const char* c_label, bool warning) {
  char ch_label[80] = "";
  int y_box_offset;
  if (c_font == NULL) {
    y_box_offset = -6; // default font offset
  } else {
    y_box_offset = 10; // custom font offset
  }
  GFXcanvas1 canvas(width, height); // creates buffer
  canvas.setFont(c_font); 
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(ch_label, "%9s", c_label);
  canvas.print(ch_label); // print to buffer
  if (warning) { // show warning background
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butOnBackground);
  } else {
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butBackground);
  }
}

// Left Justified, vertically centered
void CanvasPrint::printLJ(int x, int y, uint16_t width, uint16_t height, const char* c_label, bool warning) {
  char ch_label[80] = "";
  int y_box_offset;
  if (c_font == NULL) {
    y_box_offset = -6; // default font offset
  } else {
    y_box_offset = 10; // custom font offset
  }
  GFXcanvas1 canvas(width, height);
  canvas.setFont(c_font); 
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(ch_label, "%-9s", c_label);
  canvas.print(ch_label);
  if (warning) { // show warning background
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butOnBackground);
  } else {
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butBackground);
  }
}

// Right Justified Overload for double
void CanvasPrint::printRJ(int x, int y, uint16_t width, uint16_t height, double label, bool warning) {
  char ch_label[80] = "";
  int y_box_offset;
  if (c_font == NULL) {
    y_box_offset = -6; // default font offset
  } else {
    y_box_offset = 10; // custom font offset
  }
  GFXcanvas1 canvas(width, height);
  canvas.setFont(c_font); 
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(ch_label, "%6.1f", label);
  canvas.print(ch_label);
  if (warning) { // show warning background
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butOnBackground);
  } else {
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, textColor, butBackground);
  }
}
/*
// Right Justified Overload for double 
// ***Not sure why this doesn't work. Truncates digits after decimal. Using the one above instead. 
void CanvasPrint::printRJ(int x, int y, uint16_t width, uint16_t height, double d_label, bool warning) {
  char c_label[7]="";
  sprintf(c_label, "%6.1f", d_label);
  printRJ(x, y, width, height, c_label, warning);
}
*/
// Right JustifiedOverload for int
void CanvasPrint::printRJ(int x, int y, uint16_t width, uint16_t height, int i_label, bool warning) {
  char ch_label[7]="";
  sprintf(ch_label, "%d", i_label);
  printRJ(x, y, width, height, ch_label, warning);
 }

// Left Justified Overload for int
void CanvasPrint::printLJ(int x, int y, uint16_t width, uint16_t height, int i_label, bool warning) {
  char ch_label[7]="";
  sprintf(ch_label, "%-d", i_label);
  printLJ(x, y, width, height, ch_label, warning);
 }
