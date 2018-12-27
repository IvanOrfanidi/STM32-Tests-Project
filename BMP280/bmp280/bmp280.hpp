/**
 ******************************************************************************
 * @file    bmp280.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMP280_HPP
#define __BMP280_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_i2c.h"
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _delay_ms(__a) Board::DelayMS(__a)

#define I2C1_PORT_CLOCK RCC_APB2Periph_GPIOB
#define I2C1_SCL_PIN GPIO_Pin_6   // PB6
#define I2C1_SDA_PIN GPIO_Pin_7   // PB7
#define I2C1_GPIO_PORT GPIOB
#define I2C1_CLOCK RCC_APB1Periph_I2C1

#define I2C2_PORT_CLOCK RCC_APB2Periph_GPIOB
#define I2C2_SCL_PIN GPIO_Pin_10   // PB10
#define I2C2_SDA_PIN GPIO_Pin_11   // PB11
#define I2C2_GPIO_PORT GPIOB
#define I2C2_CLOCK RCC_APB1Periph_I2C2

/**
 * @brief Class sensor type BMP280. Pressure and temperature.
 */
class Bmp
{
 public:
   /// Перечисление режимов преобразования датчика
   enum OSS_t
   {
      OSS0,   ///< ultra low power (4.5ms)
      OSS1,   ///< standard (7.5ms)
      OSS2,   ///< high resolution (13.5ms)
      OSS3,   ///< ultra high resoultion (25.5ms)

      DEF_OSS = OSS1
   };

   /**
    * Mode of BMP280 module operation.
    * Forced - Measurement is initiated by user.
    * Normal - Continues measurement.
    */
   enum BmpMode_t
   {
      BMP280_MODE_SLEEP = 0,
      BMP280_MODE_FORCED = 1,
      BMP280_MODE_NORMAL = 3,
   };

   //
   enum BmpFilter_t
   {
      BMP280_FILTER_OFF = 0,
      BMP280_FILTER_2 = 1,
      BMP280_FILTER_4 = 2,
      BMP280_FILTER_8 = 3,
      BMP280_FILTER_16 = 4
   };

   /**
    * Pressure oversampling settings
    */
   enum BmpOversampling_t
   {
      BMP280_SKIPPED = 0,   // no measurement
      BMP280_ULTRA_LOW_POWER = 1,   // oversampling x1
      BMP280_LOW_POWER = 2,   // oversampling x2
      BMP280_STANDARD = 3,   // oversampling x4
      BMP280_HIGH_RES = 4,   // oversampling x8
      BMP280_ULTRA_HIGH_RES = 5   // oversampling x16
   };

   /**
    * Stand by time between measurements in normal mode
    */
   enum BmpStandbyTime_t
   {
      BMP280_STANDBY_05 = 0,   // stand by time 0.5ms
      BMP280_STANDBY_62 = 1,   // stand by time 62.5ms
      BMP280_STANDBY_125 = 2,   // stand by time 125ms
      BMP280_STANDBY_250 = 3,   // stand by time 250ms
      BMP280_STANDBY_500 = 4,   // stand by time 500ms
      BMP280_STANDBY_1000 = 5,   // stand by time 1s
      BMP280_STANDBY_2000 = 6,   // stand by time 2s BMP280, 10ms BME280
      BMP280_STANDBY_4000 = 7,   // stand by time 4s BMP280, 20ms BME280
   };

   /**
    * Configuration parameters for BMP280 module.
    * Use function bmp280_init_default_params to use default configuration.
    */
   struct Params_t
   {
      BmpMode_t Mode;
      BmpFilter_t Filter;
      BmpOversampling_t OversamplingPressure;
      BmpOversampling_t OversamplingTemperature;
      BmpOversampling_t OversamplingHumidity;
      BmpStandbyTime_t Standby;
   };

   /// Сonstructor
   Bmp(I2C_TypeDef*, uint32_t, const Params_t* const params = nullptr);

   virtual ~Bmp();   /// Destructor

   bool GetPressurePa(uint32_t* const);   /// Get real pressure in Pa

   bool GetPressureHg(uint16_t* const);   /// Get real pressure in Pa

   bool GetTemperature(float* const);   /// Get real temperature

   bool GetHumidity(float* const);   /// Get real humidity

   bool CreateClass();   /// Returns the status of class creation

   void Reset();   /// Reset sensor BMP

   void ResetI2C();   /// Reset the I2C bus

   bool ReadCalibration();   /// Read calibration parameters

   /// Установка режима преобразования датчика
   bool SetOSS(OSS_t oss)   /// Set OSS
   {
      /*
      (OSS=0, 4.5ms)
      (OSS=1, 7.5ms)
      (OSS=2, 13.5ms)
      (OSS=3, 25.5ms)
      */
      if (oss > OSS3)
      {
         return false;
      }

      OSS = oss;
      return true;
   }

   /// Получение режима преобразования датчика
   OSS_t GetOSS() const   /// Get OSS
   {
      return OSS;
   }

 private:
   /// BMP280 defines
   enum TimeoutDefines_t
   {
      TIMEOUT = 500
   };

   /**
    * BMP280 or BME280 address is 0x77 if SDO pin is high, and is 0x76 if
    * SDO pin is low.
    */
   enum BmpDefines_t : uint8_t
   {
      BMP280_ADDR = 0xEE,       ///< BMP280 address
      BMP280_CHIP_ID = 0x60     ///< Chip-id. This value is fixed to 0x55.
   };

