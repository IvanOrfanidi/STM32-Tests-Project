#include "includes.h"
#include "gsm_mc52iT.h"

int gsm_read(char* pBuf, int size, uint32_t wait)
{
    int n;
    int Len;
    for(n = 0; n < size;) {
        Len = USART_Rx_Len(UART_GSM);
        if((Len) || g_bDmaGsmFail)    //допускается переполнение только при чтении смс поэтому добавлен второй флаг
        {
            USART_Read(UART_GSM, pBuf + n, 1);
            n++;
        }
        else {
            if(GSM_STATUS_ON == 0) {
                // GSM модуль выключился во время работы
                return -1;
            }
            if(isEndWaitPeriod(wait * 10)) {
                break;
            }
        }
    }
    return n;
}

int gsm_gets(char* pBuf, int size, uint32_t wait)
{
    int n;

    for(n = 0; n < size;) {
        int Len = USART_Rx_Len(UART_GSM);

        if(Len > 0) {
            USART_Read(UART_GSM, pBuf + n, 1);
            if(pBuf[n] != '\r') {
                if(pBuf[n] == '\n') {
                    if(n > 0)
                        break;
                }
                else {
                    n++;
                }
            }
        }
        else {
            if(GSM_STATUS_ON == 0) {
                // GSM модуль выключился во время работы
                return -1;
            }

            if(isEndWaitPeriod(wait * 10))
                break;
        }
    }

    if(n < size)
        pBuf[n] = 0;

    return n;
}

void gsm_write(const char* pBuf, int size, uint32_t pause)
{
    if(pause != 0) {
        for(int n = 0; n < size; n++) {
            USART_Write(UART_GSM, &pBuf[n], 1);
            isEndWaitPeriod(pause);
        }
    }
    else {
        USART_Write(UART_GSM, pBuf, size);
    }
}

void mc_send(const char* pCmd, char* pPrm, uint32_t pause)
{
    int size_mc = 0;
    // выполняем команду
    if(pCmd != 0) {
        size_mc = strlen(pCmd);
        if(size_mc > 0) {
            gsm_write(pCmd, size_mc, pause);

            if(pPrm != 0) {
                size_mc = strlen(pPrm);
                if(size_mc > 0) {
                    gsm_write(pPrm, size_mc, pause);
                }
            }
            gsm_write("\r", 1, pause);
        }
    }
}

RET_INFO SetGsmFunctional(GSM_FUNCTIONALITI eState)
{
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));

    /* Full functionality */
    if(eState == FULL_FUNCTIONALITI) {
        mc_get("at+cfun=1", M_STRING, &data_mc, 3, 3);
    }

    /* Minimum functionality */
    if(eState == MIN_FUNCTIONALITI) {
        mc_get("at+cfun=0", M_STRING, &data_mc, 3, 3);
    }

    return RET_OK;
}

