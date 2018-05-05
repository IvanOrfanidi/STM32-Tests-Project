#ifndef DS3231_H_
#define DS3231_H_

//#include "includes.h"
//#include "rtc.h"
#ifndef _I2C_PORT
#   define _I2C_PORT 1
#endif

#if _I2C_PORT == 1
#   define I2C_PORT I2C1
#   define I2C_SCL_PIN GPIO_Pin_6   // PB6
#   define I2C_SDA_PIN GPIO_Pin_7   // PB7
#   define I2C_GPIO_PORT GPIOB
#   define I2C_CLOCK RCC_APB1Periph_I2C1
#elif _I2C_PORT == 2
#   define I2C_PORT I2C2
#   define I2C_SCL_PIN GPIO_Pin_10   // PB10
#   define I2C_SDA_PIN GPIO_Pin_11   // PB11
#   define I2C_GPIO_PORT GPIOB
#   define I2C_CLOCK RCC_APB1Periph_I2C2
#endif

#define DS3231_addr 0xD0   // I2C 7-bit slave address shifted for 1 bit to the left
#define DS3231_seconds 0x00   // DS3231 seconds address
#define DS3231_control 0x0E   // DS3231 control register address
#define DS3231_tmp_MSB 0x11   // DS3231 temperature MSB
#define DS3231_alarm1 0x07   // DS3231 alarm1 address

#define A1IE (1 << 0)
#define A2IE (1 << 1)
#define INTCN (1 << 2)
#define CONV (1 << 5)
#define BBSQW (1 << 6)
#define EOSC (1 << 7)

// All DS3231 registers
typedef struct
{
   uint8_t seconds;
   uint8_t minutes;
   uint8_t hours;
   uint8_t day;
   uint8_t date;
   uint8_t month;
   uint8_t year;
   uint8_t alarm1_secconds;
   uint8_t alarm1_minutes;
   uint8_t alarm1_hours;
   uint8_t alarm1_day;
   uint8_t alarm1_date;
   uint8_t alarm2_minutes;
   uint8_t alarm2_hours;
   uint8_t alarm2_day;
   uint8_t alarm2_date;
   uint8_t control;
   uint8_t status;
   uint8_t aging;
   uint8_t msb_temp;
   uint8_t lsb_temp;
} DS3231_registers_TypeDef;

// DS3231 date
typedef struct
{
   uint8_t seconds;
   uint8_t minutes;
   uint8_t hours;
   uint8_t day_of_week;
   uint8_t mday;
   uint8_t month;
   uint8_t year;
} DS3231_date_TypeDef;

// Human Readable Format date
typedef struct
{
   uint8_t Seconds;
   uint8_t Minutes;
   uint8_t Hours;
   uint8_t Day;
   uint8_t Month;
   uint16_t Year;
   uint8_t DOW;
} HRF_date_TypeDef;

void InitDs3231(void);

int DS3231_ReadDateRAW(DS3231_date_TypeDef* date);
void DS3231_WriteDateRAW(DS3231_date_TypeDef* date);
int DS3231_ReadAlarm1RAW(DS3231_date_TypeDef* date);
void DS3231_WriteAlarm1RAW(DS3231_date_TypeDef* date);
void DS3231_ReadDate(HRF_date_TypeDef* hrf_date);
void DS3231_ReadAlarm1(HRF_date_TypeDef* hrf_date);
void DS3231_DateToTimeStr(DS3231_date_TypeDef* raw_date, char* str);
void DS3231_DateToDateStr(DS3231_date_TypeDef* raw_date, char* str);
void ConvertDsToRtc(const DS3231_date_TypeDef* pDateI, RTC_t* pDateO);
void ConvertRtcToDs(const RTC_t* pDateI, DS3231_date_TypeDef* pDateO);
uint8_t DS3231_ReadTemp(void);
void ResetAlarmDs3231(void);
void InitI2C(void);

#endif