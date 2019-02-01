
#include "includes.h"
#include "gsm_http.h"

char g_aucBufDownHttpFirm[SIZE_RECORD_EXT_FLASH];    //Буфер для хранения данных прошивки девайса.

static _Bool openHttpSoket();
static void settingsHttp();
int iLenDownloadFirwmware;
uint32_t AddressFlashFirmware;
/*
89.223.42.88/lfirm.php?f=1458917674&i=353437069574298&t=15&b=0 - пример запроса
f=1458917674 - имя прошивки
i=2142343335435 - IMEI модуля
b=0 - с какого байта читать файл
t=15 - ндивидуальный номер устройства (15 - FindMe3,  16 - FindMe4), добавлено в версииях устройств FM3 и FM4.
*/

uint8_t profile_http_read(PROF_CONNECT_SERVER usProf)
{
    int ret;
    _Bool bErrHttp;
    char strIMEI[SIZE_IMEI] = { '\0' };

    static char strBackNameServer[SIZE_SERV_FTP];
    char strNameSer[SIZE_SERV_FTP] = { 0 };
    memset(g_aucBufDownHttpFirm, 0, SIZE_RECORD_EXT_FLASH);
    GetAddrFirmSer(strNameSer);
    for(ret = 0; ret < SIZE_SERV_FTP; ret++) {
        if(strBackNameServer[ret] != strNameSer[ret]) {
            DP_GSM("D_NEW FIRMWARE FILE\r\n");
            //Отчищаем 31 субсектора внешней flash для записи прошивки.
            EraseFirmwareFlash();
            iLenDownloadFirwmware = 0;
            AddressFlashFirmware = ADDR_EXT_FLASH_NEW_FIRMWARE;
            loop(SIZE_SERV_FTP)
            {
                strBackNameServer[i] = strNameSer[i];
            }
            break;
        }
    }

    settingsHttp();    // Settings HTTP

    int ucMaxCountDownFirmErr = 0;
    while(iLenDownloadFirwmware <
          (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH))    //Основной цикл закачки прошивки
    {
        GetStrIMEI(strIMEI);
        profile_deactivate(usProf);    // Close HTTP Connect
        osDelay(SLEEP_MS_1000);
        sprintf(g_asCmdBuf, "at+httppara=\"URL\",\"http://%s", strNameSer);    // 89.223.42.88/lfirm.php?f=1458917674
        strcat(g_asCmdBuf, "&i=");
        strcat(g_asCmdBuf, strIMEI);    //&i=353437069574298
        strcat(g_asCmdBuf, "&t=");
        strcat(g_asCmdBuf, DEV_VER);    //&t=DEV_VER
        char strTemp[64];
        sprintf(strTemp, "&b=%d", iLenDownloadFirwmware);
        strcat(g_asCmdBuf, strTemp);    //&b=0
        strcat(g_asCmdBuf, "\"");
        mc(g_asCmdBuf, 5, 1);

        if(openHttpSoket()) {
            return ERR_CONNECT_FTP_OR_HTTP;
        }

        uint8_t ucMaxCountDownSesErr = 0;
        //Необходимый параметр чтения размера файла на HTTP.
        int iTotalSizeDataHTTP = 0;
        while(iTotalSizeDataHTTP <= 0) {
            iTotalSizeDataHTTP = socket_read(usProf, g_aucBufDownHttpFirm, SIZE_RECORD_EXT_FLASH);
            if(iTotalSizeDataHTTP == -400)
                return ERR_FIRMWARE_BAD_REQ_400;
            if(iTotalSizeDataHTTP == -401)
                return ERR_FIRMWARE_UNAUTHORIZED_401;
            if(iTotalSizeDataHTTP == -404)
                return ERR_FIRMWARE_NOT_FOUND_404;
            osDelay(SLEEP_MS_1000);
            ucMaxCountDownSesErr++;
            if(ucMaxCountDownSesErr > MAX_ERR_DOWN_SESSION) {
                DPS("-D_ERR HTTP SESSION CONNECT-\r\n");
                // return ERR_CONNECT_FTP_OR_HTTP;
                iTotalSizeDataHTTP =
                    (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH + 1) - iLenDownloadFirwmware;
                break;
            }
        }

        /* проверяем сходися ли размер прошивки */
        if((iTotalSizeDataHTTP + iLenDownloadFirwmware) !=
            (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH + 1)) {
            DPS("-D_ERR HTTP FIRM SIZE-\r\n");
            return ERR_FIRMWARE_SIZE;
        }

        int http_buf_data_len = 0;
        _Bool exit_ftp_or_http = FALSE;
        while(iLenDownloadFirwmware < (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH)) {
            /* Качаем и пишем одну страницу 256 */
            memset(g_aucBufDownHttpFirm, 0, SIZE_RECORD_EXT_FLASH);
            bErrHttp = FALSE;
            int iLen = 0;
            uint8_t ucTimeoutDownload = 0;
            while(http_buf_data_len != SIZE_RECORD_EXT_FLASH) {
                sprintf(g_asCmdBuf, "at+httpread=%d,%d", iLenDownloadFirwmware, SIZE_RECORD_EXT_FLASH - http_buf_data_len);
                mc_send(g_asCmdBuf, NULL, 0);
                iLen =
                    socket_read(usProf, g_aucBufDownHttpFirm + http_buf_data_len, SIZE_RECORD_EXT_FLASH - http_buf_data_len);
                if(iLen > 0)
                    http_buf_data_len += iLen;
                if(iLen < 0) {    // Error HTTP
                    switch(iLen) {
                        case -401:
                            return ERR_FIRMWARE_UNAUTHORIZED_401;
                        case -426:
                            return ERR_FIRMWARE_UPGRADE_REQ_426;
                        case -400:
                            return ERR_FIRMWARE_BAD_REQ_400;
                        case -404:
                            return ERR_FIRMWARE_NOT_FOUND_404;
                        default:
                            exit_ftp_or_http = 1;    //закачка прервалась:(
                    }
                }
                if(exit_ftp_or_http == 1) {
                    bErrHttp = 1;    //Закачка файла прервалась, файл закончился:(
                    break;
                }
                if(!(iLen)) {
                    osDelay(5000);
                    ucTimeoutDownload++;
                    if(ucTimeoutDownload >= MAX_ERR_DOWNLOAD_PAGE) {
                        bErrHttp = 1;    //Закачка файла прервалась, слишком много ошибок:(
                        break;
                    }
                }
            }
            http_buf_data_len = 0;

            if(bErrHttp == FALSE && exit_ftp_or_http == FALSE) {
                //Пишем страницу flash
                FLASH_Take_Semaphore();
                EXT_FLASH_Write((uint8_t*)g_aucBufDownHttpFirm, AddressFlashFirmware, SIZE_RECORD_EXT_FLASH);
                FLASH_Give_Semaphore();
                AddressFlashFirmware += SIZE_RECORD_EXT_FLASH;
                iLenDownloadFirwmware += SIZE_RECORD_EXT_FLASH;
            }
            else {
                ucMaxCountDownFirmErr++;
                DP_GSM("\rD__HTTP ERR DOWNLOAD\r\n");
                DP_GSM("D__HTTP RECONNECT\r\n");
                if(ucMaxCountDownFirmErr >= MAX_ERR_DOWN_FIRM_HTTP) {
                    // Fatal Download Firmware ;(
                    DPS("\r-FIRM ERR DOWN-\r\n");
                    return ERR_FIRMWARE_SIZE;
                }
                break;    //Выходим в основной цикл по Err.
            }
            DP_GSM("N:%d\r\n", iLenDownloadFirwmware);
        }
    }

    profile_deactivate(usProf);    // Close HTTP Connect
    iLenDownloadFirwmware = 0;
    memset(strBackNameServer, 0, SIZE_SERV_FTP);

    DPS("\r-D_FIRMWARE DOWNLOAD OK-\r\n");
    return FIRMWARE_OK;
}

