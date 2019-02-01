#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOTLOADER
#include "includes.h"
#include "gsm_parser.h"
#include "ram.h"
#include "findme_911.h"
#include "gps_parser_ver2.h"
#endif

#ifdef FM3
#include "platform_config_fm3.h"
#endif
#ifdef FM4
#include "platform_config_fm4.h"
#endif

#define END_ADDRESS_CODE_INT_FLASH (uint32_t)0x0801FFFF
#define START_ADDRESS_CODE_INT_FLASH (uint32_t)0x08001000

// Size write flash
#define SIZE_RECORD_EXT_FLASH 256
//Размер только прошивки
#define COUNT_FLASH_PAGE (END_ADDRESS_CODE_INT_FLASH - (START_ADDRESS_CODE_INT_FLASH - 1)) / SIZE_RECORD_EXT_FLASH

#define MAX_COUNT_ERR_FLASH 5
#define MAX_COUNT_ERR_NEW_FIRM 10
#define MAX_COUNT_TIME_NEW_FIRM 1500

#define MAX_ERROR_DOWNLOAD_FIRMWARE 3

//Полный размер выделенный для прошивок.
#define TOTAL_SIZE_FIRMWARE SIZE_SUBSECTOR_FLASH * 31    //
// Адрес во внешней флеш, где мы храним новую скаченную прошивку.
#define ADDR_EXT_FLASH_NEW_FIRMWARE (uint32_t)(SIZE_EXT_FLASH - TOTAL_SIZE_FIRMWARE)

#define TOTAL_SIZE_CONFIG_DATA SIZE_SUBSECTOR_FLASH * 2

#define ADDR_EXT_FLASH_CONFIG_DATA (uint32_t)(ADDR_EXT_FLASH_DATA_AGPS - TOTAL_SIZE_CONFIG_DATA)

#define __CONST_FIRM_VER (uint32_t)(0x0801FFF0)
#define __CONST_BUILD (uint32_t)(0x0801FFF4)

#define SIZE_CONFIG_EEPROM 768
#define ALL 0xFF

typedef enum FIRMWARE_TYPE {
    NO_BASE_FIRMWARE = 0,    //В внешней флеш памяти нет базовой прошивки.
    NO_NEW_FIRMWARE = 1,     //Работаем в нормальном режиме.
    UPDATE_FIRMWARE = 2,     //Перешиваем контроллер на новую прошивку.
    // 3 - флаг перезагрузки по команде прибора (сервера) Значение хранится в enume STATUS_DEVICE///
    BASE_FIRMWARE = 4,             //Перешиваем контроллер на стабильную-базовую прошивку.
    CONFIG_FIRMWARE = 5,           //Перекачиваем прошивку в случае рестарта.
    INITIAL_FIRMWARE = 6,          //Инициализация базовой прошивки(сброс на дефолтные настройки EEPROMA).
    END_PROGRAM_FIRMWARE = 7,      //Устройство успешно обновилось.
    FTP_DOWNLOAD_FIRMWARE = 8,     //Адрес FTP сервера получен и прошивка начнется при любом случае.
    NEW_FIRMWARE = 9,              //Информируем сервер о новой прошивке.
    INITIAL_BASE_FIRMWARE = 10,    //Прошили в первый раз устройство.
} FIRMWARE_TYPE;

// Ошибки по обновлению прошивки.
typedef enum FRAME_FIRMWARE_TYPE {
    FIRMWARE_OK = 0,                      // прошивка скачана и проверена
    ERR_FIRMWARE_SIZE = 3,                // не совпадает размер скачанной прошивки
    ERR_FIRMWARE_CRC = 4,                 // ошибка контрольной суммы
    ERR_CONNECT_FTP_OR_HTTP = 5,          // устройство не может подключиться к FTP серверу
    ERR_FIRMWARE_FLASH = 6,               // устройство не смогло сохранить файл прошивки
    ERR_FIRMWARE_HARD = 7,                // прошивка не соответствует версии железа
    ERR_FIRMWARE_VER = 8,                 // имя файла и номер прошивки не совпадают, либо версия прошивки раньше
    ERR_FIRMWARE_UNAUTHORIZED_401 = 9,    // не авторизован
    ERR_FIRMWARE_UPGRADE_REQ_426 = 10,    // необходимо обновление
    ERR_FIRMWARE_BAD_REQ_400 = 11,        // неверный запрос
    ERR_FIRMWARE_NOT_FOUND_404 = 12,      // не найдено
} FRAME_FIRMWARE_TYPE;

