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
//������ ������ ��������
#define COUNT_FLASH_PAGE (END_ADDRESS_CODE_INT_FLASH - (START_ADDRESS_CODE_INT_FLASH - 1)) / SIZE_RECORD_EXT_FLASH

#define MAX_COUNT_ERR_FLASH 5
#define MAX_COUNT_ERR_NEW_FIRM 10
#define MAX_COUNT_TIME_NEW_FIRM 1500

#define MAX_ERROR_DOWNLOAD_FIRMWARE 3

//������ ������ ���������� ��� ��������.
#define TOTAL_SIZE_FIRMWARE SIZE_SUBSECTOR_FLASH * 31    //
// ����� �� ������� ����, ��� �� ������ ����� ��������� ��������.
#define ADDR_EXT_FLASH_NEW_FIRMWARE (uint32_t)(SIZE_EXT_FLASH - TOTAL_SIZE_FIRMWARE)

#define TOTAL_SIZE_CONFIG_DATA SIZE_SUBSECTOR_FLASH * 2

#define ADDR_EXT_FLASH_CONFIG_DATA (uint32_t)(ADDR_EXT_FLASH_DATA_AGPS - TOTAL_SIZE_CONFIG_DATA)

#define __CONST_FIRM_VER (uint32_t)(0x0801FFF0)
#define __CONST_BUILD (uint32_t)(0x0801FFF4)

#define SIZE_CONFIG_EEPROM 768
#define ALL 0xFF

typedef enum FIRMWARE_TYPE {
    NO_BASE_FIRMWARE = 0,    //� ������� ���� ������ ��� ������� ��������.
    NO_NEW_FIRMWARE = 1,     //�������� � ���������� ������.
    UPDATE_FIRMWARE = 2,     //���������� ���������� �� ����� ��������.
    // 3 - ���� ������������ �� ������� ������� (�������) �������� �������� � enume STATUS_DEVICE///
    BASE_FIRMWARE = 4,             //���������� ���������� �� ����������-������� ��������.
    CONFIG_FIRMWARE = 5,           //������������ �������� � ������ ��������.
    INITIAL_FIRMWARE = 6,          //������������� ������� ��������(����� �� ��������� ��������� EEPROMA).
    END_PROGRAM_FIRMWARE = 7,      //���������� ������� ����������.
    FTP_DOWNLOAD_FIRMWARE = 8,     //����� FTP ������� ������� � �������� �������� ��� ����� ������.
    NEW_FIRMWARE = 9,              //����������� ������ � ����� ��������.
    INITIAL_BASE_FIRMWARE = 10,    //������� � ������ ��� ����������.
} FIRMWARE_TYPE;

// ������ �� ���������� ��������.
typedef enum FRAME_FIRMWARE_TYPE {
    FIRMWARE_OK = 0,                      // �������� ������� � ���������
    ERR_FIRMWARE_SIZE = 3,                // �� ��������� ������ ��������� ��������
    ERR_FIRMWARE_CRC = 4,                 // ������ ����������� �����
    ERR_CONNECT_FTP_OR_HTTP = 5,          // ���������� �� ����� ������������ � FTP �������
    ERR_FIRMWARE_FLASH = 6,               // ���������� �� ������ ��������� ���� ��������
    ERR_FIRMWARE_HARD = 7,                // �������� �� ������������� ������ ������
    ERR_FIRMWARE_VER = 8,                 // ��� ����� � ����� �������� �� ���������, ���� ������ �������� ������
    ERR_FIRMWARE_UNAUTHORIZED_401 = 9,    // �� �����������
    ERR_FIRMWARE_UPGRADE_REQ_426 = 10,    // ���������� ����������
    ERR_FIRMWARE_BAD_REQ_400 = 11,        // �������� ������
    ERR_FIRMWARE_NOT_FOUND_404 = 12,      // �� �������
} FRAME_FIRMWARE_TYPE;

