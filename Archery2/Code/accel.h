
#ifndef __ACCEL_H
#define __ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Axelerometr Config */
#define ACCEL_THRESACT (uint8_t)(8000 / 62.5)   //Установка порога Толчка
#define ACCEL_THRESHOLD (uint8_t)(8000 / 62.5)   //Установка порога Толчка
#define ACCEL_DURATION (uint8_t)(10000 / 625)   //Установка лимита времени для Толчка
#define ACCEL_LATENT (uint8_t)(10 / 1.25)   //Установка задержки для Толчка
#define ACCEL_WINDOWS (uint8_t)(60 / 1.25)   //Установка временного окна для Второго Толчка
#define ACCEL_TIME_INACTIVITY 30   //Установка времени Неактивности

//#define USE_ACCEL_LOW_PWR
#ifdef USE_ACCEL_LOW_PWR
#   define _ACCEL_LOW_POWER_MODE_ 1
#endif

#define NUM_AXIS 3

/* Значения осей при изготовке к стрельбе. Первый шаг.
   Лук поднят и находится в горизонтальной плоскости. */
#define X_MAX_VALUE_STEP1 50
#define X_MIN_VALUE_STEP1 -80

#define Y_MAX_VALUE_STEP1 70
#define Y_MIN_VALUE_STEP1 -70

#define Z_MAX_VALUE_STEP1 300
#define Z_MIN_VALUE_STEP1 200

#define TIMEOUT_TRANSITION_STEP1 300
/* ------------------------------------------------- */

/* Значения осей во время и после выстрела.
   Опрокидывания лука в плоскости стрельбы после выстрела. */
#define X_VALUE_STEP2 200

#define Y_MAX_VALUE_STEP2 70
#define Y_MIN_VALUE_STEP2 -70

#define Z_VALUE_STEP2 170
#define TIMEOUT_TRANSITION_STEP2 15
#define MAX_TIMEOUT_RST_STEP2 10000
#define MAX_TIMEOUT_TAP_RST 1000
/* ------------------------------------------------- */

/* Значения осей когда лук находится на подставке в ожидании выстрела.
   Лук находится в вертикальном положении, сброс шага.*/
#define X_VALUE_STEP_RST -200

#define Y_MAX_VALUE_STEP_RST 150
#define Y_MIN_VALUE_STEP_RST -150

#define MAX_TIMEOUT_STEP_RST 50
/* ------------------------------------------------- */

typedef __packed struct
{
   int16_t sValueAxisX;
   int16_t sValueAxisY;
   int16_t sValueAxisZ;
   uint8_t ucInterrupt;
} TAccel_Data;

void vAccelTask(void* pvParameters);
void setCountShot(int shot);

int getCountShot(void);
void AccelEnable(void);
void AccelDisable(void);
_Bool AccelState(void);

#ifdef __cplusplus
}
#endif

#endif /* __ACCEL_H */