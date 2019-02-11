/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include "stm32f0xx_hal.h"

#include "main.h"
#include "gpio.h"

#define I2C_TIMEOUT_FOR_STORE   5000

#define I2C_ADDRESS_POTNUM_PREAMP 0x18
#define I2C_ADDRESS_POTNUM_POWAMP 0x4C
#define I2C_ADDRESS_ANALOG_SWITCH 0x4B

#define I2C_AD5259_CDE_RDAC     0
#define I2C_AD5259_CDE_EEPROM   1
#define I2C_AD5259_CDE_WP       2
#define I2C_AD5259_CDE_NOP      4
#define I2C_AD5259_CDE_RESTORE  5
#define I2C_AD5259_CDE_STORE    6

I2C_HandleTypeDef hi2c1;

/* Buffer pour la trame i2c */
#define TXBUFFERSIZE 2
static uint8_t aTxBuffer[TXBUFFERSIZE];

#define RXBUFFERSIZE 1
static uint8_t aRxBuffer[RXBUFFERSIZE];

/* Gestion de la mise à jour des devices */
typedef enum { I2C_POTNUM_PREAMP = 0, I2C_POTNUM_POWAMP, I2C_ANALOG_SWITCH, I2C_LAST } i2cdev_t;

typedef struct {
    uint8_t address;
    uint8_t value;
    uint8_t update_needed;
    uint32_t last_change_time;
    uint8_t stored_value;
    uint8_t step;
} i2cval_t;

static i2cval_t i2c_devices[] = {
    { I2C_ADDRESS_POTNUM_PREAMP, 0, 0, 0, 0, 0 },
    { I2C_ADDRESS_POTNUM_POWAMP, 0, 0, 0, 0, 0 },
    { I2C_ADDRESS_ANALOG_SWITCH, 0, 0, 0, 0, 0 }
};


void i2c_set_preamp_gain(uint8_t gain)
{
    if (i2c_devices[I2C_POTNUM_PREAMP].value == gain)
        return ;   // même valeur demandée :rien à faire
    i2c_devices[I2C_POTNUM_PREAMP].value = gain;
    i2c_devices[I2C_POTNUM_PREAMP].update_needed = 1;
    return ;
}

void i2c_set_powamp_gain(uint8_t gain)
{
    if (i2c_devices[I2C_POTNUM_POWAMP].value == gain)
        return ;   // même valeur demandée :rien à faire
    i2c_devices[I2C_POTNUM_POWAMP].value = gain;
    i2c_devices[I2C_POTNUM_POWAMP].update_needed = 1;
    return ;
}

void i2c_set_analog_switch(uint8_t flags)
{
    if (i2c_devices[I2C_ANALOG_SWITCH].value == flags)
        return ;   // même valeur demandée :rien à faire
    i2c_devices[I2C_ANALOG_SWITCH].value = flags;
    i2c_devices[I2C_ANALOG_SWITCH].update_needed = 1;
    return ;
}

/* Retourne la valeur courante du gain du préampli
*/
uint8_t i2c_get_preamp_gain(void)
{
    return  i2c_devices[I2C_POTNUM_PREAMP].value;
}

/* Retournela valeur courante du gain de l'ampli de puissance
*/
uint8_t i2c_get_powamp_gain(void)
{
    return i2c_devices[I2C_POTNUM_POWAMP].value;
}

/* Retourne la valeur courante des switchs
*/
uint8_t i2c_get_analog_switchs(void)
{
    return i2c_devices[I2C_ANALOG_SWITCH].value;
}

/* Sequence : ecriture RESTORE puis NOP, ensuite ecriture RDAC bidon ensuite lecture RDAC */
static int restore_and_read(i2cdev_t dev)
{
    if ((dev >= I2C_LAST) || (dev == I2C_ANALOG_SWITCH))
        return 1;
    printls("Restore + read\r\n");
    aTxBuffer[0] = I2C_AD5259_CDE_RESTORE << 5;
    i2c_devices[dev].step = 1;
    if (HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aTxBuffer, 1) != HAL_OK)
        return 1;
    return 0;
}

static int update_value(i2cdev_t dev) {
    uint8_t size = 2;
    if (dev >= I2C_LAST)
        return 1;
    if (dev == I2C_ANALOG_SWITCH) {
        size = 1;
        aTxBuffer[0] = i2c_devices[dev].value;
    }
    else {
        size = 2;
        aTxBuffer[0] = I2C_AD5259_CDE_RDAC << 5;
        aTxBuffer[1] = i2c_devices[dev].value;
    }
    i2c_devices[dev].update_needed = 0;
    i2c_devices[dev].last_change_time = HAL_GetTick();
    if (HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aTxBuffer, size) != HAL_OK)
        return 1;
    return 0;
}