/* ���������������� ������ ����������������� */
typedef enum USER_PWR_STATUS {
    USER_POWER_AUTO = 0,             //�������������� ����� ����������� �����������������
    USER_POWER_RUN_MODE = 1,         //������� ����� ������ ^_^
    USER_POWER_LOW_PWR1_MODE = 2,    //������ -_-
    USER_POWER_LOW_PWR2_MODE = 3,    //����      -_-oO
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
//���� ������������ DMA
V_GLOBAL _Bool g_bDmaGsmFail _EQU(FALSE);
// V_GLOBAL _Bool g_bDmaGsmFailAccept       _EQU(FALSE);

// FTP and Firmware Config
typedef __packed struct
{
    uint32_t uiNameFirmware;                     // Name work firmware(no use)
    uint32_t uiNameNewFirmware;                  // Name new firmware
    char strNameFirmSer[SIZE_SERV_FTP];          // Name ftp server and name firm file
    FRAME_FIRMWARE_TYPE eFlagsStatusFirmware;    // ���� ���������� ��������� ��������
    uint32_t uiSizeFirmware;                     // Size firmware
    char strDefNameFirmSer[SIZE_SERV_FTP];       // Default address Firmware serwer (online.irz.net/f.php?f=)
} TFirmwareConfig;

// User Config
typedef __packed struct
{
    char strPassword[SIZE_PASW];    //������ �� ������, �� ��������� '123456'.
    char strTel[SIZE_TEL];          //����� �������� ������������ ��������
    TYPE_MESSAGE eMaskMessage;      //����� �������� ����������� ��� �� �������.
} TUserConfig;

// Connect Config //
typedef __packed struct
{
    // ��������� ��������
    char str_name_first_server[SIZE_SERV];     //������ ���� dev
    char str_name_second_server[SIZE_SERV];    //������ ���� 911
    TYPE_SERVER eUseNumSer;                    //��� �������� �������
} TConnectConfig;

/* ��������� GPRS ���������������� ��� ����� */
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

    _Bool flagSimSecondInstld;          // ���� ��� ����������� ��������� SIM �����.
    uint32_t uiTimeContactSecondSim;    // ����� ����������� ������ � ��������� SIM.

    char strPin[SIZE_PIN_CODE];       // User Pin Code SIM.
    _Bool bPinLock;                   // Use User Pin.
    _Bool bJamDetect;                 //��������� ���� ������ ��������, 0-��� �����������, 1-� ������������.
    char strFIRST_SCID[SIZE_SCID];    // SCID ��������� ��� �����
    char strFIRST_IMEI[SIZE_IMEI];    // IMEI GSM module
    uint32_t uiWaitSMS;               //����� �������� ���(������� �������� ���� �� ����)
} TSimConfig;

/* GSM Modem Config */
typedef __packed struct
{
    uint8_t uc_gsm_find_timeout;       // ����� ������ gsm ����
    uint8_t uc_gprs_timeout;           // ����� �������� GPRS ���������� (���)
    uint8_t uc_lbs_find_timeout;       // ����� �������� ���������� � GSM ��������
    uint8_t uc_gprs_open_count;        // ���������� ������� ������������ GPRS ����������
    _Bool bRoamingGprsEnable;          // 1 - ���������� �������� ������ � ��������
    uint8_t uc_wait_answer_timeout;    // ����� �������� ������ �� ������� (���);
} TGsmBasicConfig;

// DEVICE Work Config
typedef __packed struct
{
    USER_PWR_STATUS
    eUserPwrDevice;                      // User Power Config Device(1 - RUN, 2 - LOW POWER_1, 3 - LOW POWER_2, 4 - SLEEP, 5 - AUTO)
    uint32_t uiTimeoutLowPwrMode1;       //����� �������� � ����� ����������������� LOW PWR 1 (����� � ������� �����������
                                         //��������).
    uint32_t uiTimeoutLowPwrMode2;       //����� �������� � ����� ����������������� LOW PWR 2 (����� � ������� �����������
                                         //��������).
    uint16_t usTimeSleepLowPwrMode1;     //����� ��� � ������ ����������������� LOW PWR 1 (����� � min ������� �����
                                         //�������� GSM �����).
    uint16_t usTimeSleepLowPwrMode2;     //����� ��� � ������ ����������������� LOW PWR 2 (����� � min ������� �����
                                         //�������� GSM �����).
    uint32_t uiLenDataFlashReady;        //������ ������ �� ������ ��� ���������� � ����� ����������������.
    _Bool bEnableUseLowPwr1Mode;         //���������� ������ ���������� 1.
    _Bool bEnableUseLowPwr2Mode;         //���������� ������ ���������� 2.
    uint32_t uiSleepTimeStandart;        //����� ��� � ����������� ������.
    uint32_t uiSleepTimeFind;            //����� ������ � ������ �����.
    _Bool bEnableAccelToFind;            //������������� ������������� � ������ ������.
    TYPE_MODE_DEV eModeDevice;           //�������� ������ �������(ION FM, STANDART, TIMER_FIND).
    TYPE_MODE_PROTOCOL eModeProtocol;    //�������� �� �������� ����� �������� ������.
    uint32_t a_uiTimeReConnect[5];       //���������������� ��� ������ �� ���� ��������, ������������� ����� ���������� �
                                         //�������� � ������ ���������� �����
    uint8_t ucCountReConnect;
    uint32_t uiGpsWait;           //����� ������ GPS ���������.
    _Bool bLedEnable;             // ���/���� ���������.
    int8_t cMinTemperaturWork;    //����������� ������� ����������� � �������� �������.
} TDeviceConfig;

/* ACCEL Config */
typedef __packed struct
{
    uint8_t ucSensitivity;       //���������������� ������������� 1...8
    uint16_t usTimeCurrState;    //����� ������ ����� ��������. ���� �������� ��� �� ��������� ����� �������, �� �������,
                                 //��� ������ �����.
} TAccelConfig;

// GPS Record Config Mode  //
typedef __packed struct
{
    uint16_t us_gps_real_time_record_data;    //����� ������ �� GPS � ������� Real Time.
    uint16_t us_gps_time_record_data;         //������ ������ �� �������.
    _Bool b_gps_record_accel_data;            //������ ������ ������ ��� ��������.
    uint8_t uc_gps_record_course;             //������ ������ �� �����.
    uint16_t us_gps_record_distance;          //������ �� ���������.
    uint16_t us_gps_record_min_speed;         //���. ��������
    float uc_hdop_fix_coordinates;

    GPS_INFO stGpsOldData;    //��������� � ����������� ������������
} TGpsRecordConfig;

