
#include "includes.h"
#include "findme_911.h"

const char strMsgRusSmsRegUser[] = "FindMe включен. Ожидайте регистрации на сервере.";    //Ответка СМС RUS
// const char strMsgEngSmsRegUser[] = "FindMe is switched on. Wait for registration on the server."; //Ответка СМС ENG

static uint8_t RxLevDec2dBm(uint8_t);

/* Данные с GPS координатами */
int FrameBuildGpsDataFm911(char* pOut, int OffsetData)
{
    if(((GetModeDevice() == STANDART) && (GetTypeConnectFm911() != TYPE_REG_TO_BASE)) &&
            (GetTypeConnectFm911() != TYPE_GOODBYE) ||
        (GetTypeConnectFm911() == TYPE_REG_USER)) {
        return 0;
    }

    GPS_INFO stGpsDataTemp;
    _Bool gps_valid = FALSE;

#if(_OLD_POSITION_GPS_)    // отправка прошлых координат при неудачном выходе
    /* Проверим тип выхода и если он не резервный, то возьмем реальные координаты*/
    if(GetTypeConnectFm911() != TYPE_ERR_CONSRV) {
        /* Выход резервный, подставим реальные координаты */
        if(GetPositionGps(&stGpsDataTemp)) {
            gps_valid = TRUE;
        }
    }
    else {
        /* Резервный выход, подставим предыдущие координаты */
        if(getOldPositionGps(&stGpsDataTemp)) {
            gps_valid = TRUE;
        }
    }
#else
    if(GetPositionGps(&stGpsDataTemp)) {
        gps_valid = TRUE;
    }
#endif

    if(gps_valid) {
        RTC_t DateRTC;
        Sec2Date(&DateRTC, stGpsDataTemp.time);

        float latitude = ConvertLatitudeGpsFindMe(stGpsDataTemp.latitude);
        float longitude = ConvertLongitudeGpsFindMe(stGpsDataTemp.longitude);

        sprintf(&pOut[OffsetData],
            "P,\"%.2f,%d.000,A,%f,N,%f,E,%.2f,%.2f,%i%i%i ,,,A\";",
            stGpsDataTemp.hdop,
            stGpsDataTemp.time,
            latitude,
            longitude,
            stGpsDataTemp.speed,
            stGpsDataTemp.course,
            DateRTC.mday,
            DateRTC.month,
            DateRTC.year);
        DP_GSM("\r\nD_GPS DATA\r\n");
        DP_GSM(" Date: %d\r\n", stGpsDataTemp.time);
        DP_GSM(" Latitude: %f\r\n", latitude);
        DP_GSM(" Longitude: %f\r\n", longitude);
        DP_GSM(" Course: %.1f\r\n", stGpsDataTemp.course);
        DP_GSM(" Speed: %.2f\r\n", stGpsDataTemp.speed);
        DP_GSM(" Satellites: %d\r\n", stGpsDataTemp.sat);
        DP_GSM(" Hdop: %.2f\r\n", stGpsDataTemp.hdop);
        DP_GSM("\r\n");
    }

#if(_FAST_REGISTRATION_TO_BASE_)
    if(GetTypeConnectFm911() == TYPE_REG_TO_BASE) {
        strcat(&pOut[OffsetData], "P,\"8.1,115101.394,A,6000.3758,N,03021.6927,E,0.28,241.91,020710,,,A\";");
    }
#endif
    return strlen(&pOut[OffsetData]);
}

/* Данные с системной информацией по устройству */
int FrameBuildSystemFm911(char* pOut, int OffsetData)
{
    GSM_INFO out_check;
    check_csq(&out_check);
    float fVbat = (float)GetMeasVin();
#define K_UB 200    // 512.0/2.56
    fVbat = (fVbat / 1000) * K_UB;
    uint16_t isVbat = (uint16_t)fVbat;
    uint8_t num_sim = 1;
    if(SelectNumSim(GET_NUM_SIM) == CURRENT_SECOND_SIMCARD) {
        num_sim = 2;
    }

    sprintf(&pOut[OffsetData],
        "S,%04X,%02d00,%+d,%d;",
        isVbat,             // уровень напряжения батареи
        GetQualityGsm(),    // уровень сигнала GSM
        GetTemperatur(),    // температура
        num_sim             // номер симкарты, с которой осуществляется текущий сеанс. S1 – SIM1, S2 – SIM2
    );

    return strlen(&pOut[OffsetData]);
}

