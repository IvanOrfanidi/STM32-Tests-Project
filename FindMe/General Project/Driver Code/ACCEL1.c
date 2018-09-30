#include "includes.h"
#include "ACCEL.h"

#define ACC_DATA_BUF_SIZE 32
vector __packed acc_data[ACC_DATA_BUF_SIZE];
vector curr_a;
u8 read_irq_flag = 0;

uint8_t Accel_CMD(uint8_t adr, uint8_t data);
uint8_t Accel_Read(uint8_t adr);

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

   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;   //__--
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;

   EXTI_ClearITPendingBit(EXTI_Line0);
   EXTI_Init(&EXTI_InitStructure);

   NVIC_InitTypeDef NVIC_InitStructure;

   NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

   NVIC_Init(&NVIC_InitStructure);
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

// Внешнее прерывание от Акселероиетра.
void EXTI0_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXTI_Line0) != RESET)
   {
      // Clear WAKEUP_BUTTON_EXTI_LINE pending bit
      EXTI_ClearITPendingBit(EXTI_Line0);
      read_irq_flag = 1;
   }
}

void AccMoveDetect(void)
{
   read_irq_flag = 1;
}

void Accel_Reset(void)
{
   AccelIRQDeInit();
   Accel_CMD(0x23, 0x01);
   uint32_t i = 100000;
   while (i)
      i--;

   Accel_CMD(0x23, 0x48);   // int1 enable, pulsed, polarity - high
   Accel_CMD(0x23, 0x08);   // int1 enable, pulsed, polarity - high
#ifdef ACC_INT_ENABLE
   Accel_CMD(0x23, 0x48);   // int1 enable, pulsed, polarity - high
#else
   Accel_CMD(0x23, 0x00);   // int1 enable, pulsed, polarity - high
#endif
   CS_FREE(CS_ACCEL);
}

void Accel_Init(void)
{
   Accel_Reset();

#if (ACC_FREQ_HZ == 1600)
   Accel_CMD(0x20, 0x97);   // 1600Hz XYZ enable
#elif (ACC_FREQ_HZ == 100)
   Accel_CMD(0x20, 0x67);   // 100Hz XYZ enable
#else
#   error error accelerometr freq
#endif

   Accel_CMD(0x23, 0x00);   // int1 enable, pulsed, polarity - high
   Accel_CMD(0x24, 0x08);   // 800Hz BW filter, 4G max

   // FIFO enable, auto inc enable
   Accel_CMD(0x25, 0x50);
   // FIFO stream mode
   Accel_CMD(0x2E, 0x40);

   DSm_Init();
}

void AccelPowerDown(u8 sens)
{
   Accel_Reset();

   Accel_CMD(0x20, 0x17);   // 3.125 Hz XYZ enable
#ifdef ACC_INT_ENABLE
   Accel_CMD(0x23, 0x48);   // int1 enable, pulsed, polarity - high
#else
   Accel_CMD(0x23, 0x00);   // int1 enable, pulsed, polarity - high
#endif
   Accel_CMD(0x24, 0xC0);   // 50Hz BW filter

   Accel_CMD(0x77, sens);   // thr1 - 150mg wakeup threshold

   Accel_CMD(0x60, 0x31);   // sm2 code
   Accel_CMD(0x61, 0x05);   // sm2 code
   Accel_CMD(0x62, 0x11);   // sm2 code
   Accel_CMD(0x7A, 0xFC);   // sm2 XYZ wakeup
   Accel_CMD(0x7C, 0x00);   // sm2 programm adress
   Accel_CMD(0x7B, 0x11);   // sm2 generate int, diff data for sm2
   Accel_CMD(0x78, 0x00);   // sm2 input frequensy divider
   Accel_CMD(0x22, 0x01);   // sm2 enable
   Accel_Read(0x7F);
   AccelIRQInit();   //Включаем внешнее прерывания от Акселерометра.
   Accel_Read(0x7F);
}

uint8_t Accel_CMD(uint8_t adr, uint8_t data)
{
   DelayResolution100us(100);
   uint8_t ret;

   CS_SET(CS_ACCEL);

   SPI1_SendByte(adr);
   ret = SPI1_SendByte(data);
   CS_FREE(CS_ACCEL);

   return ret;
}

uint8_t Accel_Read(uint8_t adr)
{
   uint8_t ret;

   CS_SET(CS_ACCEL);

   SPI1_SendByte(adr | 0x80);
   ret = SPI1_SendByte(0x00);
   CS_FREE(CS_ACCEL);

   return ret;
}

int8_t GetTemperaturAccel(void)
{
   return g_stRamAccState.s8Temperatur;
}

ACC_STATE AccelState(void)
{
   return g_stRamAccState.curr_state;
}

// Возвращаем температуру с акселерометра.
int8_t CalculTemperaturAccel(void)
{
   return Accel_Read(0x0C) + 25;
}

