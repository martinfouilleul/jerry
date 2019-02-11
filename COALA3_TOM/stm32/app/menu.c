/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include <stdint.h>

#include "main.h"
#include "Adafruit_ILI9340.h"
#include "Adafruit_GFX.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"

#define MENU_TOP_NBR_ITEMS (sizeof(topmenu) / sizeof(char *))

#define STRING_WIDTH_PX(nbr_char, size) (6 * size * nbr_char)

char *topmenu[] = {
    "Preamp  > Amp in A",
    "Preamp  > ADC in",
    "Preamp  > RIM out",
    "RIM in  > ADC in",
    "RIM in  > Amp in B",
    "DAC out > Amp in A"
};

static uint16_t bgcolor = 0x8410;
static uint8_t _menu = 0;
static int16_t gain_preamp = -1;
static int16_t gain_powamp = -1;
static uint8_t _val_switchs = 0;

void menu_init(void)
{
    spi_init();
    begin();
    setRotation(3);
    fillScreen(0x8410);
    setTextSize(2);
    setTextColor(ILI9340_BLACK, bgcolor);
    setCursor(60, 10);
    write_str("IRCAM - COALA v2\n");
    setCursor(56, 40);
    write_str("Switchs settings\n");
    uint8_t idx;
    uint16_t color;
    for (idx = 0 ; idx < MENU_TOP_NBR_ITEMS ; idx++) {
        setCursor(10, 70 + 20 * idx);
        write_str(topmenu[idx]);
        if (idx == _menu)
            color = ILI9340_RED;
        else
            color = bgcolor;
        fillRect(0, 70 + 20 * idx, 5, 20, color);
    }
    setCursor(10, 210);
    write_str("Preamp");
    setCursor(175, 210);
    write_str("Amp");
    updateSwitchesDisplay();
    updateRunning( 0 ); 
}

void updateSwitchesDisplay()
{
    _val_switchs = i2c_get_analog_switchs();
    uint8_t idx;
    for (idx = 0 ; idx < MENU_TOP_NBR_ITEMS ; idx++)
    {
        setCursor(280, 70 + 20 * idx);
        if(_val_switchs & (1 << idx))
        {
            setTextColor(ILI9340_GREEN, bgcolor);
            write_str("on ");
        }
        else
        {
            setTextColor(ILI9340_VERYDARKRED, bgcolor);
            write_str("off");
        }
    }
}

/* Affiche le curseur rouge sur la ligne correspondant à la valeur de la variable menu */
void update_menu_select(void)
{
    uint8_t idx;
    uint16_t color;
    printls_int(_menu);
    printls("\r\n");
    for (idx = 0 ; idx < MENU_TOP_NBR_ITEMS ; idx++) {
        if (idx == _menu)
            color = ILI9340_RED;
        else
            color = bgcolor;
        fillRect(0, 70 + 20 * idx, 5, 20, color);
    }
}

/* Inverse la valeur d'un switch, au niveau de l'affichage et dans _val_switchs */
void toggle_switch(uint8_t number)
{
    fillRect(280, 70 + 20 * number, 40, 20, bgcolor);
    setCursor(280, 70 + 20 * number);
    uint8_t status = _val_switchs & (1 << number);
    if(!status){
        _val_switchs |= (1 << number);
        setTextColor(ILI9340_GREEN, bgcolor);
        write_str("on");
    }
    else{
        _val_switchs &= ~(1 << number);
        setTextColor(ILI9340_VERYDARKRED, bgcolor);
        write_str("off");
    }
}

/* Met à jour l'affichage du gain préampli */
void update_gain_preamp(uint8_t gain)
{
    char str[5];
    itoa(gain, str);
    fillRect(95, 204, 70, 22, bgcolor);
    setCursor(95, 204);
    setTextSize(3);
    setTextColor(ILI9340_DARKBLUE, bgcolor);
    write_str(str);
    setTextSize(2);
    int lmax = 136;
    unsigned short l = (int)((float)lmax*((float)gain/255));
    unsigned short n = 0;
    unsigned short startcolor = ILI9340_DARKBLUE;
    unsigned short endcolor = ILI9340_BLUE;
    for ( n = 0; n < l; ++n )
    {
       unsigned short color = startcolor + (unsigned short)((endcolor - startcolor) * ((float) n/lmax));
       drawPixel( 10+n, 232, color );
       drawPixel( 10+n, 233, color );
       drawPixel( 10+n, 234, color );
       drawPixel( 10+n, 235, color );
    }
    fillRect(10+l, 232, lmax-l, 4, bgcolor);
}

/* Met à jour l'affichage du gain ampli */
void update_gain_powamp(uint8_t gain)
{
    char str[5];
    itoa(gain, str);
    fillRect(220, 204, 70, 22, bgcolor);
    setCursor(220, 204);
    setTextSize(3);
    setTextColor(ILI9340_DARKMAGENTA, bgcolor);
    write_str(str);
    setTextSize(2);
    unsigned short lmax = 136;
    unsigned short l = (unsigned short)((float)lmax*((float)gain/255));
    unsigned short n = 0;
    unsigned short startcolor = ILI9340_DARKMAGENTA;
    unsigned short endcolor = ILI9340_MAGENTA;
    for ( n = 0; n < l; ++n )
    {
       unsigned short color = startcolor + ((unsigned short)( ((unsigned short)(endcolor-startcolor) >> 11) * ( (float) n/lmax)) << 11);
       drawPixel( 175+n, 232, color );
       drawPixel( 175+n, 233, color );
       drawPixel( 175+n, 234, color );
       drawPixel( 175+n, 235, color );
    }
    fillRect(175+l, 232, lmax-l, 4, bgcolor);
}

void updateRunning( int status )
{
    uint16_t color = ILI9340_VERYDARKRED;
    if ( status )
      color = ILI9340_GREEN;
    fillRect(5, 5, 20, 20, color);
}

/* Fonction à appeler périodiquement en tache de fond */
void menu_background_process(void)
{
    uint8_t gain;
    if (gpio_apply_menu_count(&_menu, 0, MENU_TOP_NBR_ITEMS - 1))
        update_menu_select();
    gain = i2c_get_preamp_gain();
    if (gain != gain_preamp) {
        update_gain_preamp(gain);
        gain_preamp = gain;
    }
    gain = i2c_get_powamp_gain();
    if (gain != gain_powamp) {
        update_gain_powamp(gain);
        gain_powamp = gain;
    }
    if (gpio_get_button_menu()) {
        toggle_switch(_menu);
        i2c_set_analog_switch(_val_switchs);
    }
}
