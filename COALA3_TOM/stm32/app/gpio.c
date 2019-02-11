/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include "stm32f0xx_hal.h"

#include "main.h"
#include "gpio.h"

#define CODEUR_MENU_PIN_A       GPIO_PIN_3
#define CODEUR_MENU_PIN_B       GPIO_PIN_4
#define CODEUR_MENU_PIN_S       GPIO_PIN_5
#define CODEUR_MENU_PORT        GPIOB

#define CODEUR_PREAMP_PIN_A     GPIO_PIN_0
#define CODEUR_PREAMP_PIN_B     GPIO_PIN_1
#define CODEUR_PREAMP_PIN_S     GPIO_PIN_3
#define CODEUR_PREAMP_PORT      GPIOA

#define CODEUR_POWAMP_PIN_A     GPIO_PIN_12
#define CODEUR_POWAMP_PIN_B     GPIO_PIN_13
#define CODEUR_POWAMP_PIN_S     GPIO_PIN_14
#define CODEUR_POWAMP_PORT      GPIOA

#define BUTTON_START_PIN        GPIO_PIN_1
#define BUTTON_START_PORT       GPIOB

#define BUTTON_STOP_PIN         GPIO_PIN_0
#define BUTTON_STOP_PORT        GPIOB

#define BUTTON_ESCAPE_PIN       GPIO_PIN_15
#define BUTTON_ESCAPE_PORT      GPIOA

#define GPIO_OUT_POWAMP_MUTE_PIN    GPIO_PIN_1
#define GPIO_OUT_POWAMP_MUTE_PORT   GPIOF

#define GPIO_OUT_POWAMP_STDBY_PIN   GPIO_PIN_2
#define GPIO_OUT_POWAMP_STDBY_PORT  GPIOA

typedef union {
    uint8_t val;
    struct {
        uint8_t bit_start: 1 ;
        uint8_t bit_stop: 1 ;
        uint8_t bit_escape: 1 ;
        uint8_t bit_menu: 1 ;
        uint8_t bit_preamp: 1 ;
        uint8_t bit_powamp: 1 ;
        uint8_t bit_unused1: 1 ;
        uint8_t bit_unused2: 1 ;
    };
} buttons_t;

int32_t _counter_menu = 0;
int32_t counter_preamp = 0;
int32_t counter_powamp = 0;
uint32_t date_menu = 0;
uint32_t date_preamp = 0;
uint32_t date_powamp = 0;

buttons_t buttons = { 0 };
buttons_t buttons_prev = { 0 };

uint8_t increments[10] = {10,10,5,5,2,2,2,1,1,1};

void gpio_set_powamp_mute(uint8_t value)
{
    if (value)
        GPIO_SET_PIN(GPIO_OUT_POWAMP_MUTE_PORT, GPIO_OUT_POWAMP_MUTE_PIN);
    else
        GPIO_RESET_PIN(GPIO_OUT_POWAMP_MUTE_PORT, GPIO_OUT_POWAMP_MUTE_PIN);
}

void gpio_set_powamp_stdby(uint8_t value)
{
    if (value)
        GPIO_SET_PIN(GPIO_OUT_POWAMP_STDBY_PORT, GPIO_OUT_POWAMP_STDBY_PIN);
    else
        GPIO_RESET_PIN(GPIO_OUT_POWAMP_STDBY_PORT, GPIO_OUT_POWAMP_STDBY_PIN);
}

void gpio_set_display_dc(uint8_t value)
{
    if (value)
        GPIO_SET_PIN(GPIO_OUT_DISPLAY_DC_PORT, GPIO_OUT_DISPLAY_DC_PIN);
    else
        GPIO_RESET_PIN(GPIO_OUT_DISPLAY_DC_PORT, GPIO_OUT_DISPLAY_DC_PIN);
}

void apply_count(uint8_t *pvalue, int32_t count, uint8_t min, uint8_t max)
{
    int32_t val = (int32_t)*pvalue + count;
    if (val > max)
        val = max;
    else if (val < min)
        val = min;
    *pvalue = val;
}

int gpio_apply_menu_count(uint8_t *pvalue, uint8_t min, uint8_t max)
{
    if (_counter_menu)
    {
        int tmp = *pvalue;
        apply_count(pvalue, _counter_menu, min, max);
        if ( tmp + _counter_menu > max )
           *pvalue = 0;
        else if ( tmp + _counter_menu < min )
           *pvalue = max;
        _counter_menu = 0;
        return 1;
    }
    return 0;
}

int gpio_apply_preamp_count(uint8_t *pvalue)
{
    if (counter_preamp) {
        apply_count(pvalue, counter_preamp, 0, 255);
        counter_preamp = 0;
        return 1;
    }
    return 0;
}

