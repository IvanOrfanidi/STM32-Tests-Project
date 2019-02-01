/**
 ******************************************************************************
 * @file    bme180.cpp
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
#include "bme280.hpp"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "board.hpp"
#include <math.h>

/**
 * @brief Static instances of a class
 */
Bme* Bme::Bme280[Bme::BME280_MAX_COUNT];

/// Natural logarithm
static inline double ln(double x);

/**
 * @brief Сonstructor
 */
Bme::Bme(I2C_TypeDef* i2c, uint32_t clock, const Params_t* const params)
{
    GPIO_InitTypeDef port;
    port.GPIO_Mode = GPIO_Mode_AF_OD;
    port.GPIO_Speed = GPIO_Speed_50MHz;

    I2C_InitTypeDef init;
    init.I2C_Mode = I2C_Mode_I2C;                                   // I2Cx mode is I2Cx
    init.I2C_DutyCycle = I2C_DutyCycle_2;                           // I2Cx fast mode duty cycle (WTF is this?)
    init.I2C_OwnAddress1 = 1;                                       // This device address (7-bit or 10-bit)
    init.I2C_Ack = I2C_Ack_Enable;                                  // Acknowledgment enable
    init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;    // choose 7-bit address for acknowledgment
    init.I2C_ClockSpeed = clock;

    I2Cx = nullptr;

    if(i2c == I2C1) {
        if(Bme280[BME1]) {
            return;
        }
        Bme280[BME1] = this;

        /* Init Pins I2Cx */
        RCC_APB2PeriphClockCmd(I2C1_PORT_CLOCK, ENABLE);
        port.GPIO_Pin = I2C1_SDA_PIN | I2C1_SCL_PIN;

        /* Enable I2Cx clock */
        RCC_APB1PeriphClockCmd(I2C1_CLOCK, ENABLE);    // Enable I2Cx clock
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    }
    else if(i2c == I2C2) {
        if(Bme280[BME2]) {
            return;
        }
        Bme280[BME2] = this;

        /* Init Pins I2Cx */
        RCC_APB2PeriphClockCmd(I2C2_PORT_CLOCK, ENABLE);
        port.GPIO_Pin = I2C2_SDA_PIN | I2C2_SCL_PIN;

        /* Enable I2Cx clock */
        RCC_APB1PeriphClockCmd(I2C2_CLOCK, ENABLE);    // Enable I2Cx clock
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    }
    else {
        return;
    }

    /* Init Pins GPIO */
    GPIO_Init(I2C2_GPIO_PORT, &port);

    I2Cx = i2c;

    Clock = clock;

    Address = BME280_ADDR;

    /* Init I2Cx */
    I2C_DeInit(I2Cx);         // De Init
    I2C_Cmd(I2Cx, ENABLE);    // Enable I2Cx
    I2C_Init(I2Cx, &init);    // Configure I2Cx

    int timeout = TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY)) {
        timeout--;
        if(!(timeout)) {
            break;
        }
    }

    Reset();

    /* Get service information */
    BmeInfo.ChipID = ReadReg(BME280_CHIP_ID_REG);
    if(BmeInfo.ChipID == BME280_CHIP_ID) {
        // Если не задали параметры датчика, то подставляем дефолтные
        if(params) {
            memcpy(&Params, params, sizeof(Params_t));
        }
        else {
            Params.Mode = BME280_MODE_NORMAL;
            Params.Filter = BME280_FILTER_OFF;
            Params.OversamplingTemperature = BME280_STANDARD;
            Params.OversamplingHumidity = BME280_STANDARD;
            Params.OversamplingPressure = BME280_STANDARD;
            Params.Standby = BME280_STANDBY_500;
        }
    }
    else {
        // Датчик не читаеться
        I2Cx = nullptr;
    }
}

/**
 * @brief Destructor
 */
