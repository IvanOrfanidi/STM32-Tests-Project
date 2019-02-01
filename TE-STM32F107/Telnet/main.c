#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

#define ETH_RXBUFNB 8
#define ETH_TXBUFNB 2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Ethernet Rx & Tx DMA Descriptors */
ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
/* Ethernet buffers */
u8 Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];
ErrorStatus HSEStartUpStatus;

// ID Task
xTaskHandle xHandleDebugTask;
xTaskHandle xHandleuIPMainTask;

int main()
{
    /* Configure the GPIO ports */
    InitGPIO();

    /* System Clocks Configuration */
    RCC_Configuration();

    // InitIWDG();    // Init Watch Dog
    // InitBKP();
    /*Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();
   rtc_init();*/

    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);

    /* Configure the GPIO ports */
    ETH_GPIO_Configuration();

    /* Reset ETHERNET on AHB Bus */
    ETH_DeInit();

    /* Software reset */
    ETH_SoftwareReset();

    /* Wait for software reset */
    while(ETH_GetSoftwareResetStatus() == SET)
        ;

    /*Found Phy Address*/
    unsigned int PhyAddr;
    for(PhyAddr = 30; 32 >= PhyAddr; PhyAddr++) {
        if((0x0000 == ETH_ReadPHYRegister(PhyAddr, 2)) && (0x8201 == (ETH_ReadPHYRegister(PhyAddr, 3))))
            break;
    }

    if(32 < PhyAddr) {
        while(1)
            ;
    }

    /* ETHERNET Configuration ------------------------------------------------------*/
    /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
    ETH_InitTypeDef ETH_InitStructure;
    ETH_StructInit(&ETH_InitStructure);

    /* Fill ETH_InitStructure parametrs */
    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
    // ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    // ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    /* Configure ETHERNET */
    /* Configure Ethernet */
    if(0 == ETH_Init(&ETH_InitStructure, PhyAddr)) {
        while(1)
            ;
    }

    /* Initialize Tx Descriptors list: Chain Mode */
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    /* Initialize Rx Descriptors list: Chain Mode  */
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

    /* Enable MAC and DMA transmission and reception */
    ETH_Start();

    // Start Task //
    // xTaskCreate(vDebugTask, "vDebugTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1,
    // &xHandleDebugTask);
    xTaskCreate(uIPMain, "uIPMain", configMINIMAL_STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 1, &xHandleuIPMainTask);

    // Start scheduler //
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
{
    unsigned long ulFreeSizeStackTask;    //свободное место в процессах RTOS.
    unsigned long ulFreeHeapSize;         //свободное место в "куче" RTOS
    TickType_t WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;

    while(1) {
        if(xTaskGetTickCount() >= WakeTick) {
#if(INCLUDE_uxTaskGetStackHighWaterMark == 1)
            ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleDebugTask) << 2;
            ulFreeHeapSize = (unsigned long)xPortGetFreeHeapSize();    // in Byte
#endif

            /* Определяем загруженность OS */
            WakeTick += configTICK_RATE_HZ;
        }
        if(WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1) {
            WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
        }
        IWDG_ReloadCounter();    // Reload IWDG counter
    }
}