/* ПОЛЬЗОВАТЕЛЬСКИЕ РЕЖИМЫ ЭНЕРГОПОТРЕБЛЕНИЯ */
typedef enum USER_PWR_STATUS {
    USER_POWER_AUTO = 0,             //автоматический режим определения энергопотребления
    USER_POWER_RUN_MODE = 1,         //обычный режим работы ^_^
    USER_POWER_LOW_PWR1_MODE = 2,    //дремим -_-
    USER_POWER_LOW_PWR2_MODE = 3,    //спим      -_-oO
} USER_PWR_STATUS;

typedef enum {
    TRACK_ION = 0,
    STANDART = 1,
    TIMER_FIND = 2,
} TYPE_MODE_DEV;

typedef enum {
    ION_FM = 0,
    FINDME_911 = 1
} TYPE_MODE_PROTOCOL;

#ifndef BOOTLOADER
//Фикс переполнения DMA
V_GLOBAL _Bool g_bDmaGsmFail _EQU(FALSE);
// V_GLOBAL _Bool g_bDmaGsmFailAccept       _EQU(FALSE);

// FTP and Firmware Config
typedef __packed struct
{
    uint32_t uiNameFirmware;                     // Name work firmware(no use)
    uint32_t uiNameNewFirmware;                  // Name new firmware
    char strNameFirmSer[SIZE_SERV_FTP];          // Name ftp server and name firm file
    FRAME_FIRMWARE_TYPE eFlagsStatusFirmware;    // Флаг результата скаченной прошивки
    uint32_t uiSizeFirmware;                     // Size firmware
    char strDefNameFirmSer[SIZE_SERV_FTP];       // Default address Firmware serwer (online.irz.net/f.php?f=)
} TFirmwareConfig;

// User Config
typedef __packed struct
{
    char strPassword[SIZE_PASW];    //Пароль на доступ, по умолчанию '123456'.
    char strTel[SIZE_TEL];          //Номер телефона пользователя девайсем
    TYPE_MESSAGE eMaskMessage;      //Маска отправки уведомлений СМС на телефон.
} TUserConfig;

// Connect Config //
typedef __packed struct
{
    // Настройки серверов
    char str_name_first_server[SIZE_SERV];     //сервер типа dev
    char str_name_second_server[SIZE_SERV];    //сервер типа 911
    TYPE_SERVER eUseNumSer;                    //тип рабочего сервера
} TConnectConfig;

/* Настройки GPRS пользовательской СИМ карты */
typedef __packed struct
{
    char strApnFirst[LOGIN_PASS_SIZE];         // User APN.
    char strPasswordFirst[LOGIN_PASS_SIZE];    // User Password.
    char strLoginFirst[LOGIN_PASS_SIZE];       // User Login.
    _Bool bManualModeSimFirst;                 // Use user config connect GPRS for first SIM card.

    char strApnSecond[LOGIN_PASS_SIZE];         // User APN Second SIM card.
    char strPasswordSecond[LOGIN_PASS_SIZE];    // User Password Second SIM card.
    char strLoginSecond[LOGIN_PASS_SIZE];       // User Login Second SIM card.
    _Bool bManualModeSimSecond;                 // Use user config connect GPRS for second SIM card.

    _Bool flagSimSecondInstld;          // Флаг что установлена резервная SIM карта.
    uint32_t uiTimeContactSecondSim;    // Время последненго выхода с резервной SIM.

    char strPin[SIZE_PIN_CODE];       // User Pin Code SIM.
    _Bool bPinLock;                   // Use User Pin.
    _Bool bJamDetect;                 //Настройки типа поиска глушения, 0-без определения, 1-с определением.
    char strFIRST_SCID[SIZE_SCID];    // SCID заводской СИМ карты
    char strFIRST_IMEI[SIZE_IMEI];    // IMEI GSM module
    uint32_t uiWaitSMS;               //время ожидания СМС(считать активным если не ноль)
} TSimConfig;

/* GSM Modem Config */
typedef __packed struct
{
    uint8_t uc_gsm_find_timeout;       // время поиска gsm сети
    uint8_t uc_gprs_timeout;           // время ожидания GPRS соединения (сек)
    uint8_t uc_lbs_find_timeout;       // время ожидания информации о GSM станциях
    uint8_t uc_gprs_open_count;        // Количество попыток установления GPRS соединения
    _Bool bRoamingGprsEnable;          // 1 - разрешение передачи данных в роуминге
    uint8_t uc_wait_answer_timeout;    // время ожидания ответа от сервера (сек);
} TGsmBasicConfig;