// выполнить команду и получить в ответ данные
// данные приходят сразу за командой и состоят из одной строки
int mc_get(const char* pCmd, u8 m_type, GSM_INFO* pOut, u8 count, u32 second)
{
    GSM_INFO out_mc;
    memset(&out_mc, 0, sizeof(out_mc));
    int size_mc = 0;
    int state_mc = -1;
    int n_size = 0;
    memset(pOut, 0, sizeof(GSM_INFO));

    for(; count > 0; count--) {
        // выполняем команду
        mc_send(pCmd, NULL, 0);

        // portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        // vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_100 / portTICK_RATE_MS));

        for(;;) {
            int iRxSize = gsm_parser(pCmd, &out_mc, g_asRxBuf + size_mc, sizeof(g_asRxBuf) - size_mc, second);
            if(iRxSize <= 0) {
                if(iRxSize == -1) {
                    g_bDmaGsmFail = FALSE;
                    DS_GSM("[f] ", pCmd);
                }
                else {
                    // истекло время ожидания ответа на команду
                    DS_GSM("[t] ", pCmd);
                }
                return ERR_TIMEOUT;
            }

            if(state_mc < 0) {
                if(m_type == out_mc.m_type || m_type == M_OTHER) {
                    // копируем принятые данные
                    switch(out_mc.m_type) {
                        case M_CSQ:
                        case M_CGATT:
                        case M_COPS:
                        case M_CREG:
                        case M_SCID:
                        case M_SISI:
                        case M_SISW:
                        case M_0_CONNECT_OK:
                        case M_1_CONNECT_OK:
                        case M_0_CONNECT_FAIL:
                        case M_1_CONNECT_FAIL:
                        case M_0_SEND_OK:
                        case M_1_SEND_OK:
                        case M_RECEIVE:
                        case M_SISR:
                        case M_SCTM:
                        case M_STRING:
                        case M_SPIC:
                        case M_FTPGET:
                        case M_CENG:
                            for(n_size = 0; n_size < out_mc.count; n_size++) {
                                size_mc += out_mc.msg[n_size].size + 1;
                            }
                        case M_SIM_PIN:
                        case M_SIM_PUK:
                        case M_SIM_READY:
                        case M_SIND_NITZ:
                            memcpy(pOut, &out_mc, sizeof(GSM_INFO));
                            state_mc++;
                            break;
                    }
                }
                else if(m_type == M_OTHER_PIN) {
                    // копируем принятые данные
                    switch(out_mc.m_type) {
                        case M_SIM_PIN:
                        case M_SIM_PUK:
                        case M_SIM_READY:
                            pOut->m_type = out_mc.m_type;
                            state_mc++;
                            break;
                    }
                }
            }

            if(out_mc.m_type == M_OK) {
                // команда выполнена
                // GSM_DC(pCmd, '0');
                DS_GSM("[+] ", pCmd);

                if(state_mc == 0) {
                    // данные не получены
                    return ERR_TIMEOUT;
                }

                // данные не получены
                return ERR_CMD;
            }

            if(out_mc.m_type == M_ERROR || out_mc.m_type == M_ABORTED) {
                // получен ответ неверная команда
                // GSM_DC(pCmd, '4');
                DS_GSM("[-] ", pCmd);
                return ERR_CMD;
            }
            if(out_mc.m_type == M_RDY)
                return ERR_RESTART;
        }
    }

    return out_mc.m_type;
}

RET_INFO isOK(const char* pCmd, uint32_t wait)
{
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));

    for(;;) {
        int iRxSize = gsm_parser(pCmd, &data_mc, g_asRxBuf, sizeof(g_asRxBuf), wait);
        if(iRxSize <= 0) {
            // истекло время ожидания ответа на команду
            // GSM_DC(pCmd, 't');
            DS_GSM("[t] ", pCmd);
            return ERR_TIMEOUT;
        }

        switch(data_mc.m_type) {
            case M_OK:
                // команда выполнена
                // GSM_DC(pCmd,'0');
                DS_GSM("[+] ", pCmd);
                return RET_OK;
            case M_CME_ERROR:
            case M_ERROR:
                // получен ответ не верная команда
                // GSM_DC(pCmd, '4');
                DS_GSM("[-]", pCmd);
                return ERR_CMD;
            case M_ABORTED:
                // получен ответ команда прервана
                // GSM_DC(pCmd, '4');
                DS_GSM("[-] ", pCmd);
                return ERR_ABORTED;
            case M_0_CLOSE_OK:
                DS_GSM("[+] ", pCmd);
                return RET_CLOSE0;    //Закрытие соединения
            case M_1_CLOSE_OK:
                DS_GSM("[+] ", pCmd);
                return RET_CLOSE1;    //Закрытие соединения
            case M_RDY:
                // GSM модуль перезагрузился во время выполнения команды
                return ERR_RESTART;
        }
        // ответ не распознан
    }
}

RET_INFO ModemSettingStart(void)
{
    // mc("at", 4);
    // не установлена скорость работы
    mc_send("at+ipr=115200", NULL, 0);

    if(isOK("at+ipr=115200", 2) == RET_OK) {
        return RET_OK;
    }
    return ERR_POWER;
}

RET_INFO GsmModemCmdOn(void)
{
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));

    gsm_parser(0, &data_mc, g_asRxBuf, sizeof(g_asRxBuf), 10);

    if(data_mc.m_type == M_RDY) {
        DP_GSM("D_RDY\r\n");
        osDelay(2000);
        return RET_OK;
    }

    if(data_mc.m_type == M_SIM_READY) {
        DP_GSM("D_CPIN: READY\r\n");
        osDelay(2000);
        return RET_OK;
    }

    if(data_mc.m_type == M_CALL_READY) {
        DP_GSM("D_CALL READY\r\n");
        osDelay(2000);
        return RET_OK;
    }

    // mc(ATZ, 5, MC_COUNT);
    return ERR_TIMEOUT;
}

