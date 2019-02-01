/*
  Проект LoRaWAN, счетчик.
Подсчет импульсов энергоэфективным таймером и передача по сети LoRaWAN.
Реализация с записью суточных показаний счетчика в EEPROM.

08.2017.
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"

#ifdef _MY_PRINTF_
#include "printf-stdarg.h"
#else
#include <stdio.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <intrinsics.h>
#include <assert.h>

#include "adc.h"
#include "lptim.h"
#include "uart.h"
#include "gpio.h"
#include "rtc.h"

#include "protocol.h"
#include "parser.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config();
void MX_RTC_Init();
void MX_NVIC_Init();

/* Отладочные макросы */
//#define __USE_DEBUG__         // Вывод всей отладочной информации.
#define __USE_TEST__    // Вывод только пакетов принятых/отданных от LoRa

/* Глобальные переменные для отладки */

/* включение всей отладки(выводятся все сообщения) */
#ifdef __USE_DEBUG__
_Bool __DEBUG__ = 1;
#else
_Bool __DEBUG__ = 0;
#endif

/* включение тестовой отладки(выводятся сообщения связанные только с протоколом обмена) */
#ifdef __USE_TEST__
_Bool __TEST__ = 1;
#else
_Bool __TEST__ = 0;
#endif

/* структуры будильников для вывода отладочной информации по будильникам */
RTC_AlarmTypeDef sAlarmA;    // будильник записи суточного архива
RTC_AlarmTypeDef sAlarmB;    // будильник контроля востановления тампера.

/* Function for debug */
void saveAlarmA(RTC_AlarmTypeDef* pAlarm)
{
    memcpy(&sAlarmA, pAlarm, sizeof(RTC_AlarmTypeDef));
}

void saveAlarmB(RTC_AlarmTypeDef* pAlarm)
{
    memcpy(&sAlarmB, pAlarm, sizeof(RTC_AlarmTypeDef));
}

/* переменные для вывода отладочной информации */
uint8_t count_daily;
uint8_t interval_send;
void saveInterval(uint32_t count, uint32_t interval)
{
    count_daily = (uint8_t)count;
    interval_send = (uint8_t)interval;
}
/**********************/

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    /* Configure LPUSART */
    MX_USART_DEBUG_Init();

    /* Configure LPTIM */
    MX_LPTIM1_Init();

    /* Configure RTC */
    MX_RTC_Init();

    /* Configure ADC */
    MX_ADC_Init();

    /* Configure NVIC */
    MX_NVIC_Init();

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("RUN mode.\r\n");
    }

    while(1) {
        if(__DEBUG__) {
            RTC_TimeTypeDef stTime;
            RTC_DateTypeDef stDate;
            extern RTC_HandleTypeDef hrtc;
            HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);
            uint32_t cur_sec = Date2Sec(&stTime, &stDate);

            printf("Time: %02d:%02d:%02d\r\n", stTime.Hours, stTime.Minutes, stTime.Seconds);

            printf("Date: %02d\\%02d\\%02d\r\n", stDate.Month, stDate.Date, stDate.Year);

            printf("Unix Timestamp: %d\r\n", cur_sec);

            printf("Alarm A: %02d:%02d:%02d\r\n",
                sAlarmA.AlarmTime.Hours,
                sAlarmA.AlarmTime.Minutes,
                sAlarmA.AlarmTime.Seconds);

            printf("Alarm B: %02d:%02d:%02d\r\n",
                sAlarmB.AlarmTime.Hours,
                sAlarmB.AlarmTime.Minutes,
                sAlarmB.AlarmTime.Seconds);

            printf("Count Daily: %d, Interval Send: %d\r\n", count_daily, interval_send);

            getLtimCountVal();

            printf("\r\n");
        }

        loop(10000000)
        {
            __NOP();
        }

        stopDevice();
    }
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
