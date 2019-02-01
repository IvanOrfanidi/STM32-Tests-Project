#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleDebugTask;

FLASH_Status FlashStatus;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NumbOfVar] = { 0x5555, 0x6666, 0x7777 };

uint16_t g_usTempChar[1000];

int main()
{
    InitGPIO();

    InitBKP();

    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();

    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* EEPROM Init */
    EE_Init();

    /* --- Store successively many values of the three variables in the EEPROM ---*/
    /* Store 1000 values of Variable1 in EEPROM */
    for(uint16_t VarValue = 0; VarValue < 1000; VarValue++) {
        EE_WriteVariable(VirtAddVarTab[0], VarValue);
    }

    /* Store 500 values of Variable2 in EEPROM */
    for(uint16_t VarValue = 0; VarValue < 500; VarValue++) {
        EE_WriteVariable(VirtAddVarTab[1], VarValue);
    }

    /* Store 800 values of Variable3 in EEPROM */
    for(uint16_t VarValue = 0; VarValue < 800; VarValue++) {
        EE_WriteVariable(VirtAddVarTab[2], VarValue);
    }

    EE_ReadVariable(VirtAddVarTab[0], g_usTempChar);
    EE_ReadVariable(VirtAddVarTab[1], g_usTempChar);
    EE_ReadVariable(VirtAddVarTab[2], g_usTempChar);

    // Start Task //
    // xTaskCreate(vHumidityTask, "vHumidityTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1,
    // &xHandleHumidityTask);
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