
#include "pressure_general.h"
#include "includes.h"

void vPressureTask(void* pvParameters)
{
   TPressure_Data stPressureData;

   uint32_t u_temp, u_pres;   //Вспомагательные переменные "сырых" данных с датчика.

   // Создаём очередь
   xQueuePressureForLcd = xQueueCreate(sizeof(uint8_t), sizeof(stPressureData));
   vQueueAddToRegistry(xQueuePressureForLcd, "xQueuePressureForLcd");

   BMP180_Init(400000);
   uint8_t ChipID = BMP180_ReadReg(BMP180_CHIP_ID_REG);
   uint8_t Version = BMP180_ReadReg(BMP180_VERSION_REG);
   BMP180_ReadCalibration();

   while (1)
   {
      u_temp = BMP180_Read_UT();
      stPressureData.iRealTemperatur = BMP180_Calc_RT(u_temp);

      u_pres = BMP180_Read_PT(0);
      stPressureData.iRealPressurePa = BMP180_Calc_RP(u_pres, 0);
      stPressureData.iRealPressureHg = BMP180_kpa_to_mmhg(stPressureData.iRealPressurePa);

      if (xQueuePressureForLcd)
      {
         xQueueSendToFront(xQueuePressureForLcd, &stPressureData, (portTickType)100);
      }
      _delay_ms(500);
   }
}