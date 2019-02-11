/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include "stm32f0xx_hal.h"

#include "main.h"


/* Definition for SPIx clock resources */
#define SPIx                             SPI1
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT               GPIOA
#define SPIx_SCK_AF                      GPIO_AF0_SPI1
#define SPIx_MISO_PIN                    GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT              GPIOA
#define SPIx_MISO_AF                     GPIO_AF0_SPI1
#define SPIx_MOSI_PIN                    GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT              GPIOA
#define SPIx_MOSI_AF                     GPIO_AF0_SPI1

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI1_IRQn

/* Size of buffer */
#define BUFFERSIZE                       200

SPI_HandleTypeDef SpiHandle;

uint8_t TxBuffer[BUFFERSIZE];
uint8_t TxBufIdx = 0;

void spi_init(void)
{
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;

    HAL_SPI_Init(&SpiHandle);
}

void spi_send_char_blocking(uint8_t c)
{
    TxBuffer[0] = c;
    HAL_SPI_Transmit_IT(&SpiHandle, TxBuffer, 1);
    while (HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {
    }
}

void spi_send_buffer_start(void)
{
    TxBufIdx = 0;
}

void spi_send_buffer_fill(uint8_t c)
{
    TxBuffer[TxBufIdx] = c;
    TxBufIdx++;
    if (TxBufIdx >= BUFFERSIZE) {
        HAL_SPI_Transmit_IT(&SpiHandle, TxBuffer, BUFFERSIZE);
        while (HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
        {
        }
        TxBufIdx = 0;
    }
}

void spi_send_buffer_end(void)
{
    if (TxBufIdx) {
        HAL_SPI_Transmit_IT(&SpiHandle, TxBuffer, TxBufIdx);
        while (HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
        {
        }
        TxBufIdx = 0;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if (hspi->Instance == SPIx)
    {
        SPIx_CLK_ENABLE();
        DMAx_CLK_ENABLE();

        /* SPI SCK GPIO pin configuration  */
        GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = SPIx_SCK_AF;
        HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

        /* SPI MISO GPIO pin configuration  */
        GPIO_InitStruct.Pin = SPIx_MISO_PIN;
        GPIO_InitStruct.Alternate = SPIx_MISO_AF;
        HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

        /* SPI MOSI GPIO pin configuration  */
        GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
        GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
        HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

        /* NVIC for SPI */
        HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(SPIx_IRQn);

    }
}

/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPIx)
    {
        /*##-1- Reset peripherals ##################################################*/
        SPIx_FORCE_RESET();
        SPIx_RELEASE_RESET();

        /*##-2- Disable peripherals and GPIO Clocks ################################*/
        /* Configure SPI SCK as alternate function  */
        HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
        /* Configure SPI MISO as alternate function  */
        HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
        /* Configure SPI MOSI as alternate function  */
        HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

        /*##-3- Disable the NVIC for SPI ###########################################*/
        HAL_NVIC_DisableIRQ(SPIx_IRQn);

    }
}


