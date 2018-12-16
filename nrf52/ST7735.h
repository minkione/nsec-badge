/***************************************************
  This is a library for the Adafruit 1.8" SPI display.
This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  Ported to tm4c123gh6pm by Julian Fell.
 ****************************************************/

#ifndef _ADAFRUIT_ST7735H_
#define _ADAFRUIT_ST7735H_

#include <nrf_drv_spi.h>

//*****************************************************************************
//
// Screen commands
//
//*****************************************************************************

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

//*****************************************************************************
//
// Colour Definitions
//
//*****************************************************************************
#define	ST7735_BLACK   0x0000
#define	ST7735_RED     0x001F
#define	ST7735_BLUE    0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF

//*****************************************************************************
//
// structure Definitions
//
//*****************************************************************************

typedef struct
{
    nrf_drv_spi_t   spi; // SPI instance to use
    uint8_t         sck_pin;
    uint8_t         miso_pin;
    uint8_t         mosi_pin;
    uint8_t         cs_pin;
    uint8_t         dc_pin;
    uint8_t         rst_pin;
} st7735_config_t;

//*****************************************************************************
//
// Function Prototypes
//
//*****************************************************************************

void st7735_init(st7735_config_t init_st7735_config);
void st7735_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void st7735_push_color(uint16_t color);
void st7735_draw_pixel(int16_t x, int16_t y, uint16_t color);
void st7735_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color);
void st7735_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color);
void st7735_fill_screen(uint16_t color);
void st7735_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
uint16_t st7735_color_565(uint8_t r, uint8_t g, uint8_t b);
void st7735_set_rotation(uint8_t m);
void invertDisplay(int8_t i);
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t colour);
void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t colour);
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t colour);
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, 
      int16_t delta, uint16_t colour);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour);
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t colour);
void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
        int16_t r, uint16_t colour);
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
        int16_t x2, int16_t y2, uint16_t colour);
void fillTriangle ( int16_t x0, int16_t y0, int16_t x1, int16_t y1,
          int16_t x2, int16_t y2, uint16_t colour);
void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h,
            uint16_t colour, uint16_t bg);
void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
            int16_t w, int16_t h, uint16_t colour);
void drawString(char *c, int16_t x, int16_t y, uint16_t colour, uint16_t bg, uint8_t size);
void drawChar(int16_t x, int16_t y, unsigned char c,
          uint16_t colour, uint16_t bg, uint8_t size);
void setCursor(int16_t x, int16_t y);
void setTextSize(uint8_t s);
void setTextColour(uint16_t c, uint16_t b);
void setTextWrap(uint16_t w);


