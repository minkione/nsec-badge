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

  based on port to tm4c123gh6pm by Julian Fell.
  Ported to nrf52 by Eric Tremblay.
 ****************************************************/
#include <nrf.h>
#include <nrf_gpio.h>
#include <nrf_delay.h>
#include <app_util_platform.h>
#include <stdlib.h>
#include <string.h>
#include "ST7735.h"
#include "app/gfx_effect.h"
#include "boards.h"

// This increase the bss section by 25k !!!!
//#define USE_DOUBLE_BUFFERING

//*****************************************************************************
//
// Local Defines
//
//*****************************************************************************

#define DELAY 0x80

#define SET_LOW(x) nrf_gpio_pin_write(x, 0)
#define SET_HIGH(x) nrf_gpio_pin_write(x, 1)

#define swap(a, b) { int16_t t = a; a = b; b = t; }

typedef enum {
    COMMAND = 0,
    DATA = 1,
} t_screen_mode;

//*****************************************************************************
//
// Private Variables
//
//*****************************************************************************

int8_t colstart, rowstart, rotation, textsize;
int16_t wrap,cursor_y,cursor_x;
uint16_t textcolour,textbgcolour;
uint32_t width, height;
static bool is_init = false;

//*****************************************************************************
//
// SPI stuff
//
//*****************************************************************************
static nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0); // change for two with real board
static st7735_config_t st7735_config;

static void spi_init(void) {
    nrf_drv_spi_config_t spi_config;

    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
    spi_config.sck_pin = st7735_config.sck_pin;
    spi_config.miso_pin = st7735_config.miso_pin;
    spi_config.mosi_pin = st7735_config.mosi_pin;
    spi_config.ss_pin = st7735_config.cs_pin;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.irq_priority = APP_IRQ_PRIORITY_LOW;
    spi_config.orc = 0xFF;

    APP_ERROR_CHECK(nrf_drv_spi_init(&st7735_config.spi, &spi_config, NULL, NULL));
}

static void spi_master_tx(const uint8_t * p_tx_data, uint16_t len) {
    while(len > 0) {
        const uint8_t packet_len = MIN(len, UINT8_MAX);
        APP_ERROR_CHECK(nrf_drv_spi_transfer(&st7735_config.spi, p_tx_data, packet_len, NULL, 0));
        len -= packet_len;
        p_tx_data += packet_len;
    }
}

static void spi_write(uint8_t byte) {
    spi_master_tx(&byte, 1);
}

static void st7735_command(uint8_t c) {
    nrf_gpio_pin_write(st7735_config.dc_pin, COMMAND);
    spi_write(c);
}

static void st7735_data(uint8_t d) {
    nrf_gpio_pin_write(st7735_config.dc_pin, DATA);
    spi_write(d);
}


//*****************************************************************************
//
// Screen Initialisation
//
//*****************************************************************************

#ifdef USE_DOUBLE_BUFFERING
static uint8_t buffer[ST7735_HEIGHT * ST7735_WIDTH * 2] = {0};
#endif

static const uint8_t
  initCommands[] = {                 // Init for 7735R, (green tab)
    21,                             // 21 commands in list:
    ST7735_SWRESET,   DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST7735_SLPOUT ,   DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR , 1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1 , 3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2 , 1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3 , 2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4 , 2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5 , 2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0,              // 13: Don't invert display, no args
    ST7735_MADCTL , 1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST7735_COLMOD , 1,              // 15: set color mode, 1 arg, no delay:
      0x05,                       //     16-bit color                  //     100 ms delay
    ST7735_CASET  , 4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 79
    ST7735_RASET  , 4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F,                 //     XEND = 159
     ST7735_GMCTRP1, 16      ,       //  1: Gamma setting, I think?
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      ,       //  2: More gamma?
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,   DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST7735_DISPON ,   DELAY, //  4: Main screen turn on, no args w/delay
      100 };                        //     100 ms delay

