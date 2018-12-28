/**
 ******************************************************************************
 * @file    bme280.hpp
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
#ifndef __BME280_HPP
#define __BME280_HPP


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_i2c.h"
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
 * @brief Class sensor type BME280. Pressure and temperature.
 */
class Bme
{
    public:

        /**
        * Mode of BME280 module operation.
        * Forced - Measurement is initiated by user.
        * Normal - Continues measurement.
        */
        enum BmeMode_t
        {
            BME280_MODE_SLEEP = 0,
            BME280_MODE_FORCED = 1,
            BME280_MODE_NORMAL = 3,
        };

        //
        enum BmeFilter_t
        {
            BME280_FILTER_OFF = 0,
            BME280_FILTER_2 = 1,
            BME280_FILTER_4 = 2,
            BME280_FILTER_8 = 3,
            BME280_FILTER_16 = 4
        };

        /**
        * Pressure oversampling settings
        */
        enum BmeOversampling_t
        {
            BME280_SKIPPED = 0,           ///< no measurement
            BME280_ULTRA_LOW_POWER = 1,   ///< oversampling x1
            BME280_LOW_POWER = 2,         ///< oversampling x2
            BME280_STANDARD = 3,          ///< oversampling x4
            BME280_HIGH_RES = 4,          ///< oversampling x8
            BME280_ULTRA_HIGH_RES = 5     ///< oversampling x16
        };

        /**
        * Stand by time between measurements in normal mode
        */
        enum BmeStandbyTime_t
        {
            BME280_STANDBY_05 = 0,    ///< stand by time 0.5ms
            BME280_STANDBY_62 = 1,    ///< stand by time 62.5ms
            BME280_STANDBY_125 = 2,   ///< stand by time 125ms
            BME280_STANDBY_250 = 3,   ///< stand by time 250ms
            BME280_STANDBY_500 = 4,   ///< stand by time 500ms
            BME280_STANDBY_1000 = 5,  ///< stand by time 1s
            BME280_STANDBY_2000 = 6,  ///< stand by time 2s BME280, 10ms BME280
            BME280_STANDBY_4000 = 7,  ///< stand by time 4s BME280, 20ms BME280
        };

        /**
        * Configuration parameters for BME280 module.
        * Use function bme280_init_default_params to use default configuration.
        */
        struct Params_t
        {
            BmeMode_t Mode;
            BmeFilter_t Filter;
            BmeOversampling_t OversamplingPressure;
            BmeOversampling_t OversamplingTemperature;
            BmeOversampling_t OversamplingHumidity;
            BmeStandbyTime_t Standby;
        };

        /// Сonstructor
        Bme(I2C_TypeDef*, uint32_t, const Params_t* const params = nullptr);

        virtual ~Bme();   /// Destructor

        bool GetPressurePa(uint32_t* const);     /// Get real pressure in Pa

        bool GetPressureHg(uint16_t* const);     /// Get real pressure in Pa

        bool GetTemperature(float* const);       /// Get real temperature

        bool GetHumidity(float* const);          /// Get real humidity

        bool CreateClass() const;    /// Returns the status of class creation

        void Reset();                /// Reset sensor BME

        void ResetI2C();             /// Reset the I2C bus

        bool ReadCalibration();      /// Read calibration parameters

    private:
 
    /// BME280 defines
    enum TimeoutDefines_t
    {
        TIMEOUT = 500
    };

    /**
    * BME280 or BME280 address is 0x77 if SDO pin is high, and is 0x76 if
    * SDO pin is low.
    */
    enum BmeDefines_t : uint8_t
    {
        BME280_ADDR = 0xEE,       ///< BME280 address
        BME280_CHIP_ID = 0x60     ///< Chip-id. This value is fixed to 0x55.
    };

    /// Length
    enum BmeLengthDefines_t
    {
        BME280_PROM_DATA_LEN = 24,   ///< E2PROM length
    };

