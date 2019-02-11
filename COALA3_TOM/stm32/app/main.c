/**************************************************************
Projet COALA 2 
Copyright (c) 2016 IRCAM , 1 place Igor Stravinkky 75004 Paris
Auteur : Francois Beaulier fbeaulier@ingelibre.fr 
***************************************************************/

#include <string.h>
#include "stm32f0xx_hal.h"

#include "main.h"
#include "i2c.h"
#include "gpio.h"
#include "menu.h"
#include "uart.h"

volatile uint8_t _flag_main_loop = 0;

void MX_TIM1_Init(void);
void init_loop_timer(void);

void SystemClock_Config(void);

typedef struct
{
    uint8_t *Buf;
    uint8_t *readp;
    uint8_t *writep;
    uint32_t size;
} bytefifo_t;
#define PRINTF_BUFFER_SIZE 256
uint8_t logbuf[PRINTF_BUFFER_SIZE];
void bytefifo_Init(bytefifo_t *pFifo, uint8_t *buffer, uint32_t buf_size);
int bytefifo_Write(bytefifo_t *pFifo, uint8_t *pNbr);
int bytefifo_Read(bytefifo_t *pFifo, uint8_t *pNbr);
bytefifo_t log_fifo;
#define idle_func() ll_putchar()
int ll_putchar(void);
void print_usart(char *str);
uint32_t timestamp = 0;
int _running = -1;

void process_com(char *buf)
{
    //buf++;
    char txbuf[16];
    if(!strcmp(buf, "GET_GAIN_PREAMP")){
        itoa(i2c_get_preamp_gain(), txbuf);
        strcat(txbuf,"\r\n");
        printls(txbuf);
    }
    else if(!strcmp(buf, "GET_GAIN_POWAMP")){
        itoa(i2c_get_powamp_gain(), txbuf);
        strcat(txbuf,"\r\n");
        printls(txbuf);
    }
    else if(!strcmp(buf, "GET_SWITCHES")){
        itoa(i2c_get_analog_switchs(), txbuf);
        strcat(txbuf,"\r\n");
        printls(txbuf);
    }
    else if(!strncmp(buf, "/coala/mcu/preamp/gain/", 23))
    {
       uint8_t g = atoi( buf+23 );
       i2c_set_preamp_gain( g );
       printls("set preamp gain ack\r\n");
    }
    else if(!strncmp(buf, "/coala/mcu/powamp/gain/", 23))
    {
       uint8_t g = atoi( buf+23 );
       i2c_set_powamp_gain( g );
       printls("set power amp gain ack\r\n");
    }
    else if(!strncmp(buf, "/coala/mcu/switches/", 20))
    {
       uint8_t switches = atoi( buf+20 );
       i2c_set_analog_switch( switches );
       updateSwitchesDisplay();
       printls("set switches ack\r\n");
    }
    else if(!strncmp(buf, "SET_RUNNING=", 12))
    {
       int value = atoi( buf+12 );
       if ( _running != value )
       {
          _running = value;
          updateRunning( _running ); 
       }
       printls("set running ack\r\n");
    }
}

static int _semaphore = 0;
int main(void)
{
    char *rxframe;
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();

    bytefifo_Init( &log_fifo, logbuf, PRINTF_BUFFER_SIZE );
 
    gpio_init();
    uart_init();
    i2c_init();
    MX_TIM1_Init();
    menu_init();
    init_loop_timer();

    //printls("Starting at ");
    //printls_int(SystemCoreClock);
    //printls(" Hz\r\n");
    //while (ll_putchar());

    /* Allume l'ampli de puissance */
    gpio_set_powamp_mute(1);
    gpio_set_powamp_stdby(1);

    /* Infinite loop */
    while (1)
    {
        while (!_flag_main_loop)
            idle_func();
        _flag_main_loop = 0;
        i2c_background_process();
        gpio_background_process();
        menu_background_process();
        if( uart_receive(&rxframe) )
        {
            process_com(rxframe);
        }
        //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
        if (gpio_get_button_start())
        {
            //printls("Button start\r\n");
        }
        if (gpio_get_button_stop())
        {
            //printls("Button stop\r\n");
        }
        if (gpio_get_button_escape())
        {
            //printls("Button escape\r\n");
        }
        if (gpio_get_button_preamp())
        {
            //printls("Button preamp\r\n");
        }
        if (gpio_get_button_powamp())
        {
            //printls("Button powamp\r\n");
        }
        if (gpio_get_button_menu())
        {
            //printls("Button menu\r\n");
        }
    }
}

