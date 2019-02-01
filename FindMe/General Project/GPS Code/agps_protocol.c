

#include "includes.h"
#include "agps_protocol.h"

#ifdef _AGPS_USE_

typedef __packed struct
{
    uint16_t SizeDataAGps;
    uint32_t TimeDataAGps;
} TAGpsSysData;

#define MAX_SIZE_DATA_AGPS 256    // GPS_TX_BUFFER_SIZE

void FlashDataSysAGpsWrite(uint16_t SizeDataAGps);
void FlashDataSysAGpsRead(TAGpsSysData* pAGpsSysData);

RET_INFO GprsSendDataInitAGps(void)
{
    int ret;
    memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
    GetAGpsAddrServ(g_asInpDataFrameBuffer);

    mc("at+sapbr=3,1,\"CONTYPE\",\"GPRS\"",
        5,
        MC_COUNT);    //задаем настройки подключения (3- Set bearer parameters; 1- Bearer is connected)
    osDelay(10);

    profile_gprs_connect(PROF_AGPS_SERVER);

    mc("at+sapbr=1,1", 5, MC_COUNT);    //Открыть несущую (1- Open bearer; 1- Bearer is connected)
    osDelay(10);

    //Проверим назначенный IP
    mc_get("at+sapbr=2,1", M_STRING, &g_stDataGsmInfo, 3, 3);
    /*
   if(g_stDataGsmInfo.m_type == M_STRING) {
     char *p = strstr(g_stDataGsmInfo.msg[0].str, "+SAPBR: 1,1");
     if(p <= 0) {
        return ERR_GPRS_ACTIVATE;
     }
   }
   osDelay(10);
   */

    mc("at+httpinit", 5, MC_COUNT);    //Инициализировать HTTP.
    osDelay(10);

    mc("at+httppara=\"CID\",1", 5, MC_COUNT);    //Получить индификатор http(обязательный параметр).
    osDelay(10);

    char* ptr_colon = strrchr((char*)g_asInpDataFrameBuffer, ':');    //ищем и убираем ":80"
    if(ptr_colon) {
        *ptr_colon = '\0';
    }

    char srtTokenServ[SIZE_TOKEN] = { 0 };
    GetAGpsTokenServ(srtTokenServ);
    sprintf(g_aucOutDataFrameBuffer, "at+httppara=\"URL\",\"http://%s", g_asInpDataFrameBuffer);
    sprintf(g_asInpDataFrameBuffer, "/GetOnlineData.ashx?token=%s;gnss=", srtTokenServ);
    strcat(g_aucOutDataFrameBuffer, g_asInpDataFrameBuffer);

    if(GetAGpsFlagParam() & GNSS_TYPE_GPS) {
        strcat(g_aucOutDataFrameBuffer, "gps,");
    }
    if(GetAGpsFlagParam() & GNSS_TYPE_GLO) {
        strcat(g_aucOutDataFrameBuffer, "glo,");
    }
    if(GetAGpsFlagParam() & GNSS_TYPE_QZSS) {
        strcat(g_aucOutDataFrameBuffer, "qzss,");
    }

    g_aucOutDataFrameBuffer[(strlen(g_aucOutDataFrameBuffer) - 1)] = ';';
    strcat(g_aucOutDataFrameBuffer, "datatype=");

    if(GetAGpsFlagParam() & DATA_TYPE_EPHEMERIS) {
        strcat(g_aucOutDataFrameBuffer, "eph,");
    }
    if(GetAGpsFlagParam() & DATA_TYPE_ALMANAC) {
        strcat(g_aucOutDataFrameBuffer, "alm,");
    }
    if(GetAGpsFlagParam() & DATA_TYPE_AUXILIARY) {
        strcat(g_aucOutDataFrameBuffer, "aux,");
    }
    if(GetAGpsFlagParam() & DATA_TYPE_POSITION) {
        strcat(g_aucOutDataFrameBuffer, "pos,");
    }

    g_aucOutDataFrameBuffer[strlen(g_aucOutDataFrameBuffer) - 1] = 0;

    if(GetAGpsFlagParam() & FILTER_EPHEMERIS_ON) {
        strcat(g_aucOutDataFrameBuffer, ";filteronpos");
    }

    strcat(g_aucOutDataFrameBuffer, "\"");
    mc(g_aucOutDataFrameBuffer, 5, 1);
    osDelay(10);

    /* Open HTTP Socket */
    uint8_t ucMaxCountConnectErr = 0;
    while(1) {
        ret = mc("at+httpaction=0", 5, 1);    ////Запросить данные методом GET.
        osDelay(SLEEP_MS_1000);

        if(ret == RET_OK) {
            DP_GSM("D_HTTP CONNECT\r\n");
            ucMaxCountConnectErr = 0;
            break;
        }
        else {
            osDelay(SLEEP_MS_10000);
            ucMaxCountConnectErr++;
        }
        if(ucMaxCountConnectErr > MAX_ERR_CONNECT_HTTP) {
            DP_GSM("D_ERR HTTP CONNECT\r\n");
            return ERR_ABORTED;    //устройство не может подключиться к HTTP серверу
        }
    }
    return RET_OK;
}

