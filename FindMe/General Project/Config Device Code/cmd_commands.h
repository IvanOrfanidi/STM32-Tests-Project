
#ifndef _CMD_COMMANDS_H_
#define _CMD_COMMANDS_H_

#include "includes.h"
#include "eeprom.h"
#include "int_eeprom.h"
#include "ram.h"
#include "global_eep.h"

extern TEepConfig g_stEepConfig;

typedef struct
{
    size_t adr;                                        //адрес переменной в озу
    const char* ptrName;                               //указатель на текстовую команду
    const char* ptrHelp;                               //указатель на краткое описание команды
    int (*pUserParse)(u8* buff, u8* parg, u16 len);    //пользовательская обработка команды
    float limits[2];                                   //минимальное и максимальное значения параметра
    u32 flags;                                         //флаги для настройки доступа и формата вывода

#define TYPE_U8 BIT(0)
#define TYPE_S8 BIT(1)
#define TYPE_U16 BIT(2)
#define TYPE_S16 BIT(3)
#define TYPE_U32 BIT(4)
#define TYPE_S32 BIT(5)
#define TYPE_FLOAT BIT(6)
#define NUM_MASK 0x007F

#define OUT_U32 BIT(7)
#define TYPE_STRING BIT(8)

#define LIM BIT(9)            //проверять ли макс и мин. значения
#define CFG_VAL BIT(10)       //сохранять ли в еепром
#define PASS_LVL_0 BIT(11)    //нужен пароль для редактирования переменной
#define PASS_LVL_1 BIT(12)    //нужен пароль для редактирования переменной
#define PASS_LVL_2 BIT(13)
#define SMS_ACCESS_DIS BIT(14)    //запрещено менять через смс
#define READ_DIS BIT(15)          //чтение параметра запрещено
#define READ_PASS BIT(16)         //пароль на чтение параметра
#define ECHO BIT(17)              //посылать ли эхо
#define RAM_VAL BIT(18)           //переменная только в ОЗУ
#define FLAG_OK BIT(19)
#define USER_PROCESSING BIT(20)
#define NOT_SUPPORTED_CMD BIT(21)

#define PASS_EN (PASS_LVL_0 | PASS_LVL_1 | PASS_LVL_2 | READ_PASS)
    u8 uclen;    //размер параметра, для контроля типа TYPE_STRING и записи в еепром

} CMD;

#define SHORT_CMD_RAM(var, name, help, limLOW, limUP, flags) \
    { (size_t)&var, name, help, NULL, limLOW, limUP, flags | RAM_VAL, sizeof(var) },

#define SHORT_CMD(var, name, help, limLOW, limUP, flags) \
    { (size_t)&var, name, help, NULL, limLOW, limUP, flags, sizeof(var) },
#define SHORT_CMD_STR(var, name, help, flags) { (size_t)&var, name, help, NULL, 0, 0, flags, sizeof(var) },

#define SHORT_CMD_USER(name, help, func, flags) { 0, name, help, func, 0, 0, flags, 0 },

#define DEF_FLAGS_U8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_S8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_S8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_U16 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U16 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_S16 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_S16 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_U32 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U32 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_FL (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_FLOAT | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_STR (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_USER (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | FLAG_OK | ECHO)
#define DEF_FLAGS_NO_OK (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | ECHO)

#define PASS_DEF_FLAGS (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK)
#define PASS_DEF_FLAGS_U8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define PASS_DEF_FLAGS_STR (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)
#define PASS_DEF_FLAGS_STR_USER (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)