void AccelReadIRQ(void)
{
   //проверим инициализирован ли spi
   if ((SPI1->CR1 & SPI_CR1_SPE) == 0)
   {
      return;
   }
   //сбросили прерывание
   Accel_Read(0x7F);

   //пересчитаем новое время прерывания
   rtc_gettime(&g_stRamAccState.irq_date);
   g_stRamAccState.irq_sec = Date2Sec(&g_stRamAccState.irq_date);
   g_stRamAccState.curr_state = ACC_STATE_MOVE;
   g_stRamAccState.sec_state_move = 0;
}

void AccelReadFIFO()
{
   u8 fifo_status;
   u8 len;
   u8* ptr;

   //получаем количество измерений в буфере акселерометра
   fifo_status = Accel_Read(0x2F);

   //получаем данные из акселерометра
   len = fifo_status & 0x1f;
   if (len)
   {
      if (len >= ACC_DATA_BUF_SIZE)
         len = ACC_DATA_BUF_SIZE - 1;

      ptr = (u8*)&acc_data[0].x;

      //читаем данные
      CS_SET(CS_ACCEL);
      SPI1_SendByte(0x28 | 0x80);
      for (u16 i = 0; i < len; i++)
         for (u8 j = 0; j < 6; j++)
            *ptr++ = SPI1_SendByte(0);
      CS_FREE(CS_ACCEL);
   }

   //переполнение буфера акселерометра
   if (fifo_status & BIT(6))
   {
      // ACCEL_DPD("ACCEL OVERFLOW\r", strlen("ACCEL OVERFLOW\r"));
   }

   const float acc_sens = 0.12;
   //обсчитываем стиль вождения
   for (u8 i = 0; i < len; i++)
   {
      acc_data[i].x *= acc_sens;
      acc_data[i].y *= acc_sens;
      acc_data[i].z *= acc_sens;

      DSm_Calc(&acc_data[i]);
   }

   __disable_interrupt();
   curr_a.x = acc_data[0].x;
   curr_a.y = acc_data[0].y;
   curr_a.z = acc_data[0].z;
   __enable_interrupt();
}

void ReadAxisDataAccel(TAcc_state* pAcc_state)
{
   int16_t temp;
   temp = Accel_Read(OUT_X_H_REGISTER);
   pAcc_state->X = temp << 8;
   // temp = Accel_Read(OUT_X_L_REGISTER);
   // pAcc_state->X |= temp;

   temp = Accel_Read(OUT_Y_H_REGISTER);
   pAcc_state->Y = temp << 8;
   // temp = Accel_Read(OUT_Y_L_REGISTER);
   // pAcc_state->Y |= temp;

   temp = Accel_Read(OUT_Z_H_REGISTER);
   pAcc_state->Z = temp << 8;
   // temp = Accel_Read(OUT_Z_L_REGISTER);
   // pAcc_state->Z |= temp;

   if (!(pAcc_state->X || pAcc_state->Y || pAcc_state->Z))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
      g_stRam.stDevice.eResetStatusDevice = WARNING_ACCEL_FAIL;
      DPS("WARNING: ACCEL FAIL!\r");
   }
}

void AccelHandler(void)
{
   static portTickType ms500_tick = 0;

   //вычитываем буфер с ускорениями
   AccelReadFIFO();

   //секундный обработчик акселерометра
   if (ms500_tick < xTaskGetTickCount())
   {
      ms500_tick = xTaskGetTickCount() + configTICK_RATE_HZ / 2;
      static u8 sec_flag = 0;
      sec_flag = 1 - sec_flag;

      if (read_irq_flag)
      {
         read_irq_flag = 0;
         AccelReadIRQ();
         ACCEL_DPD("-D_ACCEL MOVE-\r", strlen("-D_ACCEL MOVE-\r"));
      }

      if (g_stRamAccState.curr_state == ACC_STATE_MOVE)
      {
         g_stRamAccState.sec_state_move += sec_flag;
         g_stRamAccState.sec_state_stop = 0;
         if (g_stRamAccState.sec_state_move >= g_stEepConfig.stAccel.usTimeCurrState)
         {
            g_stRamAccState.curr_state = ACC_STATE_STOP;
            ACCEL_DPD("-D_ACCEL STOP\r", strlen("-D_ACCEL STOP-\r"));
         }
      }
      else
      {
         g_stRamAccState.sec_state_stop += sec_flag;
      }

#ifdef TEMPERATURE_ACCEL
      g_stRamAccState.s8Temperatur = CalculTemperaturAccel();
#else
      g_stRamAccState.s8Temperatur = -128;
#endif
      /*
      TAcc_state stAcc_state;
      ReadAxisDataAccel(&stAcc_state);
      if(!(stAcc_state.X || stAcc_state.Y || stAcc_state.Z)) {
          g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
          g_stRam.stDevice.eResetStatusDevice = WARNING_ACCEL_FAIL;
          DPS("WARNING: ACCEL FAIL!\r");
      }
      */
   }
}
