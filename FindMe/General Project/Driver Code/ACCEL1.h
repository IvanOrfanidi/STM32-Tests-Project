#ifndef _ACCEL_H_
#define _ACCEL_H_

#include "includes.h"
#include "eeprom.h"
#include "sram.h"

#define ACC_INT_ENABLE 1
#define ACC_FREQ_HZ 100

#define OUT_X_L_REGISTER 0x28
#define OUT_X_H_REGISTER 0x29
#define OUT_Y_L_REGISTER 0x2A
#define OUT_Y_H_REGISTER 0x2B
#define OUT_Z_L_REGISTER 0x2C
#define OUT_Z_H_REGISTER 0x2D

//различные сообщения
#define CMD_ACCEL_READ_IRQ 0   //прочитать прерывание из акселерометра

// void vAccelHandler(void *pvParameters);

void Accel_Init(void);
void Accel_Reset(void);
uint8_t Accel_CMD(uint8_t adr, uint8_t data);
uint8_t Accel_Read(uint8_t adr);
ACC_STATE AccelState(void);
void AccelIRQInit(void);
void AccelIRQDeInit(void);
void AccMoveDetect(void);
void AccelPowerDown(u8 sens);

int8_t GetTemperaturAccel(void);

void ReadAxisDataAccel(TAcc_state* pAcc_state);

void AccelHandler(void);   //Обработчик Акселерометра.

void AccelSetMove(void);

void EXTI0_IRQHandler(void);

#endif