// DEVICE Work Config
typedef __packed struct
{
    USER_PWR_STATUS
    eUserPwrDevice;                      // User Power Config Device(1 - RUN, 2 - LOW POWER_1, 3 - LOW POWER_2, 4 - SLEEP, 5 - AUTO)
    uint32_t uiTimeoutLowPwrMode1;       //Время перехода в режим энергопотребления LOW PWR 1 (время с момента прекращения
                                         //движения).
    uint32_t uiTimeoutLowPwrMode2;       //Время перехода в режим энергопотребления LOW PWR 2 (время с момента прекращения
                                         //движения).
    uint16_t usTimeSleepLowPwrMode1;     //Время сна в режиме энергопотребления LOW PWR 1 (время в min сколько будет
                                         //выключен GSM модем).
    uint16_t usTimeSleepLowPwrMode2;     //Время сна в режиме энергопотребления LOW PWR 2 (время в min сколько будет
                                         //выключен GSM модем).
    uint32_t uiLenDataFlashReady;        //Размер данных во флешке для перерехода в режим энергосбережения.
    _Bool bEnableUseLowPwr1Mode;         //Разрешение режима энегросбер 1.
    _Bool bEnableUseLowPwr2Mode;         //Разрешение режима энегросбер 2.
    uint32_t uiSleepTimeStandart;        //Время сна в Стандартном режиме.
    uint32_t uiSleepTimeFind;            //Время выхода в режиме Поиск.
    _Bool bEnableAccelToFind;            //Использование акселерометра в режиме поиска.
    TYPE_MODE_DEV eModeDevice;           //Алгоритм работы девайса(ION FM, STANDART, TIMER_FIND).
    TYPE_MODE_PROTOCOL eModeProtocol;    //Протокол по которому будет работать девайс.
    uint32_t a_uiTimeReConnect[5];       //Последовательный ряд времен до пяти значений, откладывающих время соединения с
                                         //сервером в случае отсутствия связи
    uint8_t ucCountReConnect;
    uint32_t uiGpsWait;           //Время поиска GPS спутников.
    _Bool bLedEnable;             // Вкл/Выкл светодиод.
    int8_t cMinTemperaturWork;    //Минимальная рабочая температура в градусах Цельсия.
} TDeviceConfig;

/* ACCEL Config */
typedef __packed struct
{
    uint8_t ucSensitivity;       //Чувствительность Акселерометра 1...8
    uint16_t usTimeCurrState;    //Время сброса флага движения. Если движения нет по истечению этого времени, то считаем,
                                 //что девайс стоит.
} TAccelConfig;

// GPS Record Config Mode  //
typedef __packed struct
{
    uint16_t us_gps_real_time_record_data;    //время выхода по GPS с пакетом Real Time.
    uint16_t us_gps_time_record_data;         //фильтр записи по времени.
    _Bool b_gps_record_accel_data;            //фильтр записи только при движении.
    uint8_t uc_gps_record_course;             //фильтр записи по курсу.
    uint16_t us_gps_record_distance;          //Фильтр по дистанции.
    uint16_t us_gps_record_min_speed;         //мин. скорость
    float uc_hdop_fix_coordinates;

    GPS_INFO stGpsOldData;    //структура с предыдущими координатами
} TGpsRecordConfig;

typedef __packed struct
{
    uint16_t usTimeRecord;    //период сохранения данных напряжении
} TGpioRecordConfig;