typedef __packed struct
{
    uint16_t usTimeRecord;    //������ ���������� ������ ����������
} TGpioRecordConfig;

/* ��� ������ ���������� */
typedef __packed struct
{
    uint16_t auiCountRebootDevice[2];    //����� ���������� ��������� ���������� �� ����� ������(1-�����, 2-�� ������ ��
                                         //LowPower)
    uint16_t uiCountTimerStop;           //���������� ����������� ��������� ������� �������
    uint16_t ausGpsFind[2];              //����� GPS ��������� ����� �������� �� ��� 2 ������ - �� �������
    uint16_t ausGsmLog[2];               //���������� ���� GSM
    uint16_t
        ausGsmPwrErr[4];                //������ ������� �� GSM ������ (1 - ����� ����� �������, 2 - ����� ��������� �������� ��������
                                        //GSM, 3 - ������� ��� ��� ������ ������� �������, 4 - ������� ��� �� ��������� � ����)
    uint16_t ausServerConnectErr[2];    //����������� ���������� � ��������, �� ��� �� �������
    uint16_t usGsmFindErr;              //���������� �� ������� ������� ������������������ � ���� ��������� �� �������
    uint16_t ausGsmWorkErr[2];          //���������� ���������� GSM ������ �� ����� ������
    uint16_t usGsmGprsErr;              //���������� �� �������� GPRS �������
    uint16_t usServerErr;               //���������� �� ���������� ������� �� ������� �� �������� ����� ��������
    uint16_t usDeviceWakeup;            //����� ���������� ������ (�����������) ����������
    uint32_t uiDeviceWorkTime;          //����� ����� ����������� ����������� �� ���������� ���������(1,15,45,5)
    uint32_t uiTimePwrGps;              //��������� ����� ������ ������  GPS
    uint32_t uiTimePwrGsm;              //��������� ����� ������ ������  GSM
    uint16_t usRtcFail;                 //������� ����, ������� ��� � ����� ����� ������ �������� ������ � ���� RTC
    uint16_t usEepromFail;              //������� ����� ������������ � Eeprom.
} TLogDevice;

//������������� ������
typedef __packed struct
{
    int8_t on;
    int8_t brake_num;    //������� ������
    int8_t g_num;
#define DSM_CALIB_VEC_NUM 7
    int16_t brake_vector[DSM_CALIB_VEC_NUM][3];    //������ ����������
    int16_t g_vector[DSM_CALIB_VEC_NUM][3];        //������ ���� �������
} TDSm_Calib;

//��������� �������� ��� ���������
typedef __packed struct
{
    uint16_t g_turn;
#define DEFAULT_G_TURN 370
    uint16_t g_brake;
#define DEFAULT_G_BRAKE 500
    uint16_t g_accel;
#define DEFAULT_G_ACCEL 300
    uint16_t g_shake;
    float k_j_shake;    //��� ������� ����� ��� ����� g � j
#define DEFAULT_K_J_SHAKE 5
#define DEFAULT_G_SHAKE 900
    uint16_t w_shake;
#define DEFAULT_W_SHAKE 2500
    //��������� ������������ ��� ���������
    uint16_t violation_dur_ms[4];
#define DEFAULT_DUR_BRAKE 100
#define DEFAULT_DUR_ACCEL 2000
#define DEFAULT_DUR_TURN 500
#define DEFAULT_DUR_W_SHAKE 1000
} TDSm_Limits;

//��������� � ������
typedef __packed struct
{
    TDSm_Calib stCalib;
    TDSm_Limits stLimits;
    uint8_t bDriverSimple;    //��������� ����� �������� � ���������� ���������� � �����.
} TDSm_DATA;

typedef __packed struct
{
    char strServAddr[40];             // AGPS ������
    char srtTokenServ[SIZE_TOKEN];    //����� ����
    uint16_t usFlagParam;             //��������� ������� AGPS
    _Bool bUseAGps;
} TAGpsData;

typedef __packed struct
{
    TFirmwareConfig stFirmware;    //���������� � ���������
    TUserConfig stUser;            //������ ������������
    TConnectConfig stConnect;      //��������� ��������
    TSimConfig stSim;              // USER APN
    TGsmBasicConfig stGsm;         //����� ��������� GSM
    TDeviceConfig stDevice;        //������ �������
    uint16_t usMainCRC;

    TAccelConfig stAccel;        //��������� �������������
    TGpsRecordConfig stGps;      //������ GPS ������
    TGpioRecordConfig stGpio;    //������ ������
    TLogDevice stLogDevice;
#ifdef _DRIVE_SIMPLE_
    TDSm_DATA stDSm_Data;    //������ ����� ��������
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
    uint16_t usAdditionalCRC;    //����������� ����� �������� ����� ������������

} TEepConfig;
#endif

#ifdef __cplusplus
}
#endif

#endif
