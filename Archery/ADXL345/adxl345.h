
#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

#define _SPI_ADXL345_PORT 1
#define SPI_SPEED SPI_BaudRatePrescaler_16

#if _SPI_ADXL345_PORT == 1

#define ADXL345_SPI SPI1
#define ADXL345_SPI_CLK RCC_APB2Periph_SPI1
#define ADXL345_SPI_MOSI_GPIO_CLK RCC_APB2Periph_GPIOA

#define ADXL345_SPI_SCK_PIN GPIO_Pin_5    // SCL
#define ADXL345_SPI_MISO_PIN GPIO_Pin_6
#define ADXL345_SPI_MOSI_PIN GPIO_Pin_7
#define ADXL345_SPI_GPIO_PORT GPIOA

#elif _SPI_ADXL345_PORT == 2

#define ADXL345_SPI SPI2
#define ADXL345_SPI_CLK RCC_APB1Periph_SPI2
#define ADXL345_SPI_MOSI_GPIO_CLK RCC_APB2Periph_GPIOB

#define ADXL345_SPI_SCK_PIN GPIO_Pin_13
#define ADXL345_SPI_MISO_PIN GPIO_Pin_14
#define ADXL345_SPI_MOSI_PIN GPIO_Pin_15
#define ADXL345_SPI_GPIO_PORT GPIOB

#endif

#define ADXL345_SPI_CS_PIN GPIO_Pin_4
#define ADXL345_SPI_CS_GPIO_PORT GPIOA
#define ADXL345_SPI_CS_GPIO_CLK RCC_APB2Periph_GPIOA

#define ACCEL_CS_ENABLE \
    GPIO_ResetBits(ADXL345_SPI_CS_GPIO_PORT, ADXL345_SPI_CS_PIN)    // for(int _ii = 0; _ii < 10; _ii++)
#define ACCEL_CS_DISABLE \
    GPIO_SetBits(ADXL345_SPI_CS_GPIO_PORT, ADXL345_SPI_CS_PIN)    // for(int _ii = 0; _ii < 10; _ii++)

#define USE_ADXL345_INT1
#define USE_ADXL345_INT2

#ifdef USE_ADXL345_INT1
#define ADXL345_INT1_PIN GPIO_Pin_1
#define ADXL345_INT1_GPIO_PORT GPIOA
#define ADXL345_INT1_GPIO_CLK RCC_APB2Periph_GPIOA
#define ADXL345_INT1_EXTI_LINE EXTI_Line1
#define ADXL345_INT1_EXTI_IRQn EXTI1_IRQn
#endif

#ifdef USE_ADXL345_INT2
#define ADXL345_INT2_PIN GPIO_Pin_2
#define ADXL345_INT2_GPIO_PORT GPIOA
#define ADXL345_INT2_GPIO_CLK RCC_APB2Periph_GPIOA
#define ADXL345_INT2_EXTI_LINE EXTI_Line2
#define ADXL345_INT2_EXTI_IRQn EXTI2_IRQn
#endif

#define ADXL345_DEVID 0x00    // Device ID

#define ADXL345_WHO_AM_I 0x0F
#define ADXL345_THRESH_TAP 0x1D        //����� ������
#define ADXL345_OFSX 0x1E              //�������� ��� x
#define ADXL345_OFSY 0x1F              //�������� ��� y
#define ADXL345_OFSZ 0x20              //�������� ��� Z
#define ADXL345_DUR 0x21               //������������ ������
#define ADXL345_LATENT 0x22            //�������� ������
#define ADXL345_WINDOWS 0x23           //��������� ���� ���������� ������
#define ADXL345_THRESH_ACT 0x24        //����� ����������
#define ADXL345_THRESH_INACT 0x25      //����� ������������
#define ADXL345_TIME_INACT 0x26        //����� ������������
#define ADXL345_ACT_INACT_CTL 0x27     //���������� ����� ��� ����������� ����������/������������
#define ADXL345_THRESH_FF 0x28         //����� ���������� �������
#define ADXL345_TIME_FF 0x29           //����� ���������� �������
#define ADXL345_TAP_AXES 0x2A          //���������� ����� ��� ������/�������� ������
#define ADXL345_ACT_TAP_STATUS 0x2B    //�������� ������/�������� ������
#define ADXL345_BW_RATE 0x2C           //���������� �������� ������� � �������� �������
#define ADXL345_POWER_CTL 0x2D         //���������� �����������������
#define ADXL345_INT_ENABLE 0x2E        //���������� ����������� ���������
#define ADXL345_INT_MAP 0x2F           //���������� ������� ����������
#define ADXL345_INT_SOURCE 0x30        //�������� ����������

#define ADXL345_DATA_FORMAT 0x31    //��������� ������� ������
#define ADXL345_DATAX0 0x32         // X-Asix Data 0
#define ADXL345_DATAX1 0x33         // X-Asix Data 1
#define ADXL345_DATAY0 0x34         // Y-Asix Data 0
#define ADXL345_DATAY1 0x35         // Y-Asix Data 1
#define ADXL345_DATAZ0 0x36         // Z-Asix Data 0
#define ADXL345_DATAZ1 0x37         // Z-Asix Data 1

#define ADXL345_FIFO_CTL 0x38       //���������� ������� FIFO
#define ADXL345_FIFO_STATUS 0x39    //������ FIFO

#define ADXL345_ID 0xE5    // The DEVID register holds a fixed device ID code of 0xE5 (345 octal).

#define ADXL345_LOW_POWER_MODE 0x10    // Low Power Mode Actions
#define ADXL345_SLEEP_BIT 0x04

typedef enum {
    BANDWIDTH_000005 = 0,
    BANDWIDTH_000010,
    BANDWIDTH_000020,
    BANDWIDTH_000039,
    BANDWIDTH_000078,
    BANDWIDTH_000156,
    BANDWIDTH_000313,
    BANDWIDTH_000625,
    BANDWIDTH_001200,
    BANDWIDTH_002500,
    BANDWIDTH_005000,
    BANDWIDTH_010000,
    BANDWIDTH_020000,
    BANDWIDTH_040000,
    BANDWIDTH_080000,
    BANDWIDTH_160000
} TSet_Bandwidth;

//����� ����������� �����������
typedef enum {
    RESOLUTION_2G = 0,
    RESOLUTION_4G = 1,
    RESOLUTION_8G = 2,
    RESOLUTION_16G = 3,
    RESOLUTION_FULL = 8,
} TSet_Resol;

//��� ������� ����������
typedef enum {
    OVERRUN = 1,
    WATERMARK = 2,
    FREE_FALL = 4,
    INACTIVITY = 8,
    ACTIVITY = 16,
    DOUBLE_TAP = 32,
    SINGLE_TAP = 64,
    DATA_READY = 128
} TInt_Func;

//���������� ������ ����������
typedef enum {
    INT_1 = 1,
    INT_2 = 2
} TSet_Map;

typedef enum {
    FREQ_SM8 = 0,
    FREQ_SM4,
    FREQ_SM2,
    FREQ_SM1
} TFreq_Read_Sleep_Mode;

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

void AccelInit(void);
void Accel_Clear_Settings(void);
int ADXL345_ReadXYZ(int16_t* ptr);

void Set_Resolution(TSet_Resol eResol);
void Standby_Mode(_Bool val);
void Set_Tap_Threshold(uint8_t ucData);
void Set_Tap_Duration(unsigned char ucData);
void Set_Tap_Latency(unsigned char ucData);
void Set_Tap_Window(unsigned char ucData);
void Set_Activity_Threshold(unsigned char ucData);
void Set_Inactivity_Threshold(unsigned char ucData);
void Set_Time_Inactivity(unsigned char ucData);
uint8_t Get_Source_Interrupt(void);
void Reset_Interrupt(void);
void Set_Interrupt(TInt_Func eFunc, _Bool bVal, TSet_Map ePin);
uint8_t ADXL345_write_byte(uint8_t add, uint8_t val);
uint8_t ADXL345_read_byte(uint8_t add);
void Accel_Sleep(void);

#endif