/* Данные с GSM координатами */
int FrameBuildGsmDataFm911(char* pOut, int OffsetData)
{
    GPS_INFO stGpsDataTemp;
    /* Если есть GPS координаты и не нажата кнопка, то не формируем пакет */
    /* В режимах рег. пользователя, рег. в БД и в окончании работы с сервером 911 шлем пакет */
    if(GetPositionGps(&stGpsDataTemp) && GetTypeConnectFm911() != TYPE_REG_TO_BASE &&
        GetTypeConnectFm911() != TYPE_REG_USER && GetTypeConnectFm911() != TYPE_GOODBYE) {
        return 0;
    }

    GSM_INFO stGsm;
    INIT_GSM_INFO(stGsm);
    int offset = OffsetData;
    char* ptr = pOut + 3;
    if(getInfoLbsData(&stGsm)) {
        //Проверим нужно, ли искать LBS
    }
    else {
        //поиск по домашнему оператору
        stGsm.count = 0;
        memset(&stGsm, 0, sizeof(stGsm));
        uint8_t sta_find_timeout = MC_COUNT;
        while(!(stGsm.count)) {
            if(!(sta_find_timeout--)) {
                return -1;    // Не нашли ни одной GSM станций ;(
            }
            if(!(getInfoLbsData(&stGsm))) {
                DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");        // Ищем LBS
                getActiveLbsInfo(&stGsm, GetLbsFindTimeout());    //"at^smond"
                if(stGsm.count) {
                    setInfoLbsData(&stGsm);    //Сохраним структуру данных LBS
                }
            }
            osDelay(1000);
        }
    }

    /* добовляем в структуру БС cenga еще структуру из netscana */
    additionOtherLbsForFm911(&stGsm);

    DP_GSM("D_LBS DATA\r\n");
    DP_GSM("COUNT: %d\r\n", stGsm.count);

    loop(stGsm.count)
    {
        DP_GSM("N%d) ", i);
        DP_GSM("MCC:%d ", stGsm.inf->station[i].mcc);
        DP_GSM("MNC:%d ", stGsm.inf->station[i].mnc);
        DP_GSM("LAC:%04X ", stGsm.inf->station[i].lac);
        DP_GSM("CELL:%04X ", stGsm.inf->station[i].cell);
        DP_GSM("RXLEV:%d\r\n", stGsm.inf->station[i].rxlev);
    }

    if(!(GetPositionGps(&stGpsDataTemp))) {
        uint16_t ausGsmLog[2];
        GetGsmLog(&ausGsmLog[0], &ausGsmLog[1]);
        ausGsmLog[0]++;
        if(!(stGsm.count)) {
            ausGsmLog[1]++;
        }
        SetGsmLog(ausGsmLog[0], ausGsmLog[1]);
    }

    if(!(stGsm.count)) {
        return 0;
    }

    /* Проверка на одинаковые GSM станций */
    /*_Bool lbs_data_true [SIZE_LBS] = { FALSE };
   loop(stGsm.count) {
     lbs_data_true[i] = TRUE;
   }
   loop(stGsm.count) {
     for(size_t n = i+1; n < stGsm.count; n++) {
       if(stGsm.inf->station[i].cell == stGsm.inf->station[n].cell &&
          stGsm.inf->station[i].lac == stGsm.inf->station[n].lac &&
          stGsm.inf->station[i].mcc == stGsm.inf->station[n].mcc &&
          stGsm.inf->station[i].mnc == stGsm.inf->station[n].mnc) {
            DPS("The same lbs!\r\n");
            lbs_data_true[i] = FALSE;
            break;
          }
     }
   }*/

    /* Заполняем буфер данными GSM станций */
    loop(stGsm.count)
    {
        // if(lbs_data_true[i]) {
        static uint8_t RxLevDec2dBm(uint8_t);
        sprintf(ptr + offset,
            "%03d,%02d,%04X,%04X,%d,",
            stGsm.inf->station[i].mcc,
            stGsm.inf->station[i].mnc,
            stGsm.inf->station[i].lac,
            stGsm.inf->station[i].cell,
            RxLevDec2dBm(stGsm.inf->station[i].rxlev));
        offset += strlen(ptr + offset);
        //}
    }

    if(offset > 0) {
        memcpy(&pOut[OffsetData], "C,\"", 3);
        offset += 3;

        pOut[offset - 1] = '"';
        pOut[offset++] = ';';
        pOut[offset] = 0;
    }
    return strlen(&pOut[OffsetData]);
}