static const CMD cmd_array[] = {

    /**********************************************
                  User
  **********************************************/
    SHORT_CMD_USER("PASS", "Password entry", pass_entry, PASS_DEF_FLAGS | READ_DIS | USER_PROCESSING)
        SHORT_CMD_USER("CPWD", "Password change", pass_change, DEF_FLAGS_USER)    //Команда смены пароля

/**********************************************
                Device
**********************************************/
#if FM3
    SHORT_CMD_USER("TEL#EMRG", "tel", CmdUserTel, DEF_FLAGS_USER)    //Смена номера пользователя
    SHORT_CMD(g_stEepConfig.stDevice.eModeDevice, "DEV#MODE", "1...3", STANDART, TIMER_FIND, DEF_FLAGS_U8)
        SHORT_CMD(g_stEepConfig.stDevice.bEnableAccelToFind,
            "MODE#FIND",
            "0 - disable; 1 - enable",
            FALSE,
            TRUE,
            DEF_FLAGS_U8)                                                                //Использование акселерометра в режиме поиска.
    SHORT_CMD_USER("GSM#2#SCID", "sim serial", InfoSimCardScidSecond, DEF_FLAGS_USER)    // SCID сим карты
    /* EXHORTATION ----------------------------*/
    SHORT_CMD_USER("EMRG#M",
        "0-OFF, 1-SMS, 2-Call, 3 - SMS+Call",
        CmdEmrg,
        DEF_FLAGS_USER)    //Тревожные уведомления (0 - OFF, 1 - SMS, 2 - Call, 3 - SMS + Call)
/*-----------------------------------------*/
#endif
#if FM4
    SHORT_CMD(g_stEepConfig.stDevice.eModeDevice, "DEV#MODE", "0...3", TRACK_ION, TIMER_FIND, DEF_FLAGS_U8)
#endif
        SHORT_CMD_USER("RESET", "Reboot Device", CmdResetDevice, DEF_FLAGS_USER)    //Перезагрузить девайс.
    SHORT_CMD_USER("CFG#RESET",
        "Reset Config",
        CmdResetConfig,
        DEF_FLAGS_USER)                                                               //Применить настройки по умолчанию (не влияет на адрес сервера и ключ шифрования)
    SHORT_CMD_USER("VER", "software revision", CmdGetNameFirmvare, DEF_FLAGS_USER)    //Версия софта
    SHORT_CMD_USER("ETRACK", "----", CmdEraseArchive, DEF_FLAGS_USER)                 //Отчиска флешки
    SHORT_CMD_USER(
        "SRV#1",
        "911->dev",
        CmdChangeServAndProt,
        DEF_FLAGS_USER)    //Смена сервера и типа протокола с 911 на dev(выполняется только если СИМ карта не заводская)

    /**********************************************
                   SIM, GPRS
   **********************************************/
    SHORT_CMD(g_stEepConfig.stSim.bPinLock,
        "SIM#LOCK",
        "0 - disable; 1 - enable",
        FALSE,
        TRUE,
        DEF_FLAGS_U8)                                                        //Вкл./выкл. ввода пин кода на запрос gsm модуля
    SHORT_CMD_USER("GSM#1#PIN", "sim Pin", CmdPinSimCard, DEF_FLAGS_USER)    //Пин код сим карты

    SHORT_CMD_STR(g_stEepConfig.stSim.strLoginFirst,
        "APN#1#USER",
        "set user",
        DEF_FLAGS_STR)    //Логин APN для соответствующей сим-карты
    SHORT_CMD_STR(g_stEepConfig.stSim.strPasswordFirst,
        "APN#1#PSW",
        "set Password",
        DEF_FLAGS_STR)    //Пароль APN для соответствующей сим-карты
    SHORT_CMD_STR(g_stEepConfig.stSim.strApnFirst,
        "APN#1#NAME",
        "set Apn",
        DEF_FLAGS_STR)    // APN оператора для соответствующей сим-карты

    SHORT_CMD_STR(g_stEepConfig.stSim.strLoginSecond,
        "APN#2#USER",
        "set user",
        DEF_FLAGS_STR)    //Логин APN для соответствующей сим-карты
    SHORT_CMD_STR(g_stEepConfig.stSim.strPasswordSecond,
        "APN#2#PSW",
        "set Password",
        DEF_FLAGS_STR)    //Пароль APN для соответствующей сим-карты
    SHORT_CMD_STR(g_stEepConfig.stSim.strApnSecond,
        "APN#2#NAME",
        "set Apn",
        DEF_FLAGS_STR)    // APN оператора для соответствующей сим-карты

    SHORT_CMD(g_stEepConfig.stSim.bManualModeSimFirst,
        "APN#1#MODE",
        "0 - auto; 1 - manual",
        0,
        1,
        DEF_FLAGS_U8)    //Режим выбора APN: 0 – автоматическое определение 1 – заданное настройками 2 - подставляем
                         //internet
    SHORT_CMD(g_stEepConfig.stSim.bManualModeSimSecond,
        "APN#2#MODE",
        "0 - auto; 1 - manual",
        0,
        1,
        DEF_FLAGS_U8)    //Режим выбора APN: 0 – автоматическое определение 1 – заданное настройками 2 - подставляем
                         //internet

    SHORT_CMD(g_stEepConfig.stSim.bJamDetect, "JMD#EN", "jamming enable", FALSE, TRUE, DEF_FLAGS_U8)

        SHORT_CMD_USER("GSM#IMEI", "gsm imei", InfoGsmModemImei, DEF_FLAGS_USER)        // IMEI модема
    SHORT_CMD_USER("GSM#1#SCID", "sim serial", InfoSimCardScidFirst, DEF_FLAGS_USER)    // SCID сим карты
    SHORT_CMD(g_stEepConfig.stGsm.bRoamingGprsEnable,
        "GSM#ROAM",
        "enable roaming",
        FALSE,
        TRUE,
        DEF_FLAGS_U8)    //Разрешение передачи данных в роуминге 0-запрещена, 1-разрещена.

    /**********************************************
                   GPS
   **********************************************/
    SHORT_CMD_USER("GPS#F1", "10...180 deg; 0-disable", CmdFilterCourse, DEF_FLAGS_USER)    //Фильтр gps по углу.
    SHORT_CMD_USER("GPS#F2",
        "5...65535 metre; 0-disable",
        CmdFilterDistance,
        DEF_FLAGS_USER)    //Фильтр дистанции в метрах
    SHORT_CMD_USER("GPS#F4",
        "5...30 km/h; 0-disable",
        CmdFilterMinSpeed,
        DEF_FLAGS_USER)    //Фильтр минимальной скорости в км/ч.

    SHORT_CMD(g_stEepConfig.stGps.us_gps_time_record_data,
        "GPS#T1",
        "0...65535 seconds",
        MIN_VAL_FTIME,
        MAX_VAL_FTIME,
        DEF_FLAGS_U16)
        SHORT_CMD_USER("GPS#T2", "60...3600 sec; 0-disable", CmdTimeFindGps, DEF_FLAGS_USER)    //Время поиска GPS
    SHORT_CMD(g_stEepConfig.stGps.b_gps_record_accel_data,
        "GPS#F3",
        "0 - off; 1 - on",
        FALSE,
        TRUE,
        DEF_FLAGS_U8)    //Фильтр по датчику движения
    SHORT_CMD(g_stEepConfig.stGps.us_gps_real_time_record_data,
        "RT#T1",
        "0...3600 seconds",
        MIN_VAL_RT,
        MAX_VAL_RT,
        DEF_FLAGS_U16)    //Период отправки реалтайма в секундах

    /**********************************************
                   POWER MODE
   **********************************************/
    SHORT_CMD_USER("PW#MODE",
        "1 - RUN, 2 - LOW POWER_1, 3 - LOW POWER_2, 0 - AUTO",
        CmdPwrMode,
        DEF_FLAGS_USER)    //перевести девайс в режим экономии энергопотребления.
    SHORT_CMD_USER("PW#TW1",
        "time in sleep low power1, min",
        CmdSetTimeModeLowPwr1,
        DEF_FLAGS_USER)    // 5, 1440,  //Время, при котором, без движения, устройство уйдет в режим LOW PWR1.
    SHORT_CMD_USER("PW#TW2",
        "time in sleep low power2, min",
        CmdSetTimeModeLowPwr2,
        DEF_FLAGS_USER)    // 10, 1440,//Время, при котором, без движения, устройство уйдет в режим LOW PWR2.
    SHORT_CMD(g_stEepConfig.stDevice.usTimeSleepLowPwrMode1,
        "PW#TS1",
        "time sleep low power1, min",
        MIN_VAL_LOW_POWER1,
        MAX_VAL_LOW_POWER1,
        DEF_FLAGS_U16)    //Время, в котором девайс будет в режиме LOW PWR1 в мин.
    SHORT_CMD(g_stEepConfig.stDevice.usTimeSleepLowPwrMode2,
        "PW#TS2",
        "time sleep low power2, min",
        MIN_VAL_LOW_POWER2,
        MAX_VAL_LOW_POWER2,
        DEF_FLAGS_U16)    //Время, в котором девайс будет в режиме LOW PWR2 в мин.

    SHORT_CMD(g_stEepConfig.stDevice.uiLenDataFlashReady,
        "PW#PACK",
        "packeg size in power mode, byte",
        1024,
        32768,
        DEF_FLAGS_U32)    //Минимальный размер накопленных данных для перехода в режим LOW PWR1.

    SHORT_CMD(g_stEepConfig.stDevice.bEnableUseLowPwr1Mode,
        "PW#EN1",
        "0 - off; 1 - on",
        FALSE,
        TRUE,
        DEF_FLAGS_U8)    //разрещение режима энегросбер 1
    SHORT_CMD(g_stEepConfig.stDevice.bEnableUseLowPwr2Mode,
        "PW#EN2",
        "0 - off; 1 - on",
        FALSE,
        TRUE,
        DEF_FLAGS_U8)    //разрещение режима энегросбер 2

    SHORT_CMD(g_stEepConfig.stDevice.uiSleepTimeStandart,
        "PW#SLS",
        "time sleep standart mode",
        MIN_VAL_SLEEP_TIME_STANDART,
        MAX_VAL_SLEEP_TIME_STANDART,
        DEF_FLAGS_U32)    //Время выхода в стандартном режиме
    SHORT_CMD(g_stEepConfig.stDevice.uiSleepTimeFind,
        "PW#SLF",
        "time sleep standart mode",
        MIN_VAL_SLEEP_TIME_FIND,
        MAX_VAL_SLEEP_TIME_FIND,
        DEF_FLAGS_U32)    //Время выхода в стандартном режиме

    /**********************************************
                    Accel
    **********************************************/
    SHORT_CMD_USER("MOVE#F2", "1..8", CmdSensitivityAccel, DEF_FLAGS_USER)    //Чувствительность акселерометра.
    SHORT_CMD(g_stEepConfig.stAccel.usTimeCurrState,
        "MOVE#F1",
        "1..3600",
        1,
        3600,
        DEF_FLAGS_U16)    //Инертность движения акселерометра в сек.

    /**********************************************
                   Update Firmware
   **********************************************/
    SHORT_CMD_USER("FWA",
        "load firmware server",
        CmdFirmLoad,
        DEF_FLAGS_USER)    //Команда для перепрошивки через принудительно заданный ftp сервер.
    SHORT_CMD_STR(g_stEepConfig.stFirmware.strDefNameFirmSer,
        "FWC",
        "config firmware server",
        DEF_FLAGS_STR)    //Настройка сервера где лежит прошивка

    SHORT_CMD(g_stEepConfig.stGpio.usTimeRecord, "IN#T", "0..65535", 0, 65535, DEF_FLAGS_U16)    //Период сохранения
                                                                                                 //данных GPIO секунд.

/**********************************************
                Drive Simple
**********************************************/
#ifdef _DRIVE_SIMPLE_
    SHORT_CMD(g_stEepConfig.stDSm_Data.bDriverSimple, "DS#MODE", "on-off driver simple", 0, 1, DEF_FLAGS_U8)
        SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.g_turn, "DS#TURN", "200-2000mG", 200, 2000, DEF_FLAGS_U16) SHORT_CMD(
            g_stEepConfig.stDSm_Data.stLimits.g_brake,
            "DS#BRAKE",
            "200-2000mG",
            200,
            2000,
            DEF_FLAGS_U16) SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.g_accel,
            "DS#ACC",
            "200-2000mG",
            200,
            2000,
            DEF_FLAGS_U16)
            SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.g_shake, "DS#G_SHAKE", "200-4000mG", 200, 4000, DEF_FLAGS_U16)
                SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.w_shake, "DS#W_SHAKE", "200-8000", 200, 8000, DEF_FLAGS_U16)
                    SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.k_j_shake, "DS#K_SHAKE", "0.0-10.0", 0, 10, DEF_FLAGS_FL)

                        SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.violation_dur_ms[0],
                            "DS#T_BRAKE",
                            "0-10000ms",
                            0,
                            10000,
                            DEF_FLAGS_U16) SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.violation_dur_ms[1],
                            "DS#T_ACC",
                            "0-10000ms",
                            0,
                            10000,
                            DEF_FLAGS_U16)
                            SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.violation_dur_ms[2],
                                "DS#T_TURN",
                                "0-10000ms",
                                0,
                                10000,
                                DEF_FLAGS_U16) SHORT_CMD(g_stEepConfig.stDSm_Data.stLimits.violation_dur_ms[3],
                                "DS#T_SHAKE",
                                "0-10000ms",
                                0,
                                10000,
                                DEF_FLAGS_U16)

                                SHORT_CMD(g_stEepConfig.stDSm_Data.stCalib.on, "DS#CALIB", "0-off, 3 - on", 0, 3, DEF_FLAGS_U8)

                                    SHORT_CMD_USER("DS#INIT", "drive style init", ds_init, DEF_FLAGS_USER)
                                        SHORT_CMD_USER("DS#VECTOR", "show calib data", ds_print_calib, DEF_FLAGS_USER)
#endif

    /**********************************************
                   More CMD
   **********************************************/
    SHORT_CMD_USER("GPS#D", "gebug gps", CmdDebugGps, DEF_FLAGS_USER)
        SHORT_CMD_USER("GSM#D", "gebug gsm", CmdDebugGsm, DEF_FLAGS_USER)
};

#endif