RET_INFO GprsDownloadDataServerAGps(void)
{
    int LenDataAGps = 0;
    uint16_t SizeDataAGps = 0;
    int DataAGpsLen = 0;
    int TotalSizeDataAGPS = 0;    //Полный размер данных передаваемых сервером в начале посылки.
    uint32_t StartAddress = ADDR_EXT_FLASH_DATA_AGPS;
    char asTempBuf[MAX_SIZE_DATA_AGPS];

    //Выполним отчистку флеш.
    FlashDataAGpsErase();

    TotalSizeDataAGPS = socket_read(PROF_AGPS_SERVER, g_aucOutDataFrameBuffer, sizeof(g_aucOutDataFrameBuffer));
    if(TotalSizeDataAGPS < 0) {
        FLASH_Give_Semaphore();
        DP_GSM("HTTP ERR: ");
        switch(TotalSizeDataAGPS) {
            case -401:
                DP_GSM("UNAUTHORIZED 401\r\n");
                break;
            case -426:
                DP_GSM("UPGRADE REQ 426\r\n");
                break;
            case -400:
                DP_GSM("BAD REQ 400\r\n");
                break;
            case -404:
                DP_GSM("NOT FOUND 404\r\n");
                break;
            default:
                DP_GSM("%d\r\n", TotalSizeDataAGPS);
        }
        return ERR_ABORTED;
    }

    memcpy(asTempBuf, &g_aucOutDataFrameBuffer[sizeof(g_aucOutDataFrameBuffer) - DataAGpsLen], DataAGpsLen);
    do {
        memset(asTempBuf + DataAGpsLen, 0, sizeof(asTempBuf) - DataAGpsLen);
        sprintf(
            g_asCmdBuf, "at+httpread=%d,%d", StartAddress - ADDR_EXT_FLASH_DATA_AGPS, MAX_SIZE_DATA_AGPS - DataAGpsLen);
        mc_send(g_asCmdBuf, NULL, NULL);
        LenDataAGps = socket_read(PROF_AGPS_SERVER, asTempBuf + DataAGpsLen, MAX_SIZE_DATA_AGPS - DataAGpsLen);

        if(LenDataAGps > 0) {
            EXT_FLASH_Write((uint8_t*)asTempBuf, StartAddress, LenDataAGps + DataAGpsLen);
            StartAddress += LenDataAGps + DataAGpsLen;
            SizeDataAGps += LenDataAGps + DataAGpsLen;
        }
        else {
            DP_GSM("D_DOWNLOAD END\r\n");
        }
        DataAGpsLen = 0;
        DP_GSM("N: %d\r\n", SizeDataAGps);
    } while(LenDataAGps > 0);

    FLASH_Give_Semaphore();

    mc("at+httpterm", 5, MC_COUNT);    //Завершить работу HTTP службы.
    osDelay(10);

    if(SizeDataAGps != TotalSizeDataAGPS) {
        return ERR_ABORTED;
    }
    //Запишем служебную информацию
    FlashDataSysAGpsWrite(SizeDataAGps);
    return RET_OK;
}

