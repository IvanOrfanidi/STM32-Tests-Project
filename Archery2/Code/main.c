#define __ALLOCATE_NOW__

#include "includes.h"

osStatus InitOS();

// ID Task
xTaskHandle xHandleDisplayTask;
xTaskHandle xHandleAccelTask;
xTaskHandle xHandleBuzzerTask;

int main(void)
{
    SystemCoreClockUpdate();
    /* Configure Previousup */
    BKP_Configuration();

    /* Read control/status register (RCC_CSR) */
    if(GetFlagsControlRegister() & RCC_CSR_IWDGRSTF) {    // If independent watchdog reset
        SleepDevice();
    }

    /* System Clocks Configuration */
    RCC_Configuration();

    /* Configure Debug Interface */
#ifdef __USE_DEBUG_UART__
    initDebugUart();
#endif

    /* Configure the GPIO ports */
    GPIO_Configuration();

    /* Configure Timer */
    TIM4_Configuration();

    /* Configure ADC */
    ADC_Configuration();

    /* Configure Watchdog */
    IWDG_Configuration();

#ifdef __USE_DEBUG_USB__
    /* Configure USB Driver */
    usbInitDriver();
#endif

    if(InitOS() != osOK) {
        cout("Init FreeRTOS error!\r\n");
    }

    /* start scheduler */
    osKernelStart(NULL, NULL);

    cout("Start kernel error!\r\n");
    while(1) {
    }
}

/* Return osStatus: osOK - Rtos OK, osErrorOS - Rtos Fail */
osStatus InitOS(void)
{
    /* periph mutex */
    sBinSemDbg = xSemaphoreCreateMutex();
    sBinSemReservDbgBuf = osMutexCreate(NULL);
    xSemaphoreTake(sBinSemReservDbgBuf, NULL);
    /* --------------------------------------- */

    xBuzQueue = xQueueCreate(sizeof(uint8_t), sizeof(TickType_t));
    if(!(xBuzQueue)) {
        return osErrorOS;
    }

    /* Init Task */
    if(xTaskCreate(vDisplayTask,
           "vDisplayTask",
           configMINIMAL_STACK_SIZE * 6,
           NULL,
           tskIDLE_PRIORITY + 3,
           &xHandleDisplayTask) == NULL) {
        return osErrorOS;
    }
    if(xTaskCreate(
           vAccelTask, "vAccelTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleAccelTask) ==
        NULL) {
        return osErrorOS;
    }
    if(xTaskCreate(
           vBuzzerTask, "vBuzzerTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 2, &xHandleBuzzerTask) ==
        NULL) {
        return osErrorOS;
    }
    return osOK;
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
{
    portTickType WakeTickSleep = xTaskGetTickCount() + configTICK_RATE_HZ;

    while(1) {
        if(xTaskGetTickCount() >= WakeTickSleep) {
#if(INCLUDE_uxTaskGetStackHighWaterMark == 1)
            static unsigned long ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleDebugTask) << 2;
            static unsigned long ulFreeHeapSize = (unsigned long)xPortGetFreeHeapSize();    // in Byte
#endif
            IWDG_ReloadCounter();    // Reload IWDG counter
            WakeTickSleep += configTICK_RATE_HZ;
        }
        if(WakeTickSleep > xTaskGetTickCount() + (configTICK_RATE_HZ << 1)) {
            WakeTickSleep = configTICK_RATE_HZ + xTaskGetTickCount();
        }
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
      ex: ("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1) {
    }
}
#endif