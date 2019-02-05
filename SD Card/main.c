#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleSDCardTask;

int main()
{
    InitGPIO();

    InitBKP();

    //rtc_init();

    SD_Init();

    OutPutFile();

    // Start Task //
    xTaskCreate(vSdCardTask, "vSdCardTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleSDCardTask);

    // Start scheduler //
    osKernelStart(NULL, NULL);
}

void vApplicationMallocFailedHook(void)
{
    for(;;) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
    for(;;) {
    }
}

void vApplicationIdleHook(void)
{
    portTickType WakeTick = 0;

    WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
    while(1) {
        if(xTaskGetTickCount() >= WakeTick) {
        }
        if(WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1) {
            WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
        }

        IWDG_ReloadCounter();    // Reload IWDG counter
    }
}