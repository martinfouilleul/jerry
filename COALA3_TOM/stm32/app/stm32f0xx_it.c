/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim14;

extern SPI_HandleTypeDef SpiHandle;

/**
* @brief This function handles EXTI Line 4 to 15 interrupts.lma c m
*/
void EXTI4_15_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

/**
* @brief This function handles EXTI Line 2 and Line 3 interrupts.
*/
void EXTI2_3_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
* @brief This function handles EXTI Line 0 and Line 1 interrupts.
*/
void EXTI0_1_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void I2C1_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
    HAL_I2C_ER_IRQHandler(&hi2c1);
}

void TIM14_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim14);
}

void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&SpiHandle);
}
