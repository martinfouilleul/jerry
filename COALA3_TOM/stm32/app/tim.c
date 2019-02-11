/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include "stm32f0xx_hal.h"

#define MAIN_LOOP_PERIOD_MS 100

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim14;

/* TIM1 init function */
void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
    TIM_OC_InitTypeDef sConfigOC;

    uint32_t uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000000) - 1;

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = uwPrescalerValue;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 100;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_Base_Init(&htim1);

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);

    HAL_TIM_PWM_Init(&htim1);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 100;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

// Timer for main loop
// Initializes the timer, turn on the interrupt and put the interrupt time to zero
void init_loop_timer(void)
{
    /* Compute the prescaler value to have TIM counter clock equal to 10000 Hz */
    uint32_t uwPrescalerValue = (uint32_t)(SystemCoreClock / 10000) - 1;

    /* Set TIMx instance */
    htim14.Instance = TIM14;

    htim14.Init.Period            = 10 * MAIN_LOOP_PERIOD_MS - 1;
    htim14.Init.Prescaler         = uwPrescalerValue;
    htim14.Init.ClockDivision     = 0;
    htim14.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim14.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&htim14);

    HAL_TIM_Base_Start_IT(&htim14);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    if (htim_base->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();

        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    }
    else if (htim_base->Instance == TIM14) {
        __HAL_RCC_TIM14_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM14_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(TIM14_IRQn);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

    if (htim_base->Instance == TIM1)
    {
        /* Peripheral clock disable */
        __TIM1_CLK_DISABLE();

        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);

    }
}