static uint8_t RxLevDec2dBm(uint8_t rxlev)
{
    int16_t rxlev_mod_fm = (int16_t)rxlev;
    rxlev_mod_fm -= 113;
    uint8_t rxlev_ret = (uint8_t)rxlev_mod_fm;
    if(rxlev_mod_fm < 0) {
        rxlev_ret = (uint8_t)(rxlev_mod_fm * -1);
    }
    return rxlev_ret;
}
/*----------------------------------------------------------------------------*/

/* Данные о движении */
int FrameBuildAccelDataFm911(char* pOut, int OffsetData)
{
    int iFlagMove = 0;
    uint32_t sec_state_stop = SecStateStop();
    uint16_t sec_state_move = SecStateMove();

    //Ставим флаг движения Акселерометра(только акселерометр, без анализа скорости по GPS).
    if((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE)) {
        if(g_stRam.stAccel.eAccelState)
            g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
        iFlagMove = 1;
    }

    sprintf(&pOut[OffsetData],
        "Q,%d,%d,%d;",
        iFlagMove,         // наличие движения
        sec_state_move,    // число секунд после последнего прерывания о движении
        sec_state_stop     // время последнего события остановки
    );

    return strlen(&pOut[OffsetData]);
}
/*----------------------------------------------------------------------------*/

/* Параметры устройства */
int FrameBuildDeviceDataFm911(char* pOut, int OffsetData)
{
    char Temp[8];
    sprintf(Temp, "N,[");
    sprintf(&pOut[OffsetData], Temp);

    /* Режим работы устройства */
    sprintf(Temp, "M");
    switch(GetTypeRegBase911()) {
        case TYPE_REG_TO_BASE:    //режим регистрация в базе.
            strcat(Temp, "3");
            break;
        case TYPE_REG_USER:    //режим активации устройства
            strcat(Temp, "1");
            break;

        /* основной режим работы на отличные от регистраций выходы */
        default:
            strcat(Temp, "2");
            break;
    }
    strcat(&pOut[OffsetData], Temp);
    /*********************************/

    /* Время ожидания ответа от сервера (сек) */
    sprintf(Temp, ",S%d", GetAnswerTimeout());
    strcat(&pOut[OffsetData], Temp);

    /* Время ожидания координат от GPS (мин) */
    sprintf(Temp, ",P%d", (GetGpsWait() / 60));
    strcat(&pOut[OffsetData], Temp);

    /* Время регистрации в сети (сек) */
    sprintf(Temp, ",N%d", GetGsmFindTimeout());
    strcat(&pOut[OffsetData], Temp);

    /* Минимальная рабочая температура  */
    sprintf(Temp, ",t%+i", GetMinTemperaturWorkDevice());
    strcat(&pOut[OffsetData], Temp);

    /* Время на установление GPRS соединения (сек) */
    sprintf(Temp, ",G%d", GetGsmGprsTimeout());
    strcat(&pOut[OffsetData], Temp);

    sprintf(Temp, ",g%d", GetGprsOpenCount());
    strcat(&pOut[OffsetData], Temp);

    /* Время ожидания информации о GSM станциях (по 5 сек) */
    sprintf(Temp, ",F%d", (GetLbsFindTimeout() / 5));
    strcat(&pOut[OffsetData], Temp);

    strcat(&pOut[OffsetData], "];");
    return strlen(&pOut[OffsetData]);
}
/*----------------------------------------------------------------------------*/

/*- uint16_t, uint16_t -------------------------------------------------------*/
/* поиск GPS координат всего запросов, из них запрос не удачные */
static void LogGetGpsFind(char* pOut)
{
    uint16_t ucTemp1 = 0, ucTemp2 = 0;
    GetGpsFind(&ucTemp1, &ucTemp2);
    sprintf(pOut + strlen(pOut), ",p%d,%d", ucTemp1, ucTemp2);
}

/* мониторинг сети GSM всего запросов, из них запрос не удачный */
static void LogGetGsmLog(char* pOut)
{
    uint16_t ucTemp1 = 0, ucTemp2 = 0;
    GetGsmLog(&ucTemp1, &ucTemp2);
    sprintf(pOut + strlen(pOut), ",s%d,%d", ucTemp1, ucTemp2);
}

/* подача питания на GSM модуль всего попыток, из них не удачных */
static void LogGetGsmPwr(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0, usTemp3 = 0, usTemp4 = 0;
    GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
    sprintf(pOut + strlen(pOut), ",m%d,%d,%d,%d", usTemp1, usTemp2, usTemp3, usTemp4);
}