/* лог работы устройства */
typedef __packed struct
{
    uint16_t auiCountRebootDevice[2];    //общее количество включений устройства за время работы(1-общее, 2-по выходу из
                                         //LowPower)
    uint16_t uiCountTimerStop;           //количество обнаружения состояния останов таймера
    uint16_t ausGpsFind[2];              //поиск GPS координат всего запросов из них 2 запрос - не удачные
    uint16_t ausGsmLog[2];               //мониторинг сети GSM
    uint16_t
        ausGsmPwrErr[4];                //подача питания на GSM модуль (1 - общее колво попыток, 2 - колво неудачных попоыток включить
                                        //GSM, 3 - сколько раз был низкий уровень сигнала, 4 - сколько раз не зарегался в сети)
    uint16_t ausServerConnectErr[2];    //установлено соединений с сервером, из них не удачных
    uint16_t usGsmFindErr;              //количество не удачных попыток зарегистрироваться в сети оператора не найдена
    uint16_t ausGsmWorkErr[2];          //количество отключений GSM модуля во время работы
    uint16_t usGsmGprsErr;              //количество не открытых GPRS сеансов
    uint16_t usServerErr;               //количество не полученных ответов от сервера за заданное время ожидания
    uint16_t usDeviceWakeup;            //общее количество циклов (пробуждений) устройства
    uint32_t uiDeviceWorkTime;          //общее время проведенное устройством во включенном состоянии(1,15,45,5)
    uint32_t uiTimePwrGps;              //суммарное время работы модуля  GPS
    uint32_t uiTimePwrGsm;              //суммарное время работы модуля  GSM
    uint16_t usRtcFail;                 //счетчик того, сколько раз в маяке сбита работа внешнего кварца и узла RTC
    uint16_t usEepromFail;              //счетчик битой конфигурации в Eeprom.
} TLogDevice;

//калибровочные данные
typedef __packed struct
{
    int8_t on;
    int8_t brake_num;    //текущий вектор
    int8_t g_num;
#define DSM_CALIB_VEC_NUM 7
    int16_t brake_vector[DSM_CALIB_VEC_NUM][3];    //вектор торможения
    int16_t g_vector[DSM_CALIB_VEC_NUM][3];        //вектор силы тяжести
} TDSm_Calib;

//пороговые значения для нарушений
typedef __packed struct
{
    uint16_t g_turn;
#define DEFAULT_G_TURN 370
    uint16_t g_brake;
#define DEFAULT_G_BRAKE 500
    uint16_t g_accel;
#define DEFAULT_G_ACCEL 300
    uint16_t g_shake;
    float k_j_shake;    //для отладки коэфф для связи g и j
#define DEFAULT_K_J_SHAKE 5
#define DEFAULT_G_SHAKE 900
    uint16_t w_shake;
#define DEFAULT_W_SHAKE 2500
    //пороговая длительность для нарушения
    uint16_t violation_dur_ms[4];
#define DEFAULT_DUR_BRAKE 100
#define DEFAULT_DUR_ACCEL 2000
#define DEFAULT_DUR_TURN 500
#define DEFAULT_DUR_W_SHAKE 1000
} TDSm_Limits;

//структура в еепром
typedef __packed struct
{
    TDSm_Calib stCalib;
    TDSm_Limits stLimits;
    uint8_t bDriverSimple;    //Включение стиля вождения и отладочной информации о стиле.
} TDSm_DATA;

typedef __packed struct
{
    char strServAddr[40];             // AGPS сервер
    char srtTokenServ[SIZE_TOKEN];    //Токен ключ
    uint16_t usFlagParam;             //Параметры запроса AGPS
    _Bool bUseAGps;
} TAGpsData;

typedef __packed struct
{
    TFirmwareConfig stFirmware;    //информация о прошивках
    TUserConfig stUser;            //конфиг пользователя
    TConnectConfig stConnect;      //настройки серверов
    TSimConfig stSim;              // USER APN
    TGsmBasicConfig stGsm;         //Общие настройки GSM
    TDeviceConfig stDevice;        //конфиг девайса
    uint16_t usMainCRC;

    TAccelConfig stAccel;        //настройки акселерометра
    TGpsRecordConfig stGps;      //конфиг GPS записи
    TGpioRecordConfig stGpio;    //конфиг входов
    TLogDevice stLogDevice;
#ifdef _DRIVE_SIMPLE_
    TDSm_DATA stDSm_Data;    //конфиг стиля вождения
#endif

    uint8_t
        a_ucReserved[SIZE_CONFIG_EEPROM - (sizeof(TFirmwareConfig) + sizeof(TConnectConfig) + sizeof(TSimConfig) +
                                              sizeof(TGsmBasicConfig) + sizeof(TAccelConfig) + sizeof(TUserConfig) +
                                              sizeof(TDeviceConfig) + sizeof(TGpsRecordConfig) + sizeof(TGpioRecordConfig) +

                                              sizeof(TLogDevice) +
#ifdef _DRIVE_SIMPLE_
                                              sizeof(TDSm_DATA) +
#endif
                                              sizeof(uint16_t) + sizeof(uint16_t))];
    uint16_t usAdditionalCRC;    //контрольная сумма основной части конфигурации

} TEepConfig;
#endif

#ifdef __cplusplus
}
#endif

#endif
