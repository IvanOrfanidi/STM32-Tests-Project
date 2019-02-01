#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleDebugTask;
xTaskHandle xHandlePressureTask;

int main()
{
    InitGPIO();

    InitBKP();

    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();

    // Start Task //
    xTaskCreate(vPressureTask, "vPressureTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandlePressureTask);
    xTaskCreate(vDebugTask, "vDebugTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleDebugTask);

    // Start scheduler //
    osKernelStart(NULL, NULL);
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