/* установлено соединений с сервером, из них не удачных */
static void LogGetServerConnect(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetServerConnect(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",c%d,%d", usTemp1, usTemp2);
}

/* количество отключений GSM модуля во время работы из-за просадки по питанию, из-за низкой температуры  */
static void LogGetGsmWork(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetGsmWorkErr(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",d%d,%d", usTemp1, usTemp2);
}

/* общее количество включений устройства за время работы
сюда включается перезагрузка устройства и количество по выходу из LowPower */
static void LogGetCountRebootDevice(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetCountRebootDevice(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",r%d,%d", usTemp1, usTemp2);
}

/* общее количество циклов (пробуждений) устройства */
static void LogGetDeviceWakeup(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",n%d,0", GetDeviceWakeup());
}

/*- uint16_t -----------------------------------------------------------------*/
/* количество не удачных попыток зарегистрироваться в сети оператора  */
static void LogGetGsmFind(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",o%d", GetGsmFind());
}

/* количество не открытых GPRS сеансов */
static void LogGetGsmGprs(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",g%d", GetGsmGprsErr());
}

/* количество не полученных ответов от сервера за заданное время ожидания */
static void LogGetServerErr(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",a%d", GetServerErr());
}

/* количество обнаружения состояния останов таймера */
static void LogGetCountTimerStop(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",t0");
}

/* затраченное количество минут на поиск GPS координаты */
static void LogGetGpsWait(char* pOut)
{
    if(g_stRam.stDevDiag.stHard.uiTimePwrGps >= GetGpsWait()) {
        sprintf(pOut + strlen(pOut), ",m0");
    }
    else {
        sprintf(pOut + strlen(pOut), ",m%d", g_stRam.stDevDiag.stHard.uiTimePwrGps / 60);
    }
}
/*----------------------------------*/

/* суммарное время работы модуля  GPS */
static void LogGetTimePwrGpsAndGsm(char* pOut)
{
    uint32_t AddSec = 1480550400 + GetTimeAllPwrGps();
    RTC_t stDate;
    Sec2Date(&stDate, AddSec);
    stDate.mday--;
    sprintf(pOut + strlen(pOut), ":%d,%d,%d,%d:", stDate.mday, stDate.hour, stDate.min, stDate.sec);

    /* суммарное время работы модуля  GSM */
    uint32_t Sec = GetTimePwrGsm();
    AddSec = 1480550400 + Sec;
    Sec2Date(&stDate, AddSec);
    stDate.mday--;
    sprintf(pOut + strlen(pOut), "%d,%d,%d,%d", stDate.mday, stDate.hour, stDate.min, stDate.sec);
}

/* возвращает имя нового сервера куда уходит девайс после смены протокола и сервера.
   активно только в состоянии типа конекта TYPE_GOODBYE */
static void LogGetNewServer(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",F0");
    if(GetTypeRegBase911() != TYPE_GOODBYE) {    //не отправляем этот лог если тип конекта не "прощальный"
        return;
    }

    char str_name_first_server[SIZE_SERV] = { 0 };
    GetAddrFirstServ(str_name_first_server);
    char* ptr = strchr(str_name_first_server, ':');
    if(ptr) {
        *ptr = NULL;
    }
    sprintf(pOut + strlen(pOut), ",%s", str_name_first_server);
}

/* состояние кнопки */
static void LogGetStatusButton(char* pOut)
{
    strcat(pOut, ":rn");    //Всегда rn
}
/*----------------------------------------------------------------------------*/

void (*ptrCallbackLogFindme911Scheduler[])(char* pOut) = {
    LogGetCountRebootDevice,    // r (сумма всех нештатных перезагрузок: кнопка + питание)
    LogGetCountTimerStop,       // t (количество обнаруженных состояний остановки таймера (будильника))
    LogGetGpsFind,              // p (поиск GPS координат)
    LogGetGsmLog,               // s (мониторинг сети GSM)
    LogGetGsmPwr,               // m (подача питания на GSM модуль)
    LogGetGsmFind,              // o (количество не удачных попыток зарегистрироваться в сети оператора)
    LogGetServerConnect,        // c (общее количество попыток установления соединений с сервером)
    LogGetGsmGprs,              // g (количество не открытых GPRS сеансов)
    LogGetGpsWait,              // m (затраченное количество минут на поиск GPS коорд)
    LogGetServerErr,            // a (количество не полученных ответов от сервера за заданное время ожидания)
    LogGetGsmWork,              // d (выключение gsm из-за просадки по питанию)
    LogGetDeviceWakeup,         // n (число заданий по расписанию на подключение к серверу)
    LogGetNewServer,            // f (возвращает имя нового сервера куда уходит девайс после смены протокола и сервера. активно
                                // только в состоянии типа конекта TYPE_GOODBYE)
    LogGetTimePwrGpsAndGsm,     //: (суммарное время работы модуля GPS и GSM)
    LogGetStatusButton,         //:rn (состояние кнопки)
};