Bme::~Bme()
{
    GPIO_InitTypeDef port;
    port.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    port.GPIO_Speed = GPIO_Speed_2MHz;

    if(I2Cx == I2C1) {
        if(Bme280[BME1] == nullptr) {
            return;
        }

        Bme280[BME1] = nullptr;
        port.GPIO_Pin = I2C1_SDA_PIN | I2C1_SCL_PIN;
    }
    else if(I2Cx == I2C2) {
        if(Bme280[BME2] == nullptr) {
            return;
        }
        Bme280[BME2] = nullptr;
        port.GPIO_Pin = I2C2_SDA_PIN | I2C2_SCL_PIN;
    }
    else {
        return;
    }

    GPIO_Init(I2C1_GPIO_PORT, &port);
    I2C_DeInit(I2Cx);    // De Init
}

/**
 * @brief Reset the I2C bus
 */
void Bme::ResetI2C()
{
    I2C_DeInit(I2Cx);    // De Init

    I2C_InitTypeDef init;
    init.I2C_Mode = I2C_Mode_I2C;                                   // I2Cx mode is I2Cx
    init.I2C_DutyCycle = I2C_DutyCycle_2;                           // I2Cx fast mode duty cycle (WTF is this?)
    init.I2C_OwnAddress1 = 1;                                       // This device address (7-bit or 10-bit)
    init.I2C_Ack = I2C_Ack_Enable;                                  // Acknowledgment enable
    init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;    // choose 7-bit address for acknowledgment
    init.I2C_ClockSpeed = Clock;

    I2C_Init(I2Cx, &init);    // Configure I2Cx
}

/**
 * @brief Will perform the same sequence as power on reset
 */
void Bme::Reset()
{
    WriteReg(BME280_SOFT_RESET_REG, 0xB6);    // Do software reset
    int timeout = TIMEOUT;
    while(timeout--) {
        const uint8_t status = ReadReg(BME280_STATUS);
        if(!(status & 1)) {
            break;
        }
    }
}

/**
 * @brief Read calibration of coefficients
 */
bool Bme::ReadCalibration()
{
    Reset();

    I2C_AcknowledgeConfig(I2Cx, ENABLE);    // Enable I2Cx acknowledge
    I2C_GenerateSTART(I2Cx, ENABLE);        // Send START condition
    int timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        timeout--;
        if(!(timeout)) {
            return false;
        }
    }

    I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);    // Send slave address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {    // Wait for EV6
        timeout--;
        if(!(timeout)) {
            return false;
        }
    }

    I2C_SendData(I2Cx, BME280_PROM_START_ADDR);    // Send calibration first register address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        timeout--;
        if(!(timeout)) {
            return false;
        }
    }

    I2C_GenerateSTART(I2Cx, ENABLE);    // Send repeated START condition (aka Re-START)
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {    // Wait for EV5
        timeout--;
        if(!(timeout)) {
            return false;
        }
    }

    I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Receiver);    // Send slave address for READ
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        timeout--;
        if(!(timeout)) {
            return false;
        }
    }

    uint8_t buf[BME280_PROM_DATA_LEN];
    for(size_t i = 0; i < BME280_PROM_DATA_LEN; i++) {
        timeout = TIMEOUT;
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            timeout--;
            if(!(timeout)) {
                return false;
            }
        }
        buf[i] = I2C_ReceiveData(I2Cx);    // Receive byte
    }

    I2C_AcknowledgeConfig(I2Cx, DISABLE);    // Disable I2Cx acknowledgment
    I2C_GenerateSTOP(I2Cx, ENABLE);          // Send STOP condition
    I2C_ReceiveData(I2Cx);                   // Receive last byte

    /* Temperature */
    Calibration.T1 = (uint16_t)((buf[1] << 8) | buf[0]);
    Calibration.T2 = (int16_t)((buf[3] << 8) | buf[2]);
    Calibration.T3 = (int16_t)((buf[5] << 8) | buf[4]);

    /* Pressure */
    Calibration.P1 = (uint16_t)((buf[7] << 8) | buf[6]);
    Calibration.P2 = (int16_t)((buf[9] << 8) | buf[8]);
    Calibration.P3 = (int16_t)((buf[11] << 8) | buf[10]);
    Calibration.P4 = (int16_t)((buf[13] << 8) | buf[12]);
    Calibration.P5 = (int16_t)((buf[15] << 8) | buf[14]);
    Calibration.P6 = (int16_t)((buf[17] << 8) | buf[16]);
    Calibration.P7 = (int16_t)((buf[19] << 8) | buf[18]);
    Calibration.P8 = (int16_t)((buf[21] << 8) | buf[20]);
    Calibration.P9 = (int16_t)((buf[23] << 8) | buf[22]);

    /* Humidity */
    Calibration.H1 = ReadReg(BME280_HUM_CALIB_H1);

    Calibration.H2 = (int16_t)(ReadReg(BME280_HUM_CALIB_H2_MSB) << 8 | ReadReg(BME280_HUM_CALIB_H2_LSB));
    Calibration.H3 = ReadReg(BME280_HUM_CALIB_H3);

    const uint16_t h4 = ReadReg(BME280_HUM_CALIB_H4_MSB) << 8 | ReadReg(BME280_HUM_CALIB_H4_LSB);
    const uint16_t h5 = ReadReg(BME280_HUM_CALIB_H5_MSB) << 8 | ReadReg(BME280_HUM_CALIB_H5_LSB);
    Calibration.H4 = (int16_t)((h4 & 0x00FF) << 4 | (h4 & 0x0F00) >> 8);
    Calibration.H5 = (int16_t)(h5 >> 4);

    Calibration.H6 = (int8_t)ReadReg(BME280_HUM_CALIB_H6);

    /* */
    const uint8_t config = (Params.Standby << 5) | (Params.Filter << 2);
    WriteReg(BME280_CONFIG, config);

    /* */
    WriteReg(BME280_CTRL_HUM, Params.OversamplingHumidity);

    /* */
    if(Params.Mode == BME280_MODE_FORCED) {
        Params.Mode = BME280_MODE_SLEEP;    // initial mode for forced is sleep
    }

    const uint8_t ctrl = (Params.OversamplingTemperature << 5) | (Params.OversamplingPressure << 2) | (Params.Mode);
    WriteReg(BME280_CTRL, ctrl);

    return true;
}