// Companion code to the above table.  Reads and issues
// a series of LCD commands stored in byte array.
void command_list(const uint8_t *addr) 
{
    uint8_t  numCommands, numArgs;
    uint16_t ms;

    numCommands = *(addr++);   // Number of commands to follow

    while(numCommands--) {                // For each command...
        st7735_command(*(addr++)); //   Read, issue command
        numArgs  = *(addr++);    //   Number of args to follow
        ms       = numArgs & DELAY;          //   If hibit set, delay follows args
        numArgs &= ~DELAY;                   //   Mask out delay bit

        while(numArgs--) {                   //   For each argument...
            st7735_data(*(addr++));  //     Read, issue argument
        }

        if(ms) {
            ms = *(addr++); // Read post-command delay time (ms)
            if (ms == 255) {
                ms = 500;     // If 255, delay for 500 ms
            } 
            nrf_delay_ms(ms);
        }

    }
}

void st7735_init(void) 
{
    if (is_init) {
        return;
    }

    st7735_config.spi = spi;
    st7735_config.sck_pin = PIN_OLED_CLK;
    st7735_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
    st7735_config.mosi_pin = PIN_OLED_DATA;
    st7735_config.cs_pin = PIN_OLED_CS;
    st7735_config.dc_pin = PIN_OLED_DC_MODE;
    st7735_config.rst_pin = PIN_OLED_RESET;

    spi_init();

    /* Set st7735_config.dc_pin and RST Pins as outputs for manual toggling */
    nrf_gpio_cfg_output(st7735_config.dc_pin);
    nrf_gpio_cfg_output(st7735_config.rst_pin);

    /* Toggle reset */
    SET_HIGH(st7735_config.rst_pin);
    nrf_delay_ms(500);
    SET_LOW(st7735_config.rst_pin);
    nrf_delay_ms(500);
    SET_HIGH(st7735_config.rst_pin);
    nrf_delay_ms(500);

    /* Initialise default values */
    colstart = 0;
    rowstart = 24;
    width = ST7735_WIDTH;
    height = ST7735_HEIGHT;
    wrap = 1;
    cursor_y = 0;
    cursor_x = 0;
    textsize = 1;
    textcolour = 0xFFFF;
    textbgcolour = 0xFFFF;

    /* Initialise LCD screen */
    command_list(initCommands);

    st7735_set_rotation(3);
    st7735_fill_screen(ST7735_BLACK);

    is_init = true;
}

//*****************************************************************************
//
// Basic Drawing Functions
//
//*****************************************************************************

uint16_t swap_colour(uint16_t x) 
{ 
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

void st7735_update(void)
{
#ifdef USE_DOUBLE_BUFFERING
    st7735_command(ST7735_CASET); 
    st7735_data(0x00);
    st7735_data(colstart);   
    st7735_data(0x00);
    st7735_data(ST7735_WIDTH + colstart - 1);  

    st7735_command(ST7735_RASET); 
    st7735_data(0x00);
    st7735_data(rowstart);  
    st7735_data(0x00);
    st7735_data(ST7735_HEIGHT + rowstart - 1); 
    st7735_command(ST7735_RAMWR);

    SET_HIGH(st7735_config.dc_pin);
    spi_master_tx(buffer, ST7735_HEIGHT * ST7735_WIDTH * 2);
#endif
}

void st7735_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) 
{
  st7735_command(ST7735_CASET); 
  st7735_data(0x00);
  st7735_data(x0+colstart);   
  st7735_data(0x00);
  st7735_data(x1+colstart);  

  st7735_command(ST7735_RASET); 
  st7735_data(0x00);
  st7735_data(y0+rowstart);  
  st7735_data(0x00);
  st7735_data(y1+rowstart); 

  st7735_command(ST7735_RAMWR);
}

void st7735_push_colour(uint16_t colour) 
{
  SET_HIGH(st7735_config.dc_pin);
  spi_write(colour >> 8);
  spi_write(colour);
}