/* Лог работы устройства */
int FrameBuildLogDataFm911(char* pOut, int OffsetData)
{
    char* pStartMsg = &pOut[OffsetData];
    sprintf(&pOut[OffsetData], "L");
    loop(sizeof(ptrCallbackLogFindme911Scheduler) / sizeof(void*))
    {
        (*ptrCallbackLogFindme911Scheduler[i])(&pOut[OffsetData]);
    }

    strcat(&pOut[OffsetData], ";");
    DPS("D_LOG: ");
    DPS(pStartMsg);
    DPS("\r\n");
    return strlen(&pOut[OffsetData]);
}

/* Парсер ответа от сервера 911.
Устанавливает конфигурацию заданную сервером 911
return:
-3 - send server ERROR
-2 - Error CRC
-1 - Error Timeuot Resp
0 - Ok
*/
T_ANS_SRV_FM911 GprsAckDataFm911(void)
{
    int tx_size_ack_data = 0;
    uint8_t count = GetAnswerTimeout() / 10;

    //Данные пришли, прочитаем их.
    while(!(tx_size_ack_data > 0)) {
        tx_size_ack_data = socket_read(PROF_FIRST_SERVER, g_aucOutDataFrameBuffer, SIZE_OUT_DATA_BUF);
        osDelay(1);
        count--;
        if(!(count)) {
            return SRV_TIMEOUT;    // Error Timeout
        }
    }
    // strcpy(g_aucOutDataFrameBuffer,
    // "$16112911295,16112911305,M2,Rm10h01h03h05h25,f05,A0,Y\"+79216465013\",Z010*86\r\n");

    DP_GSM("D_GPRS FM911 RX<-:\r\n");
    DP_GSM((char*)g_aucOutDataFrameBuffer);
    DP_GSM("\r\n");

    char* pBuf = (char*)g_aucOutDataFrameBuffer;

    /* Проверка на $ERROR */
    if(strstr(pBuf, "$ERROR")) {
        return ANS_ERROR;    // Error CRC
    }

    /* Parser Frame */
    /* подсчет контрольной суммы */
    int fm911_check_crc(uint8_t*);
    if(fm911_check_crc((uint8_t*)pBuf)) {
        return CRC_ERROR;    // Error CRC
    }

    RTC_t DateRTC;
    int year = 0, month = 0, mday = 0, hour = 0, min = 0;

    /* текущее время сервера  */
    sscanf(pBuf, "$%2d%2d%2d%2d%2d", &year, &month, &mday, &hour, &min);
    DateRTC.sec = 0;
    DateRTC.wday = 0;
    DateRTC.year = year;
    DateRTC.month = month;
    DateRTC.mday = mday;
    DateRTC.hour = hour;
    DateRTC.min = min;
    setSystemDate(&DateRTC);    //Устанавливаем системное время

    /* время следующего сеанса связи */
    while(*pBuf++ != ',')
        ;
    sscanf(pBuf, "%2d%2d%2d%2d%2d", &year, &month, &mday, &hour, &min);
    DateRTC.sec = 0;
    DateRTC.wday = 0;
    DateRTC.year = year;
    DateRTC.month = month;
    DateRTC.mday = mday;
    DateRTC.hour = hour;
    DateRTC.min = min;

    /* не имеет значение какой режим работы девайса, время пробуждения*/
    SetSleepTimeStandart(Date2Sec(&DateRTC));
    SetSleepTimeFind(Date2Sec(&DateRTC));

    /* команда режима работы девайса */
    while(*pBuf++ != ',')
        ;
    char* ptr = strchr(pBuf, 'Z');
    if(ptr) {    //получили режим работы
        ptr++;
        TYPE_MODE_DEV eModeDevice = STANDART;    //Стандарный режим по умолчанию
        if(*ptr == '1') {
            // Режим поиска
            eModeDevice = TIMER_FIND;
        }
        /* Использовать акселерометр */
        char* ptr = strchr(pBuf, 'A');
        ptr++;
        if(*ptr == '1') {
            // Режим поиска по акселерометру
            SetEnableAccelToFind(TRUE);
        }
        else {
            SetEnableAccelToFind(FALSE);
        }
        /* Если стоял режим использовать акселерометр и было пробуждение по движению, то оставим этот режим */
        if(GetEnableAccelToFind() && GetTypeConnectFm911() == TYPE_MOVE_START) {
            eModeDevice = TIMER_FIND;
            SetEnableAccelToFind(TRUE);
        }
        SetModeDevice(eModeDevice);
    }

    /* Cмена режима (режим активации устройства, основной режим работы, режим регистрация в базе) */
    ptr = strchr(pBuf, 'M');
    if(ptr) {
        ptr++;
        switch(*ptr) {
            case '1': /* режим активации устройства */
                SetTypeRegBase911(TYPE_REG_USER);
                SetUserTel(STR_NULL);    //Сброс номера пользователя при режиме активации устройства для нового пользователя.
                break;
            case '2': /* основной режим работы */
                /* если девайс был в режиме регистрации Юзера, то переведем режим на стандартный и удалим все СМС */
                if(GetTypeConnectFm911() == TYPE_REG_USER) {
                    SetTypeRegBase911(TYPE_WU_START);
                    PDU_SMGD(6);    // удаление всех СМС
                }
                break;
            case '3': /* режим регистрация в БД */
                SetTypeRegBase911(TYPE_REG_TO_BASE);
                SetUserTel(STR_NULL);    //Сброс номера пользователя при режиме регистрации в БД.
                break;
        }
    }

    /* Время ожидания координат от GPS (мин) */
    ptr = strchr(pBuf, 'P');
    if(ptr) {
        int value = 0;
        sscanf(ptr, "P%2d,", &value);
        value *= 60;    //переводим в секунды.
        SetGpsWait(value);
    }
    /*******************************************/

    /*Последовательный ряд времен до пяти значений, откладывающих время соединения с сервером
   в случае отсутствия связи (префикс перед числовым значением означает единицы измерения: m-минуты, h-часы, d-дни) */
    ptr = strstr(pBuf, "R");
    if(ptr) {
        ptr++;
        char Temp[5];
        uint32_t TimeReconnect[5];
        uint32_t K;
        memset(TimeReconnect, 0, sizeof(TimeReconnect));
        for(uint8_t i = 0; i < (sizeof(TimeReconnect) / sizeof(uint32_t)); i++) {
            memset(Temp, 0, sizeof(Temp));
            _Bool NextValue = 0;
            for(uint8_t j = 0, n = 0; n < (sizeof(Temp) - 1); j++) {
                if((*ptr == 'm') || (*ptr == 'h') || (*ptr == 'd')) {
                    if((NextValue) || (*ptr == ',')) {
                        break;
                    }
                    NextValue = 1;
                    switch(*ptr) {
                        case 'm':
                            K = 60;
                            break;
                        case 'h':
                            K = 60 * 60;
                            break;
                        case 'd':
                            K = 60 * 60 * 24;
                            break;
                    }
                    ptr++;
                }
                else {
                    Temp[n++] = *ptr++;
                }
            }

            TimeReconnect[i] = atoi(Temp) * K;
            if(*ptr == ',') {
                break;
            }
        }

        for(uint8_t i = 0; i < (sizeof(TimeReconnect) / sizeof(uint32_t)); i++) {
            if(!(TimeReconnect[i]))
                break;
            SetTimeReconnect(i, TimeReconnect[i]);
        }
    }

    /* Время поиска GSM сети (регистрации в сети ) в сек. */
    ptr = strchr(pBuf, 'N');
    if(ptr) {
        int value;
        sscanf(ptr, "N%2d,", &value);
        SetGsmFindTimeout(value);
    }

    /* Минимальная рабочая температура */
    ptr = strchr(pBuf, 't');
    if(ptr) {
        int value;
        sscanf(ptr, "t%3d,", &value);
        SetMinTemperaturWorkDevice(value);
    }

    /* Время ожидания ответа от сервера */
    ptr = strchr(pBuf, 'S');
    if(ptr) {
        int value;
        sscanf(ptr, "S%2d,", &value);
        SetAnswerTimeout(value);
    }

    /* Время на установление GPRS соединения (сек) */
    ptr = strchr(pBuf, 'G');
    if(ptr) {
        int value;
        sscanf(ptr, "G%2d,", &value);
        SetGsmGprsTimeout(value);
    }

    /* Количество попыток установления GPRS соединения */
    ptr = strchr(pBuf, 'g');
    if(ptr) {
        int value;
        sscanf(ptr, "g%2d,", &value);
        SetGprsOpenCount(value);
    }

    /* Время ожидания информации о GSM станциях (по 5 сек) */
    ptr = strchr(pBuf, 'F');
    if(ptr) {
        int value;
        sscanf(ptr, "F%2d,", &value);
        value *= 5;
        SetLbsFindTimeout(value);
    }
    /*******************************************/

    /* Адрес первого сервера (будет поддерживаться только один сервер протокола FM911) */
    char strAddrServer[SIZE_SERV] = { 0 };
    ptr = strchr(pBuf, 'U');
    if(ptr) {
        ptr++;
        loop(SIZE_SERV)
        {
            if(*ptr == ',')
                break;
            strAddrServer[i] = *ptr++;
        }
        if(strlen(strAddrServer)) {
            SetAddrSecondServ(strAddrServer);
        }
    }

    /* Адрес сервера куда будет ходить девайс по протоколу iON */
    memset(strAddrServer, 0, sizeof(strAddrServer));
    ptr = strchr(pBuf, 'V');
    if(ptr) {
        ptr++;
        loop(SIZE_SERV)
        {
            if(*ptr == ',')
                break;
            strAddrServer[i] = *ptr++;
        }
        if(strlen(strAddrServer)) {
            // SetAddrFirstServ(strAddrServer);
            // changeServerFm2iON();
            SetTypeRegBase911(TYPE_GOODBYE);    //ставим тип соединения с 911 для отправки последних данных
            setGsmStep(GSM_PROFILE_GPRS_CONNECT);
            SaveConfigCMD();
        }
    }
    /*******************************************/

    /* Смена телефонного номера пользователя */
    char strNewUserTel[SIZE_TEL] = { 0 };
    ptr = strchr(pBuf, 'Y');
    if(ptr && ptr[1] != ',') {    // проверка указателя и проверка что в ответе задан этот параметр.
        ptr += 3;
        loop(SIZE_SERV)
        {
            if(*ptr == '"')
                break;
            strNewUserTel[i] = *ptr++;
        }
        if(strlen(strNewUserTel)) {
            SetUserTel(strNewUserTel);    // сохраним новый номер пеользователя.
        }
    }

    SaveConfigCMD();
    return ANS_OK;    // Ok
}

