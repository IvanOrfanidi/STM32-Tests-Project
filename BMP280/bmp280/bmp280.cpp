/**
 ******************************************************************************
 * @file    bmp180.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    10-April-2018
 * @brief
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2018 </center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bmp280.hpp"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "board.hpp"

/**
 * @brief Статические экземпляры класса
 */
Bmp* Bmp::Bmp280[Bmp::BMP280_MAX_COUNT];

/**
 * @brief Сonstructor
 */
Bmp::Bmp(I2C_TypeDef* i2c, uint32_t clock, const Params_t* const params)
{
   GPIO_InitTypeDef port;
   port.GPIO_Mode = GPIO_Mode_AF_OD;
   port.GPIO_Speed = GPIO_Speed_50MHz;

   I2C_InitTypeDef init;
   init.I2C_Mode = I2C_Mode_I2C;   // I2Cx mode is I2Cx
   init.I2C_DutyCycle = I2C_DutyCycle_2;   // I2Cx fast mode duty cycle (WTF is this?)
   init.I2C_OwnAddress1 = 1;   // This device address (7-bit or 10-bit)
   init.I2C_Ack = I2C_Ack_Enable;   // Acknowledgment enable
   init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;   // choose 7-bit address for acknowledgment
   init.I2C_ClockSpeed = clock;

   I2Cx = nullptr;

   if (i2c == I2C1)
   {
      if (Bmp280[BMP1])
      {
         return;
      }
      Bmp280[BMP1] = this;

      /* Init Pins I2Cx */
      RCC_APB2PeriphClockCmd(I2C1_PORT_CLOCK, ENABLE);
      port.GPIO_Pin = I2C1_SDA_PIN | I2C1_SCL_PIN;

      /* Enable I2Cx clock */
      RCC_APB1PeriphClockCmd(I2C1_CLOCK, ENABLE);   // Enable I2Cx clock
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
   }
   else if (i2c == I2C2)
   {
      if (Bmp280[BMP2])
      {
         return;
      }
      Bmp280[BMP2] = this;

      /* Init Pins I2Cx */
      RCC_APB2PeriphClockCmd(I2C2_PORT_CLOCK, ENABLE);
      port.GPIO_Pin = I2C2_SDA_PIN | I2C2_SCL_PIN;

      /* Enable I2Cx clock */
      RCC_APB1PeriphClockCmd(I2C2_CLOCK, ENABLE);   // Enable I2Cx clock
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
   }
   else
   {
      return;
   }

   /* Init Pins GPIO */
   GPIO_Init(I2C2_GPIO_PORT, &port);

   I2Cx = i2c;

   Clock = clock;

   Address = BMP280_ADDR;

   OSS = DEF_OSS;

   /* Init I2Cx */
   I2C_DeInit(I2Cx);   // De Init

   I2C_Cmd(I2Cx, ENABLE);   // Enable I2Cx

   I2C_Init(I2Cx, &init);   // Configure I2Cx

   int timeout = TIMEOUT;
   while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
   {   // Wait until I2Cx free
      timeout--;
      if (!(timeout))
      {
         break;
      }
   }

   Reset();

   /* Get service information */
   BmpInfo.ChipID = ReadReg(BMP280_CHIP_ID_REG);
   if (BmpInfo.ChipID == BMP280_CHIP_ID)
   {
      // Если не задали параметры датчика, то подставляем дефолтные
      if (params)
      {
         memcpy(&Params, params, sizeof(Params_t));
      }
      else
      {
         Params.Mode = BMP280_MODE_NORMAL;
         Params.Filter = BMP280_FILTER_OFF;
         Params.OversamplingTemperature = BMP280_STANDARD;
         Params.OversamplingHumidity = BMP280_STANDARD;
         Params.OversamplingPressure = BMP280_STANDARD;
         Params.Standby = BMP280_STANDBY_250;
      }
   }
   else
   {
      // Датчик не читаеться
      I2Cx = nullptr;
   }
}

/**
 * @brief Destructor
 */
Bmp::~Bmp()
{
   GPIO_InitTypeDef port;
   port.GPIO_Mode = GPIO_Mode_AIN;
   port.GPIO_Speed = GPIO_Speed_2MHz;

   if (I2Cx == I2C1)
   {
      if (Bmp280[BMP1] == nullptr)
      {
         return;
      }
      Bmp280[BMP1] = nullptr;
      port.GPIO_Pin = I2C1_SDA_PIN | I2C1_SCL_PIN;
   }
   else if (I2Cx == I2C2)
   {
      if (Bmp280[BMP2] == nullptr)
      {
         return;
      }
      Bmp280[BMP2] = nullptr;
      port.GPIO_Pin = I2C2_SDA_PIN | I2C2_SCL_PIN;
   }
   else
   {
      return;
   }

   GPIO_Init(I2C1_GPIO_PORT, &port);
   I2C_DeInit(I2Cx);   // De Init
}

/**
 * @brief Reset the I2C bus
 */
