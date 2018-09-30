

#ifndef _PROTOCOL_RELIZE_H_
#define _PROTOCOL_RELIZE_H_

#include "includes.h"
#include "gps_parser_ver2.h"

#define PROTOCOL_ID '#'   //Заголовок
#define PROTOCOL_VERSION 0x03   //Версия протокола

#define RECEIVED_FIRMWARE_PACKET 13   //Получена прошивка с FTP сервера

#define NAVIGATIONAL_PACKET 21   //Навигационный, лог по GPS трек
#define NAVIGATIONAL_PACKET_REAL_TIME 22   //Навигационный, реальное время по GPS

#define NAVIGATIONAL_GSM_PACKET 24   //Навигационный, реальное время по LBS

#define NAVIGATIONAL_ADD_GSM_PACKET 69   //Навигационный LBS, реальное время по множеству операторов

#define PERIPHERY_PACKET 91   //Данные по одному или нескольким универсальным входам GPIO
#define SENSOR_PACKET 92   //Данные с датчиков по RS485

#define CAN_LOG_PACKET 104   //Пакет данных с CAN LOG
#define DT_DRIVE_STYLE 106   //пакет по стилю вождения
#define STATUS_DEVICE_PACKET 1   //Пакет состояние устройства("системная таблица")
#define HARDWARE_EVENT_PACKET 2   //Пакет аппаратное событие

#define CONFIG_FIRMWARE_PACKET 201   //Передача ссылки на файл прошивки
#define CHANGE_FIRMWARE_PACKET 203   //Команда сменить прошивку

#define SYNCHRO_TIME_SERVER_PACKET 210   //Команда устройству выполнить синхронизацию RTC
#define SYNCHRO_TIME_DEVICE_PACKET 209   //Запрос от устройства на получения данных для синхронизации RTC

#define CONFIG_DEVICE_PACKET 220   //Вся конфигурация девайса, посылается сервером
#define USER_CONFIG_DEVICE_PACKET \
   221   //Вся конфигурация девайса, посылается девайсем при конфигурации пользователем по СМС или командами
#define SERVER_CONFIG_DEVICE_PACKET 222   //Вся конфигурация девайса, посылается девайсем в ответ на пакет 220

#define LOG_DEVICE_PACKET 205   //Диагностическая информация

#define COMMAND_SERVER_TO_DEVICE_PACKET 206   //Передача команды конфигурирования на устройство
#define COMMAND_DEVICE_TO_SERVER_PACKET 14   //Ответ на команду конфигурирования

#define TYPE_ID_SENSOR 1   // ID датчика уровня LLS;

#define WIALON_PACKET 25   //Пакет для Wialon сервера.

#define LEN_NAVIGATIONAL_PACKET_REAL_TIME 16

#define MAX_UI8 19
#define MAX_UI16 8
#define MAX_UI32 11
#define MAX_STR 9

//системные события
typedef enum
{
   //отладка по стилю вождения
   EV_G_PEAK = 21,
   EV_CNT_PEAK = 23,
   EV_W0_PEAK = 24,
   EV_W1_PEAK = 25,
   EV_W2_PEAK = 26,

   EV_G_FILT_X = 29,   //показания акселерометра по 3м осям
   EV_G_FILT_Y = 30,
   EV_G_FILT_Z = 31,

   //нарушения по стилю вождения
   EV_DS_TURN = 41,
   EV_DS_ACC_BRAKE = 42,
   EV_DS_SHAKE = 43,
   EV_DS_SPEED = 44,
} SYS_EVENT;

//пакет по стилю вождения
typedef enum
{
   DS_ACC_ALARM = 1,
   DS_BRAKE_ALARM = 2,
   DS_TURN_ALARM = 3,
   DS_SHAKE_ALARM = 4
} DS_MESSAGE;

typedef enum FRAME_TYPE
{
   C_DATA = 1,   // данные от устройства
   S_ACK = 2,   // квитанция от сервера данные приняты
   S_FAIL = 3,   // квитанция от сервера данные не приняты
   S_REQ = 4,   // сервер запрашивает данные
   S_ACK_REQ = 5,   // квитанция от сервера данные приняты и есть данные для устройства
   S_DATA = 6,   // данные от сервера
   C_ACK = 7,   // квитанция от устройства данные приняты
   C_FAIL = 8,   // квитанция от устройства данные не приняты
   C_REQ = 9,   // устройство запрашивает данные
   S_FIN = 10,   // сервер завершает сеанс связи
   S_ACK_DLY = 11,   // квитанция от сервера данные приняты и сообщение о перегрузке сервера
   C_CONNECT = 13,   // инициализация соединения
   S_ASK_DATA = 14,   // квитанция от сервера данные приняты и список команды от сервера
} FRAME_TYPE;

typedef enum PERIPHERY_PACKET_ID_TYPE
{
   UI_1 = 1,
   UI_2 = 2,
   UI_3 = 3,
   UI_4 = 4,
   UI_5 = 5,
   UI_6 = 6,
   VBAT_1 = 7,
   VPWR_1 = 8,
   DI_1 = 9,
   DI_2 = 10,
   DI_3 = 11,
   DI_4 = 12,
   DI_5 = 13,
   DI_6 = 14,
   DI_7 = 15,
   DI_8 = 16,
   ID_CSG_GSM = 19,
   ID_TEMPERATUR_ACCSEL = 20,
   ID_TEMPERATUR_MCU = 21,
   ID_TEMPERATUR_GSM = 22,
} PERIPHERY_PACKET_ID_TYPE;