static const unsigned char font[] = {
        0x00, 0x00, 0x00, 0x00, 0x00,   
  0x3E, 0x5B, 0x4F, 0x5B, 0x3E,   
  0x3E, 0x6B, 0x4F, 0x6B, 0x3E,   
  0x1C, 0x3E, 0x7C, 0x3E, 0x1C, 
  0x18, 0x3C, 0x7E, 0x3C, 0x18, 
  0x1C, 0x57, 0x7D, 0x57, 0x1C, 
  0x1C, 0x5E, 0x7F, 0x5E, 0x1C, 
  0x00, 0x18, 0x3C, 0x18, 0x00, 
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF, 
  0x00, 0x18, 0x24, 0x18, 0x00, 
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF, 
  0x30, 0x48, 0x3A, 0x06, 0x0E, 
  0x26, 0x29, 0x79, 0x29, 0x26, 
  0x40, 0x7F, 0x05, 0x05, 0x07, 
  0x40, 0x7F, 0x05, 0x25, 0x3F, 
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A, 
  0x7F, 0x3E, 0x1C, 0x1C, 0x08, 
  0x08, 0x1C, 0x1C, 0x3E, 0x7F, 
  0x14, 0x22, 0x7F, 0x22, 0x14, 
  0x5F, 0x5F, 0x00, 0x5F, 0x5F, 
  0x06, 0x09, 0x7F, 0x01, 0x7F, 
  0x00, 0x66, 0x89, 0x95, 0x6A, 
  0x60, 0x60, 0x60, 0x60, 0x60, 
  0x94, 0xA2, 0xFF, 0xA2, 0x94, 
  0x08, 0x04, 0x7E, 0x04, 0x08, 
  0x10, 0x20, 0x7E, 0x20, 0x10, 
  0x08, 0x08, 0x2A, 0x1C, 0x08, 
  0x08, 0x1C, 0x2A, 0x08, 0x08, 
  0x1E, 0x10, 0x10, 0x10, 0x10, 
  0x0C, 0x1E, 0x0C, 0x1E, 0x0C, 
  0x30, 0x38, 0x3E, 0x38, 0x30, 
  0x06, 0x0E, 0x3E, 0x0E, 0x06, 
  0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x5F, 0x00, 0x00, 
  0x00, 0x07, 0x00, 0x07, 0x00, 
  0x14, 0x7F, 0x14, 0x7F, 0x14, 
  0x24, 0x2A, 0x7F, 0x2A, 0x12, 
  0x23, 0x13, 0x08, 0x64, 0x62, 
  0x36, 0x49, 0x56, 0x20, 0x50, 
  0x00, 0x08, 0x07, 0x03, 0x00, 
  0x00, 0x1C, 0x22, 0x41, 0x00, 
  0x00, 0x41, 0x22, 0x1C, 0x00, 
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 
  0x08, 0x08, 0x3E, 0x08, 0x08, 
  0x00, 0x80, 0x70, 0x30, 0x00, 
  0x08, 0x08, 0x08, 0x08, 0x08, 
  0x00, 0x00, 0x60, 0x60, 0x00, 
  0x20, 0x10, 0x08, 0x04, 0x02, 
  0x3E, 0x51, 0x49, 0x45, 0x3E, 
  0x00, 0x42, 0x7F, 0x40, 0x00, 
  0x72, 0x49, 0x49, 0x49, 0x46, 
  0x21, 0x41, 0x49, 0x4D, 0x33, 
  0x18, 0x14, 0x12, 0x7F, 0x10, 
  0x27, 0x45, 0x45, 0x45, 0x39, 
  0x3C, 0x4A, 0x49, 0x49, 0x31, 
  0x41, 0x21, 0x11, 0x09, 0x07, 
  0x36, 0x49, 0x49, 0x49, 0x36, 
  0x46, 0x49, 0x49, 0x29, 0x1E, 
  0x00, 0x00, 0x14, 0x00, 0x00, 
  0x00, 0x40, 0x34, 0x00, 0x00, 
  0x00, 0x08, 0x14, 0x22, 0x41, 
  0x14, 0x14, 0x14, 0x14, 0x14, 
  0x00, 0x41, 0x22, 0x14, 0x08, 
  0x02, 0x01, 0x59, 0x09, 0x06, 
  0x3E, 0x41, 0x5D, 0x59, 0x4E, 
  0x7C, 0x12, 0x11, 0x12, 0x7C, 
  0x7F, 0x49, 0x49, 0x49, 0x36, 
  0x3E, 0x41, 0x41, 0x41, 0x22, 
  0x7F, 0x41, 0x41, 0x41, 0x3E, 
  0x7F, 0x49, 0x49, 0x49, 0x41, 
  0x7F, 0x09, 0x09, 0x09, 0x01, 
  0x3E, 0x41, 0x41, 0x51, 0x73, 
  0x7F, 0x08, 0x08, 0x08, 0x7F, 
  0x00, 0x41, 0x7F, 0x41, 0x00, 
  0x20, 0x40, 0x41, 0x3F, 0x01, 
  0x7F, 0x08, 0x14, 0x22, 0x41, 
  0x7F, 0x40, 0x40, 0x40, 0x40, 
  0x7F, 0x02, 0x1C, 0x02, 0x7F, 
  0x7F, 0x04, 0x08, 0x10, 0x7F, 
  0x3E, 0x41, 0x41, 0x41, 0x3E, 
  0x7F, 0x09, 0x09, 0x09, 0x06, 
  0x3E, 0x41, 0x51, 0x21, 0x5E, 
  0x7F, 0x09, 0x19, 0x29, 0x46, 
  0x26, 0x49, 0x49, 0x49, 0x32, 
  0x03, 0x01, 0x7F, 0x01, 0x03, 
  0x3F, 0x40, 0x40, 0x40, 0x3F, 
  0x1F, 0x20, 0x40, 0x20, 0x1F, 
  0x3F, 0x40, 0x38, 0x40, 0x3F, 
  0x63, 0x14, 0x08, 0x14, 0x63, 
  0x03, 0x04, 0x78, 0x04, 0x03, 
  0x61, 0x59, 0x49, 0x4D, 0x43, 
  0x00, 0x7F, 0x41, 0x41, 0x41, 
  0x02, 0x04, 0x08, 0x10, 0x20, 
  0x00, 0x41, 0x41, 0x41, 0x7F, 
  0x04, 0x02, 0x01, 0x02, 0x04, 
  0x40, 0x40, 0x40, 0x40, 0x40, 
  0x00, 0x03, 0x07, 0x08, 0x00, 
  0x20, 0x54, 0x54, 0x78, 0x40, 
  0x7F, 0x28, 0x44, 0x44, 0x38, 
  0x38, 0x44, 0x44, 0x44, 0x28, 
  0x38, 0x44, 0x44, 0x28, 0x7F, 
  0x38, 0x54, 0x54, 0x54, 0x18, 
  0x00, 0x08, 0x7E, 0x09, 0x02, 
  0x18, 0xA4, 0xA4, 0x9C, 0x78, 
  0x7F, 0x08, 0x04, 0x04, 0x78, 
  0x00, 0x44, 0x7D, 0x40, 0x00, 
  0x20, 0x40, 0x40, 0x3D, 0x00, 
  0x7F, 0x10, 0x28, 0x44, 0x00, 
  0x00, 0x41, 0x7F, 0x40, 0x00, 
  0x7C, 0x04, 0x78, 0x04, 0x78, 
  0x7C, 0x08, 0x04, 0x04, 0x78, 
  0x38, 0x44, 0x44, 0x44, 0x38, 
  0xFC, 0x18, 0x24, 0x24, 0x18, 
  0x18, 0x24, 0x24, 0x18, 0xFC, 
  0x7C, 0x08, 0x04, 0x04, 0x08, 
  0x48, 0x54, 0x54, 0x54, 0x24, 
  0x04, 0x04, 0x3F, 0x44, 0x24, 
  0x3C, 0x40, 0x40, 0x20, 0x7C, 
  0x1C, 0x20, 0x40, 0x20, 0x1C, 
  0x3C, 0x40, 0x30, 0x40, 0x3C, 
  0x44, 0x28, 0x10, 0x28, 0x44, 
  0x4C, 0x90, 0x90, 0x90, 0x7C, 
  0x44, 0x64, 0x54, 0x4C, 0x44, 
  0x00, 0x08, 0x36, 0x41, 0x00, 
  0x00, 0x00, 0x77, 0x00, 0x00, 
  0x00, 0x41, 0x36, 0x08, 0x00, 
  0x02, 0x01, 0x02, 0x04, 0x02, 
  0x3C, 0x26, 0x23, 0x26, 0x3C, 
  0x1E, 0xA1, 0xA1, 0x61, 0x12, 
  0x3A, 0x40, 0x40, 0x20, 0x7A, 
  0x38, 0x54, 0x54, 0x55, 0x59, 
  0x21, 0x55, 0x55, 0x79, 0x41, 
  0x22, 0x54, 0x54, 0x78, 0x42, 
  0x21, 0x55, 0x54, 0x78, 0x40, 
  0x20, 0x54, 0x55, 0x79, 0x40, 
  0x0C, 0x1E, 0x52, 0x72, 0x12, 
  0x39, 0x55, 0x55, 0x55, 0x59, 
  0x39, 0x54, 0x54, 0x54, 0x59, 
  0x39, 0x55, 0x54, 0x54, 0x58, 
  0x00, 0x00, 0x45, 0x7C, 0x41, 
  0x00, 0x02, 0x45, 0x7D, 0x42, 
  0x00, 0x01, 0x45, 0x7C, 0x40, 
  0x7D, 0x12, 0x11, 0x12, 0x7D, 
  0xF0, 0x28, 0x25, 0x28, 0xF0, 
  0x7C, 0x54, 0x55, 0x45, 0x00, 
  0x20, 0x54, 0x54, 0x7C, 0x54, 
  0x7C, 0x0A, 0x09, 0x7F, 0x49, 
  0x32, 0x49, 0x49, 0x49, 0x32, 
  0x3A, 0x44, 0x44, 0x44, 0x3A, 
  0x32, 0x4A, 0x48, 0x48, 0x30, 
  0x3A, 0x41, 0x41, 0x21, 0x7A, 
  0x3A, 0x42, 0x40, 0x20, 0x78, 
  0x00, 0x9D, 0xA0, 0xA0, 0x7D, 
  0x3D, 0x42, 0x42, 0x42, 0x3D, 
  0x3D, 0x40, 0x40, 0x40, 0x3D, 
  0x3C, 0x24, 0xFF, 0x24, 0x24, 
  0x48, 0x7E, 0x49, 0x43, 0x66, 
  0x2B, 0x2F, 0xFC, 0x2F, 0x2B, 
  0xFF, 0x09, 0x29, 0xF6, 0x20, 
  0xC0, 0x88, 0x7E, 0x09, 0x03, 
  0x20, 0x54, 0x54, 0x79, 0x41, 
  0x00, 0x00, 0x44, 0x7D, 0x41, 
  0x30, 0x48, 0x48, 0x4A, 0x32, 
  0x38, 0x40, 0x40, 0x22, 0x7A, 
  0x00, 0x7A, 0x0A, 0x0A, 0x72, 
  0x7D, 0x0D, 0x19, 0x31, 0x7D, 
  0x26, 0x29, 0x29, 0x2F, 0x28, 
  0x26, 0x29, 0x29, 0x29, 0x26, 
  0x30, 0x48, 0x4D, 0x40, 0x20, 
  0x38, 0x08, 0x08, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x08, 0x38, 
  0x2F, 0x10, 0xC8, 0xAC, 0xBA, 
  0x2F, 0x10, 0x28, 0x34, 0xFA, 
  0x00, 0x00, 0x7B, 0x00, 0x00, 
  0x08, 0x14, 0x2A, 0x14, 0x22, 
  0x22, 0x14, 0x2A, 0x14, 0x08, 
  0xAA, 0x00, 0x55, 0x00, 0xAA, 
  0xAA, 0x55, 0xAA, 0x55, 0xAA, 
  0x00, 0x00, 0x00, 0xFF, 0x00, 
  0x10, 0x10, 0x10, 0xFF, 0x00, 
  0x14, 0x14, 0x14, 0xFF, 0x00, 
  0x10, 0x10, 0xFF, 0x00, 0xFF, 
  0x10, 0x10, 0xF0, 0x10, 0xF0, 
  0x14, 0x14, 0x14, 0xFC, 0x00, 
  0x14, 0x14, 0xF7, 0x00, 0xFF, 
  0x00, 0x00, 0xFF, 0x00, 0xFF, 
  0x14, 0x14, 0xF4, 0x04, 0xFC, 
  0x14, 0x14, 0x17, 0x10, 0x1F, 
  0x10, 0x10, 0x1F, 0x10, 0x1F, 
  0x14, 0x14, 0x14, 0x1F, 0x00, 
  0x10, 0x10, 0x10, 0xF0, 0x00, 
  0x00, 0x00, 0x00, 0x1F, 0x10, 
  0x10, 0x10, 0x10, 0x1F, 0x10, 
  0x10, 0x10, 0x10, 0xF0, 0x10, 
  0x00, 0x00, 0x00, 0xFF, 0x10, 
  0x10, 0x10, 0x10, 0x10, 0x10, 
  0x10, 0x10, 0x10, 0xFF, 0x10, 
  0x00, 0x00, 0x00, 0xFF, 0x14, 
  0x00, 0x00, 0xFF, 0x00, 0xFF, 
  0x00, 0x00, 0x1F, 0x10, 0x17, 
  0x00, 0x00, 0xFC, 0x04, 0xF4, 
  0x14, 0x14, 0x17, 0x10, 0x17, 
  0x14, 0x14, 0xF4, 0x04, 0xF4, 
  0x00, 0x00, 0xFF, 0x00, 0xF7, 
  0x14, 0x14, 0x14, 0x14, 0x14, 
  0x14, 0x14, 0xF7, 0x00, 0xF7, 
  0x14, 0x14, 0x14, 0x17, 0x14, 
  0x10, 0x10, 0x1F, 0x10, 0x1F, 
  0x14, 0x14, 0x14, 0xF4, 0x14, 
  0x10, 0x10, 0xF0, 0x10, 0xF0, 
  0x00, 0x00, 0x1F, 0x10, 0x1F, 
  0x00, 0x00, 0x00, 0x1F, 0x14, 
  0x00, 0x00, 0x00, 0xFC, 0x14, 
  0x00, 0x00, 0xF0, 0x10, 0xF0, 
  0x10, 0x10, 0xFF, 0x10, 0xFF, 
  0x14, 0x14, 0x14, 0xFF, 0x14, 
  0x10, 0x10, 0x10, 0x1F, 0x00, 
  0x00, 0x00, 0x00, 0xF0, 0x10, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 
  0xFF, 0xFF, 0xFF, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 
  0x38, 0x44, 0x44, 0x38, 0x44, 
  0xFC, 0x4A, 0x4A, 0x4A, 0x34,
  0x7E, 0x02, 0x02, 0x06, 0x06, 
  0x02, 0x7E, 0x02, 0x7E, 0x02, 
  0x63, 0x55, 0x49, 0x41, 0x63, 
  0x38, 0x44, 0x44, 0x3C, 0x04, 
  0x40, 0x7E, 0x20, 0x1E, 0x20, 
  0x06, 0x02, 0x7E, 0x02, 0x02, 
  0x99, 0xA5, 0xE7, 0xA5, 0x99, 
  0x1C, 0x2A, 0x49, 0x2A, 0x1C, 
  0x4C, 0x72, 0x01, 0x72, 0x4C, 
  0x30, 0x4A, 0x4D, 0x4D, 0x30, 
  0x30, 0x48, 0x78, 0x48, 0x30, 
  0xBC, 0x62, 0x5A, 0x46, 0x3D, 
  0x3E, 0x49, 0x49, 0x49, 0x00, 
  0x7E, 0x01, 0x01, 0x01, 0x7E, 
  0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 
  0x44, 0x44, 0x5F, 0x44, 0x44, 
  0x40, 0x51, 0x4A, 0x44, 0x40, 
  0x40, 0x44, 0x4A, 0x51, 0x40, 
  0x00, 0x00, 0xFF, 0x01, 0x03, 
  0xE0, 0x80, 0xFF, 0x00, 0x00, 
  0x08, 0x08, 0x6B, 0x6B, 0x08,
  0x36, 0x12, 0x36, 0x24, 0x36,
  0x06, 0x0F, 0x09, 0x0F, 0x06,
  0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x00, 0x10, 0x10, 0x00,
  0x30, 0x40, 0xFF, 0x01, 0x01,
  0x00, 0x1F, 0x01, 0x01, 0x1E,
  0x00, 0x19, 0x1D, 0x17, 0x12,
  0x00, 0x3C, 0x3C, 0x3C, 0x3C,
  0x00, 0x00, 0x00, 0x00, 0x00
};

#endif