void Bmp::ResetI2C()
{
   I2C_DeInit(I2Cx);   // De Init

   I2C_InitTypeDef init;
   init.I2C_Mode = I2C_Mode_I2C;   // I2Cx mode is I2Cx
   init.I2C_DutyCycle = I2C_DutyCycle_2;   // I2Cx fast mode duty cycle (WTF is this?)
   init.I2C_OwnAddress1 = 1;   // This device address (7-bit or 10-bit)
   init.I2C_Ack = I2C_Ack_Enable;   // Acknowledgment enable
   init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;   // choose 7-bit address for acknowledgment
   init.I2C_ClockSpeed = Clock;

   I2C_Init(I2Cx, &init);   // Configure I2Cx
}

/**
 * @brief Will perform the same sequence as power on reset
 */
void Bmp::Reset()
{
   WriteReg(BMP280_SOFT_RESET_REG, 0xB6);   // Do software reset
   int timeout = TIMEOUT;
   while (timeout--)
   {
      uint8_t status = ReadReg(BMP280_STATUS);
      if (!(status & 1))
      {
         break;
      }
   }
}

/**
 * @brief Read calibration of coefficients
 */
bool Bmp::ReadCalibration()
{
   Reset();

   I2C_AcknowledgeConfig(I2Cx, ENABLE);   // Enable I2Cx acknowledge
   I2C_GenerateSTART(I2Cx, ENABLE);   // Send START condition
   int timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
   {
      timeout--;
      if (!(timeout))
      {
         return false;
      }
   }   // Wait for EV5
   I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);   // Send slave address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      timeout--;
      if (!(timeout))
      {
         return false;
      }
   }
   I2C_SendData(I2Cx, BMP280_CALIB);   // Send calibration first register address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      timeout--;
      if (!(timeout))
      {
         return false;
      }
   }
   I2C_GenerateSTART(I2Cx, ENABLE);   // Send repeated START condition (aka Re-START)
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      timeout--;
      if (!(timeout))
      {
         return false;
      }
   }
   I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Receiver);   // Send slave address for READ
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {
      timeout--;
      if (!(timeout))
      {
         return false;
      }
   }   // Wait for EV6

   uint8_t buf[BMP280_PROM_DATA_LEN];
   for (auto i = 0; i < BMP280_PROM_DATA_LEN - 1; i++)
   {
      timeout = TIMEOUT;
      while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
      {   // Wait for EV7 (Byte
          // received from slave)

         timeout--;
         if (!(timeout))
         {
            return false;
         }
      }
      buf[i] = I2C_ReceiveData(I2Cx);   // Receive byte
   }

   I2C_AcknowledgeConfig(I2Cx, DISABLE);   // Disable I2Cx acknowledgment
   I2C_GenerateSTOP(I2Cx, ENABLE);   // Send STOP condition
   I2C_ReceiveData(I2Cx);   // Receive last byte

   Calibration.T1 = (uint16_t)((buf[1] << 8) | buf[0]);
   Calibration.T2 = (int16_t)((buf[3] << 8) | buf[2]);
   Calibration.T3 = (int16_t)((buf[5] << 8) | buf[4]);

   Calibration.P1 = (uint16_t)((buf[7] << 8) | buf[6]);
   Calibration.P2 = (int16_t)((buf[9] << 8) | buf[8]);
   Calibration.P3 = (int16_t)((buf[11] << 8) | buf[10]);
   Calibration.P4 = (int16_t)((buf[13] << 8) | buf[12]);
   Calibration.P5 = (int16_t)((buf[15] << 8) | buf[14]);
   Calibration.P6 = (int16_t)((buf[17] << 8) | buf[16]);
   Calibration.P7 = (int16_t)((buf[19] << 8) | buf[18]);
   Calibration.P8 = (int16_t)((buf[21] << 8) | buf[20]);
   Calibration.P9 = (int16_t)((buf[23] << 8) | buf[22]);

   /* Hum */
   Calibration.H1 = ReadReg(BMP280_HUM_CALIB_H1);
   Calibration.H2 = (int16_t)(ReadReg(BMP280_HUM_CALIB_H2_LSB) << 8 | ReadReg(BMP280_HUM_CALIB_H2_MSB));
   Calibration.H3 = ReadReg(BMP280_HUM_CALIB_H3);

   uint16_t h4 = ReadReg(BMP280_HUM_CALIB_H4_LSB) << 8 | ReadReg(BMP280_HUM_CALIB_H4_MSB);
   uint16_t h5 = ReadReg(BMP280_HUM_CALIB_H5_LSB) << 8 | ReadReg(BMP280_HUM_CALIB_H5_MSB);

   Calibration.H4 = (int16_t)((h4 & 0x00ff) << 4 | (h4 & 0x0f00) >> 8);
   Calibration.H5 = (int16_t)(h5 >> 4);

   Calibration.H6 = (int8_t)ReadReg(BMP280_HUM_CALIB_H6);

   /* */
   uint8_t config = (Params.Standby << 5) | (Params.Filter << 2);
   WriteReg(BMP280_CONFIG, config);

   /* */
   WriteReg(BMP280_CTRL_HUM, Params.OversamplingHumidity);

   /* */
   if (Params.Mode == BMP280_MODE_FORCED)
   {
      Params.Mode = BMP280_MODE_SLEEP;   // initial mode for forced is sleep
   }

   uint8_t ctrl = (Params.OversamplingTemperature << 5) | (Params.OversamplingPressure << 2) | (Params.Mode);
   WriteReg(BMP280_CTRL, ctrl);

   return true;
}

