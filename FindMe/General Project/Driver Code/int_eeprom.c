#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "stm32l1xx.h"
#include "int_eeprom.h"

#include "stm32l1xx_conf.h"
#include "stm32l1xx_flash.h"

#include "arm_comm.h"
#include "eeprom.h"
#include "crc16.h"

uint8_t EE_RD_Byte(uint32_t offset)
{
    __IO uint8_t* address;

    address = (__IO uint8_t*)(BASE_EEPROM_ADDRESS + offset);

    return (*address);
}

FLASH_Status EE_WR_Byte(uint32_t offset, uint8_t data)
{
    uint32_t address;
    FLASH_Status status = FLASH_COMPLETE;
    if(EE_RD_Byte(offset) != data) {
        DATA_EEPROM_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR);

        address = (uint32_t)(BASE_EEPROM_ADDRESS + offset);
        status = DATA_EEPROM_ProgramByte(address, data);

        DATA_EEPROM_Lock();
    }
    return status;
}

#ifndef BOOTLOADER

void SaveConfigCMD(void)
{
    if(osKernelRunning())    //Проверяем запущена ли RTOS.
    {
        int cmd = CMD_EEPROM_SAVE_CONFIG;
        if(xEepromQueue != NULL) {
            xQueueSend(xEepromQueue, &cmd, NULL);
        }
    }
    else {
        SaveConfig();
    }
}

void InitConfig(INIT_CONFIG TypeInitConfig)
{
    if(TypeInitConfig == INIT_MAIN || TypeInitConfig == INIT_ALL) {
        /* TFirmwareConfig */
        SetIntNewFirmware(0);
        SetAddrFirmSer(STR_NULL);
        SetFlagsStatusFirmware(FIRMWARE_OK);
        SetDefAddrFirmSer(DEF_HTTP_SERVER);
        /****************************/

        /* TUserConfig */
        SetAccessPass(DEF_PASSWORD);
        SetUserTel(STR_NULL);
        SetMaskMessageUser(NO_MESS);
#if 0
      //SetUserTel(DEF_TEL);
      SetMaskMessageUser(fBUTTON_MESS_TEL);
#endif
        /****************************/

        /* TConnectConfig */
        SetAddrFirstServ(DEF_FIRST_SERVER);
        SetAddrSecondServ(DEF_SECOND_SERVER);
#ifdef FM3
        SetUseTypeServ(SECOND_SERVER);
        SetJamDetect(TRUE);
        SetModeDevice(STANDART);
        SetLedEnable(FALSE);
#endif
#ifdef FM4
        SetUseTypeServ(FIRST_SERVER);
        SetJamDetect(FALSE);
        SetModeDevice(TRACK_ION);
        SetLedEnable(TRUE);
#endif
        /****************************/

        /* TSimConfig */
        SetNameUserSimApn(STR_NULL);
        SetNameUserSimLogin(STR_NULL);
        SetNameUserSimPass(STR_NULL);
        SetManualModeSimFirst(FALSE);
        SetManualModeSimSecond(FALSE);
        SetUserSimPin(STR_NULL);
        SetPinLock(FALSE);

        SetScidFirstSim(STR_NULL);
        SetImeiFirstGsm(STR_NULL);
        /****************************/

        /* TGsmBasicConfig */
        SetGsmFindTimeout(DEF_GSM_TIMEOUT);
        SetGsmGprsTimeout(DEF_GPRS_TIMEOUT);
        SetLbsFindTimeout(DEF_MAX_TIME_FIND_LBS);
        SetGprsOpenCount(MC_COUNT);
        SetRoamingGprs(FALSE);
        SetAnswerTimeout(DEF_WAIT_SEND_OK);
        /****************************/

        /* TDeviceConfig */
        SetUserPwrDevice(USER_POWER_AUTO);
        SetTimeoutLowPwrMode1(DEF_TIMEOUT_LOW_POWER_MODE1);
        SetTimeoutLowPwrMode2(DEF_TIMEOUT_LOW_POWER_MODE2);
        SetTimeLowPwrMode1(DEF_SLEEP_LOW_POWER_MODE1);
        SetTimeLowPwrMode2(DEF_SLEEP_LOW_POWER_MODE2);
        SetLenDataFlashReady(DEF_FLASH_DATA_LEN);
        SetEnableUseLowPwr1Mode(TRUE);
        SetEnableUseLowPwr2Mode(TRUE);
        SetSleepTimeStandart(DEF_TIME_SLEEP_STANDART_DEVICE);
        SetSleepTimeFind(DEF_TIME_SLEEP_FIND_DEVICE);
        SetEnableAccelToFind(FALSE);

        SetTimeReconnect(0, DEF_TIME1_RECONNECT);
        SetTimeReconnect(1, DEF_TIME2_RECONNECT);
        SetTimeReconnect(2, DEF_TIME3_RECONNECT);
        SetTimeReconnect(3, DEF_TIME4_RECONNECT);
        SetTimeReconnect(4, DEF_TIME5_RECONNECT);
        SetCountReConnect(0);
        SetGpsWait(DEF_TIME_GPS_WAIT);
        SetMinTemperaturWorkDevice(MIN_TEMPERATUR_WORK);
        setTimeContactSecondSim(0);
        /****************************/
    }

    if(TypeInitConfig == INIT_ADD || TypeInitConfig == INIT_ALL) {
        SetAccelSensitivity(DEF_ACCEL_SENSITIVITY);
        SetAccelTimeCurrState(DEF_ACCEL_TIME_CURR_STATE);

        SetGpsRealtime(DEF_GPS_REAL_TIME_MODE);    // sec
        SetRecordAccel(DEF_GPS_RECORD_ACCEL_MODE1);
        SetGpsRealtime(DEF_GPS_REAL_TIME_MODE);    // sec
        SetGpsRecordtime(DEF_GPS_RECORD_TIME_MODE1);
        SetGpsRecordDistanse(DEF_GPS_DISTANCE);
        SetGpsRecordMinSpeed(MIN_SPEED_GPS);
        SetGpsRecordCourse(DEF_GPS_COURSE);
        SetGpioRecordTime(DEF_GPIO_TIME);
        SetGpsHdopFixCoordinates(DEF_HDOP_FIX_COORD);
        DriveSimpleInit();
    }
}