RET_INFO modem_jamming_detected(uint8_t ucTimeFindJammingDetect)
{
    GSM_INFO data_mc;
    int ret;
    int iCountNoJD;
    iCountNoJD = 30;
    mc(AT_JD_ON, 4, MC_COUNT);    //Включаем режим детектирования.
    osDelay(100);
    for(uint8_t i = 0; i < ucTimeFindJammingDetect; i++) {
        ret = gsm_parser(0, &data_mc, g_asRxBuf, sizeof(g_asRxBuf), 10);
        if(ret > 0) {
            if(data_mc.m_type == M_STRING) {
                char* pFindNoJD = strstr(data_mc.msg[0].str, "+SJDR: NO JAMMING");
                if(pFindNoJD > 0) {
                    iCountNoJD--;
                    if(!(iCountNoJD)) {
                        mc(AT_JD_OFF, 4, MC_COUNT);    //Выключаем режим детектирования.
                        return RET_OK;
                    }
                }
                char* pFindYesJD = strstr(data_mc.msg[0].str, "+SJDR: JAMMING DETECTED");
                if(pFindYesJD > 0) {
                    mc(AT_JD_OFF, 4, MC_COUNT);    //Выключаем режим детектирования.
                    return RET_JD_OK;
                }
                char* pFindInterferJD = strstr(data_mc.msg[0].str, "+SJDR: INTERFERENCE DETECTED");
                if(pFindInterferJD > 0) {
                    mc(AT_JD_OFF, 4, MC_COUNT);    //Выключаем режим детектирования.
                    return RET_JD_OK;
                }
            }
        }
        if(ret < 0)
            break;
        osDelay(1000);
    }
    mc(AT_JD_OFF, 4, MC_COUNT);    //Выключаем режим детектирования.
    return ERR_GPRS_TIMEOUT;
}

