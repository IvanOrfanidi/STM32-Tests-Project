
#include "includes.h"
#include "adxl345.h"

void SPI_ACCEL_Init(void);
void Set_Resolution(TSet_Resol eResol);
void Set_Tap_Threshold(uint8_t ucData);
void Set_Interrupt(TInt_Func eFunc, _Bool bVal, TSet_Map ePin);

int ADXL345_ReadXYZ(int16_t* pX, int16_t* pY, int16_t* pZ)
{
   uint8_t a_ucDataAccel[6];
   int16_t temp;

   uint8_t ChipID = ADXL345_read_byte(ADXL345_DEVID);
   if (ChipID != ADXL345_ID)
   {
      return -1;
   }
   a_ucDataAccel[0] = ADXL345_read_byte(ADXL345_DATAX0);
   a_ucDataAccel[1] = ADXL345_read_byte(ADXL345_DATAX1);
   a_ucDataAccel[2] = ADXL345_read_byte(ADXL345_DATAY0);
   a_ucDataAccel[3] = ADXL345_read_byte(ADXL345_DATAY1);
   a_ucDataAccel[4] = ADXL345_read_byte(ADXL345_DATAZ0);
   a_ucDataAccel[5] = ADXL345_read_byte(ADXL345_DATAZ1);

   temp = (a_ucDataAccel[1] << 8) + a_ucDataAccel[0];
   *pX = (temp);
   temp = (a_ucDataAccel[3] << 8) + a_ucDataAccel[2];
   *pY = (temp);
   temp = (a_ucDataAccel[5] << 8) + a_ucDataAccel[4];
   *pZ = (temp);

   return 0;
}

