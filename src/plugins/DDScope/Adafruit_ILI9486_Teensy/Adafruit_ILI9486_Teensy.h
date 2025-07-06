//==================================================
// Adafruit_ILI9486_Teensy.h
//
//See rights and use declaration in License.h
//based on Adafruit ili9341 library @ Dec 2016
//modified for the Maple Mini by Steve Strong 2017
//modified for Teensy by Richard Palmer 2017

#ifndef _ADAFRUIT_ILI9486H_Teensy
#define _ADAFRUIT_ILI9486H_Teensy

#include <Adafruit_GFX.h>
#include <SPI.h> 
#include <ILI9341_t3.h>

#define SPISET SPISettings(36000000,MSBFIRST,SPI_MODE0)
#define SPIBLOCKMAX 320 // one ROW is a good value to avoid really long SPI transfers

extern uint8_t useDMA;

#define TFTWIDTH	320
#define TFTHEIGHT	480

#define ILI9486_INVOFF 0x20
#define ILI9486_INVON  0x21
#define ILI9486_CASET	 0x2A
#define ILI9486_PASET	 0x2B
#define ILI9486_RAMWR	 0x2C
#define ILI9486_MADCTL 0x36
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

//Define pins and Output Data Registers
//Control pins |RS |CS |RST|
#define TFT_RST        5
#define TFT_RS         9
#define TFT_CS         10

#define CS_ACTIVE    digitalWrite(TFT_CS, LOW);  
#define CS_IDLE      digitalWrite(TFT_CS, HIGH); 
#define CD_COMMAND   digitalWrite(TFT_RS, LOW);  
#define CD_DATA      digitalWrite(TFT_RS, HIGH);  

// name changed to swap16 from swap because same name elsewhere caused error
#define swap16(a, b) { int16_t t = a; a = b; b = t; }

//======================================================================
class Adafruit_ILI9486_Teensy : public Adafruit_GFX
{
  public:
    Adafruit_ILI9486_Teensy(void);
    
    void begin(void);
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void fillScreen(uint16_t color);
    void drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void setRotation(uint8_t r);
    void invertDisplay(boolean i);
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    

 private:
    void writecommand(uint8_t c);
    void writedata(uint8_t d);
    void writedata16(uint16_t d);
    void writedata16(uint16_t d, uint32_t num);
    void commandList(uint8_t *addr);
    
};

#endif