/**
 * @brief  Returns the status of class creation.
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bmp::CreateClass()
{
   return (I2Cx != nullptr);
}

///**
// * @brief  Returns the real pressure in Pa
// * @param [out] pressure - the real pressure in Pa
// * @retval true -  Success;
// *         false - Fail.
// */
// bool Bmp::GetPressurePa(uint32_t * const pressure)
//{
//    uint32_t raw_pres;
//    if(ReadRawPressure(&raw_pres)) {
//        *pressure = CalcPressure(raw_pres);
//        return true;
//    }
//
//    return false;
//}
//
//
///**
// * @brief  Returns the real pressure in mmHg
// * @param [out] pressure - the real pressure in Pa
// * @retval true -  Success;
// *         false - Fail.
// */
// bool Bmp::GetPressureHg(uint16_t* const pressure)
//{
//    uint32_t raw_pres;
//    if(ReadRawPressure(&raw_pres)) {
//        uint32_t pa_press = CalcPressure(raw_pres);
//
//        /* Fast integer Pa -> mmHg conversion (Pascals to millimeters of mercury) */
//        *pressure = (pa_press * 75) / 10000;
//        return true;
//    }
//
//    return false;
//}

/**
 * @brief  Returns the real temperature.
 * @param [in] temperature - the real temperature
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bmp::GetTemperature(float* const temperature)
{
   int32_t raw_temp;
   if (ReadRawTemperature(&raw_temp))
   {
      float c_temp = CalcTemperature(raw_temp);
      *temperature = c_temp / 100;
      return true;
   }

   return false;
}

bool Bmp::ReadRawTemperature(int32_t* raw_temp)
{
   uint8_t temp_msb = ReadReg(BMP280_TEMP_MSB);
   uint8_t temp_lsb = ReadReg(BMP280_TEMP_LSB);
   uint8_t temp_xlsb = ReadReg(BMP280_TEMP_XLSB);
   *raw_temp = (int32_t)((temp_msb << 12) | (temp_lsb << 4) | (temp_xlsb >> 4));
   return true;
}

/**
 * @brief  Calculating temperature
 * @param [in] raw_temp - raw value
 * @retval general value
 */
int32_t Bmp::CalcTemperature(int32_t raw_temp)
{
   int32_t var1 = ((((raw_temp >> 3) - ((int32_t)Calibration.T1 << 1))) * (int32_t)Calibration.T2) >> 11;

   int32_t var2 = (((((raw_temp >> 4) - (int32_t)Calibration.T1) * ((raw_temp >> 4) - (int32_t)Calibration.T1)) >> 12) *
                   (int32_t)Calibration.T3) >>
                  14;

   int32_t fine_temp = var1 + var2;
   return (fine_temp * 5 + 128) >> 8;
}

/**
 * @brief  Write command
 * @param [in] reg - register address
 * @param [in] value - value data or command
 * @retval return data from BMP
 */
uint8_t Bmp::WriteReg(uint8_t reg, uint8_t value)
{
   int timeout = TIMEOUT;
   I2C_GenerateSTART(I2Cx, ENABLE);
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);   // Send slave address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {
      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }   // Wait for EV6
   I2C_SendData(I2Cx, reg);   // Send register address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_SendData(I2Cx, value);   // Send register value
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_GenerateSTOP(I2Cx, ENABLE);
   return value;
}

/**
 * @brief  Read register
 * @param [in] reg - register address
 * @retval return data from BMP
 */
uint8_t Bmp::ReadReg(uint8_t reg)
{
   uint8_t value;
   int timeout = TIMEOUT;

   I2C_GenerateSTART(I2Cx, ENABLE);
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);   // Send slave address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_SendData(I2Cx, reg);   // Send register address
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_GenerateSTART(I2Cx, ENABLE);   // Send repeated START condition (aka Re-START)
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Receiver);   // Send slave address for READ
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {   // Wait for EV6

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   timeout = TIMEOUT;
   while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte
       // received from slave)

      timeout--;
      if (!(timeout))
      {
         return 0;
      }
   }
   value = I2C_ReceiveData(I2Cx);   // Receive
   I2C_AcknowledgeConfig(I2Cx, DISABLE);   // Disable I2Cx acknowledgment
   I2C_GenerateSTOP(I2Cx, ENABLE);   // Send STOP condition
   return value;
}
