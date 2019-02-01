

#include "includes.h"
#include "gsm_ftp.h"

_Bool exit_ftp_or_http = 0;
int len_ftp = 0;
int rx_length = 0;
int state_ftp = 0;

int cnt_len_zero = 0;
GSM_INFO data_ftp;

uint8_t count_connect_ftp_server;

#define MAX_COUNT_ERR_FLASH 5

RET_INFO ProfileFtpSocketInit(void)
{
    RET_INFO ret = profile_ftp(PROF_FTP_SERVER);
    if(ret == RET_OK) {
        DP_GSM("___FTP SETTING OK\r");
    }
    else {
        DP_GSM("___FTP SETTING ERROR\r");
    }
    return ret;
}

RET_INFO DownloadFirmware(void)
{
    g_stEepConfig.stFirmware.eFlagsStatusFirmware = ERR_FIRMWARE_SIZE;    //Пока ставим флажок в ошибку.
    g_bNewFirmware = TRUE;
    if(g_stEepConfig.stFirmware.uiNameNewFirmware != g_uiNameBaseFirmware) {
        g_stEepConfig.stFirmware.eFlagsStatusFirmware =
            ftp_socket_read_firmware();    //Если прошивка не базовая, то качаем её и ставим флажки статуса скаченной
                                           //прошивки.
        SaveConfigCMD();
    }
    else {
        g_stEepConfig.stFirmware.eFlagsStatusFirmware = FIRMWARE_OK;    //Ставим флажок, что прошивка успешно скачалась.
        SaveConfigCMD();
        return RET_OK;    //Если базовая прошивка, то выходим без скачивания с ОК
    }

    if(g_stEepConfig.stFirmware.eFlagsStatusFirmware != FIRMWARE_OK) {
        return ERR_DOWNLOAD_FIRMWARE;
    }

    return RET_OK;
}

FRAME_FIRMWARE_TYPE ftp_socket_read_firmware(void)
{
    int ftp_buf_data_len = 0;
    int Len;
    uint16_t crc;
    memset(g_asTempBuf, 0, sizeof(g_asTempBuf));
    uint32_t StartAddress = ADDR_EXT_FLASH_NEW_FIRMWARE;
    //Отчищаем 32 субсектора внешней flash для записи прошивки.
    EraseFirmwareFlash();

    uint32_t StartAddressBack = 0;
    count_connect_ftp_server = g_stEepConfig.stGsm.uc_gprs_connect_count;
    ;
    uint16_t usCountPageDownload = 0;
    crc = 0xFFFF;

    while(!(exit_ftp_or_http)) {
        g_usTasksMonitorGsm++;
        IWDG_ReloadCounter();    // Reload IWDG counter
        if(GSM_STATUS_ON == 0) {
            DPS("!ERR POWER GSM!\r");
            return ERR_FIRMWARE_SIZE;
        }

        if(StartAddress == StartAddressBack) {
            count_connect_ftp_server--;
        }
        else {
            StartAddressBack = StartAddress;
            count_connect_ftp_server = g_stEepConfig.stGsm.uc_gprs_connect_count;
            ;
        }

        if(!(count_connect_ftp_server)) {
            DPS("!ERR FIRMWARE TIMEOUT!\r");    // Добавить переинициализацию
            return ERR_FIRMWARE_SIZE;
        }

        osDelay(100);
        Len = ftp_or_http_read(2, &g_asTempBuf[ftp_buf_data_len], SIZE_RECORD_EXT_FLASH - ftp_buf_data_len);
        ftp_buf_data_len += Len;

        if(ftp_buf_data_len != SIZE_RECORD_EXT_FLASH) {
            RET_INFO ret = wait_rx_ready(0, g_stEepConfig.stGsm.uc_gprs_timeout);
            if(ret != RET_GPRS_RECEIVE_OK) {
                if(usCountPageDownload != COUNT_FLASH_PAGE) {
                    isEndWaitPeriod(1000);
                }
                else {
                    break;
                }
            }
        }

        if((ftp_buf_data_len == -1) || (Len < 0)) {
            DPS("!ERR FIRMWARE LEN!\r");
            return ERR_FIRMWARE_SIZE;
        }

        if((ftp_buf_data_len == SIZE_RECORD_EXT_FLASH) || (exit_ftp_or_http == 1)) {
            if(ftp_buf_data_len)    // Здесь пишем страницу флеш, пишем 512 байт.
            {
                IWDG_ReloadCounter();    // Reload IWDG counter

                FLASH_Take_Semaphore();
                EXT_FLASH_Write((uint8_t*)&g_asTempBuf[0], StartAddress, SIZE_RECORD_EXT_FLASH / 2);
                EXT_FLASH_Write((uint8_t*)&g_asTempBuf[SIZE_RECORD_EXT_FLASH / 2],
                    StartAddress + SIZE_RECORD_EXT_FLASH / 2,
                    SIZE_RECORD_EXT_FLASH / 2);
                FLASH_Give_Semaphore();

                usCountPageDownload++;
                DP_GSM("N:%i\r", (COUNT_FLASH_PAGE - usCountPageDownload));
                crc = CRC16_FILL((uint8_t*)g_asTempBuf, SIZE_RECORD_EXT_FLASH, crc);

                StartAddress += SIZE_RECORD_EXT_FLASH;
            }
            ftp_buf_data_len = 0;
        }
    }

    if(crc) {
        DPS("!ERR FIRMWARE CRC!\r");
        return ERR_FIRMWARE_CRC;    // Выходим не обновив прошивку по ошибке записи во внешнию flash.
    }

    // usCountPageDownload = COUNT_FLASH_PAGE ;
    //Проверяем CRC на внешней Flash, читаем 256 байт.
    uint8_t count_err_flash = MAX_COUNT_ERR_FLASH;
    crc = 0xFFFF;
    memset(g_asTempBuf, 0, sizeof(g_asTempBuf));
    while(crc) {
        StartAddress = ADDR_EXT_FLASH_NEW_FIRMWARE;
        uint16_t CountPageData = usCountPageDownload;
        crc = 0xFFFF;
        /* Reload IWDG counter */
        IWDG_ReloadCounter();

        if(!(count_err_flash)) {
            DPS("!ERR FIRMWARE FLASH!\r");
            return ERR_FIRMWARE_FLASH;    // Выходим не обновив прошивку по ошибке записи во внешнию flash.
        }

        while(CountPageData) {
            IWDG_ReloadCounter();
            FLASH_Take_Semaphore();
            EXT_FLASH_Read((uint8_t*)g_asTempBuf, StartAddress, SIZE_RECORD_EXT_FLASH);
            FLASH_Give_Semaphore();
            crc = CRC16_FILL((uint8_t*)g_asTempBuf, SIZE_RECORD_EXT_FLASH, crc);

            StartAddress += SIZE_RECORD_EXT_FLASH;
            CountPageData--;
        }
        count_err_flash--;
    }

    DPS("\r-FIRMWARE OK-\r");
    return FIRMWARE_OK;
}