void deviceDefConfig(void)
{
#ifdef FM3
    SetJamDetect(TRUE);
    SetLedEnable(FALSE);
    SetModeDevice(STANDART);
#endif
#ifdef FM4
    SetJamDetect(FALSE);
    SetLedEnable(TRUE);
#endif

    /* TFirmwareConfig */
    SetIntNewFirmware(0);
    SetAddrFirmSer(STR_NULL);
    SetFlagsStatusFirmware(FIRMWARE_OK);
    SetDefAddrFirmSer(DEF_HTTP_SERVER);
    /****************************/

    SetUserTel(STR_NULL);
    SetMaskMessageUser(NO_MESS);
    SetNameUserSimApn(STR_NULL);
    SetNameUserSimLogin(STR_NULL);
    SetNameUserSimPass(STR_NULL);
    SetManualModeSimFirst(FALSE);
    SetManualModeSimSecond(FALSE);
    SetUserSimPin(STR_NULL);
    SetPinLock(FALSE);
    SetGsmFindTimeout(DEF_GSM_TIMEOUT);
    SetGsmGprsTimeout(DEF_GPRS_TIMEOUT);
    SetLbsFindTimeout(DEF_MAX_TIME_FIND_LBS);
    SetGprsOpenCount(MC_COUNT);
    SetRoamingGprs(FALSE);
    SetAnswerTimeout(DEF_WAIT_SEND_OK);
    SetUserPwrDevice(USER_POWER_AUTO);
    SetTimeoutLowPwrMode1(DEF_TIMEOUT_LOW_POWER_MODE1);
    SetTimeoutLowPwrMode2(DEF_TIMEOUT_LOW_POWER_MODE2);
    SetTimeLowPwrMode1(DEF_SLEEP_LOW_POWER_MODE1);
    SetTimeLowPwrMode2(DEF_SLEEP_LOW_POWER_MODE2);
    SetLenDataFlashReady(DEF_FLASH_DATA_LEN);
    SetEnableUseLowPwr1Mode(TRUE);
    SetEnableUseLowPwr2Mode(TRUE);
    SetSleepTimeStandart(DEF_TIME_SLEEP_STANDART_DEVICE);
    SetSleepTimeFind(DEF_TIME_SLEEP_FIND_DEVICE);
    SetEnableAccelToFind(FALSE);
    SetTimeReconnect(0, DEF_TIME1_RECONNECT);
    SetTimeReconnect(1, DEF_TIME2_RECONNECT);
    SetTimeReconnect(2, DEF_TIME3_RECONNECT);
    SetTimeReconnect(3, DEF_TIME4_RECONNECT);
    SetTimeReconnect(4, DEF_TIME5_RECONNECT);
    SetCountReConnect(0);
    SetGpsWait(DEF_TIME_GPS_WAIT);
    SetMinTemperaturWorkDevice(MIN_TEMPERATUR_WORK);
    SetAccelSensitivity(DEF_ACCEL_SENSITIVITY);
    SetAccelTimeCurrState(DEF_ACCEL_TIME_CURR_STATE);

    SetGpsRealtime(DEF_GPS_REAL_TIME_MODE);    // sec
    SetRecordAccel(DEF_GPS_RECORD_ACCEL_MODE1);
    SetGpsRealtime(DEF_GPS_REAL_TIME_MODE);    // sec
    SetGpsRecordtime(DEF_GPS_RECORD_TIME_MODE1);
    SetGpsRecordDistanse(DEF_GPS_DISTANCE);
    SetGpsRecordMinSpeed(MIN_SPEED_GPS);
    SetGpsRecordCourse(DEF_GPS_COURSE);
    SetGpioRecordTime(DEF_GPIO_TIME);
    SetGpsHdopFixCoordinates(DEF_HDOP_FIX_COORD);

    DriveSimpleInit();
    extern char g_aucBufDownHttpFirm[];
#define SERVER_TRUE_VALUE g_aucBufDownHttpFirm
    memset(SERVER_TRUE_VALUE, USER_CONFIG_DEVICE_PACKET, MAX_PARAM_CONFIG_VALUE);

    SaveConfigCMD();
}

