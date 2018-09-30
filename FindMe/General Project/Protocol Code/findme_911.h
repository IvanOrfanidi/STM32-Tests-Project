#ifndef __FINDME_911_H
#define __FINDME_911_H

#ifdef __cplusplus
extern "C" {
#endif

#define MIN_SIZE_NUMBER_SMS 5   //Фильтруем спамовые номера меньше 5 цифр.
#define MAX_SIZE_NAME_DEVICE 15   //Максимальное имя названия девайса на сервере.

#define MAX_SIZE_LBS_FOR_FM911 6
#define SIZE_LBS_FOR_FM911 6   //не более 6
#if (SIZE_LBS_FOR_FM911 > MAX_SIZE_LBS_FOR_FM911)
#   error "The size LBS of the stations to the server FM911 is exceeded!"
#endif

#define _OLD_POSITION_GPS_ FALSE   // отправка прошлых координат при неудачном выходе

#define TIME_STANDBY 31556926

#define TIME_WAIT_SMS_REG_USER (15 * 60)   // wait sms registration (min)

#define DEFAULT_PINCODE_SIM "0000"

typedef enum
{
   TYPE_REG_USER = 'R',   // регистрация пользователя на сервере
   TYPE_REG_TO_BASE = 'B',   // регистрация устройства в базе данных сервера и проверка температуры
   // TYPE_ERR_POWER =             'D',     // GSM модуль отключился из-за пониженного напряжения питания
   TYPE_WU_START = 'A',   // выключение прибора по низкому напряжению питания(просадка батереи или её замена)
   TYPE_WU_BUTTON = 'E',   // перезагрузка по кнопке
   TYPE_WU_TIMER = 'T',   // пробуждение от таймера
   TYPE_ERR_COLD = 'H',   // низкая температура
   TYPE_ERR_CONSRV = 'X',   // не получен ответ от сервера, аварийный режим работы
   TYPE_MOVE_START = 'M',   // начало движения
   TYPE_MOVE_STOP = 'N',   // оставновка
   TYPE_GOODBYE = 'J'   // уход с сервера 911 на новый с сменой протокола
} T_TYPE_CONNECT;

typedef enum
{
   ANS_OK = 0,
   SRV_TIMEOUT = 1,
   ANS_ERROR = 2,
   CRC_ERROR = 3
} T_ANS_SRV_FM911;

int FrameBuildGpsDataFm911(char* pOut, int OffsetData);
int FrameBuildSystemFm911(char* pOut, int OffsetData);
int FrameBuildGsmDataFm911(char* pOut, int OffsetData);
int FrameBuildAccelDataFm911(char* pOut, int OffsetData);
int FrameBuildDeviceDataFm911(char* pOut, int OffsetData);
int FrameBuildLogDataFm911(char* pOut, int OffsetData);
int FrameBuildAccelDataFm911(char* pOut, int OffsetData);

T_ANS_SRV_FM911 GprsAckDataFm911(void);
int CheckSmsRegUserFm911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode);

T_TYPE_CONNECT GetTypeConnectFm911(void);
uint8_t gsm_cs(char* buf, int size);
int getNameFmVersia(char* ptr);

#ifdef __cplusplus
}
#endif

#endif