#ifdef GSM_MODULE_SIM800
RET_INFO profile_ftp(u8 nProf)
{
    RET_INFO ret = RET_OK;
    char strTemp[40];
    memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));

    g_stEepConfig.stFirmware.eFlagsStatusFirmware = ERR_CONNECT_FTP_OR_HTTP;
    SaveConfigCMD();

    /* справочные команды без котрых не откроется ftp сессия */
    if(ret == RET_OK) {
        ret = mc("at+sapbr=1,1", 4, MC_COUNT);    //Открытие GPRS соединения.
        osDelay(3000);
    }
    if(ret == RET_OK) {
        ret = mc("at+sapbr=2,1", 4, MC_COUNT);    //Запрос состояния GPRS соединения.
        osDelay(500);
    }
    /**************************************/

    if(ret == RET_OK) {
        //режим работы ftp (пассивный)
        ret = mc("at+ftpmode=1", 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        //тип передаваемых данных, двоичный режим(8bit).
        ret = mc("at+ftptype=\"I\"", 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        //установка CID параметра FTP сессии
        ret = mc("at+ftpcid=1", 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at+ftpun=\"%s\"", g_stEepConfig.stFirmware.strUserFtpSer);    // User login "monitor"
        ret = mc(g_asCmdBuf, 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at+ftppw=\"%s\"", g_stEepConfig.stFirmware.strPswFtpSer);    // Password "ftp"
        ret = mc(g_asCmdBuf, 4, MC_COUNT);
    }

    if((ret == RET_OK) && (strlen(g_stEepConfig.stFirmware.strNameFtpSer))) {
        u8 ucIndexNameServ;
        u8 ucIndexNameFile;
        u8 ucIndexPath;
        for(ucIndexNameServ = 0; ucIndexNameServ < strlen(g_stEepConfig.stFirmware.strNameFtpSer); ucIndexNameServ++) {
            if(g_stEepConfig.stFirmware.strNameFtpSer[ucIndexNameServ] == '/') {
                strTemp[ucIndexNameServ] = 0;
                break;
            }
            strTemp[ucIndexNameServ] = g_stEepConfig.stFirmware.strNameFtpSer[ucIndexNameServ];
        }
        sprintf(g_asCmdBuf, "at+ftpserv=\"%s\"", strTemp);    // Address ftp server        "ftp.irzonline.ru"
        ret = mc(g_asCmdBuf, 4, MC_COUNT);

        if(ret == RET_OK) {
            for(ucIndexNameFile = strlen(g_stEepConfig.stFirmware.strNameFtpSer); ucIndexNameFile > 0; ucIndexNameFile--) {
                if(g_stEepConfig.stFirmware.strNameFtpSer[ucIndexNameFile] == '/') {
                    strTemp[ucIndexNameFile] = 0;
                    break;
                }
            }
            strcpy(strTemp, &g_stEepConfig.stFirmware.strNameFtpSer[++ucIndexNameFile]);
        }
        sprintf(g_asCmdBuf, "at+ftpgetname=\"%s\"", strTemp);    // Name firmware file ""
        ret = mc(g_asCmdBuf, 4, MC_COUNT);

        if(ret == RET_OK) {
            for(ucIndexPath = 0; ucIndexPath < ucIndexNameFile - ucIndexNameServ; ucIndexPath++) {
                strTemp[ucIndexPath] = g_stEepConfig.stFirmware.strNameFtpSer[ucIndexPath + ucIndexNameServ];
            }
            strTemp[ucIndexPath] = 0;

            sprintf(g_asCmdBuf, "at+ftpgetpath=\"%s\"", strTemp);    // Path file    "/fm_fw/"
            ret = mc(g_asCmdBuf, 4, MC_COUNT);
        }
    }

    return ret;
}
#endif    // GSM_MODULE_SIM800

#ifdef GSM_MODULE_BGS2
RET_INFO profile_ftp(u8 nProf)
{
    RET_INFO ret = RET_OK;
    memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));

    // соединяемся с указанным адресом
    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at^siss=%d,srvtype,ftp", nProf);
        ret = mc(g_asCmdBuf, 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at^siss=%d,alphabet,1", nProf);
        ret = mc(g_asCmdBuf, 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at^siss=%d,conid,0", nProf);
        ret = mc(g_asCmdBuf, 4, MC_COUNT);
    }

    if(ret == RET_OK) {
        sprintf(g_asCmdBuf, "at^siss=%d,address,\"ftp://", nProf);
        strcat(g_asCmdBuf, g_stEepConfig.stFirmware.strUserFtpSer);    // User  login
        strcat(g_asCmdBuf, ":");
        strcat(g_asCmdBuf, g_stEepConfig.stFirmware.strPswFtpSer);    // Password
        strcat(g_asCmdBuf, "@");
        strcat(g_asCmdBuf, g_stEepConfig.stFirmware.strNameFtpSer);    // Name SRV and firm file
        strcat(g_asCmdBuf, ";type=i\"");

        // sprintf(g_asCmdBuf, "at^siss=%d,address,\"ftp://monitor:ftp@ftp.irzonline.ru/fm_fw/1455714330.bin;type=i\"",
        // nProf);

        ret = mc(g_asCmdBuf, 10, 10);
    }

    g_stEepConfig.stFirmware.eFlagsStatusFirmware = ERR_CONNECT_FTP_OR_HTTP;
    SaveConfigCMD();
    if(ret != RET_OK) {
        ret = ERR_SERVER_CONNECT;
    }

    return ret;
}
#endif    // nGSM_MODULE_SIM800