void FlashDataSysAGpsWrite(const uint16_t SizeDataAGps)
{
    int n = 0;
    uint8_t bitFree = 8;
    uint32_t StartAddress;
    char asTempBuf[MAX_SIZE_DATA_AGPS] = { 0 };

    FLASH_Take_Semaphore();
    uint32_t SecRTC = time();
    n += bit_packing(asTempBuf + n, SecRTC, &bitFree, 32);
    n += bit_packing(asTempBuf + n, SizeDataAGps, &bitFree, 16);

    StartAddress = ADDR_EXT_FLASH_DATA_AGPS + TOTAL_SIZE_DATA_AGPS - n;
    EXT_FLASH_Write((uint8_t*)asTempBuf, StartAddress, n);
    FLASH_Give_Semaphore();
}

void FlashDataSysAGpsRead(TAGpsSysData* pAGpsSysData)
{
    int n = 0;
    uint8_t bitFree = 8;
    uint32_t temp_buf;
    char asTempBuf[MAX_SIZE_DATA_AGPS] = { 0 };

    uint32_t StartAddress = ADDR_EXT_FLASH_DATA_AGPS + TOTAL_SIZE_DATA_AGPS - 6;

    FLASH_Take_Semaphore();
    EXT_FLASH_Read((uint8_t*)asTempBuf, StartAddress, 6);
    FLASH_Give_Semaphore();

    //Получим время записи. Первые 4 байта из флеш.
    n += bit_unpacking(asTempBuf + n, &temp_buf, &bitFree, 32);
    pAGpsSysData->TimeDataAGps = temp_buf;

    //Получим размер записи.
    n += bit_unpacking(asTempBuf + n, &temp_buf, &bitFree, 16);
    pAGpsSysData->SizeDataAGps = temp_buf;
}

void GprsDownloadDataModulAGps(void)
{
    TAGpsSysData stAGpsSysData;
    uint32_t StartAddress = ADDR_EXT_FLASH_DATA_AGPS;
    uint16_t LenDataAGps = 0;
    char asTempBuf[MAX_SIZE_DATA_AGPS];

    FlashDataSysAGpsRead(&stAGpsSysData);
    FLASH_Take_Semaphore();

    for(int i = 0; i < stAGpsSysData.SizeDataAGps / MAX_SIZE_DATA_AGPS + 1; i++) {
        memset(asTempBuf, 0, sizeof(asTempBuf));
        EXT_FLASH_Read((uint8_t*)asTempBuf, StartAddress, MAX_SIZE_DATA_AGPS);
        StartAddress += MAX_SIZE_DATA_AGPS;

        for(int n = 0; n < MAX_SIZE_DATA_AGPS; n++) {
            LenDataAGps++;
            if(LenDataAGps > stAGpsSysData.SizeDataAGps) {
                FLASH_Give_Semaphore();
                return;
            }
            USART_Write(UART_GPS, &asTempBuf[n], 1);
        }
    }
}

void FlashDataAGpsErase(void)
{
    DP_GSM("D_ERASE FLASH DATA AGPS\r\n");
    uint32_t StartAddress = ADDR_EXT_FLASH_DATA_AGPS;
    //Отчищаем 4 субсектора внешней flash для записи данных по AGPS.
    FLASH_Take_Semaphore();
    for(uint32_t i = StartAddress; i < (StartAddress + TOTAL_SIZE_DATA_AGPS); i += SIZE_SUBSECTOR_FLASH) {
        IWDG_ReloadCounter();    // Reload IWDG counter
        FlashSubSectorEarse(i);
        isEndWaitPeriod(100);
    }
    FLASH_Give_Semaphore();
}

#endif

/*Возвращает:
0 - Данные AGPS корректны и не требуют обновления;
1 - Требуется обновления данных.*/
_Bool UpdateDataAGps(void)
{
#ifndef _AGPS_USE_
    return 0;    //Данные AGPS не требуют обновления.
#else

    TAGpsSysData stAGpsSysData;
    FlashDataSysAGpsRead(&stAGpsSysData);

    uint32_t SecRTC = time();

    if(SecRTC <= stAGpsSysData.TimeDataAGps + TIME_DATA_VALID_AGPS) {
        DPS("D_DATA AGPS VALID\r\n");
        return 0;    //Данные AGPS не требуют обновления.
    }
    DPS("D_DATA AGPS NO VALID\r\n");
    return 1;
#endif
}