uint8_t GetStatusDeviceReset(void)
{
    // return EE_RD_Byte(END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t));
    return (uint8_t)RTC_ReadBackupRegister(RESET_INFO_BACKUP_REGISTER);
}

void SetStatusDeviceReset(uint8_t ucResDevStat)
{
    PWR_RTCAccessCmd(ENABLE);
    while(RTC_ReadBackupRegister(RESET_INFO_BACKUP_REGISTER) != (uint32_t)ucResDevStat) {
        RTC_WriteBackupRegister(RESET_INFO_BACKUP_REGISTER, (uint32_t)ucResDevStat);
    }
    PWR_RTCAccessCmd(DISABLE);
}

void SetDeviceSleep(uint32_t uiTime)
{
    PWR_RTCAccessCmd(ENABLE);
    while(RTC_ReadBackupRegister(DEVICE_SLEEP) != uiTime) {
        RTC_WriteBackupRegister(DEVICE_SLEEP, uiTime);
    }
    PWR_RTCAccessCmd(DISABLE);
}

uint32_t GetDeviceSleep(void)
{
    return RTC_ReadBackupRegister(DEVICE_SLEEP);
}

void ResetFirmware(void)
{
    SetAddrFirmSer(STR_NULL);
    SaveConfigCMD();
}

//буфер для безопасного сохранения конфигурации
TEepConfig cfg_copy;
extern TEepConfig g_stEepConfig;
void SaveConfig(void)
{
    uint8_t* ptr = (uint8_t*)&cfg_copy;

    uint32_t uiLengthMainConfig = sizeof(TFirmwareConfig) + sizeof(TUserConfig) + sizeof(TConnectConfig) +
                                  sizeof(TSimConfig) + sizeof(TGsmBasicConfig) + sizeof(TDeviceConfig);

    uint32_t uiLengthAddConfig = sizeof(TEepConfig) - uiLengthMainConfig - 4;

    // Копирование главной структуры в копию с защитой копирования.
    __disable_interrupt();
    memcpy(&cfg_copy, &g_stEepConfig, sizeof(TEepConfig));
    __enable_interrupt();

    /* находим СRC двух типов конфигурации, основной и дополнительной */
    cfg_copy.usMainCRC = CRC16(ptr, uiLengthMainConfig);
    cfg_copy.usAdditionalCRC = CRC16(ptr + uiLengthMainConfig + sizeof(cfg_copy.usMainCRC), uiLengthAddConfig);

    g_stEepConfig.usMainCRC = cfg_copy.usMainCRC;
    g_stEepConfig.usAdditionalCRC = cfg_copy.usAdditionalCRC;

    //сохраняем конфигурацию
    for(uint32_t i = 0; i < sizeof(TEepConfig); i++) {
        EE_WR_Byte(CFG_BASE_ADR + i, ptr[i]);
    }

    //сохраняем копию конфигурации
    for(uint32_t i = 0; i < sizeof(TEepConfig); i++) {
        EE_WR_Byte(CFG_COPY_BASE_ADR + i, ptr[i]);
    }
}