int ftp_or_http_read(uint8_t nProf /*<-В sim800 это не профиль, а команда на чтение*/, char* buf, int size)
{
    int ret = 0;
    len_ftp = 0;
    rx_length = 0;
    state_ftp = 0;
    exit_ftp_or_http = 0;
    cnt_len_zero = 0;
    u8 ucCountWait = 0;
    uint8_t ucTimeout = 0;
    int8_t count_server_wait = 3;
    char* pFindHTTP;
    if(nProf != 2) {
        // return -1;
    }

    // читаем ответ сервера
    state_ftp = 0;
    memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
    do {
        switch(state_ftp) {
            case 0:
                // запрашиваем блок данных размером size
                if(!(exit_ftp_or_http)) {
                    sprintf(g_asCmdBuf, "%s%d,%d", AT_SISR_FTP, nProf, size);
                    mc_send(g_asCmdBuf, NULL, NULL);
                    state_ftp = 1;
                    len_ftp = 0;
                }
            case 1:
                // ждём заголовок от команды at^sisr
                ret = gsm_parser(g_asCmdBuf, &data_ftp, g_asRxBuf, RX_BUFFER_SIZE, 15);
                if(ret == 0) {
                    return rx_length;
                }

                switch(data_ftp.m_type) {
                    case M_SIS:
                        for(int i = 0; i < sizeof(g_asRxBuf); i++) {
                            if(!(g_asRxBuf[i]))
                                g_asRxBuf[i] = 1;
                        }
                        pFindHTTP = strstr(g_asRxBuf, "401");
                        if(pFindHTTP > 0)
                            return -401;
                        pFindHTTP = strstr(g_asRxBuf, "426");
                        if(pFindHTTP > 0)
                            return -426;
                        pFindHTTP = strstr(g_asRxBuf, "400");
                        if(pFindHTTP > 0)
                            return -400;
                        pFindHTTP = strstr(g_asRxBuf, "404");
                        if(pFindHTTP > 0)
                            return -404;
                        break;
                    case M_FTPGET:
                        if((data_ftp.msg[0].var == nProf)) {
                            //Нам что то пришло.
                            state_ftp = 2;
                            cnt_len_zero = 0;
                            len_ftp = data_ftp.msg[1].var;    //Сколько пришло.
                            if(len_ftp == 0) {                //Данных больше нет.
                                state_ftp = 5;
                            }
                            break;
                        }
                        else {
                            //Информационное сообщение от SIM800 о состоянии соединения ("+FTPGET: 1,1")
                            if((data_ftp.msg[0].var == 1) && (data_ftp.msg[0].var != 1)) {
                                DP_GSM("___FTP ERROR CONNECT: ");
                                switch(data_ftp.msg[1].var) {
                                    case 61:
                                        DP_GSM("Net");
                                        break;
                                    case 62:
                                        DP_GSM("DNS");
                                        break;
                                    case 63:
                                        DP_GSM("Connect");
                                        break;
                                    case 64:
                                        DP_GSM("Timeout");
                                        break;
                                    case 65:
                                        DP_GSM("Server");
                                        break;
                                    case 66:
                                        DP_GSM("Operation not allow");
                                        break;
                                    case 70:
                                        DP_GSM("Replay");
                                        break;
                                    case 71:
                                        DP_GSM("User");
                                        break;
                                    case 72:
                                        DP_GSM("Password");
                                        break;
                                    case 73:
                                        DP_GSM("Type");
                                        break;
                                    case 74:
                                        DP_GSM("Rest");
                                        break;
                                    case 75:
                                        DP_GSM("Passive");
                                        break;
                                    case 76:
                                        DP_GSM("Active");
                                        break;
                                    case 77:
                                        DP_GSM("Operate");
                                        break;
                                    case 78:
                                        DP_GSM("Upload");
                                        break;
                                    case 79:
                                        DP_GSM("Download");
                                        break;
                                    case 86:
                                        DP_GSM("Manual quit");
                                        break;
                                    default:
                                        DP_GSM("Unknown error %i", data_ftp.msg[1].var);
                                        break;
                                        break;
                                }
                                DP_GSM("\r");
                                //Ответил ошибкой, ловить нам здесь больше нечего.
                                return -1;
                            }
                            break;
                        }
                        break;
                    case M_SISR:
                        if(data_ftp.msg[0].var == nProf) {
                            len_ftp = data_ftp.msg[1].var;
                            if(data_ftp.m_echo != 0) {
                                // получен ответ на команду
                                if(len_ftp > 0) {
                                    // переходим к режиму считывания данных
                                    state_ftp = 2;
                                    cnt_len_zero = 0;
                                }
                                else {
                                    if(len_ftp == 0) {
                                        state_ftp = 0;
                                        exit_ftp_or_http = 2;
                                    }
                                    else {
                                        state_ftp = 5;
                                        exit_ftp_or_http = 1;
                                        rx_length = -2;
                                    }
                                }
                                // isEndWaitPeriod(100);
                                DP_GSM("  [a]_SISR FTP/HTTP: %d,%d\n", data_ftp.msg[0].var, data_ftp.msg[1].var);
                                ucTimeout++;
                                if(ucTimeout > 50) {
                                    exit_ftp_or_http = 1;
                                    state_ftp = 5;
                                    rx_length = -2;
                                    return rx_length;
                                }
                            }
                            else {
                                if(data_ftp.msg[1].var != 1) {
                                    data_ftp.msg[1].var = data_ftp.msg[1].var;
                                    // получено инициативное сообщение
                                    // isEndWaitPeriod(100);
                                }
                                DP_GSM("[i] SISR FTP/HTTP: %d,%d\n", data_ftp.msg[0].var, data_ftp.msg[1].var);
                                if(len_ftp == 2 || len_ftp < 0) {
                                    // соединение закрыто данных больше не будет
                                    if(len_ftp == 2) {
                                        if(count_server_wait < 0) {
                                            state_ftp = 5;
                                            exit_ftp_or_http = 1;
                                        }
                                        count_server_wait--;
                                    }
                                    else {
                                        state_ftp = 5;
                                        exit_ftp_or_http = 1;
                                    }
                                }
                            }
                        }
                        break;

                    case M_OK:
                        if(len_ftp < 0 && exit_ftp_or_http != 0) {
                            // данные закончались, выходим
                            return rx_length;
                        }
                        else {
                            if(len_ftp == 0 && exit_ftp_or_http == 0) {
                                if(cnt_len_zero++ == 12) {
                                    // ftp не присылает данных в течении 3 мин
                                    return rx_length;
                                }
                                // переходим к ожиданию поступления данных
                                state_ftp = 4;
                            }
                            else {
                                // иначе повторяем запрос данных
                                state_ftp = 0;
                                exit_ftp_or_http = 0;
                                if(cnt_len_zero++ == 12) {
                                    // ftp не присылает данных в течении 3 мин
                                    return rx_length;
                                }
                            }
                        }
                        break;

                    case M_SYSSTART:
                        // GSM модуль перезагрузился во время выполнения команды
                        return -1;
                }
                break;

            case 2:
                // принимаем данные
                ret = gsm_read(buf, len_ftp - rx_length, 5);
                if(ret <= 0) {
                    ucCountWait++;
                    // модуль не отвечает, выходим
                    //не выходим, а подождем
                    isEndWaitPeriod(1000);
                    if(ucCountWait > 3) {
                        state_ftp = 5;
                    }
                    break;
                }
                /*
         DP_GSM("FTP/HTTP RX<-: ");
         for(int size_ret=0; size_ret<ret; size_ret++) {
            DP_GSM("0x%02X ", buf[size_ret]);
         }
         DP_GSM("\r");
         */
                count_server_wait = 3;
                ucCountWait = 0;
                rx_length += ret;
                if(len_ftp > rx_length) {
                    break;
                }
                state_ftp = 3;

            case 3:
                // ждём окончания команды
                ret = gsm_parser(0, &data_ftp, g_asRxBuf, RX_BUFFER_SIZE, 15);
                if(ret == 0) {
                    // timeout, выходим
                    GSM_DC(g_asCmdBuf, 't');
                    state_ftp = 5;
                    break;
                }

                switch(data_ftp.m_type) {
                    case M_OK:
                        ret = strlen(g_asCmdBuf), sprintf(g_asCmdBuf + ret, " (%d)", rx_length);
                        GSM_DC(g_asCmdBuf, '0');
                        state_ftp = 5;
                        break;

                    case M_SYSSTART:
                        return -1;
                }
                break;

            case 4:

                // ждём инициативное сообщение at^sisr=0,1(SIM800 сюда не заходит)
                ret = gsm_parser(g_asCmdBuf, &data_ftp, g_asRxBuf, RX_BUFFER_SIZE, 15);
                if(ret == 0) {
                    state_ftp = 0;
                    break;
                }

                switch(data_ftp.m_type) {
                    case M_SISR:
                        if(data_ftp.msg[0].var == nProf) {
                            len_ftp = data_ftp.msg[1].var;
                            DP_GSM("[i] SISR FTP/HTTP: %d,%d\n", data_ftp.msg[0].var, data_ftp.msg[1].var);
                            if(len_ftp == 2 || len_ftp < 0) {
                                if(len_ftp == 2) {
                                    if(count_server_wait < 0) {
                                        state_ftp = 5;
                                        exit_ftp_or_http = 1;
                                    }
                                    count_server_wait--;
                                }
                                else {
                                    state_ftp = 5;
                                    exit_ftp_or_http = 1;
                                }
                            }
                            else if(len_ftp == 1) {
                                // получено инициативное сообщение о приходе данных
                                state_ftp = 0;
                            }
                        }
                        break;

                    case M_SYSSTART:
                        // GSM модуль перезагрузился во время выполнения команды
                        return -1;
                        break;
                }
                break;
        }
    } while(state_ftp < 5);

    return rx_length;
}