int gpio_apply_powamp_count(uint8_t *pvalue)
{
    if (counter_powamp) {
        apply_count(pvalue, counter_powamp, 0, 255);
        counter_powamp = 0;
        return 1;
    }
    return 0;
}

int gpio_get_button_start(void)
{
    uint8_t ret = buttons.bit_start;
    buttons.bit_start = 0;
    return ret;
}

int gpio_get_button_stop(void)
{
    uint8_t ret = buttons.bit_stop;
    buttons.bit_stop = 0;
    return ret;
}

int gpio_get_button_escape(void)
{
    uint8_t ret = buttons.bit_escape;
    buttons.bit_escape = 0;
    return ret;
}

int gpio_get_button_preamp(void)
{
    uint8_t ret = buttons.bit_preamp;
    buttons.bit_preamp = 0;
    return ret;
}

int gpio_get_button_powamp(void)
{
    uint8_t ret = buttons.bit_powamp;
    buttons.bit_powamp = 0;
    return ret;
}

int gpio_get_button_menu(void)
{
    uint8_t ret = buttons.bit_menu;
    buttons.bit_menu = 0;
    return ret;
}

static void read_buttons(buttons_t *but)
{
    but->bit_start = GPIO_READ_PIN(BUTTON_START_PORT, BUTTON_START_PIN);
    but->bit_stop = GPIO_READ_PIN(BUTTON_STOP_PORT, BUTTON_STOP_PIN);
    but->bit_escape = GPIO_READ_PIN(BUTTON_ESCAPE_PORT, BUTTON_ESCAPE_PIN);
    but->bit_menu = GPIO_READ_PIN(CODEUR_MENU_PORT, CODEUR_MENU_PIN_S);
    but->bit_preamp = GPIO_READ_PIN(CODEUR_PREAMP_PORT, CODEUR_PREAMP_PIN_S);
    but->bit_powamp = GPIO_READ_PIN(CODEUR_POWAMP_PORT, CODEUR_POWAMP_PIN_S);
}

void gpio_background_process(void)
{
    buttons_t buttons_current, buttons_changed;
    read_buttons(&buttons_current);
    buttons_changed.val = buttons_current.val ^ buttons_prev.val;
    buttons.val = buttons_changed.val & (~buttons_current.val);
    buttons_prev.val = buttons_current.val;
}

void gpio_init(void)
{
    /* PA2 , PA4, PF0 et PF1 en sortie push-pull */
    GPIO_InitTypeDef  GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* Bits du port A en entrée simple*/
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 ;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Bits du port B en entrée simple*/
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 ;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Bits du port A en entrée irq */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Bits du port B en entrée irq */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Enable and set EXTI line 4_15 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

    /* Enable and set EXTI line 2_3 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

    /* Enable and set EXTI line 0_1 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

/* Fonction de calcul de la valeur absolue d'incrémentation en fonction de 
   l'écart temporel entre deux interruptions.
   Si l'écart est trop court on considère que c'est un rebond et on ne fait
   rien.
*/
void process_encoder(int32_t *pcodeur, uint32_t *pdate, int8_t sens)
{
    uint32_t now = HAL_GetTick();
    uint32_t ecart_date_ms =  now - *pdate;
    uint32_t incr = 0;
    *pdate = now;
    if(ecart_date_ms < 5)
        return;
    if(ecart_date_ms <= 109)
        incr = increments[ecart_date_ms/10 - 1];
    else
        incr = 1;
    *pcodeur = *pcodeur + sens * incr;
}

/* Handler d'interruption appelé dés que l'on bouge un des trois codeurs */
static int _gpio_semaphor = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ( _gpio_semaphor )
      return;
    _gpio_semaphor = 1;
    int8_t sens = 0;
    if (GPIO_Pin == CODEUR_MENU_PIN_A) {
        if (GPIO_READ_PIN(CODEUR_MENU_PORT, CODEUR_MENU_PIN_B))
            sens = 1;
        else
            sens = -1;
        process_encoder(&_counter_menu, &date_menu, sens);
    }
    else if (GPIO_Pin == CODEUR_PREAMP_PIN_A) {
        if (GPIO_READ_PIN(CODEUR_PREAMP_PORT, CODEUR_PREAMP_PIN_B))
            sens = -1;
        else
            sens = 1;
        process_encoder(&counter_preamp, &date_preamp, sens);
    }
    else if (GPIO_Pin == CODEUR_POWAMP_PIN_A) {
        if (GPIO_READ_PIN(CODEUR_POWAMP_PORT, CODEUR_POWAMP_PIN_B))
            sens = -1;
        else
            sens = 1;
        process_encoder(&counter_powamp, &date_powamp, sens);
    }
    _gpio_semaphor = 0;
}

