/***************************************************
  This is an Arduino Library for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include "stm32f0xx_hal.h"

#include "Adafruit_ILI9340.h"
#include "Adafruit_GFX.h"
#include "spi.h"
#include "gpio.h"

#define dcport    GPIO_OUT_DISPLAY_DC_PORT
#define dcpinmask GPIO_OUT_DISPLAY_DC_PIN

#define csport    GPIO_OUT_DISPLAY_CS_PORT
#define cspinmask GPIO_OUT_DISPLAY_CS_PIN

#define delay HAL_Delay

void spiwrite(uint8_t c)
{
    spi_send_char_blocking(c);
}

void writecommand(uint8_t c) {
    GPIO_RESET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spiwrite(c);
    GPIO_SET_PIN(csport, cspinmask);
}

void writedata(uint8_t c) {
    GPIO_SET_PIN(dcport,  dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spiwrite(c);
    GPIO_SET_PIN(csport, cspinmask);
}

void begin(void) {
    adafruit_GFX();

    writecommand(0xEF);
    writedata(0x03);
    writedata(0x80);
    writedata(0x02);

    writecommand(0xCF);
    writedata(0x00);
    writedata(0XC1);
    writedata(0X30);

    writecommand(0xED);
    writedata(0x64);
    writedata(0x03);
    writedata(0X12);
    writedata(0X81);

    writecommand(0xE8);
    writedata(0x85);
    writedata(0x00);
    writedata(0x78);

    writecommand(0xCB);
    writedata(0x39);
    writedata(0x2C);
    writedata(0x00);
    writedata(0x34);
    writedata(0x02);

    writecommand(0xF7);
    writedata(0x20);

    writecommand(0xEA);
    writedata(0x00);
    writedata(0x00);

    writecommand(ILI9340_PWCTR1);    //Power control
    writedata(0x23);   //VRH[5:0]

    writecommand(ILI9340_PWCTR2);    //Power control
    writedata(0x10);   //SAP[2:0];BT[3:0]

    writecommand(ILI9340_VMCTR1);    //VCM control
    writedata(0x3e); //�Աȶȵ���
    writedata(0x28);

    writecommand(ILI9340_VMCTR2);    //VCM control2
    writedata(0x86);  //--

    writecommand(ILI9340_MADCTL);    // Memory Access Control
    writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

    writecommand(ILI9340_PIXFMT);
    writedata(0x55);

    writecommand(ILI9340_FRMCTR1);
    writedata(0x00);
    writedata(0x18);

    writecommand(ILI9340_DFUNCTR);    // Display Function Control
    writedata(0x08);
    writedata(0x82);
    writedata(0x27);

    writecommand(0xF2);    // 3Gamma Function Disable
    writedata(0x00);

    writecommand(ILI9340_GAMMASET);    //Gamma curve selected
    writedata(0x01);

    writecommand(ILI9340_GMCTRP1);    //Set Gamma
    writedata(0x0F);
    writedata(0x31);
    writedata(0x2B);
    writedata(0x0C);
    writedata(0x0E);
    writedata(0x08);
    writedata(0x4E);
    writedata(0xF1);
    writedata(0x37);
    writedata(0x07);
    writedata(0x10);
    writedata(0x03);
    writedata(0x0E);
    writedata(0x09);
    writedata(0x00);

    writecommand(ILI9340_GMCTRN1);    //Set Gamma
    writedata(0x00);
    writedata(0x0E);
    writedata(0x14);
    writedata(0x03);
    writedata(0x11);
    writedata(0x07);
    writedata(0x31);
    writedata(0xC1);
    writedata(0x48);
    writedata(0x08);
    writedata(0x0F);
    writedata(0x0C);
    writedata(0x31);
    writedata(0x36);
    writedata(0x0F);

    writecommand(ILI9340_SLPOUT);    //Exit Sleep
    delay(120);
    writecommand(ILI9340_DISPON);    //Display on
}


void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                   uint16_t y1) {

    writecommand(ILI9340_CASET); // Column addr set
    writedata(x0 >> 8);
    writedata(x0 & 0xFF);     // XSTART
    writedata(x1 >> 8);
    writedata(x1 & 0xFF);     // XEND

    writecommand(ILI9340_PASET); // Row addr set
    writedata(y0 >> 8);
    writedata(y0);     // YSTART
    writedata(y1 >> 8);
    writedata(y1);     // YEND

    writecommand(ILI9340_RAMWR); // write to RAM
}


void pushColor(uint16_t color) {
    GPIO_SET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spiwrite(color >> 8);
    spiwrite(color);
    GPIO_SET_PIN(csport, cspinmask);
}

void drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
    setAddrWindow(x, y, x + 1, y + 1);
    GPIO_SET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spiwrite(color >> 8);
    spiwrite(color);
    GPIO_SET_PIN(csport, cspinmask);
}


void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;
    if ((y + h - 1) >= _height)
        h = _height - y;
    setAddrWindow(x, y, x, y + h - 1);
    uint8_t hi = color >> 8, lo = color;
    GPIO_SET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spi_send_buffer_start();
    while (h--) {
        spi_send_buffer_fill(hi);
        spi_send_buffer_fill(lo);
    }
    spi_send_buffer_end();
    GPIO_SET_PIN(csport, cspinmask);
}


void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{

    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width)  w = _width - x;
    setAddrWindow(x, y, x + w - 1, y);
    uint8_t hi = color >> 8, lo = color;
    GPIO_SET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spi_send_buffer_start();
    while (w--) {
        spi_send_buffer_fill(hi);
        spi_send_buffer_fill(lo);
    }
    spi_send_buffer_end();
    GPIO_SET_PIN(csport, cspinmask);
}

void fillScreen(uint16_t color)
{
    fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width)  w = _width  - x;
    if ((y + h - 1) >= _height) h = _height - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    uint8_t hi = color >> 8, lo = color;
    GPIO_SET_PIN(dcport, dcpinmask);
    GPIO_RESET_PIN(csport, cspinmask);
    spi_send_buffer_start();
    for (y = h; y > 0; y--) {
        for (x = w; x > 0; x--) {
            spi_send_buffer_fill(hi);
            spi_send_buffer_fill(lo);
        }
    }
    spi_send_buffer_end();
    GPIO_SET_PIN(csport, cspinmask);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


void setRotation(uint8_t m) {

    writecommand(ILI9340_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
    case 0:
        writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
        _width  = ILI9340_TFTWIDTH;
        _height = ILI9340_TFTHEIGHT;
        break;
    case 1:
        writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
        _width  = ILI9340_TFTHEIGHT;
        _height = ILI9340_TFTWIDTH;
        break;
    case 2:
        writedata(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
        _width  = ILI9340_TFTWIDTH;
        _height = ILI9340_TFTHEIGHT;
        break;
    case 3:
        writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
        _width  = ILI9340_TFTHEIGHT;
        _height = ILI9340_TFTWIDTH;
        break;
    }
}


void invertDisplay(uint8_t i) {
    writecommand(i ? ILI9340_INVON : ILI9340_INVOFF);
}