typedef enum HARD_EVENT_PACKET_ID_TYPE
{
   ALARM_BUTTON = 1,
   IGNITION = 2,
   OBD_CONNECT = 3,
   EVACUATION = 6,
   ACCIDENT = 7,
   ROTATION = 8,
} HARD_EVENT_PACKET_ID_TYPE;

typedef enum CAN_LOG_PACKET_ID_TYPE
{
   ID_SECURITY_CAN_LOG_TYPE = 1,   // ID Флаги состояния безопасности.
   ID_TOTAL_TIME_ENGINE_OPERATION_CAN_LOG_TYPE = 2,   // ID Полное время работы двигателя, ч (float)
   ID_FULL_MILEAGE_VEHICLE_CAN_LOG_TYPE = 3,   // ID Полный пробег транспортного средства, км (float)
   ID_TOTAL_FUEL_CONSUMPTION_CAN_LOG_TYPE = 4,   // ID Общий и полный расход топлива л (float)
   ID_FUEL_LEVEL_IN_LITERS_CAN_LOG_TYPE = 5,   // ID Уровень топлива (в литрах)
   ID_ENGINE_SPEED_RPM_CAN_LOG_TYPE = 6,   // ID Обороты двигателя RPM
   ID_TEMPERATURE_ENGINE_CAN_LOG_TYPE = 7,   // ID Температура двигателя
   ID_VEHICLE_SPEED_CAN_LOG_TYPE = 8,   // ID Скорость транспортного средства
   ID_AXLE_LOAD1_CAN_LOG_TYPE = 9,   // ID Нагрузка на ось 1
   ID_AXLE_LOAD2_CAN_LOG_TYPE = 10,   // ID Нагрузка на ось 2
   ID_AXLE_LOAD3_CAN_LOG_TYPE = 11,   // ID Нагрузка на ось 3
   ID_AXLE_LOAD4_CAN_LOG_TYPE = 12,   // ID Нагрузка на ось 4
   ID_AXLE_LOAD5_CAN_LOG_TYPE = 13,   // ID Нагрузка на ось 5
   ID_STATE_CONTROL_CAN_LOG_TYPE = 14,   // ID Флаги состояния контроля.
} CAN_LOG_PACKET_ID_TYPE;

typedef enum CAN_LOG_ID_TYPE
{
   SECURITY_CAN_LOG_ID = 1,
   TOTAL_TIME_ENGINE_OPERATION_CAN_ID = 2,
} CAN_LOG_ID_TYPE;

typedef __packed struct
{
   uint16_t Meas_VIN;
   uint8_t ucGsmCsq;
   int8_t cTemperatur;
} tBACK_DATA_PERIPH;   //Структура с предыдущими данными пакета периферии, для оптимизации трафика.

//#define fC_DATA         (1<<C_DATA)
#define fC_ACK (1 << C_ACK)
#define fC_FAIL (1 << C_FAIL)
//#define fC_REQ          (1<<C_REQ)

#define fS_ACK (1 << S_ACK)
#define fS_FAIL (1 << S_FAIL)
#define fS_REQ (1 << S_REQ)
#define fS_ACK_REQ (1 << S_ACK_REQ)
#define fS_DATA (1 << S_DATA)
#define fS_REQ (1 << S_REQ)
#define fS_ACK_REQ (1 << S_ACK_REQ)
#define fS_DATA (1 << S_DATA)
#define fS_FIN (1 << S_FIN)
#define fS_ACK_DLY (1 << S_ACK_DLY)
#define fS_ASK_DATA (1 << S_ASK_DATA)

int FrameGeneralBuildAckC(uint8_t TypePack, const char* pInp, uint16_t LenInp, char* pOut);
int FrameGeneralBuild(const DATA_FRAME* stTypePack, const char* pInp, uint16_t LenInp, char* pOut);

int ack_data_parser(const char* pInp, uint16_t LenInp);

GSM_STEP configFirmware(uint8_t* pIn);   //Команда конфига прошивки.
GSM_STEP changeFirmware(uint8_t* pIn);   //Команда смены прошивки.

int frame_build_received_firmware(char* pOut);
int frame_build_status_device_package(char* pOutint, int OffsetData);   //Отправка пакета статуса устройства.

int frame_build_navigational_packet_realtime(const GPS_INFO* const pGps, char* pOut);
int frame_build_navigational_packet_track(const GPS_INFO* const pGps, char* pOut);

uint32_t frame_build_navigational_not_valid_packet(char* pOut, int OffsetData);   //Пакет невалидной навигации.

int GetDataNavigationalGsmPacket(char* pOutDataFrameBuffer, int iOffsetData);
int frame_build_lbs_packet(GSM_INFO* oGsm, char* pBuf, int iOffsetData);

int frame_build_hardware_event_packet(char* pOut, int OffsetData);

int frame_build_ans_cmd_paket(char* pOut, int OffsetData, char* pIn);

int frame_build_log_device_paket(char* pOut, int OffsetData);

int ServerSynchroTime(uint32_t SecGPS, char* pOut, int OffsetData);
GSM_STEP configTime(uint8_t* pIn);
GSM_STEP configCmdDevice(uint8_t* pIn);

int put_ds_report(u8 id, u8 val, u8 speed, u8* out);
int VarLog(u8 id, float data, u8* out);
GSM_STEP configDevice(uint8_t* pIn);

int frame_build_config_packet(uint8_t ucType, char* pOut, int OffsetData);

#endif