void st7735_draw_pixel(int16_t x, int16_t y, uint16_t colour) 
{
  if((x < 0) ||(x >= width) || (y < 0) || (y >= height)) return;

#ifdef USE_DOUBLE_BUFFERING
  uint32_t index = (x + y*ST7735_WIDTH) * 2;
  buffer[index] = colour >> 8;
  buffer[index + 1] = colour;
#else
  st7735_set_addr_window(x, y, x+1, y+1);
  st7735_push_colour(colour);
#endif
}


void st7735_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t colour) 
{
  // Rudimentary clipping
  if((x >= width) || (y >= height)) return;
  
  if((y+h-1) >= height) {
    h = height-y;
  }

  uint8_t hi = colour >> 8, lo = colour;

#ifdef USE_DOUBLE_BUFFERING
  while (h--) {
    uint32_t index = (x + (y+h)*ST7735_WIDTH) * 2;
    buffer[index] = hi;
    buffer[index + 1] = lo;
  }
#else
  st7735_set_addr_window(x, y, x, y+h-1);
  SET_HIGH(st7735_config.dc_pin);
  
  
  while (h--) {
    spi_write(hi);
    spi_write(lo);
  }
#endif

}


void st7735_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t colour) 
{
  // Rudimentary clipping
  if((x >= width) || (y >= height)) return;
  if((x+w-1) >= width)  {
    w = width-x;
  }
  uint8_t hi = colour >> 8, lo = colour;

#ifdef USE_DOUBLE_BUFFERING
  while (w--) {
    uint32_t index = ((x+w) + y*ST7735_WIDTH) * 2;
    buffer[index] = hi;
    buffer[index + 1] = lo;
  }
#else
  st7735_set_addr_window(x, y, x+w-1, y);
  SET_HIGH(st7735_config.dc_pin);

  while (w--) {
    spi_write(hi);
    spi_write(lo);
  }
#endif
}

void st7735_fill_screen_black(void)
{
#ifdef USE_DOUBLE_BUFFERING
    memset(buffer, 0x00, sizeof(buffer));
#else
    st7735_fill_screen(ST7735_BLACK);
#endif
}

void st7735_fill_screen_white(void)
{
#ifdef USE_DOUBLE_BUFFERING
    memset(buffer, 0xFF, sizeof(buffer));
#else
    st7735_fill_screen(ST7735_WHITE);
#endif
}

void st7735_fill_screen(uint16_t colour) 
{
  st7735_fill_rect(0, 0,  width, height, colour);
}

void st7735_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t colour) 
{
  if((x >= width) || (y >= height)) return;
  if((x + w - 1) >= width)  {
    w = width  - x;
  }
  if((y + h - 1) >= height) {
    h = height - y;
  }

#ifdef USE_DOUBLE_BUFFERING
   for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      st7735_draw_pixel(x, y, colour);
    }
  }
#else
  uint8_t hi = colour >> 8, lo = colour;
  st7735_set_addr_window(x, y, x+w-1, y+h-1);
  SET_HIGH(st7735_config.dc_pin);

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spi_write(hi);
      spi_write(lo);
    }
  }
#endif
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed colour
uint16_t st7735_colour_565(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void st7735_set_rotation(uint8_t m) 
{
  st7735_command(ST7735_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     st7735_data(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
     width  = ST7735_WIDTH;
     height = ST7735_HEIGHT;
     break;
   case 1:
     st7735_data(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     width  = ST7735_HEIGHT;
     height = ST7735_WIDTH;
     break;
  case 2:
     st7735_data(MADCTL_BGR);
     width  = ST7735_WIDTH;
     height = ST7735_HEIGHT;
    break;
   case 3:
     st7735_data(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
     width  = ST7735_HEIGHT;
     height = ST7735_WIDTH;
     break;
  }

  gfx_set_rotation(m);
}

void st7735_invert_display(uint8_t i) 
{
  st7735_command(i ? ST7735_INVON : ST7735_INVOFF);
}