/*
Проверка СМС от пользователя на его регистрацию.
Input:
1 - указатель на заполняемое Имя девайса;
2 - указатель на заполняемый Номер тел. от куда прислали СМС(пользовательский номер).
3 - тип кодировки СМС
Return:
0 - нет СМС;
1 - пришла СМС.
*/
int CheckSmsRegUserFm911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode)
{
    SMS_INFO sms_cmd;
    memset(&sms_cmd, 0, sizeof(SMS_INFO));

#if(_FAST_REGISTRATION_USER_)
    //Копируем название девайса + IMEI.
    char strIMEI[SIZE_IMEI];
    GetStrIMEI(strIMEI);
    strcpy(ptrNameDevice, NAME_DEVICE);
    int n = strlen(strIMEI) - (MAX_SIZE_NAME_DEVICE - strlen(ptrNameDevice) - 1);
    for(int i = strlen(ptrNameDevice); i < MAX_SIZE_NAME_DEVICE - 1; i++) {
        ptrNameDevice[i] = strIMEI[n++];
    }
    ptrNameDevice[MAX_SIZE_NAME_DEVICE - 1] = NULL;

    strcpy(ptrNumUserTel, TEL_REG_USER);    //Копируем номер пользователя.
    *pCode = CODE_LAT;
    return 1;
#endif

    sms_cmd.txt.buf = (uint8_t*)g_aucOutDataFrameBuffer;
    SMS_RESPONSE ret = PDU_SMGL_FM911(ptrNumUserTel, ptrNameDevice, pCode);
    if(ret == SMS_TRUE) {
        return 1;
    }

    /* проверка на ошибки приема СМС */
    if(ret < 0) {
        switch(ret) {
            case OPERATION_NOT_ALLOWED:
                DP_GSM("D_SMS ERR: OPERATION NOT ALLOWED\r\n");
                break;

            case PARESER_FREEZES:
                DP_GSM("D_SMS ERR: PARESER FREEZES\r\n");
                break;

            case DMA_OVERFLOW:
                DP_GSM("D_SMS ERR: DMA OVERFLOW\r\n");
                ReStartDmaGsmUsart();     //Перезапускаем DMA.
                g_bDmaGsmFail = FALSE;    //решили проблему с DMA - сбрасывем флаг
                PDU_SMGD(6);              // Delete all SMS
                break;
        }
    }
    return 0;
}

