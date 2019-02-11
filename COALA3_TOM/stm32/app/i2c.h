
#ifndef __I2C__
#define __I2C__

/* Definition des switchs, on peut commuter plusieurs switch en même temps
   en faisant un ou loqique entre ces flags */
#define I2C_SWITCH_PREAMP_TO_POWAMP_A  0x01
#define I2C_SWITCH_PREAMP_TO_ADC_IN    0x02
#define I2C_SWITCH_PREAMP_TO_RIM_OUT   0x04
#define I2C_SWITCH_RIM_IN_TO_ADC_IN    0x08
#define I2C_SWITCH_RIM_IN_TO_POWAMP_B  0x10
#define I2C_SWITCH_DAC_OUT_TO_POWAMP_A 0x20


void i2c_set_preamp_gain(uint8_t gain);
void i2c_set_powamp_gain(uint8_t gain);
void i2c_set_analog_switch(uint8_t flags);

uint8_t i2c_get_preamp_gain(void);
uint8_t i2c_get_powamp_gain(void);
uint8_t i2c_get_analog_switchs(void);

void i2c_background_process(void);
void i2c_init(void);

#endif