void AccelInit(void)
{
   // Init SPI
   SPI_ACCEL_Init();

   loop(1000)
   {
      __NOP();
   }
#ifndef _ACCEL_LOW_POWER_MODE_
#   error "Accelerometr config error"
#endif
   /* Выбор пропускной способности */
#if (_ACCEL_LOW_POWER_MODE_)
   ADXL345_write_byte(ADXL345_BW_RATE, BANDWIDTH_020000 | ADXL345_LOW_POWER_MODE);
#else
   ADXL345_write_byte(ADXL345_BW_RATE, BANDWIDTH_010000);
#endif

   ADXL345_write_byte(ADXL345_POWER_CTL, 0x00);
   ADXL345_write_byte(ADXL345_POWER_CTL, (1 << 3));

   /* Выбор разрешающей способности */
#if (_ACCEL_LOW_POWER_MODE_)
   Set_Resolution(RESOLUTION_2G);
#else
   Set_Resolution(RESOLUTION_FULL);
#endif

#ifdef USE_ADXL345_INT1
   GPIO_InitTypeDef GPIO_InitStructure;
   EXTI_InitTypeDef EXTI_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   RCC_APB2PeriphClockCmd(ADXL345_INT1_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = ADXL345_INT1_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(ADXL345_INT1_GPIO_PORT, &GPIO_InitStructure);

   /* Enable AFIO clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* Configure EXTI0 line */
   EXTI_InitStructure.EXTI_Line = ADXL345_INT1_EXTI_LINE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = ADXL345_INT1_EXTI_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
   ;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef USE_ADXL345_INT2
#   ifndef USE_ADXL345_INT1
   GPIO_InitTypeDef GPIO_InitStructure;
   EXTI_InitTypeDef EXTI_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
#   endif

   RCC_APB2PeriphClockCmd(ADXL345_INT2_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = ADXL345_INT2_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(ADXL345_INT2_GPIO_PORT, &GPIO_InitStructure);

   /* Enable AFIO clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* Configure EXTI0 line */
   EXTI_InitStructure.EXTI_Line = ADXL345_INT2_EXTI_LINE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = ADXL345_INT2_EXTI_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
#endif
}

void SPI_ACCEL_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   SPI_InitTypeDef SPI_InitStructure;

#if _SPI_ADXL345_PORT == 1
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
   RCC_APB2PeriphClockCmd(ADXL345_SPI_CLK, ENABLE);
#elif _SPI_ADXL345_PORT == 2
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
   RCC_APB1PeriphClockCmd(ADXL345_SPI_CLK, ENABLE);
#endif

   GPIO_StructInit(&GPIO_InitStructure);
   RCC_APB2PeriphClockCmd(ADXL345_SPI_CS_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = ADXL345_SPI_CS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(ADXL345_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
   ACCEL_CS_DISABLE;

   GPIO_StructInit(&GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Pin = ADXL345_SPI_SCK_PIN | ADXL345_SPI_MISO_PIN | ADXL345_SPI_MOSI_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(ADXL345_SPI_GPIO_PORT, &GPIO_InitStructure);

   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_SPEED;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;
   SPI_Init(ADXL345_SPI, &SPI_InitStructure);
   SPI_Cmd(ADXL345_SPI, ENABLE);
}

static uint8_t ADXL345_read_byte(uint8_t add)
{
   ACCEL_CS_ENABLE;

   SPI_I2S_SendData(ADXL345_SPI, (add | 0x80) << 8 | 0x00);

   while (SPI_I2S_GetFlagStatus(ADXL345_SPI, SPI_I2S_FLAG_TXE) == RESET)
      ;
   while (SPI_I2S_GetFlagStatus(ADXL345_SPI, SPI_I2S_FLAG_RXNE) == RESET)
      ;

   ACCEL_CS_DISABLE;

   return SPI_I2S_ReceiveData(ADXL345_SPI);
}

static uint8_t ADXL345_write_byte(uint8_t add, uint8_t val)
{
   ACCEL_CS_ENABLE;

   SPI_I2S_SendData(ADXL345_SPI, add << 8 | val);

   while (SPI_I2S_GetFlagStatus(ADXL345_SPI, SPI_I2S_FLAG_TXE) == RESET)
      ;
   while (SPI_I2S_GetFlagStatus(ADXL345_SPI, SPI_I2S_FLAG_RXNE) == RESET)
      ;

   ACCEL_CS_DISABLE;

   return SPI_I2S_ReceiveData(ADXL345_SPI);
}

/*===============================================================
//           Установка разрешающей способности
//           resol - выбор разрешающей способности
//==============================================================*/
void Set_Resolution(TSet_Resol eResol)
{
   uint8_t ucData = ADXL345_read_byte(ADXL345_DATA_FORMAT);
   ucData |= eResol;
   ADXL345_write_byte(ADXL345_DATA_FORMAT, eResol);
}

/*===============================================================
//                       Режим Ожидания
//           ENABLE - разрешить, DISABLE - запретить
//==============================================================*/
void Standby_Mode(_Bool bVal)
{
   uint8_t ucData = 0;
   ucData = ADXL345_read_byte(ADXL345_POWER_CTL);
   if (bVal)
   {
      ucData |= (1 << 3);
   }
   else
   {
      ucData &= ~(1 << 3);
   }
   ADXL345_write_byte(ADXL345_POWER_CTL, ucData);
}

/*===============================================================
//                   Установка порога Толчка
//                62.5mg * ucData (max 0xFF=16g)
//==============================================================*/
void Set_Tap_Threshold(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_THRESH_TAP, ucData);
}

/*===============================================================
//           Установка лимита времени для Толчка (DUR)
//              625 µs * ucData (0- disable)
//==============================================================*/
void Set_Tap_Duration(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_DUR, ucData);
}

/*===============================================================
//           Установка задержки для Толчка (Latent)
//              1.25 ms * ucData (0- disable)
//==============================================================*/
void Set_Tap_Latency(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_LATENT, ucData);
}

/*===============================================================
//     Установка временного окна для Второго Толчка (Window)
                1.25 ms * ucData (0- disable)
//==============================================================*/
void Set_Tap_Window(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_WINDOWS, ucData);
}

/*===============================================================
//                   Установка порога Активности
//              62.5mg * ucData (max 0xFF=16g)
//==============================================================*/
void Set_Activity_Threshold(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_THRESH_ACT, ucData);
}

/*===============================================================
//                 Установка порога Неактивности
//              62.5mg * ucData (max 0xFF=16g)
//==============================================================*/
void Set_Inactivity_Threshold(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_THRESH_INACT, ucData);
}

