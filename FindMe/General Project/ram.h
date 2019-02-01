
#ifndef _SRAM_H
#define _SRAM_H

#ifndef BOOTLOADER
#include "includes.h"
#endif
#include "eeprom.h"

typedef enum TYPE_GSM {
    GSM_ERROR_TYPE = 0,
    CINTERION = 1,
    SIMCOM = 2,
} TYPE_GSM;

enum {
    SIZE_MOBILE_COUNTRY_CODE = 6,
    SIZE_PIN_CODE = 6,
    SIZE_NAME_FIRMWARE = 11,
    SIZE_PASW = 12,
    SIZE_FTP_USER = 16,
    SIZE_MODEM_IDENT = 18,
    SIZE_TEL = 20,
    SIZE_TIME = 20,
    SIZE_IMEI = 20,
    SIZE_SCID = 24,
    SIZE_TNUM = 24,
    SIZE_TOKEN = 25,
    LOGIN_PASS_SIZE = 30,
    SIZE_SERV = 32,
    SIZE_SERV_FTP = 40,
    SIZE_NAME_GSM_SOFT = 42,
    SIZE_USER_REG_INFO = 100,
    SIZE_GPS_POSITION = 100,
};

typedef enum SIM_STATUS {
    SIM_WAIT = 0,
    HOME_NET = 1,
    ROAMING_NET = 2,
    FIND_NET = 3,
    SIM_ERROR = 10,
    SIM_NO_READY = 11,
    SIM_PIN = 12,
    SIM_PUK = 13,
    SIM_PIN_GUESSING = 14,
    SIM_PIN_NO_SET = 15,
} SIM_STATUS;

//Работа с серверами
typedef enum TYPE_SERVER {
    FIRST_SERVER = 0,     //сервер dev
    SECOND_SERVER = 1,    //сервер 911
} TYPE_SERVER;

/*РЕЖИМЫ ЭНЕРГОПОТРЕБЛЕНИЯ */
typedef enum {
    POWER_RUN_MODE = 0,         //обычный режим работы ^_^
    POWER_LOW_PWR1_MODE = 1,    //дремим -_-
    POWER_LOW_PWR2_MODE = 2,    //спим      -_-oO
    POWER_SLEEP_MODE = 3,       //глубокий сон  -_-zZ
} PWR_STATUS;

//Статусы мигания индикационного светодиода.
typedef enum LED_Status {
    LOAD_DEVICE = 0,       //Загрузка устройства
    FIND_SIM = 1,          //Поиск СИМ карты
    REG_GSM = 2,           //Ожидание регистрации GSM
    SERVER_CONNECT = 3,    //Регистрация в сети GSM пройдена, ожидание подключения к серверу
    NORMAL_RUN = 4,        //Регистрация в сети GSM пройдена, устройство подключено к серверу
    LOW_PWR1 = 5,          //Устройство находится в «спящем режиме» LOW_PWR1
    LOW_PWR2 = 6,          //Устройство находится в «спящем режиме» LOW_PWR2
} LED_Status;

typedef enum {
    ERROR_CMD = -4,
    ERROR_ALL_SIMCARD = -3,
    SECOND_SIMCARD_ERROR = -2,
    FIRST_SIMCARD_ERROR = -1,

    GET_NUM_SIM = 0,

    CURRENT_FIRST_SIMCARD = 1,
    CURRENT_SECOND_SIMCARD = 2,

    FIRST_SIMCARD_OK = 3,
    SECOND_SIMCARD_OK = 4,
    FIRST_AND_SECOND_SIMCARD_OK = 5,
} SIM_CARD;

//Структура пакетной обработки передачи данных
typedef __packed struct data_frame {
    uint64_t ulSerialNumber;    // IMEI
    uint8_t ucType;
    uint16_t usDevicePaketNumber;
    uint16_t usDeviceDataPaketNumber;
    uint16_t usServerPaketNumber;
    uint16_t usServerDataPaketNumber;
    uint16_t usFlagsRetDataServer;
    uint16_t eTypeInfoPack;
} DATA_FRAME;
GLOBAL DATA_FRAME g_stFrame;    //Структура пакета.

typedef __packed struct
{
    u32 uiNameNewFirmware;    //Имя новой прошивки
    _Bool bNewFirmware;
    _Bool bFirmSendServ;
} T_FIRMWARE_NAME;

