
#ifndef __GPIO__
#define __GPIO__

#define GPIO_OUT_DISPLAY_DC_PIN     GPIO_PIN_0
#define GPIO_OUT_DISPLAY_DC_PORT    GPIOF

#define GPIO_OUT_DISPLAY_CS_PIN     GPIO_PIN_4
#define GPIO_OUT_DISPLAY_CS_PORT    GPIOA

#define GPIO_READ_PIN(port, pin)    ((port->IDR & pin) ? 1 : 0)
#define GPIO_SET_PIN(port, pin)     port->BSRR=pin
#define GPIO_RESET_PIN(port, pin)   port->BRR=pin

void gpio_init(void);

int gpio_apply_menu_count(uint8_t *pvalue, uint8_t min, uint8_t max);
int gpio_apply_preamp_count(uint8_t *pvalue);
int gpio_apply_powamp_count(uint8_t *pvalue);

void gpio_set_powamp_mute(uint8_t value);
void gpio_set_powamp_stdby(uint8_t value);

int gpio_get_button_start(void);
int gpio_get_button_stop(void);
int gpio_get_button_escape(void);
int gpio_get_button_preamp(void);
int gpio_get_button_powamp(void);
int gpio_get_button_menu(void);

void gpio_background_process(void);

#endif