static int store_value(i2cdev_t dev) {
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
        return 1;   // busy
    if (dev >= I2C_LAST)
        return 1;
    if (dev == I2C_ANALOG_SWITCH)
        return 1;
    /*
    printls("Store dev #");
    printls_int(dev);
    printls("\r\n");
    */
    aTxBuffer[0] = I2C_AD5259_CDE_STORE << 5;
    i2c_devices[dev].stored_value = i2c_devices[dev].value;
    if (HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aTxBuffer, 1) != HAL_OK)
        return 1;
    return 0;
}

void i2c_background_process(void)
{
    uint8_t dev;
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
        return;   // busy
    /* On regarde si il faut initialiser un device */
    for (dev = 0 ; dev < I2C_LAST ; dev++) {
        if (dev == I2C_ANALOG_SWITCH)
            continue;
        if (i2c_devices[dev].last_change_time == 0) {
            restore_and_read(dev);
            break;
        }
    }
    /* Tant que tout n'est pas initialisé on ne fait rien d'autre */
    if (dev < I2C_LAST)
        return;
    if (gpio_apply_preamp_count(&i2c_devices[I2C_POTNUM_PREAMP].value))
        i2c_devices[I2C_POTNUM_PREAMP].update_needed = 1;
    if (gpio_apply_powamp_count(&i2c_devices[I2C_POTNUM_POWAMP].value))
        i2c_devices[I2C_POTNUM_POWAMP].update_needed = 1;
    /* On regarde si il faut mettre à jour un device */
    for (dev = 0 ; dev < I2C_LAST ; dev++) {
        if (i2c_devices[dev].update_needed) {
            update_value(dev);
            break;
        }
    }
    /* Ensuite on regarde si un potar numérique doit être sauvegardé */
    for (dev = 0 ; dev < I2C_LAST ; dev++) {
        if (dev == I2C_ANALOG_SWITCH)
            continue;
        if (i2c_devices[dev].value == i2c_devices[dev].stored_value)
            continue;
        if ((HAL_GetTick() - i2c_devices[dev].last_change_time) < I2C_TIMEOUT_FOR_STORE)
            continue;
        store_value(dev);
        break;
    }
}


/* Gestion de la séquence pour la fonction restore_and_read */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    uint8_t dev;
    for (dev = 0 ; dev < I2C_LAST ; dev++) {
        if (i2c_devices[dev].step == 1) {
            i2c_devices[dev].step = 2;
            aTxBuffer[0] = I2C_AD5259_CDE_NOP << 5;
            HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aTxBuffer, 1);
            break;
        }
        else if (i2c_devices[dev].step == 2) {
            i2c_devices[dev].step = 3;
            aTxBuffer[0] = I2C_AD5259_CDE_RDAC << 5;
            HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aTxBuffer, 1);
            break;
        }
        else if (i2c_devices[dev].step == 3) {
            i2c_devices[dev].step = 4;
            aTxBuffer[0] = I2C_AD5259_CDE_RDAC << 5;
            HAL_I2C_Master_Receive_IT(&hi2c1, (uint16_t)(i2c_devices[dev].address << 1), (uint8_t*)aRxBuffer, 1);
            break;
        }
    }
}

/* Gestion de la séquence pour la fonction restore_and_read : dernier step */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    uint8_t dev;
    for (dev = 0 ; dev < I2C_LAST ; dev++) {
        if (i2c_devices[dev].step == 4) {
            i2c_devices[dev].step = 0 ;
            i2c_devices[dev].value = aRxBuffer[0] ;
            i2c_devices[dev].stored_value = aRxBuffer[0] ;
            i2c_devices[dev].last_change_time = HAL_GetTick(); ;
            break;
        }
    }
}

/* I2C1 init function */
void i2c_init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x2000090E;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    HAL_I2C_Init(&hi2c1);

    HAL_I2CEx_AnalogFilter_Config(&hi2c1, I2C_ANALOGFILTER_ENABLED);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (hi2c->Instance == I2C1)
    {
        __I2C1_CLK_ENABLE();

        /**I2C1 GPIO Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* NVIC for I2C1 */
        HAL_NVIC_SetPriority(I2C1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(I2C1_IRQn);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        __I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
    }
}