   /// Length
   enum BmpLengthDefines_t
   {
      BMP280_PROM_DATA_LEN = 24,   ///< E2PROM length
   };

   /// BMP280 registers
   enum BmpRegisters_t : uint8_t
   {
      BMP280_PROM_START_ADDR = 0x88,    ///< E2PROM calibration data start register
      BMP280_CHIP_ID_REG = 0xD0,        ///< Chip ID
      BMP280_HUM_LSB = 0xFE,
      BMP280_HUM_MSB = 0xFD,
      BMP280_TEMP_XLSB = 0xFC,          ///< bits: 7-4
      BMP280_TEMP_LSB = 0xFB,
      BMP280_TEMP_MSB = 0xFA,
      BMP280_TEMP = BMP280_TEMP_MSB,
      BMP280_PRESS_XLSB = 0xF9,         ///< bits: 7-4
      BMP280_PRESS_LSB = 0xF8,
      BMP280_PRESS_MSB = 0xF7,
      BMP280_PRESSURE = BMP280_PRESS_MSB,
      BMP280_CONFIG = 0xF5,             ///< bits: 7-5 t_sb; 4-2 filter; 0 spi3w_en
      BMP280_CTRL = 0xF4,               ///< bits: 7-5 osrs_t; 4-2 osrs_p; 1-0 mode
      BMP280_STATUS = 0xF3,             ///< bits: 3 measuring; 0 im_update
      BMP280_CTRL_HUM = 0xF2,           ///< bits: 2-0 osrs_h;
      BMP280_CALIB = 0x88,              ///< E2PROM calibration data start register
      BMP280_HUM_CALIB_H1 = 0xA1,
      BMP280_HUM_CALIB_H2_LSB = 0xE1,
      BMP280_HUM_CALIB_H2_MSB = 0xE2,

      BMP280_HUM_CALIB_H3 = 0xE3,
      BMP280_HUM_CALIB_H4_MSB = 0xE4,
      BMP280_HUM_CALIB_H4_LSB = 0xE5,
      BMP280_HUM_CALIB_H5_MSB = 0xE5,
      BMP280_HUM_CALIB_H5_LSB = 0xE6,
      BMP280_HUM_CALIB_H6 = 0xE7,

      BMP280_SOFT_RESET_REG = 0xE0,   ///< Soft reset control

   };

   /// BMP280 control values
   enum BmpControlValues_t : uint8_t
   {
      BMP280_T_MEASURE = 0x2E,   ///< temperature measurement
      BMP280_P0_MEASURE = 0x34,   ///< pressure measurement (OSS=0, 4.5ms)
      BMP280_P1_MEASURE = 0x74,   ///< pressure measurement (OSS=1, 7.5ms)
      BMP280_P2_MEASURE = 0xB4,   ///< pressure measurement (OSS=2, 13.5ms)
      BMP280_P3_MEASURE = 0xF4,   ///< pressure measurement (OSS=3, 25.5ms)
   };

   /// Calibration parameters structure
   struct BmpCalibration_t
   {
      uint16_t T1;
      int16_t T2;
      int16_t T3;

      uint16_t P1;
      int16_t P2;
      int16_t P3;
      int16_t P4;
      int16_t P5;
      int16_t P6;
      int16_t P7;
      int16_t P8;
      int16_t P9;

      /* Humidity compensation for BME280 */
      uint8_t H1;
      int16_t H2;
      uint8_t H3;
      int16_t H4;
      int16_t H5;
      int8_t H6;

      BmpCalibration_t()
      {
         T1 = 0;
         T2 = 0;
         T3 = 0;
         P1 = 0;
         P2 = 0;
         P3 = 0;
         P4 = 0;
         P5 = 0;
         P6 = 0;
         P7 = 0;
         P8 = 0;
         P9 = 0;
         H1 = 0;
         H2 = 0;
         H3 = 0;
         H4 = 0;
         H5 = 0;
         H6 = 0;
      }
   };

   /// Bmp Information
   struct BmpInformation_t
   {
      uint8_t ChipID;   ///< Chip ID. This value is fixed to 0x55
   };

   /// Count BMP sensors
   enum Bmp_t
   {
      BMP1,
      BMP2,

      BMP280_MAX_COUNT
   };

   bool ReadRawTemperature(int32_t*);
   
   bool ReadRawPressure(int32_t*);
   
   bool ReadRawHumidity(uint32_t*);
   
   int32_t GetFineTemperature();

   int32_t CalcTemperature(int32_t);
   
   uint32_t CalcPressure(int32_t);
   
   uint32_t CalcHumidity(uint32_t);

   uint8_t WriteReg(uint8_t, uint8_t);

   uint8_t ReadReg(uint8_t);

   BmpCalibration_t Calibration;   ///< Calibration parameters from E2PROM of BMP280

   BmpInformation_t BmpInfo;   ///< Service information about BMP280

   OSS_t OSS;   ///< Режима преобразования датчика

   I2C_TypeDef* I2Cx;   ///< Указатель на экзепляр рабочей шины

   Params_t Params;   ///< Параметры настроек датчика

   uint8_t Address;   ///< Адрес датчика на шине

   uint32_t Clock;   ///< Baud Rate шины I2C

   static Bmp* Bmp280[BMP280_MAX_COUNT];   ///< Статические экземпляры класса
};

#endif
