/*
Проект LoRaWAN, коммуникатор.
Передача показаний тепловычитателя ЕЛЬФ-04п по сети LoRaWAN.

Список передаваемых данных:
суточные значения температуры теплоносителя (прямая);
суточные значения температуры теплоносителя (обратка);
часовые значения давления теплоносителя (прямая);
часовые значения давления теплоносителя (обратка);
суточные значения расхода теплоносителя (прямая);
суточные значения расхода теплоносителя (обратка);
суточные значения потребленной тепловой энергии;
суточные значения "Время исправной работы";
суточные значения температуры наружного воздуха.

*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"

#ifdef _MY_PRINTF_
#include "printf-stdarg.h"
#else
#include <stdio.h>
#endif

/* Standart lib */
#include <iostream>
using namespace std;

/* Drivers periph */
#include "adc.h"
#include "uart.h"
#include "gpio.h"
#include "rtc.h"

/* USB drivers */
//#include "usb_device.h"
//#include "usbd_cdc_if.h"

/* Modbus */
#include "modbus_elf.h"

#include "elf.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config();
void MX_RTC_Init();
void MX_NVIC_Init();

/* Отладочные макросы */
#define __USE_DEBUG__    // Вывод всей отладочной информации.
#define __USE_TEST__     // Вывод только пакетов принятых/отданных от LoRa

/* Переменные для отладки */
#ifdef __USE_DEBUG__
bool __DEBUG__ = 1;
#else
bool __DEBUG__ = 0;
#endif

#ifdef __USE_TEST__
bool __TEST__ = 1;
#else
bool __TEST__ = 0;
#endif

/**********************/

int main()
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    /* Configure GPIO */
    MX_GPIO_Init();
    /* Configure USB Device */
    // MX_USB_DEVICE_Init();
    /* Configure LP USART for Debug */
    MX_LPUSART_DEBUG_Init();
    /* Configure USART1 for Communication */
    MX_USART1_UART_Init();
    /* Configure RTC */
    MX_RTC_Init();
    /* Configure ADC */
    MX_ADC_Init();
    /* Configure NVIC */
    MX_NVIC_Init();

    extern bool __DEBUG__;
    if(__DEBUG__) {
        cout << "The application launched!\r\n";
    }

    UcEnable();    //Вкл. питание интерфейса Ельфа.
    HAL_Delay(1000);

    while(1) {
        uint32_t serialNumberElf = 0;
        pollElf(&stSerialNumber, &serialNumberElf);
        HAL_Delay(10000);
    }

    // UcDisable();   //Вкл. питание интерфейса Ельфа.
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char* file, int line)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while(1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