RET_INFO configGsmModule(void)
{
#if defined(MULTI_IP_CONNECTION_ENABLE) && defined(AT_CIPMUX)
    // Multi-IP Connection
    if(mc(AT_CIPMUX, 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }
#endif

    // Show lac and bsic information
    if(mc(AT_CNETSCAN1, 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }

    //включить сообщение о количестве байт принятых по GPRS перед самими данными
    if(mc(AT_CIPHEAD1, 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }

#ifdef SMS_PDU
    // PDU режим работы с смс
    if(mc("at+cmgf=0", 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }
#endif

#ifdef SMS_TEXT
    // текстовый режим работы с смс
    if(mc("at+cmgf=1", 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }
#endif

#ifdef CLI_ENABLE
    //включить определение входящего номера
    if(mc("at+clip=1", 5, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }
#endif

    if(mc("at&w", 4, MC_COUNT) != RET_OK) {
        return ERR_CMD;
    }

    return RET_OK;
}

RET_INFO initGsmModule(void)
{
    // включить эхо
    if(mc("ate1", 4, MC_COUNT) != RET_OK) {
        return ERR_TIMEOUT;
    }
    // osDelay(10);

    //Уровень информации об ошибке. 0, — отключено. Будет просто писать ERROR. 1, — код ошибки. Будет возвращать цифровой
    //код ошибки. 2, — описание ошибки.
    if(mc("at+cmee=2", 4, MC_COUNT) != RET_OK) {
        return ERR_TIMEOUT;
    }
    // osDelay(10);

#if(ADD_INFO_GSM_MOD)
    GSM_INFO data_mc;
    //Получим идентификатор GSM Модема. (GSM900 / GSM1800 / UMTS2100 /LTE1800 / LTE2600)
    mc_get("at+gmm", M_STRING, &data_mc, 3, 3);
    if(data_mc.m_type == M_STRING) {
        char strGsmModemIdentification[SIZE_MODEM_IDENT];
        data_mc.msg[0].str[sizeof(strGsmModemIdentification) - 1] = 0;
        sprintf(strGsmModemIdentification, "%s", data_mc.msg[0].str);
        DS_GSM("D_GMM: ", strGsmModemIdentification);
        SetModemIdentification(strGsmModemIdentification);
    }

    //Получить версию ПО GSM Модема.
    mc_get("at+gmr", M_STRING, &data_mc, 3, 3);
    if(data_mc.m_type == M_STRING) {
        char strGsmModemSoftware[SIZE_NAME_GSM_SOFT];
        char* pFindRev = strstr((char*)data_mc.msg[0].str, "Revision:");
        if(pFindRev) {
            if(strlen(data_mc.msg[0].str) - strlen("Revision:") < sizeof(strGsmModemSoftware)) {
                memcpy(
                    strGsmModemSoftware, pFindRev + strlen("Revision:"), strlen(data_mc.msg[0].str) - strlen("Revision:"));
                DS_GSM("D__GMR: ", strGsmModemSoftware);
                SetGsmModemSoftware(strGsmModemSoftware);
            }
            else {
                DS_GSM("D__GMR: ERROR ", data_mc.msg[0].str);
            }
        }
    }
#endif

    /*char strIMEI[SIZE_IMEI];
   SetIMEI(readGsmIMEI(strIMEI));       // IMEI GSM-модуля
   SetStrIMEI(strIMEI);*/

    return RET_OK;
}

// выполнить команду
// 1 - имя команды
// 2 - таймаут команды
// 3 - количество попыток ввода команды
RET_INFO mc(const char* pCmd, u32 second, int count)
{
    RET_INFO ret;

    if(!(strlen(pCmd))) {
        return RET_OK;
    }

    if(!(count)) {
        count = MC_COUNT;
    }

    for(; count > 0; count--) {
        // выполняем команду
        mc_send(pCmd, NULL, 0);

        // ждём  OK
        ret = isOK(pCmd, second);
        switch(ret) {
            case RET_OK:
                return ret;
            case RET_CLOSE0:
                return ret;
            case RET_CLOSE1:
                return ret;
        }
    }
    return ret;
}

int ActivationConnectToGprs(uint8_t usProf)
{
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(GSM_INFO));
    osDelay(1000);

    if(mc("at+ciicr", GetGsmGprsTimeout(), 1) != RET_OK) {
        return 1;
    }

    memset(&data_mc, 0, sizeof(GSM_INFO));
    // проверяем назначенный IP адрес
    mc_get("at+cifsr", M_STRING, &data_mc, 1, 1);
    if(strlen(data_mc.msg[0].str)) {
        DS_GSM("IP ADDRESS: ", data_mc.msg[0].str);
        return 0;
    }
    return 1;
}

// INPUT PARAM: user, passwd, apn.
void SetupInternetConnectionProfile(const char* pUser, const char* pPasswd, const char* pApn, _Bool bTypeProf)
{
    char strTempBuf[100];

    if(bTypeProf) {
        sprintf(strTempBuf, "at+sapbr=3,1,\"APN\",\"%s\"", pApn);
        mc(strTempBuf, 5, 1);

        sprintf(strTempBuf, "at+sapbr=3,1,\"USER\",\"%s\"", pUser);
        mc(strTempBuf, 5, 1);

        sprintf(strTempBuf, "at+sapbr=3,1,\"PWD\",\"%s\"", pPasswd);
        mc(strTempBuf, 5, 1);
    }
    else {
        strcpy(strTempBuf, "at+cstt=\"");    //Вводим имя точки APN
        strcat(strTempBuf, pApn);
        strcat(strTempBuf, "\",");

        if(strlen(pUser)) {    //Вводим пользователя точки APN
            strcat(strTempBuf, "\"");
            strcat(strTempBuf, pUser);
            strcat(strTempBuf, "\",");
        }

        if(strlen(pPasswd)) {    //Вводим пароль  точки APN
            strcat(strTempBuf, "\"");
            strcat(strTempBuf, pPasswd);
            strcat(strTempBuf, "\"");
        }
        mc(strTempBuf, 5, 1);
    }
}

/*
Получает количество попыток ввода пина по СИМ катры
return: количество попыток ввода pin кода (до 3 попыток)
*/
int8_t attemptsPinSimCard(GSM_INFO* ptr_data_mc)
{
    uint8_t cnt = 0;
    /*  Спросим количество попыток ввода pin */
    while(ptr_data_mc->m_type != M_SPIC) {
        mc_get(AT_SPIC, M_SPIC, ptr_data_mc, 3, 4);
        if(++cnt > MC_COUNT)
            return -1;
    }
    return (int8_t)ptr_data_mc->msg[0].var;
}

/*
Запрашивает статус установленного пин кода по СИМ карте
return:
  M_SIM_READY - снят пин код (СИМ готова к работе);
  M_SIM_PIN -   установлен пин код;
  M_SIM_PUK -   СИМ заблокирована PUK кодом
*/
M_INFO requestPinSimCard(GSM_INFO* ptr_data_mc)
{
    mc_get(AT_CPIN, M_OTHER_PIN, ptr_data_mc, 3, 3);
    return (M_INFO)ptr_data_mc->m_type;
}

M_INFO SimCardPin(void)
{
    char strPin[SIZE_PIN_CODE] = { 0 };
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));

    /* Спросим количество попыток ввода pin */
    int8_t attemptsPinSimCard(GSM_INFO * ptr_data_mc);
    int8_t attempts = attemptsPinSimCard(&data_mc);
    if(attempts < 2) {
        if(attempts < 0) {
            return M_ERROR;
        }
        return M_SIM_PUK;
    }

    SIM_CARD eNumSim = GetNumSim();
    if(eNumSim != FIRST_SIMCARD_OK) {
        return (M_SIM_PIN);
    }

    char strRamFIRST_SCID[SIZE_SCID];
    if(!(GetScidCurentFirstSim(strRamFIRST_SCID))) {
        DP_GSM("D_SCID SIM FAIL\r\n");
        return (M_SIM_PIN);
    }

    /* Запрос PIN кода */
    M_INFO requestPinSimCard(GSM_INFO * ptr_data_mc);
    requestPinSimCard(&data_mc);

    char strFIRST_SCID[SIZE_SCID];    // SCID заводской СИМ карты
    memset(strFIRST_SCID, 0, SIZE_SCID);
    GetScidFirstSim(strFIRST_SCID);

    /* Генерируем пин код, если заводская СИМ карта. Если пользовательская, то далее подставим его затерев
    * сгенерированный strPin*/
    generPinSim(strRamFIRST_SCID, strPin);

    if(strlen(strFIRST_SCID)) {
        if(strcmp(strRamFIRST_SCID, strFIRST_SCID)) {    //СИМ карты не равны(пользовательская СИМ карта)
            GetUserSimPin(strPin);                       //Получим пользовательский ПИН код
            if((GetPinLock() == FALSE) || (strlen(strPin) == 0)) {
                return M_ERROR;
            }
        }
    }

    sprintf(g_asCmdBuf, "at+cpin=%s", strPin);    // подставляем пин код
    if(mc(g_asCmdBuf, 10, 1) != RET_OK) {
        return M_ERROR;
    }

    uint8_t cnt = 0;
    M_INFO request = M_OTHER;    //Статус СИМ карты
    while(request != M_SIM_READY) {
        /* Добавим секундную задержку, на тормозную СИМ карту */
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));

        request = requestPinSimCard(&data_mc);                // Запрос статуса PIN кода
        if(request == M_SIM_PIN || request == M_SIM_PUK) {    //Пин код не подошел, выйдем из запроса
            return request;
        }
        if(++cnt > MC_COUNT)
            return M_ERROR;    //Кол-во попыток истекло
    }

    return M_OK;
}

// IMEI GSM-модуля
uint64_t readGsmIMEI(char* pIMEI)
{
    GSM_INFO data_mc;
    uint8_t timeout = 5;
    while(timeout--) {
        memset(&data_mc, 0, sizeof(data_mc));
        mc_get("at+gsn", M_STRING, &data_mc, 3, 3);
        if((data_mc.m_type == M_STRING) && (isdigit(data_mc.msg[0].str[0]))) {
            DS_GSM("D_IMEI: ", data_mc.msg[0].str);
            break;
        }
        osDelay(1000);
    }
    if(!(timeout))
        return 0;
    strcpy(pIMEI, data_mc.msg[0].str);
    return (uint64_t)atof(data_mc.msg[0].str);
}

int GetSimScidFromGsmModule(char* pSCID)
{
    // SCID SIM-карты
    uint8_t err_scid = 5;
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));
    while((strlen(data_mc.msg[0].str) < SIZE_SCID) && (err_scid)) {
        memset(&data_mc, 0, sizeof(data_mc));
        mc_get(AT_SCID, M_STRING, &data_mc, 1, 3);
        if(data_mc.m_type == M_STRING) {
            for(int i = 0; i < strlen(data_mc.msg[0].str); i++) {
                if(!(isdigit(data_mc.msg[0].str[i]))) {
                    data_mc.msg[0].str[i] = '\0';
                    break;
                }
            }
            if(isdigit(data_mc.msg[0].str[0])) {    //Проверим корректный SCID
                DS_GSM("D_SCID: ", data_mc.msg[0].str);
                strcpy((char*)pSCID, data_mc.msg[0].str);
                return 0;
            }
        }
        osDelay(1000);
        err_scid--;
    }

    DP_GSM("D_ERR SCID: %d,%d\r\n", strlen(data_mc.msg[0].str), err_scid);
    return 1;
}