_Bool readConfig(void)
{
    uint8_t* ptr_copy = (uint8_t*)&cfg_copy;
    uint8_t* ptr_general = (uint8_t*)&g_stEepConfig;
    uint16_t crc_main = 0;
    uint16_t crc_add = 0;
    _Bool integrity_main_copy = TRUE;
    _Bool integrity_add_copy = TRUE;
    _Bool err = FALSE;

    uint32_t uiLengthMainConfig = sizeof(TFirmwareConfig) + sizeof(TUserConfig) + sizeof(TConnectConfig) +
                                  sizeof(TSimConfig) + sizeof(TGsmBasicConfig) + sizeof(TDeviceConfig);

    uint32_t uiLengthAddConfig = sizeof(TEepConfig) - uiLengthMainConfig - 4;

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Clear WakeIp flag
    PWR_ClearFlag(PWR_FLAG_WU);

    SetStatusReset((RESET_STATUS_DEVICE)GetStatusDeviceReset());

    /* проверяем целостность копии конфигурации */
    for(uint32_t i = 0; i < sizeof(TEepConfig); i++)
        ptr_copy[i] = EE_RD_Byte(CFG_COPY_BASE_ADR + i);

    crc_main = CRC16(ptr_copy, uiLengthMainConfig);
    if(crc_main != cfg_copy.usMainCRC)
        integrity_main_copy = FALSE;    //Ставим флаг битой основной копии конфигурации.

    crc_add = CRC16(ptr_copy + uiLengthMainConfig + sizeof(crc_main), uiLengthAddConfig);
    if(crc_add != cfg_copy.usAdditionalCRC)
        integrity_add_copy = FALSE;    //Ставим флаг битой дополнительной копии конфигурации.

    /* проверяем целостность основной конфигурации */
    for(uint32_t i = 0; i < sizeof(TEepConfig); i++)
        ptr_general[i] = EE_RD_Byte(CFG_BASE_ADR + i);

    crc_main = CRC16(ptr_general, uiLengthMainConfig);
    if(crc_main != g_stEepConfig.usMainCRC) {
        /* Битая основная конфигурация главной структуры*/
        //Если копия цела то грузим копию, иначе иницилизируем дефолтом
        if(integrity_main_copy == TRUE) {
            for(uint32_t i = 0; i < uiLengthMainConfig; i++) {
                ptr_general[i] = ptr_copy[i];
            }
        }
        else {
            InitConfig(INIT_MAIN);
        }
        err = TRUE;
    }

    crc_add = CRC16(ptr_general + uiLengthMainConfig + sizeof(g_stEepConfig.usMainCRC), uiLengthAddConfig);
    if(crc_add != g_stEepConfig.usAdditionalCRC) {
        /* Битая дополнительная конфигурация главной структуры*/
        //Если копия цела то грузим копию, иначе иницилизируем дефолтом
        if(integrity_add_copy == TRUE) {
            for(uint32_t i = 0; i < uiLengthAddConfig; i++) {
                ptr_general[i + uiLengthMainConfig + sizeof(uint16_t)] =
                    ptr_copy[i + uiLengthMainConfig + sizeof(uint16_t)];
            }
        }
        else {
            InitConfig(INIT_ADD);
        }
        err = TRUE;
    }

    /* Проверяем не совпадение копии конфигурации и исправляем это */
    if(integrity_main_copy == FALSE) {
        for(uint32_t i = 0; i < uiLengthMainConfig; i++) {
            ptr_copy[i] = ptr_general[i];
        }
        err = TRUE;
    }
    if(integrity_add_copy == FALSE) {
        for(uint32_t i = 0; i < uiLengthAddConfig; i++) {
            ptr_copy[i + uiLengthMainConfig + sizeof(uint16_t)] = ptr_general[i + uiLengthMainConfig + sizeof(uint16_t)];
        }
        err = TRUE;
    }

    if(err) {
        SaveConfig();
    }

    return err;
}

