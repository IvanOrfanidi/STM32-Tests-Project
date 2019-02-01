#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

char msg[] = "Hello World!!!";

#define SIZE_STASK_OW_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STACK_LCD_TASK configMINIMAL_STACK_SIZE * 2
#define SIZE_STACK_SDCARD_TASK configMINIMAL_STACK_SIZE * 2

// ID Task
xTaskHandle xHandleSDCardTask;
xTaskHandle xHandleLcdTask;
xTaskHandle xHandleOnewireTask;

void main(void)
{
    InitPIO();

    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    for(uint16_t i = 0; i < 10000; i++) {
        __NOP();
    }

    // Configure FSMC Bank1 NOR/PSRAM //
    FSMC_LCD_Init();

    LCD_Init();

    //LCD_test();
    int x = 2;
    for(int i = 0; i < strlen(msg); i++) {
        ili9320_PutChar(10 * x, 10, msg[i], 0x0000, 0xffff);
        x++;
    }
//ili9320_Clear(0x0000);
//ili9320_Test();

//ili9320_SetWindows(0,0,100,100);

// Init UART 1
#define UART_DEB UART_1
    //UartInit(UART_DEB,3);

    InitBKP();
    RTC_Configuration();

    /*
  RTC_t rtc;
  rtc.year = 2015;
  rtc.month = 11;
  rtc.mday = 11;
  rtc.hour = 22;
  rtc.min = 36;
  rtc.sec = 0;
  rtc_settime(&rtc);
*/

    /*
  RTC_t rtc;
  rtc_gettime(&rtc);
  char rx_temp_buf[50];
  sprintf(rx_temp_buf, "Time: %02d.%02d.%02d% 02d:%02d:%02d\r", 
          rtc.mday, rtc.month, rtc.year,
          rtc.hour, rtc.min, rtc.sec);  
  //UartWrite(UART_DEB, rx_temp_buf, strlen(rx_temp_buf)); 
  */
    SDIO_Fats_Init();
    //OutPutFile();

    xTaskCreate(vLcdTask, "vLcdTask", SIZE_STACK_LCD_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandleLcdTask);
    xTaskCreate(vSdCardTask, "vSdCardTask", SIZE_STACK_SDCARD_TASK, NULL, tskIDLE_PRIORITY + 2, &xHandleSDCardTask);

    /* Start scheduler */
    osKernelStart(NULL, NULL);

    while(1)
        ;
}

void vApplicationMallocFailedHook(void)
{
    for(;;)
        ;
}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
    for(;;)
        ;
}

void vApplicationIdleHook(void)
{    //это и есть поток Idle с минимальным приоритетом.

    while(1) {
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1) {
    }
}
#endif

/******************* (C) COPYRIGHT 2009  *****END OF FILE****/