/**
 * @brief Returns the status of class creation
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::CreateClass() const
{
    return (I2Cx != nullptr);
}

/**
 * @brief For given air pressure and sea level air pressure returns the altitude 
 *  in meters i.e. altimeter function
 * @param [in] qfe - the QFE pressure
 * @param [in] qnh - the QNH pressure
 * @retval altitude in meters
 */
int Bme::GetAltitude(uint32_t qfe, uint32_t qnh) const
{
    const double height = (1.0 - pow((double)qfe / (double)qnh, 1.0 / 5.25588)) / 2.25577e-5 * 100.0;
    return (int)(height / 100);
}

/**
 * @brief Natural logarithm
 */
static inline double ln(double x)
{
    const double y = (x - 1) / (x + 1);
    const double y2 = y * y;
    double r = 0;

    for(int i = 33; i > 0; i -= 2) {
        r = 1.0 / i + y2 * r;
    }

    return (2 * y * r);
}

/**
 * @brief For given temperature and relative humidity returns the dew point 
 *  in celsius.
 * @param [in] hum - humidity
 * @param [in] temp - temperature
 * @retval dew point in celsius
 */
float Bme::GetDewpoint(float hum, float temp) const
{
    hum /= 100;
    const double x = 243.5 * (((17.67 * temp) / (243.5 + temp)) + ln(hum));
    const double y = 17.67 - (((17.67 * temp) / (243.5 + temp)) + ln(hum));
    const double dewpoint = x / y;

    return dewpoint;
}

/**
 * @brief Fast integer Pa -> mmHg conversion (Pascals to millimeters of mercury)
 * @param [in] ppa - the QNH pressure in Pa
 * @retval pressure in mmHq
 */
uint16_t Bme::Pa2mmHg(uint32_t ppa) const
{
    return ((ppa * 75) / 10000);
}

/**
 * @brief For given altitude converts the air pressure to sea level air pressure.
 * @param [in] qfe - the QFE pressure
 * @param [in] alt - the altitude of the measurement place in meters
 * @retval pressure QNH(давление на уровне моря в точке измерения)
 */
double Bme::Qfe2Qnh(uint32_t qfe, int32_t alt) const
{
    // pow(a,b) - возведение  а в степень b
    const double height = pow((double)(1.0 - 2.25577e-5 * alt), (double)(-5.25588));

    return ((double)qfe * height);
}