void EepromHandler(void)
{
    CMD_EEPROM cmd = (CMD_EEPROM)0xFF;
    while(xQueueReceive(xEepromQueue, &cmd, 0)) {
        //обрабатываем команды для сохранения конфига
        switch(cmd) {
            case CMD_EEPROM_SAVE_START:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_WU_START);
                break;
            case CMD_EEPROM_SAVE_REG_USER:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_REG_USER);
                break;
            case CMD_EEPROM_SAVE_REG_TO_BASE:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_REG_TO_BASE);
                break;
            case CMD_EEPROM_SAVE_ERR_COLD:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_ERR_COLD);
                break;
            case CMD_EEPROM_SAVE_GOODBYE:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_GOODBYE);
                break;
            case CMD_EEPROM_SAVE_ERR_CONSRV:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_ERR_CONSRV);
                break;
            case CMD_TEST_DEVICE_OK:
                EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint16_t)), 0xAA);
                break;
            case CMD_EEPROM_SAVE_CONFIG:
                SaveConfig();
                break;

            default:
                DPS("D_ERR CMD SAVE CONFIG\r\n");
        }
    }
}

void SetTypeRegBase911(T_TYPE_CONNECT eType)
{
    if(osKernelRunning())    //Проверяем запущена ли RTOS.
    {
        int cmd = 0;
        if(eType == TYPE_WU_START) {
            cmd = CMD_EEPROM_SAVE_START;
        }
        else if(eType == TYPE_REG_USER) {
            cmd = CMD_EEPROM_SAVE_REG_USER;
        }
        else if(eType == TYPE_REG_TO_BASE) {
            cmd = CMD_EEPROM_SAVE_REG_TO_BASE;
        }
        else if(eType == TYPE_ERR_COLD) {
            cmd = CMD_EEPROM_SAVE_ERR_COLD;
        }
        else if(eType == TYPE_GOODBYE) {
            cmd = CMD_EEPROM_SAVE_GOODBYE;
        }
        else if(eType == TYPE_ERR_CONSRV) {
            cmd = CMD_EEPROM_SAVE_ERR_CONSRV;
        }
        if(xEepromQueue != NULL) {
            xQueueSend(xEepromQueue, &cmd, NULL);
        }
    }
    else {
        if(eType == TYPE_REG_USER) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_REG_USER);
        }
        else if(eType == TYPE_WU_START) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_WU_START);
        }
        else if(eType == TYPE_REG_TO_BASE) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_REG_TO_BASE);
        }
        else if(eType == TYPE_ERR_COLD) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_ERR_COLD);
        }
        else if(eType == TYPE_GOODBYE) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_GOODBYE);
        }
        else if(eType == TYPE_ERR_CONSRV) {
            EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)), TYPE_ERR_CONSRV);
        }
    }
}

_Bool GetCheckTestDevice(void)
{
    return (EE_RD_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint16_t))) != 0xAA);
}

void SetCheckTestDevice(void)
{
    if(osKernelRunning()) {    //Проверяем запущена ли RTOS.
        int cmd = CMD_TEST_DEVICE_OK;
        if(xEepromQueue != NULL) {
            xQueueSend(xEepromQueue, &cmd, NULL);
        }
    }
    else {
        EE_WR_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint16_t)), 0xAA);
    }
}

T_TYPE_CONNECT GetTypeRegBase911(void)
{
    uint8_t byte = EE_RD_Byte((END_EEPROM_ADDRESS - BASE_EEPROM_ADDRESS - sizeof(uint8_t)));
    if(byte == 0x00 || byte == 0xFF || byte == TYPE_REG_TO_BASE) {
        return TYPE_REG_TO_BASE;
    }
    else if(byte == TYPE_REG_USER) {
        return TYPE_REG_USER;
    }
    else if(byte == TYPE_ERR_COLD) {
        return TYPE_ERR_COLD;
    }
    else if(byte == TYPE_GOODBYE) {
        return TYPE_GOODBYE;
    }
    else if(byte == TYPE_ERR_CONSRV) {
        return TYPE_ERR_CONSRV;
    }

    return (T_TYPE_CONNECT)TYPE_WU_START;
}

#pragma optimize = none
_Bool ResetFlagSleep(void)
{
    PWR_RTCAccessCmd(ENABLE);
    _Bool bFlagSleep = RTC_ReadBackupRegister(FLAG_SLEEP);
    if(bFlagSleep) {
        while(RTC_ReadBackupRegister(FLAG_SLEEP) != 0) {
            RTC_WriteBackupRegister(FLAG_SLEEP, 0);
        }
    }
    PWR_RTCAccessCmd(DISABLE);
    return bFlagSleep;
}

void SetFlagSleep(void)
{
    PWR_RTCAccessCmd(ENABLE);
    while(RTC_ReadBackupRegister(FLAG_SLEEP) != 1) {
        RTC_WriteBackupRegister(FLAG_SLEEP, 1);
    }
    PWR_RTCAccessCmd(DISABLE);
}

#endif