    /// BME280 registers
    enum BmeRegisters_t : uint8_t
    {
        BME280_PROM_START_ADDR = 0x88,    ///< E2PROM calibration data start register
        BME280_CHIP_ID_REG = 0xD0,        ///< Chip ID
        BME280_HUM_LSB = 0xFE,
        BME280_HUM_MSB = 0xFD,
        BME280_TEMP_XLSB = 0xFC,          ///< bits: 7-4
        BME280_TEMP_LSB = 0xFB,
        BME280_TEMP_MSB = 0xFA,
        BME280_TEMP = BME280_TEMP_MSB,
        BME280_PRESS_XLSB = 0xF9,         ///< bits: 7-4
        BME280_PRESS_LSB = 0xF8,
        BME280_PRESS_MSB = 0xF7,
        BME280_PRESSURE = BME280_PRESS_MSB,
        BME280_CONFIG = 0xF5,             ///< bits: 7-5 t_sb; 4-2 filter; 0 spi3w_en
        BME280_CTRL = 0xF4,               ///< bits: 7-5 osrs_t; 4-2 osrs_p; 1-0 mode
        BME280_STATUS = 0xF3,             ///< bits: 3 measuring; 0 im_update
        BME280_CTRL_HUM = 0xF2,           ///< bits: 2-0 osrs_h;
        BME280_HUM_CALIB_H1 = 0xA1,
        BME280_HUM_CALIB_H2_LSB = 0xE1,
        BME280_HUM_CALIB_H2_MSB = 0xE2,
        BME280_HUM_CALIB_H3 = 0xE3,
        BME280_HUM_CALIB_H4_MSB = 0xE5,   ///< H4[11:4 3:0] = 0xE4[7:0] 0xE5[3:0] 12-bit signed
        BME280_HUM_CALIB_H4_LSB = 0xE4,
        BME280_HUM_CALIB_H5_MSB = 0xE6,   ///< H5[11:4 3:0] = 0xE6[7:0] 0xE5[7:4] 12-bit signed
        BME280_HUM_CALIB_H5_LSB = 0xE5,
        BME280_HUM_CALIB_H6 = 0xE7,
        BME280_SOFT_RESET_REG = 0xE0,   ///< Soft reset control
    };

    /// BME280 control values
    enum BmeControlValues_t : uint8_t
    {
        BME280_T_MEASURE = 0x2E,   ///< temperature measurement
        BME280_P0_MEASURE = 0x34,   ///< pressure measurement (OSS=0, 4.5ms)
        BME280_P1_MEASURE = 0x74,   ///< pressure measurement (OSS=1, 7.5ms)
        BME280_P2_MEASURE = 0xB4,   ///< pressure measurement (OSS=2, 13.5ms)
        BME280_P3_MEASURE = 0xF4,   ///< pressure measurement (OSS=3, 25.5ms)
    };

    /// Calibration parameters structure
    struct BmeCalibration_t
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

        BmeCalibration_t()
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

    /// Bme Information
    struct BmeInformation_t
    {
        uint8_t ChipID;   ///< Chip ID. This value is fixed to 0x55
    };

    /// Count BME sensors
    enum Bme_t
    {
        BME1,
        BME2,

        BME280_MAX_COUNT
    };

    bool ReadRawTemperature(int32_t*);

    bool ReadRawPressure(int32_t*);

    bool ReadRawHumidity(uint32_t*);

    int32_t GetFineTemperature();

    int32_t CalcTemperature(int32_t) const;

    uint32_t CalcPressure(int32_t);

    uint32_t CalcHumidity(uint32_t);

    uint8_t WriteReg(uint8_t, uint8_t);

    uint8_t ReadReg(uint8_t);

    BmeCalibration_t Calibration;   ///< Calibration parameters from E2PROM of BME280

    BmeInformation_t BmeInfo;   ///< Service information about BME280

    I2C_TypeDef* I2Cx;   ///< Указатель на экзепляр рабочей шины

    Params_t Params;   ///< Параметры настроек датчика

    uint8_t Address;   ///< Адрес датчика на шине

    uint32_t Clock;   ///< Baud Rate шины I2C

    static Bme* Bme280[BME280_MAX_COUNT];   ///< Статические экземпляры класса
};

#endif