/*
Получает причину выхода на связь в форме сервера FM911.
Input: no.
Return:
  причины выхода на связь:
    TYPE_REG_USER =              'R',     // регистрация пользователя на сервере
    TYPE_REG_TO_BASE =           'B',     // регистрация устройства в базе данных сервера и проверка температуры
    TYPE_ERR_POWER =             'D',     // GSM модуль отключился из-за пониженного напряжения питания
    TYPE_WU_START =              'A',     // включение прибора
    TYPE_WU_BUTTON =             'E',     // перезагрузка по кнопке
    TYPE_WU_TIMER =              'T',     // пробуждение от таймера
    TYPE_ERR_COLD =              'H',     // низкая температура
    //TYPE_ERR_NET =               'O',     // сеть оператора не найдена, аварийный режим работы
    //TYPE_ERR_CMDGSM =            'K',     // не получен ответ на команду, аварийный режим работы
    //TYPE_ERR_CONGPRS =           'G',     // отсутствует GPRS соединение, аварийный режим работы
    TYPE_ERR_CONSRV =            'X',     // не получен ответ от сервера, аварийный режим работы
    TYPE_ERR_TIMER =             'F',     // не работает таймер
    TYPE_MOVE_START =            'M',     // начало движения
    TYPE_MOVE_STOP =             'N',     // оставновка
    TYPE_GOODBYE =               'J'      // уход с сервера 911 на новый с сменой протокола
*/
T_TYPE_CONNECT GetTypeConnectFm911(void)
{
    T_TYPE_CONNECT eTypeCon = GetTypeRegBase911();
    if(eTypeCon != TYPE_WU_START) {
        return eTypeCon;
    }

    RESET_STATUS_DEVICE eStatReset = GetStatusReset();    // Флаг причины перезагрузки.

    switch(eStatReset) {
        case BUTTON_RESET:
            return TYPE_WU_BUTTON;    // перезагрузка по кнопке

        case LOW_POWER:
            return TYPE_WU_START;    // перезагрузка по сбою питания

        case WAKE_UP_ACCEL:
            return TYPE_MOVE_START;    // перезагрузка по акселерометру(начало движения)

        case WAKE_UP_STOP:
            return TYPE_MOVE_STOP;    // оставновка

        case WAKE_UP_ALARM:
        case WAKE_UP_LOW_PWR1:
        case WAKE_UP_LOW_PWR2:
            return TYPE_WU_TIMER;    // пробуждение от таймера
    }

    return eTypeCon;
}