typedef enum {
    ACC_STATE_STOP = 0,    //Остановка
    ACC_STATE_MOVE = 1,    //Движение
} ACC_STATE;
typedef __packed struct
{
    int16_t X;
    int16_t Y;
    int16_t Z;
} stAccelAxisData;

typedef __packed struct
{
    RTC_t irq_date;             //время когда было получено прерывание
    uint32_t irq_sec;           //время когда было получено прерывание в секундах от 1970
    ACC_STATE curr_state;       //Текущий статус акселерометра.
    int8_t s8Temperatur;        //температура
    uint16_t sec_state_move;    //Время движения.
    uint32_t sec_state_stop;    //Время стоянки.
    stAccelAxisData;
} TAcc_state;
GLOBAL TAcc_state g_stRamAccState;    //Структура Акселерометра.

typedef __packed struct
{
    uint64_t ulIMEI;                                     // IMEI GSM модуля
    char strIMEI[SIZE_IMEI];                             // IMEI GSM модуля в строке
    uint8_t aucGsmCSQ;                                   // Уровень CSQ
    char strGsmModemIdentification[SIZE_MODEM_IDENT];    // Идентификатор gsm модема (GSM900 / GSM1800 / UMTS2100 /LTE1800
                                                         // / LTE2600).
    char strGsmModemSoftware[SIZE_NAME_GSM_SOFT];        // Версия прошивки gsm модема.
} TGsm;

// Russian Federation Operator
typedef enum {
    NO_DATA = 0,
    Beeline,
    MTS,
    MegaFon,
    Tele2,
} EBrandOperator;
// Номера телефонов для определения номера СИМ карты.
// const char *pstrTelNum [4] = {"*110*10#", "*111*0887#", "*205#", "*201#"};

typedef __packed struct
{
    char strFIRST_SCID[SIZE_SCID];                    // SCID СИМ карты
    char strSECOND_SCID[SIZE_SCID];                   // SCID СИМ карты
    SIM_STATUS eRegStatus;                            // роуминг/не роуминг
    char acMobCountCode[SIZE_MOBILE_COUNTRY_CODE];    // Код оператора мобильной связи.
    _Bool bGprsTrue;
    EBrandOperator eBrandOperator;
} TSim;

typedef __packed struct
{
    _Bool bGprsProfActivate;
    _Bool bWaitGpsModeFm;
    _Bool bServConnectAGpsTrue;    //Флаг закаченых A-GPS данных.
} TConnect;

typedef __packed struct
{
    RESET_STATUS_DEVICE eResetStatusDevice;    //Причины перезагрузки(храним в BKP REG).
    PWR_STATUS eCurPwrStat;
    LED_Status eLedStatus;
    uint8_t CountSimFirstError;
    uint8_t CountSimSecondError;
    TYPE_MESSAGE eMaskMessage;    //Маска отправки СМС на телефоны.
} TDevice;

/* ACCEL Config */
typedef __packed struct
{
    ACC_STATE eAccelState;
} TAccel;

typedef __packed struct
{
    uint32_t uiFlashDataLen;
    _Bool bFlashReady;
    _Bool bRealtimeReady;
    _Bool bTempBufReady;
} TFlashInf;

typedef __packed struct
{
    uint8_t ucCPU_load;                      //Загруска CPU
    unsigned long ulFreeSizeStackGsmTask;    //
    unsigned long ulFreeSizeStackPeriphHandler;
    unsigned long ulFreeSizeStackGpsHandler;
    unsigned long ulFreeSizeStackApplicationIdleHook;
    unsigned long ulFreeHeapSize;    // in Byte
} TRtos;

typedef __packed struct
{
    uint32_t uiTimePwrGps;    //Время работы GPS
    uint32_t uiTimePwrGsm;    //Время работы GSM
} THard;

typedef __packed struct
{
#if(INCLUDE_uxTaskGetStackHighWaterMark == 1)
    TRtos stRtos;
#endif
    THard stHard;
} TDeviceDiagnostic;

typedef __packed struct
{
    TGsm stGsm;
    TSim stSim;
    TConnect stConnect;
    TDevice stDevice;
    T_FIRMWARE_NAME stFirmware;
    TAccel stAccel;
    TFlashInf stFlash;
    TDeviceDiagnostic stDevDiag;
} TRam;
GLOBAL TRam g_stRam;

GLOBAL int g_iFramePaketNumber _EQU(0);

#endif