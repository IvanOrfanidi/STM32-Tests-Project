
#include "includes.h"
#include "ACCEL2.h"

int8_t GetTemperaturAccel(void)
{
    return g_stRamAccState.s8Temperatur;
}

ACC_STATE AccelState(void)
{
    return g_stRamAccState.curr_state;
}

int accelInit(uint8_t Sensitivity)
{
    Accel_Reset();
    Accel_CMD(CTRL_REG4_REG, XEN | YEN | ZEN | F_3HZ);    // 3.125 Hz XYZ enable

#ifdef ACC_INT_ENABLE                           // Interrupt Accel Enable
    Accel_CMD(CTRL_REG3_REG, IEA | INT1_EN);    // int1 enable, pulsed, polarity - high
#else                                           // Interrupt Accel Disable
    Accel_CMD(CTRL_REG3_REG, 0x00);    // Off
#endif

    Accel_CMD(CTRL_REG5_REG, BW_FILTER_50HZ);    // 50Hz BW filter

    Accel_CMD(THRS1_2_REG, Sensitivity);    // thr1 - 150mg wakeup threshold

    const char STATE_MACHINE_CONFIG[] = { 0x31, 0x05, 0x11 };

    for(size_t i = 0; i < sizeof(STATE_MACHINE_CONFIG) / sizeof(char); i++) {
        Accel_CMD((ST2_X_REG + i), STATE_MACHINE_CONFIG[i]);
    }

    Accel_CMD(MASK2_A_REG, P_X | N_X | N_Y | P_Y | N_Z | P_Z);    // sm2 XYZ wakeup
    Accel_CMD(SETT2_REG, 0x10 | SITR);                            // sm2 generate int, diff data for sm2
    Accel_CMD(CTRL_REG2_REG, SM2_EN);                             // sm2 enable

    while(Accel_Read(OUTS2_REG)) {    //Сбросим флаг движения
        IWDG_ReloadCounter();
        if(getFlagSpiFail()) {
            return 1;
        }
    }

#ifdef ACC_INT_ENABLE    // Interrupt Accel Enable
    AccelIRQInit();      //Включаем внешнее прерывания от Акселерометра.
#else
    AccelIRQDeInit();
#endif

    return getFlagSpiFail();
}

void Accel_Reset(void)
{
    ACCEL_CS_ON;
    AccelIRQDeInit();
    Accel_CMD(CTRL_REG3_REG, STRT);    // Soft Reset Accel
    uint32_t timeout = 200000;
    while(timeout--) {
        IWDG_ReloadCounter();
    }

#ifdef ACC_INT_ENABLE
    Accel_CMD(CTRL_REG3_REG, IEA | INT1_EN);    // int1 enable, pulsed, polarity - high
#else
    Accel_CMD(CTRL_REG3_REG, 0x00);    // int1 enable, pulsed, polarity - high
#endif
    AccelIRQInit();
    ACCEL_CS_OFF;
}

void Accel_Power_Down(void)
{
    Accel_Reset();
    ACCEL_CS_ON;
    Accel_CMD(CTRL_REG4_REG, 0x00);    // Power down
    volatile uint32_t timeout = 100000;
    while(timeout--)
        ;
    ACCEL_CS_OFF;
}

uint8_t Accel_CMD(uint8_t adr, uint8_t data)
{
    DelayResolution100us(100);
    uint8_t ret;

    ACCEL_CS_ON;

    SPI1_SendByte(adr);
    ret = SPI1_SendByte(data);
    ACCEL_CS_OFF;

    return ret;
}

uint8_t Accel_Read(uint8_t adr)
{
    ACCEL_CS_ON;
    SPI1_SendByte(adr | 0x80);
    uint8_t ret = SPI1_SendByte(0x00);
    ACCEL_CS_OFF;

    return ret;
}