void HAL_MspInit(void)
{
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    _flag_main_loop = 1;
}

/* Definit la clock :
    - HSI (high speed internal) utilisé
    - PLL paramétrée pour générer du 24Mhz : SystemCoreClock = 24000000
*/
void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct =  { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI14;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;    // HSI est divisée par 2 puis multipliée par 6
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    __SYSCFG_CLK_ENABLE();

}

char* itoa(int32_t i, char b[])
{
    char const digit[] = "0123456789";
    char* p = b;
    if(i < 0){
        i = -i;
        *p='-';
        p++;
    }
    uint32_t shifter = i;
    do { //Move to where representation ends
        ++p;
        shifter = shifter / 10;
    } while (shifter);
    *p = '\0';
    do { //Move back, inserting digits as u go
        *--p = digit[i % 10];
        i = i / 10;
    } while (i);
    return b;
}

int atoi(const char *str)
{
    int res = 0;
    int i;
    for ( i = 0; str[i] >= '0' && str[i] <= '9'; ++i)
    {
        res = res*10 + str[i] - '0';
    }
    return res;
}


void print_usart(char *str)
{
    uint8_t *ptr = (uint8_t *)str;
    while (*ptr) {
        bytefifo_Write(&log_fifo, ptr);
        ptr++;
    }
}

/* Read the log buffer and write next available character to the usart */
int ll_putchar(void)
{
    uint8_t ch = 0;
    if (bytefifo_Read(&log_fifo, &ch))
        return 0;
    /* Send 1 character in blocking mode, timeout set to 2ms */
    HAL_UART_Transmit(&huart1, &ch, 1, 2);
    return ch;
}

void printls_int(uint32_t i)
{
    char nbr[12];
    itoa(i, nbr);
    printls(nbr);
}

void print_ts(char *str)
{
    printls_int(timestamp);
    print_usart(": ");
    print_usart(str);
}

/*--------- Jeu de fonctions pour le buffer d'affichage via l'UART ----------*/

void bytefifo_Init(bytefifo_t *pFifo, uint8_t *buffer, uint32_t buf_size)
{
    pFifo->Buf = buffer;
    pFifo->readp = pFifo->writep = pFifo->Buf;
    pFifo->size = buf_size;
    uint32_t n = 0;
    for (; n < buf_size ; ++n)
      pFifo->Buf[n] = 0;
}

/* Ecriture d'un nouveau message dans le buffer
 * retour :  -1 buffer plein
 *           0 ok
 */
int bytefifo_Write(bytefifo_t *pFifo, uint8_t *pNbr)
{
    int Space;
    uint8_t *wp;
    /* calcul espace restant dans la fifo */
    Space = (pFifo->writep < pFifo->readp) ?
            (pFifo->readp - pFifo->writep - 1) : (pFifo->readp - pFifo->writep + (pFifo->size - 1));
    if (Space == 0)
        return -1;
    *(pFifo->writep) = *pNbr;
    wp = pFifo->writep;
    wp++;
    if (wp >= (pFifo->Buf + pFifo->size ))
        wp = pFifo->Buf;
    pFifo->writep = wp;
    return 0;
}

/* Lecture d'un  message dans le buffer
 * retour :  -1 buffer vide
 *           0 ok
 */
int bytefifo_Read(bytefifo_t *pFifo, uint8_t *pNbr)
{
    uint8_t *rp;
    if (pFifo->readp == pFifo->writep)
        return -1;
    *pNbr = *(pFifo->readp);
    rp = pFifo->readp;
    rp++;
    if (rp >= (pFifo->Buf + pFifo->size))
        rp = pFifo->Buf;
    pFifo->readp = rp;
    return 0;
}

