/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include <string.h>

#include "stm32f0xx_hal.h"
#include "string.h"
#include "uart.h"

static uint8_t buffer_rx[UART_BUFFER_SIZE];
static uint8_t _frame_received = 0;

UART_HandleTypeDef huart1;

void uart_init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart1);
    huart1.pRxBuffPtr = buffer_rx;
    huart1.RxXferSize = UART_BUFFER_SIZE;
    huart1.RxXferCount = 0;
    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

int uart_receive(char **buf)
{
    if(!_frame_received)
        return 0;
    _frame_received = 0;
    *buf = (char *)buffer_rx;
    huart1.pRxBuffPtr = buffer_rx;
    huart1.RxXferCount = 0;
    return 1;
}

void USART1_IRQHandler(void)
{
    if(__HAL_UART_GET_IT(&huart1, UART_IT_RXNE) != RESET){
        /* Clear RXNE interrupt flag */
        __HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
        uint8_t car = (uint8_t)(huart1.Instance->RDR & 0xff);
        if(car == '<'){
            huart1.pRxBuffPtr = buffer_rx;
            huart1.RxXferCount = 0;
        }
        else if(car == '>')
        {
            _frame_received = 1;
        }
        else
        {
            *huart1.pRxBuffPtr = car;
            if(huart1.RxXferCount < huart1.RxXferSize){
                huart1.pRxBuffPtr++;
                huart1.RxXferCount++;
            }
        }
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (huart->Instance == USART1) {
        __USART1_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* NVIC for USART */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1) {
        __USART1_CLK_DISABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}