/**
 * @brief Returns the QNH pressure(давление на уровне моря в точке измерения)
 * @param [in] alt - the altitude of the measurement place in meters 
 * @param [out] ppa - the QNH pressure in Pa
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::GetQnhPressure(uint32_t* const ppa, int32_t alt)
{
    int32_t raw_pres;
    if(ReadRawPressure(&raw_pres)) {
        const double pressure = Qfe2Qnh(CalcPressure(raw_pres), alt);
        *ppa = (uint32_t)pressure;

        return true;
    }
    return false;
}

/**
 * @brief Returns the QFE pressure(давление измеренное в точке измерения)
 * @param [out] pressure - the QFE pressure in Pa
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::GetQfePressure(uint32_t* const ppa)
{
    int32_t raw_pres;
    if(ReadRawPressure(&raw_pres)) {
        const uint32_t pressure = CalcPressure(raw_pres);
        *ppa = pressure;

        return true;
    }
    return false;
}

/**
 * @brief Returns the real humidity.
 * @param [out] humidity - the real humidity
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::GetHumidity(float* const humidity)
{
    uint32_t raw_hum;
    if(ReadRawHumidity(&raw_hum)) {
        const float cal_hum = CalcHumidity(raw_hum);
        *humidity = cal_hum / 1024;

        return true;
    }

    return false;
}

/**
 * @brief Get fine temperature
 * @retval fine temperature
 */
int32_t Bme::GetFineTemperature()
{
    int32_t raw_temp;
    ReadRawTemperature(&raw_temp);
    const int32_t var1 = ((((raw_temp >> 3) - ((int32_t)Calibration.T1 << 1))) * (int32_t)Calibration.T2) >> 11;

    const int32_t var2 = (((((raw_temp >> 4) - (int32_t)Calibration.T1) *
                               ((raw_temp >> 4) - (int32_t)Calibration.T1)) >>
                              12) *
                             (int32_t)Calibration.T3) >>
                         14;

    return (var1 + var2);
}

/**
 * @brief Returns the real temperature.
 * @param [out] temperature - the real temperature
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::GetTemperature(float* const temperature)
{
    int32_t raw_temp;
    if(ReadRawTemperature(&raw_temp)) {
        const float c_temp = CalcTemperature(raw_temp);
        *temperature = c_temp / 100;
        return true;
    }
    return false;
}

/**
 * @brief Returns the RAW humidity.
 * @param [out] raw_hum - the RAW humidity
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::ReadRawHumidity(uint32_t* raw_hum)
{
    const uint8_t hum_msb = ReadReg(BME280_HUM_MSB);
    const uint8_t hum_lsb = ReadReg(BME280_HUM_LSB);

    if((hum_msb == 0x80) && (hum_lsb == 0x00)) {
        return false;
    }

    *raw_hum = (uint32_t)((hum_msb << 8) | (hum_lsb));
    return true;
}

/**
 * @brief Returns the RAW pressure.
 * @param [out] raw_pres - the RAW pressure
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::ReadRawPressure(int32_t* raw_pres)
{
    const uint8_t pres_msb = ReadReg(BME280_PRESS_MSB);
    const uint8_t pres_lsb = ReadReg(BME280_PRESS_LSB);
    const uint8_t pres_xlsb = ReadReg(BME280_PRESS_XLSB);

    if((pres_msb == 0x80) && (pres_lsb == 0x00) && (pres_xlsb == 0x00)) {
        return false;
    }

    *raw_pres = (int32_t)((pres_msb << 12) | (pres_lsb << 4) | (pres_xlsb >> 4));
    return true;
}

/**
 * @brief Returns the RAW temperature.
 * @param [out] raw_temp - the RAW temperature
 * @retval true -  Success;
 *         false - Fail.
 */
bool Bme::ReadRawTemperature(int32_t* raw_temp)
{
    const uint8_t temp_msb = ReadReg(BME280_TEMP_MSB);
    const uint8_t temp_lsb = ReadReg(BME280_TEMP_LSB);
    const uint8_t temp_xlsb = ReadReg(BME280_TEMP_XLSB);

    if((temp_msb == 0x80) && (temp_lsb == 0x00) && (temp_xlsb == 0x00)) {
        return false;
    }

    *raw_temp = (int32_t)((temp_msb << 12) | (temp_lsb << 4) | (temp_xlsb >> 4));
    return true;
}