FRAME_FIRMWARE_TYPE check_firmware(void)
{
    volatile uint16_t crc = 0xFFFF;
    volatile uint8_t count_err_flash = MAX_COUNT_ERR_FLASH;
    volatile uint32_t uiStartAddress = 0;
    crc = 0xFFFF;
    memset(g_aucBufDownHttpFirm, 0, SIZE_RECORD_EXT_FLASH);
    while(crc) {
        uiStartAddress = ADDR_EXT_FLASH_NEW_FIRMWARE;
        uint16_t CountPageData = COUNT_FLASH_PAGE;
        crc = 0xFFFF;
        /* Reload IWDG counter */
        IWDG_ReloadCounter();

        if(!(count_err_flash)) {
            DPS("-D_ERR FIRMWARE FLASH-\r\n");
            memset(g_aucBufDownHttpFirm, 0, SIZE_RECORD_EXT_FLASH);
            return ERR_FIRMWARE_FLASH;    // Выходим не обновив прошивку по ошибке записи во внешнию flash.
        }

        while(CountPageData) {
            IWDG_ReloadCounter();
            FLASH_Take_Semaphore();
            EXT_FLASH_Read((uint8_t*)g_aucBufDownHttpFirm, uiStartAddress, SIZE_RECORD_EXT_FLASH);
            FLASH_Give_Semaphore();
            crc = CRC16_FILL((uint8_t*)g_aucBufDownHttpFirm, SIZE_RECORD_EXT_FLASH, crc);

            uiStartAddress += SIZE_RECORD_EXT_FLASH;
            CountPageData--;
        }
        count_err_flash--;
    }

    memset(g_aucBufDownHttpFirm, 0, SIZE_RECORD_EXT_FLASH);
    DPS("\r-D_FIRMWARE OK-\r\n");
    return FIRMWARE_OK;
}