void AccelIRQDeInit(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    NVIC_DisableIRQ(EXTI0_IRQn);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void AccelIRQInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_AHBPeriphClockCmd(ACCEL_INT_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure Button pin as input */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = ACCEL_INT_PIN;
    GPIO_Init(ACCEL_INT_GPIO_PORT, &GPIO_InitStructure);

    /* Connect Button EXTI Line to Button GPIO Pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
    /* Configure Button EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;    //_-
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_ClearITPendingBit(EXTI_Line0);
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void AccelSetMove(void)
{
    // uint8_t Move;
    //проверим инициализирован ли spi
    if((SPI1->CR1 & SPI_CR1_SPE) == 0) {
        return;
    }

    while(Accel_Read(OUTS2_REG)) {    //Сбросим флаг движения
        IWDG_ReloadCounter();
    }

    //пересчитаем новое время прерывания
    getSystemDate(&g_stRamAccState.irq_date);
    g_stRamAccState.irq_sec = Date2Sec(&g_stRamAccState.irq_date);
    g_stRamAccState.curr_state = ACC_STATE_MOVE;
    g_stRamAccState.sec_state_move = 0;
    ACCEL_DPD("\r\n-D_ACCEL MOVE-\r\n", strlen("\r\n-D_ACCEL MOVE-\r\n"));
}

// Возвращаем температуру с акселерометра.
int8_t CalculTemperaturAccel(void)
{
    return Accel_Read(0x0C) + 25;
}

void ReadAxisDataAccel(TAcc_state* pAcc_state)
{
    pAcc_state->X = Accel_Read(OUT_X_H_REG);
    pAcc_state->Y = Accel_Read(OUT_Y_H_REG);
    pAcc_state->Z = Accel_Read(OUT_Z_H_REG);
}

void AccelHandler(void)
{
    //обработчик сообщений
    s32 cmd;

    static portTickType ms500_tick = 0;
    //секундный обработчик акселерометра
    if(ms500_tick < xTaskGetTickCount()) {
        ms500_tick = xTaskGetTickCount() + configTICK_RATE_HZ / 2;
        while(xQueueReceive(xAccelQueue, &cmd, 0)) {
            switch(cmd) {
                case CMD_ACCEL_READ_IRQ:
                    AccelSetMove();
                    break;
            }
        }

        if(Accel_Read(OUTS2_REG)) {
            AccelSetMove();
        }

        if(g_stRamAccState.curr_state == ACC_STATE_MOVE) {
            g_stRamAccState.sec_state_move++;
            g_stRamAccState.sec_state_stop = 0;
            if(g_stRamAccState.sec_state_move >= GetAccelTimeCurrState()) {
                g_stRamAccState.curr_state = ACC_STATE_STOP;
                ACCEL_DPD("\r\n-D_ACCEL STOP-\r\n", strlen("\r\n-D_ACCEL STOP-\r\n"));
            }
        }
        else {
            g_stRamAccState.sec_state_stop++;
        }
#if(TEMPERATURE_ACCEL)
        g_stRamAccState.s8Temperatur = CalculTemperaturAccel();
#else
        g_stRamAccState.s8Temperatur = -128;
#endif

        ReadAxisDataAccel(&g_stRamAccState);

        if(!(g_stRamAccState.X || g_stRamAccState.Y || g_stRamAccState.Z)) {
            // g_stEepConfig.stDevice.eResetStatusDevice = WARNING_ACCEL_FAIL;  //Регистрируем отказ акселерометра.
            DPS("!D_WARNING ACCEL FAIL!\r\n");
        }
    }
}

uint16_t SecStateMove(void)
{
    return g_stRamAccState.sec_state_move;
}
uint32_t SecStateStop(void)
{
    return g_stRamAccState.sec_state_stop;
}

// Внешнее прерывание от Акселероиетра.
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // Clear WAKEUP_BUTTON_EXTI_LINE pending bit
        EXTI_ClearITPendingBit(EXTI_Line0);

        s32 cmd = CMD_ACCEL_READ_IRQ;
        if((osKernelRunning()) && (xAccelQueue)) {    // Проверка на запущенную RTOS и созданную очередь.
            xQueueSendFromISR(xAccelQueue, &cmd, NULL);
        }
    }
}