#ifndef _DRIVE_SIMPLE_H
#define _DRIVE_SIMPLE_H

typedef __packed struct
{
   s16 x;
   s16 y;
   s16 z;
} vector;

typedef struct
{
   int16_t ang_max;
   uint8_t num;
   int16_t mod_aver;
   int16_t dv_total;
   uint8_t strong_brake;
   int16_t d_course;
   uint8_t check;
} DS_Debug;

typedef __packed struct
{
   float turn;
   float acc_brake;
   float shake;
   float speed;
} TDS_Violation;

//инициализация
extern void DSm_Init();
//расчеты
extern void DSm_Calc(vector* a_raw);
extern void DSm_SaveSpeed(const float speed, const float course, const uint8_t valid);
//вывод отладки
extern int DS_Write_Violation(u8* out);
extern void DSm_Print();
_Bool DriverFilterData(void);

//события переворота и удара
extern int16_t GetRotateAlarm();
extern int16_t GetAccidentAlarm();
extern void ResetRotateAlarm();
extern void ResetAccidentAlarm();

//#define DEBUG_DSM

#endif