/**
 * @brief Calculating humidity
 * @param [in] raw_hum - raw humidity
 * @retval general value
 */
uint32_t Bme::CalcHumidity(uint32_t raw_hum)
{
    const int32_t fine_temp = GetFineTemperature();

    int32_t v_x1 = fine_temp - (int32_t)76800;
    v_x1 = ((((raw_hum << 14) - ((int32_t)Calibration.H4 << 20) - ((int32_t)Calibration.H5 * v_x1)) + (int32_t)16384) >> 15) * (((((((v_x1 * (int32_t)Calibration.H6) >> 10) * (((v_x1 * (int32_t)Calibration.H3) >> 11) + (int32_t)32768)) >> 10) + (int32_t)2097152) * (int32_t)Calibration.H2 + 8192) >> 14);

    v_x1 = v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int32_t)Calibration.H1) >> 4);
    return (v_x1 >> 12);
}

/**
 * @brief Calculating pressure
 * @param [in] raw_pres - raw value
 * @retval general value
 */
uint32_t Bme::CalcPressure(int32_t raw_pres)
{
    const int32_t fine_temp = GetFineTemperature();

    int64_t var1 = (int64_t)fine_temp - 128000;
    int64_t var2 = var1 * var1 * (int64_t)Calibration.P6;
    var2 = var2 + ((var1 * (int64_t)Calibration.P5) << 17);
    var2 = var2 + (((int64_t)Calibration.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)Calibration.P3) >> 8) + ((var1 * (int64_t)Calibration.P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)Calibration.P1) >> 33;

    if(var1 == 0) {
        return 0;    // avoid exception caused by division by zero
    }

    int64_t pres = 1048576 - raw_pres;
    pres = (((pres << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)Calibration.P9 * (pres >> 13) * (pres >> 13)) >> 25;
    var2 = ((int64_t)Calibration.P8 * pres) >> 19;

    pres = ((pres + var1 + var2) >> 8) + ((int64_t)Calibration.P7 << 4);
    return (pres / 256);
}

/**
 * @brief Calculating temperature
 * @param [in] raw_temp - raw value
 * @retval general value
 */
int32_t Bme::CalcTemperature(int32_t raw_temp) const
{
    const int32_t var1 = ((((raw_temp >> 3) - ((int32_t)Calibration.T1 << 1))) * (int32_t)Calibration.T2) >> 11;

    const int32_t var2 = (((((raw_temp >> 4) - (int32_t)Calibration.T1) *
                               ((raw_temp >> 4) - (int32_t)Calibration.T1)) >>
                              12) *
                             (int32_t)Calibration.T3) >>
                         14;

    int32_t fine_temp = var1 + var2;
    return ((fine_temp * 5 + 128) >> 8);
}

/**
 * @brief Write command
 * @param [in] reg - register address
 * @param [in] value - value data or command
 * @retval return data from BME
 */
uint8_t Bme::WriteReg(uint8_t reg, uint8_t value)
{
    int timeout = TIMEOUT;
    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);    // Send slave address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_SendData(I2Cx, reg);    // Send register address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_SendData(I2Cx, value);    // Send register value
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);
    return value;
}

/**
 * @brief Read register
 * @param [in] reg - register address
 * @retval return data from BME
 */
uint8_t Bme::ReadReg(uint8_t reg)
{
    int timeout = TIMEOUT;
    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Transmitter);    // Send slave address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_SendData(I2Cx, reg);    // Send register address
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_GenerateSTART(I2Cx, ENABLE);    // Send repeated START condition (aka Re-START)
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    I2C_Send7bitAddress(I2Cx, Address, I2C_Direction_Receiver);    // Send slave address for READ
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }
    timeout = TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
        timeout--;
        if(!(timeout)) {
            return 0;
        }
    }

    const uint8_t value = I2C_ReceiveData(I2Cx);    // Receive
    I2C_AcknowledgeConfig(I2Cx, DISABLE);           // Disable I2Cx acknowledgment
    I2C_GenerateSTOP(I2Cx, ENABLE);                 // Send STOP condition
    return value;
}