uint8_t gsm_cs(char* buf, int size)
{
    uint8_t sum = 0;
    for(int n = 0; n < size; n++) {
        sum += buf[n];
    }
    return sum;
}

int fm911_check_crc(uint8_t* pBuf)
{
    uint8_t calc_crc = 0;
    int crc = 0;
    while(*pBuf != '*') {
        if(*pBuf == NULL) {
            return -1;
        }
        calc_crc += *pBuf++;
    }
    crc = hex2bin(*(pBuf + 2)) | hex2bin(*(pBuf + 1)) << 4;

    return (crc != calc_crc);
}

int getNameFmVersia(char* ptr)
{
    RTC_t DateCurrentFirmware;
    Sec2Date(&DateCurrentFirmware, flash_read_word(__CONST_FIRM_VER));

    sprintf((char*)(ptr), "%s%s.", DEV_VER, HW_VER);
    sprintf((char*)(ptr + strlen((char*)ptr)), "20%.2d", DateCurrentFirmware.year);
    sprintf((char*)(ptr + strlen((char*)ptr)), "%.2d", DateCurrentFirmware.month);
    sprintf((char*)(ptr + strlen((char*)ptr)), "%.2d", DateCurrentFirmware.mday);
    sprintf((char*)(ptr + strlen((char*)ptr)), "%.2d", DateCurrentFirmware.hour);
    sprintf((char*)(ptr + strlen((char*)ptr)), "%.2d", DateCurrentFirmware.min);
    sprintf((char*)(ptr + strlen((char*)ptr)), "%.2d", DateCurrentFirmware.sec);

    return strlen(ptr);
}