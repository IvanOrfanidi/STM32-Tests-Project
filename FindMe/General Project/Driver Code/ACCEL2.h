#ifndef __ACCEL_H
#define __ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "eeprom.h"
#include "ram.h"

#define ACC_INT_ENABLE 1
#define ACC_FREQ_HZ 100

#define CTRL_REG2_REG 0x22

#define CTRL_REG3_REG 0x23
#define CTRL_REG4_REG 0x20
#define CTRL_REG5_REG 0x24

#define OUT_X_L_REG 0x28
#define OUT_X_H_REG 0x29
#define OUT_Y_L_REG 0x2A
#define OUT_Y_H_REG 0x2B
#define OUT_Z_L_REG 0x2C
#define OUT_Z_H_REG 0x2D

#define THRS1_2_REG 0x77
#define OUTS2_REG 0x7F
#define PR2_REG 0x7C
#define MASK2_A_REG 0x7A
#define SETT2_REG 0x7B

#define ST2_X_REG 0x60

typedef enum
{
   POWER_DOWN = 0,
   F_3HZ = 16,
   F_6HZ = 32,
   F_12HZ = 48,
   F_25HZ = 64,
   F_50HZ = 80,
   F_100HZ = 96,
   F_400HZ = 112,
   F_800HZ = 128,
   F_1600HZ = 144,
} T_ODR_FREQ;

typedef enum
{
   BW_FILTER_800HZ = 0,
   BW_FILTER_400HZ = 64,
   BW_FILTER_200HZ = 128,
   BW_FILTER_50HZ = 192,
} T_BW_FILTERS;

typedef enum
{
   N_V = (1 << 0),
   P_V = (1 << 1),
   N_Z = (1 << 2),
   P_Z = (1 << 3),
   N_Y = (1 << 4),
   P_Y = (1 << 5),
   N_X = (1 << 6),
   P_X = (1 << 7)
} T_MASK_SM;

#define SM2_EN (1 << 0)

#define ABS (1 << 5)
#define SITR (1 << 0)

#define XEN (1 << 0)
#define YEN (1 << 1)
#define ZEN (1 << 2)

#define STRT (1 << 0)   // Soft reset bit (0 = no soft reset, 1 = softreset (POR function))
#define VFILT \
   (1 << 2)   // Vector filter enable/disable. Default value:0 (0 = vector filter disabled, 1 = vector filter enabled)
#define INT1_EN \
   (1 << 3)   // Interrupt 2 enable/disable. Default Value:0 (0 = INT1/DRDY signal disabled, 1 = INT1/DRDY signal
              // enabled)
#define INT2_EN \
   (1 << 4)   // Interrupt 2 enable/disable. Default value:0 (0 = INT2 signal disabled, 1 = INT2 signal enabled)
#define IEL \
   (1 << 5)   // Interrupt signal latching. Default value:0 (0 = interrupt signals latched, 1 = interrupt signal pulsed)
#define IEA \
   (1 << 6)   // Interrupt signal polarity. Default value:0 (0 = interrupt signals active LOW, 1 = interrupt signals
              // active HIGH)
#define DR_EN \
   (1 << 7)   // DRDY signal enable to INT1. Default value:0 (0 = data ready signal not connected, 1 = data ready signal
              // connected to INT1)

//различные сообщения
#define CMD_ACCEL_READ_IRQ 0   //прочитать прерывание из акселерометра

// void vAccelHandler(void *pvParameters);

int accelInit(uint8_t Sensitivity);
void Accel_Power_Down(void);
void Accel_Reset(void);

uint8_t Accel_CMD(uint8_t adr, uint8_t data);
uint8_t Accel_Read(uint8_t adr);
ACC_STATE AccelState(void);
void AccelIRQInit(void);
void AccelIRQDeInit(void);
void AccMoveDetect(void);
int8_t GetTemperaturAccel(void);
void ReadAxisDataAccel(TAcc_state* pAcc_state);
void AccelHandler(void);   //Обработчик Акселерометра.
void AccelSetMove(void);
void EXTI0_IRQHandler(void);

int8_t GetTemperaturAccel(void);
uint32_t SecStateStop(void);
uint16_t SecStateMove(void);
int8_t CalculTemperaturAccel(void);

#ifdef __cplusplus
}
#endif

#endif