// выключение модема
void GsmModemCmdOff(void)
{
    GSM_INFO data_mc;
    char* cmd_pwr_off = AT_SMSO;

    mc_send(cmd_pwr_off, NULL, 0);
    int iRxSize = gsm_parser(cmd_pwr_off, &data_mc, g_asRxBuf, RX_BUFFER_SIZE, 5);
    if(iRxSize <= 0) {
        // истекло время ожидания ответа на команду или произошла ошибка
        GSM_DC(cmd_pwr_off, 't');
    }

    if(data_mc.m_type == M_OK) {
        GSM_DC(cmd_pwr_off, '0');
    }
    if(data_mc.m_type == M_SHUTDOWN) {
        DP_GSM("D_^SHUTDOWN\r\n");
    }
    if(data_mc.m_type == M_ERROR || data_mc.m_type == M_ABORTED) {
        GSM_DC(cmd_pwr_off, '4');
    }
}

#pragma optimize = none
// инициализация SIM
M_INFO SimCardInit(void)
{
    GSM_INFO data_mc;
    memset(&data_mc, 0, sizeof(data_mc));
    char strFIRST_SCID[SIZE_SCID];
    char strSECOND_SCID[SIZE_SCID];
    SetScidCurentFirstSim(STR_NULL);

    SIM_CARD eNumSim = GetNumSim();
    if(eNumSim == ERROR_ALL_SIMCARD || eNumSim == FIRST_SIMCARD_ERROR || eNumSim == SECOND_SIMCARD_ERROR) {
        return M_OTHER;
    }
    if(eNumSim == FIRST_SIMCARD_OK) {
        if(GetSimScidFromGsmModule(strFIRST_SCID)) {
            DP_GSM("D_SCID FIRST SIM FAIL\r\n");
            return M_OTHER;
        }
        SetScidCurentFirstSim(strFIRST_SCID);
    }
    if(eNumSim == SECOND_SIMCARD_OK) {
        if(GetSimScidFromGsmModule(strSECOND_SCID)) {
            DP_GSM("D_SCID SECOND SIM FAIL\r\n");
            return M_OTHER;
        }
        SetScidCurentSecondSim(strSECOND_SCID);
    }
    DP_GSM("D_SCID OK\r\n");

    //Запрос PIN кода.
    uint8_t timeout = 5;
    memset(&data_mc, 0, sizeof(data_mc));
    while(timeout) {
        M_INFO res = requestPinSimCard(&data_mc);
        timeout--;
        if(res == M_SIM_PIN || res == M_SIM_PUK || res == M_SIM_READY) {
            return res;
        }
        // SetGsmFunctional(FULL_FUNCTIONALITI);
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
    }

    return M_OTHER;
}

/*
Открытие GPRS сессии.
Input: no
Return: результат открытия GPRS.
  0 - GPRS открыт;
  1 - ошибка GPRS.
*/
_Bool GsmModemConnectGprsService(void)
{
    _Bool ret_gsm = 1;
    portTickType xLastWakeTimerDelay;
    GSM_INFO data_mc;

    uint8_t count = TIMEOUT_OPEN_GPRS / 2;
    while(ret_gsm && count) {
        memset(&data_mc, 0, sizeof(data_mc));
        mc_get(AT_CGATT, M_CGATT, &data_mc, 1, 10);
        /*xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
      char *pRxBufferGSM = (char*)(g_aucRxBufferUSART2);
      DS_GSM("GSM__ ", pRxBufferGSM);*/

        if(data_mc.m_type == M_CGATT) {
            if(data_mc.msg[0].var == 1) {
                ret_gsm = 0;
            }
            else {
                mc("at+cgatt=1", 1, 10);
                vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
            }
        }
        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_2000 / portTICK_RATE_MS));
        count--;
    }
    return ret_gsm;
}