/*===============================================================
//                Установка времени Неактивности
//              1 sec * ucData
//==============================================================*/
void Set_Time_Inactivity(uint8_t ucData)
{
   ADXL345_write_byte(ADXL345_TIME_INACT, ucData);
}

/*===============================================================
//             Определение источника прерывания
//  func - вид функции прерывания. Функция возвращает "1", если
//  прерывание произошло от опрашиваемого источника, иначе - "0"
//==============================================================*/
uint8_t Get_Source_Interrupt(void)
{
   return ADXL345_read_byte(ADXL345_INT_SOURCE);
}

/*===============================================================
//                  Выбор функции прерывания
//  func - вид функции прерывания, val - разрешение/запрещение
//  прерывания, pin - назначение вывода прерывания
//=============================================================*/
void Set_Interrupt(TInt_Func eFunc, _Bool bVal, TSet_Map ePin)
{
   uint8_t ucTempData;

   ucTempData = ADXL345_read_byte(ADXL345_INT_ENABLE);
   if (bVal)
   {
      ucTempData |= eFunc;
   }
   else
   {
      ucTempData &= ~eFunc;
   }
   ADXL345_write_byte(ADXL345_INT_ENABLE, ucTempData);
   loop(1000)
   {
      __NOP();
   }

   ucTempData = ADXL345_read_byte(ADXL345_INT_MAP);
   if (ePin == INT_1)
   {
      ucTempData &= ~eFunc;
   }
   else
   {
      ucTempData |= eFunc;
   }
   ADXL345_write_byte(ADXL345_INT_MAP, ucTempData);

   ADXL345_write_byte(ADXL345_TAP_AXES, 7);
}

void Accel_Clear_Settings(void)
{
   uint8_t value;

   ADXL345_write_byte(ADXL345_THRESH_TAP, 0);
   ADXL345_write_byte(ADXL345_DUR, 0);
   ADXL345_write_byte(ADXL345_LATENT, 0);
   ADXL345_write_byte(ADXL345_WINDOWS, 0);
   ADXL345_write_byte(ADXL345_THRESH_ACT, 0);
   ADXL345_write_byte(ADXL345_THRESH_INACT, 0);
   ADXL345_write_byte(ADXL345_TIME_INACT, 0);
   ADXL345_write_byte(ADXL345_THRESH_FF, 0);
   ADXL345_write_byte(ADXL345_TIME_FF, 0);
   ADXL345_write_byte(ADXL345_FIFO_CTL, 0);
   ADXL345_write_byte(ADXL345_INT_ENABLE, 0);
   ADXL345_write_byte(ADXL345_INT_SOURCE, 0);

   value = ADXL345_read_byte(ADXL345_ACT_INACT_CTL);
   value &= 0x88;
   ADXL345_write_byte(ADXL345_ACT_INACT_CTL, value);

   value = ADXL345_read_byte(ADXL345_TAP_AXES);
   value &= 0xF8;
   ADXL345_write_byte(ADXL345_TAP_AXES, value);
}

void Reset_Interrupt(void)
{
   ADXL345_write_byte(ADXL345_INT_SOURCE, 0);
}

void Accel_Sleep(void)
{
   Reset_Interrupt();
   Standby_Mode(ENABLE);
   ADXL345_write_byte(ADXL345_POWER_CTL, 0x00);
   ADXL345_write_byte(ADXL345_POWER_CTL, FREQ_SM1 | ADXL345_SLEEP_BIT);
}

#ifdef USE_ADXL345_INT1
void EXTI1_IRQHandler(void)
{
   if (EXTI_GetITStatus(ADXL345_INT1_EXTI_LINE) != RESET)
   {
      /*BaseType_t xHigherPriorityTaskWoken = pdTRUE;
      xSemaphoreGiveFromISR(sBinSemAccelInterrupt, &xHigherPriorityTaskWoken);*/
      EXTI_ClearITPendingBit(ADXL345_INT1_EXTI_LINE);
   }
}
#endif

#ifdef USE_ADXL345_INT2
void EXTI2_IRQHandler(void)
{
   if (EXTI_GetITStatus(ADXL345_INT2_EXTI_LINE) != RESET)
   {
      EXTI_ClearITPendingBit(ADXL345_INT2_EXTI_LINE);
   }
}

#endif