/* Функция отправляющая результат теста устройства при первом включении */
int sendDataTestDevice(PROF_CONNECT_SERVER usProf, char* pData)
{
    settingsHttp();    // Settings HTTP

    profile_deactivate(usProf);    // Close HTTP Connect
    osDelay(SLEEP_MS_1000);
    sprintf(g_asCmdBuf, "at+httppara=\"URL\",\"http://%s", pData);
    strcat(g_asCmdBuf, "\"");
    mc(g_asCmdBuf, 5, 1);

    if(openHttpSoket()) {
        return -1;
    }

    int ucMaxCountDownSesErr = 0;
    int ret = -1;
    while(ret <= 0) {
        ret = socket_read(usProf, g_aucBufDownHttpFirm, SIZE_RECORD_EXT_FLASH);
        if(!(ret)) {
            break;    // Server Ans OK
        }
        osDelay(SLEEP_MS_1000);
        ucMaxCountDownSesErr++;
        if(ucMaxCountDownSesErr > MAX_ERR_DOWN_SESSION) {
            DPS("-D_ERR HTTP SESSION CONNECT-\r\n");
            return -1;
        }
    }

    DPS("-D_HTTP SEND TEST DATA OK-\r\n");
    return 0;
}

/* Open HTTP Socket
return:
0 - HTTP Connect
1 - Error HTTP Connect
*/
static _Bool openHttpSoket(void)
{
    uint8_t ucMaxCountConnectErr = 0;
    while(1) {
        int ret = mc("at+httpaction=0", 5, 1);    ////Запросить данные методом GET.
        osDelay(SLEEP_MS_1000);

        if(!(ret)) {
            DP_GSM("D_HTTP CONNECT\r\n");
            return 0;
        }
        else {
            osDelay(SLEEP_MS_10000);
            ucMaxCountConnectErr++;
        }
        if(ucMaxCountConnectErr > MAX_ERR_CONNECT_HTTP) {
            DP_GSM("D_ERR HTTP CONNECT\r\n");
            return 1;    //устройство не может подключиться к HTTP серверу
        }
    }
}

/* Settings HTTP */
static void settingsHttp(void)
{
    mc("at+sapbr=3,1,\"CONTYPE\",\"GPRS\"",
        5,
        MC_COUNT);    //задаем настройки подключения (3- Set bearer parameters; 1- Bearer is connected)
    osDelay(10);
    profile_gprs_connect(1);
    mc("at+sapbr=1,1", 5, 1);    //Открыть несущую (1- Open bearer; 1- Bearer is connected)
    osDelay(10);
    mc("at+httpinit", 5, 1);    //Инициализировать HTTP.
    osDelay(10);
    mc("at+httppara=\"CID\",1", 5, 1);    //Получить индификатор http(обязательный параметр).
    osDelay(10);
}
