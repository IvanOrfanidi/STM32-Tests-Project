
#include "includes.h"
#include "findme_911.h"

const char strMsgRusSmsRegUser[] = "FindMe �������. �������� ����������� �� �������.";    //������� ��� RUS
// const char strMsgEngSmsRegUser[] = "FindMe is switched on. Wait for registration on the server."; //������� ��� ENG

static uint8_t RxLevDec2dBm(uint8_t);

/* ������ � GPS ������������ */
int FrameBuildGpsDataFm911(char* pOut, int OffsetData)
{
    if(((GetModeDevice() == STANDART) && (GetTypeConnectFm911() != TYPE_REG_TO_BASE)) &&
            (GetTypeConnectFm911() != TYPE_GOODBYE) ||
        (GetTypeConnectFm911() == TYPE_REG_USER)) {
        return 0;
    }

    GPS_INFO stGpsDataTemp;
    _Bool gps_valid = FALSE;

#if(_OLD_POSITION_GPS_)    // �������� ������� ��������� ��� ��������� ������
    /* �������� ��� ������ � ���� �� �� ���������, �� ������� �������� ����������*/
    if(GetTypeConnectFm911() != TYPE_ERR_CONSRV) {
        /* ����� ���������, ��������� �������� ���������� */
        if(GetPositionGps(&stGpsDataTemp)) {
            gps_valid = TRUE;
        }
    }
    else {
        /* ��������� �����, ��������� ���������� ���������� */
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

/* ������ � ��������� ����������� �� ���������� */
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
        isVbat,             // ������� ���������� �������
        GetQualityGsm(),    // ������� ������� GSM
        GetTemperatur(),    // �����������
        num_sim             // ����� ��������, � ������� �������������� ������� �����. S1 � SIM1, S2 � SIM2
    );

    return strlen(&pOut[OffsetData]);
}

/* ������ � GSM ������������ */
int FrameBuildGsmDataFm911(char* pOut, int OffsetData)
{
    GPS_INFO stGpsDataTemp;
    /* ���� ���� GPS ���������� � �� ������ ������, �� �� ��������� ����� */
    /* � ������� ���. ������������, ���. � �� � � ��������� ������ � �������� 911 ���� ����� */
    if(GetPositionGps(&stGpsDataTemp) && GetTypeConnectFm911() != TYPE_REG_TO_BASE &&
        GetTypeConnectFm911() != TYPE_REG_USER && GetTypeConnectFm911() != TYPE_GOODBYE) {
        return 0;
    }

    GSM_INFO stGsm;
    INIT_GSM_INFO(stGsm);
    int offset = OffsetData;
    char* ptr = pOut + 3;
    if(getInfoLbsData(&stGsm)) {
        //�������� �����, �� ������ LBS
    }
    else {
        //����� �� ��������� ���������
        stGsm.count = 0;
        memset(&stGsm, 0, sizeof(stGsm));
        uint8_t sta_find_timeout = MC_COUNT;
        while(!(stGsm.count)) {
            if(!(sta_find_timeout--)) {
                return -1;    // �� ����� �� ����� GSM ������� ;(
            }
            if(!(getInfoLbsData(&stGsm))) {
                DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");        // ���� LBS
                getActiveLbsInfo(&stGsm, GetLbsFindTimeout());    //"at^smond"
                if(stGsm.count) {
                    setInfoLbsData(&stGsm);    //�������� ��������� ������ LBS
                }
            }
            osDelay(1000);
        }
    }

    /* ��������� � ��������� �� cenga ��� ��������� �� netscana */
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

    /* �������� �� ���������� GSM ������� */
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

    /* ��������� ����� ������� GSM ������� */
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

/* ������ � �������� */
int FrameBuildAccelDataFm911(char* pOut, int OffsetData)
{
    int iFlagMove = 0;
    uint32_t sec_state_stop = SecStateStop();
    uint16_t sec_state_move = SecStateMove();

    //������ ���� �������� �������������(������ ������������, ��� ������� �������� �� GPS).
    if((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE)) {
        if(g_stRam.stAccel.eAccelState)
            g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
        iFlagMove = 1;
    }

    sprintf(&pOut[OffsetData],
        "Q,%d,%d,%d;",
        iFlagMove,         // ������� ��������
        sec_state_move,    // ����� ������ ����� ���������� ���������� � ��������
        sec_state_stop     // ����� ���������� ������� ���������
    );

    return strlen(&pOut[OffsetData]);
}
/*----------------------------------------------------------------------------*/

/* ��������� ���������� */
int FrameBuildDeviceDataFm911(char* pOut, int OffsetData)
{
    char Temp[8];
    sprintf(Temp, "N,[");
    sprintf(&pOut[OffsetData], Temp);

    /* ����� ������ ���������� */
    sprintf(Temp, "M");
    switch(GetTypeRegBase911()) {
        case TYPE_REG_TO_BASE:    //����� ����������� � ����.
            strcat(Temp, "3");
            break;
        case TYPE_REG_USER:    //����� ��������� ����������
            strcat(Temp, "1");
            break;

        /* �������� ����� ������ �� �������� �� ����������� ������ */
        default:
            strcat(Temp, "2");
            break;
    }
    strcat(&pOut[OffsetData], Temp);
    /*********************************/

    /* ����� �������� ������ �� ������� (���) */
    sprintf(Temp, ",S%d", GetAnswerTimeout());
    strcat(&pOut[OffsetData], Temp);

    /* ����� �������� ��������� �� GPS (���) */
    sprintf(Temp, ",P%d", (GetGpsWait() / 60));
    strcat(&pOut[OffsetData], Temp);

    /* ����� ����������� � ���� (���) */
    sprintf(Temp, ",N%d", GetGsmFindTimeout());
    strcat(&pOut[OffsetData], Temp);

    /* ����������� ������� �����������  */
    sprintf(Temp, ",t%+i", GetMinTemperaturWorkDevice());
    strcat(&pOut[OffsetData], Temp);

    /* ����� �� ������������ GPRS ���������� (���) */
    sprintf(Temp, ",G%d", GetGsmGprsTimeout());
    strcat(&pOut[OffsetData], Temp);

    sprintf(Temp, ",g%d", GetGprsOpenCount());
    strcat(&pOut[OffsetData], Temp);

    /* ����� �������� ���������� � GSM �������� (�� 5 ���) */
    sprintf(Temp, ",F%d", (GetLbsFindTimeout() / 5));
    strcat(&pOut[OffsetData], Temp);

    strcat(&pOut[OffsetData], "];");
    return strlen(&pOut[OffsetData]);
}
/*----------------------------------------------------------------------------*/

/*- uint16_t, uint16_t -------------------------------------------------------*/
/* ����� GPS ��������� ����� ��������, �� ��� ������ �� ������� */
static void LogGetGpsFind(char* pOut)
{
    uint16_t ucTemp1 = 0, ucTemp2 = 0;
    GetGpsFind(&ucTemp1, &ucTemp2);
    sprintf(pOut + strlen(pOut), ",p%d,%d", ucTemp1, ucTemp2);
}

/* ���������� ���� GSM ����� ��������, �� ��� ������ �� ������� */
static void LogGetGsmLog(char* pOut)
{
    uint16_t ucTemp1 = 0, ucTemp2 = 0;
    GetGsmLog(&ucTemp1, &ucTemp2);
    sprintf(pOut + strlen(pOut), ",s%d,%d", ucTemp1, ucTemp2);
}

/* ������ ������� �� GSM ������ ����� �������, �� ��� �� ������� */
static void LogGetGsmPwr(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0, usTemp3 = 0, usTemp4 = 0;
    GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
    sprintf(pOut + strlen(pOut), ",m%d,%d,%d,%d", usTemp1, usTemp2, usTemp3, usTemp4);
}

/* ����������� ���������� � ��������, �� ��� �� ������� */
static void LogGetServerConnect(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetServerConnect(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",c%d,%d", usTemp1, usTemp2);
}

/* ���������� ���������� GSM ������ �� ����� ������ ��-�� �������� �� �������, ��-�� ������ �����������  */
static void LogGetGsmWork(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetGsmWorkErr(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",d%d,%d", usTemp1, usTemp2);
}

/* ����� ���������� ��������� ���������� �� ����� ������
���� ���������� ������������ ���������� � ���������� �� ������ �� LowPower */
static void LogGetCountRebootDevice(char* pOut)
{
    uint16_t usTemp1 = 0, usTemp2 = 0;
    GetCountRebootDevice(&usTemp1, &usTemp2);
    sprintf(pOut + strlen(pOut), ",r%d,%d", usTemp1, usTemp2);
}

/* ����� ���������� ������ (�����������) ���������� */
static void LogGetDeviceWakeup(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",n%d,0", GetDeviceWakeup());
}

/*- uint16_t -----------------------------------------------------------------*/
/* ���������� �� ������� ������� ������������������ � ���� ���������  */
static void LogGetGsmFind(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",o%d", GetGsmFind());
}

/* ���������� �� �������� GPRS ������� */
static void LogGetGsmGprs(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",g%d", GetGsmGprsErr());
}

/* ���������� �� ���������� ������� �� ������� �� �������� ����� �������� */
static void LogGetServerErr(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",a%d", GetServerErr());
}

/* ���������� ����������� ��������� ������� ������� */
static void LogGetCountTimerStop(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",t0");
}

/* ����������� ���������� ����� �� ����� GPS ���������� */
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

/* ��������� ����� ������ ������  GPS */
static void LogGetTimePwrGpsAndGsm(char* pOut)
{
    uint32_t AddSec = 1480550400 + GetTimeAllPwrGps();
    RTC_t stDate;
    Sec2Date(&stDate, AddSec);
    stDate.mday--;
    sprintf(pOut + strlen(pOut), ":%d,%d,%d,%d:", stDate.mday, stDate.hour, stDate.min, stDate.sec);

    /* ��������� ����� ������ ������  GSM */
    uint32_t Sec = GetTimePwrGsm();
    AddSec = 1480550400 + Sec;
    Sec2Date(&stDate, AddSec);
    stDate.mday--;
    sprintf(pOut + strlen(pOut), "%d,%d,%d,%d", stDate.mday, stDate.hour, stDate.min, stDate.sec);
}

/* ���������� ��� ������ ������� ���� ������ ������ ����� ����� ��������� � �������.
   ������� ������ � ��������� ���� ������� TYPE_GOODBYE */
static void LogGetNewServer(char* pOut)
{
    sprintf(pOut + strlen(pOut), ",F0");
    if(GetTypeRegBase911() != TYPE_GOODBYE) {    //�� ���������� ���� ��� ���� ��� ������� �� "����������"
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

/* ��������� ������ */
static void LogGetStatusButton(char* pOut)
{
    strcat(pOut, ":rn");    //������ rn
}
/*----------------------------------------------------------------------------*/

void (*ptrCallbackLogFindme911Scheduler[])(char* pOut) = {
    LogGetCountRebootDevice,    // r (����� ���� ��������� ������������: ������ + �������)
    LogGetCountTimerStop,       // t (���������� ������������ ��������� ��������� ������� (����������))
    LogGetGpsFind,              // p (����� GPS ���������)
    LogGetGsmLog,               // s (���������� ���� GSM)
    LogGetGsmPwr,               // m (������ ������� �� GSM ������)
    LogGetGsmFind,              // o (���������� �� ������� ������� ������������������ � ���� ���������)
    LogGetServerConnect,        // c (����� ���������� ������� ������������ ���������� � ��������)
    LogGetGsmGprs,              // g (���������� �� �������� GPRS �������)
    LogGetGpsWait,              // m (����������� ���������� ����� �� ����� GPS �����)
    LogGetServerErr,            // a (���������� �� ���������� ������� �� ������� �� �������� ����� ��������)
    LogGetGsmWork,              // d (���������� gsm ��-�� �������� �� �������)
    LogGetDeviceWakeup,         // n (����� ������� �� ���������� �� ����������� � �������)
    LogGetNewServer,            // f (���������� ��� ������ ������� ���� ������ ������ ����� ����� ��������� � �������. �������
                                // ������ � ��������� ���� ������� TYPE_GOODBYE)
    LogGetTimePwrGpsAndGsm,     //: (��������� ����� ������ ������ GPS � GSM)
    LogGetStatusButton,         //:rn (��������� ������)
};

/* ��� ������ ���������� */
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

/* ������ ������ �� ������� 911.
������������� ������������ �������� �������� 911
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

    //������ ������, ��������� ��.
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

    /* �������� �� $ERROR */
    if(strstr(pBuf, "$ERROR")) {
        return ANS_ERROR;    // Error CRC
    }

    /* Parser Frame */
    /* ������� ����������� ����� */
    int fm911_check_crc(uint8_t*);
    if(fm911_check_crc((uint8_t*)pBuf)) {
        return CRC_ERROR;    // Error CRC
    }

    RTC_t DateRTC;
    int year = 0, month = 0, mday = 0, hour = 0, min = 0;

    /* ������� ����� �������  */
    sscanf(pBuf, "$%2d%2d%2d%2d%2d", &year, &month, &mday, &hour, &min);
    DateRTC.sec = 0;
    DateRTC.wday = 0;
    DateRTC.year = year;
    DateRTC.month = month;
    DateRTC.mday = mday;
    DateRTC.hour = hour;
    DateRTC.min = min;
    setSystemDate(&DateRTC);    //������������� ��������� �����

    /* ����� ���������� ������ ����� */
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

    /* �� ����� �������� ����� ����� ������ �������, ����� �����������*/
    SetSleepTimeStandart(Date2Sec(&DateRTC));
    SetSleepTimeFind(Date2Sec(&DateRTC));

    /* ������� ������ ������ ������� */
    while(*pBuf++ != ',')
        ;
    char* ptr = strchr(pBuf, 'Z');
    if(ptr) {    //�������� ����� ������
        ptr++;
        TYPE_MODE_DEV eModeDevice = STANDART;    //���������� ����� �� ���������
        if(*ptr == '1') {
            // ����� ������
            eModeDevice = TIMER_FIND;
        }
        /* ������������ ������������ */
        char* ptr = strchr(pBuf, 'A');
        ptr++;
        if(*ptr == '1') {
            // ����� ������ �� �������������
            SetEnableAccelToFind(TRUE);
        }
        else {
            SetEnableAccelToFind(FALSE);
        }
        /* ���� ����� ����� ������������ ������������ � ���� ����������� �� ��������, �� ������� ���� ����� */
        if(GetEnableAccelToFind() && GetTypeConnectFm911() == TYPE_MOVE_START) {
            eModeDevice = TIMER_FIND;
            SetEnableAccelToFind(TRUE);
        }
        SetModeDevice(eModeDevice);
    }

    /* C���� ������ (����� ��������� ����������, �������� ����� ������, ����� ����������� � ����) */
    ptr = strchr(pBuf, 'M');
    if(ptr) {
        ptr++;
        switch(*ptr) {
            case '1': /* ����� ��������� ���������� */
                SetTypeRegBase911(TYPE_REG_USER);
                SetUserTel(STR_NULL);    //����� ������ ������������ ��� ������ ��������� ���������� ��� ������ ������������.
                break;
            case '2': /* �������� ����� ������ */
                /* ���� ������ ��� � ������ ����������� �����, �� ��������� ����� �� ����������� � ������ ��� ��� */
                if(GetTypeConnectFm911() == TYPE_REG_USER) {
                    SetTypeRegBase911(TYPE_WU_START);
                    PDU_SMGD(6);    // �������� ���� ���
                }
                break;
            case '3': /* ����� ����������� � �� */
                SetTypeRegBase911(TYPE_REG_TO_BASE);
                SetUserTel(STR_NULL);    //����� ������ ������������ ��� ������ ����������� � ��.
                break;
        }
    }

    /* ����� �������� ��������� �� GPS (���) */
    ptr = strchr(pBuf, 'P');
    if(ptr) {
        int value = 0;
        sscanf(ptr, "P%2d,", &value);
        value *= 60;    //��������� � �������.
        SetGpsWait(value);
    }
    /*******************************************/

    /*���������������� ��� ������ �� ���� ��������, ������������� ����� ���������� � ��������
   � ������ ���������� ����� (������� ����� �������� ��������� �������� ������� ���������: m-������, h-����, d-���) */
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

    /* ����� ������ GSM ���� (����������� � ���� ) � ���. */
    ptr = strchr(pBuf, 'N');
    if(ptr) {
        int value;
        sscanf(ptr, "N%2d,", &value);
        SetGsmFindTimeout(value);
    }

    /* ����������� ������� ����������� */
    ptr = strchr(pBuf, 't');
    if(ptr) {
        int value;
        sscanf(ptr, "t%3d,", &value);
        SetMinTemperaturWorkDevice(value);
    }

    /* ����� �������� ������ �� ������� */
    ptr = strchr(pBuf, 'S');
    if(ptr) {
        int value;
        sscanf(ptr, "S%2d,", &value);
        SetAnswerTimeout(value);
    }

    /* ����� �� ������������ GPRS ���������� (���) */
    ptr = strchr(pBuf, 'G');
    if(ptr) {
        int value;
        sscanf(ptr, "G%2d,", &value);
        SetGsmGprsTimeout(value);
    }

    /* ���������� ������� ������������ GPRS ���������� */
    ptr = strchr(pBuf, 'g');
    if(ptr) {
        int value;
        sscanf(ptr, "g%2d,", &value);
        SetGprsOpenCount(value);
    }

    /* ����� �������� ���������� � GSM �������� (�� 5 ���) */
    ptr = strchr(pBuf, 'F');
    if(ptr) {
        int value;
        sscanf(ptr, "F%2d,", &value);
        value *= 5;
        SetLbsFindTimeout(value);
    }
    /*******************************************/

    /* ����� ������� ������� (����� �������������� ������ ���� ������ ��������� FM911) */
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

    /* ����� ������� ���� ����� ������ ������ �� ��������� iON */
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
            SetTypeRegBase911(TYPE_GOODBYE);    //������ ��� ���������� � 911 ��� �������� ��������� ������
            setGsmStep(GSM_PROFILE_GPRS_CONNECT);
            SaveConfigCMD();
        }
    }
    /*******************************************/

    /* ����� ����������� ������ ������������ */
    char strNewUserTel[SIZE_TEL] = { 0 };
    ptr = strchr(pBuf, 'Y');
    if(ptr && ptr[1] != ',') {    // �������� ��������� � �������� ��� � ������ ����� ���� ��������.
        ptr += 3;
        loop(SIZE_SERV)
        {
            if(*ptr == '"')
                break;
            strNewUserTel[i] = *ptr++;
        }
        if(strlen(strNewUserTel)) {
            SetUserTel(strNewUserTel);    // �������� ����� ����� �������������.
        }
    }

    SaveConfigCMD();
    return ANS_OK;    // Ok
}

/*
�������� ��� �� ������������ �� ��� �����������.
Input:
1 - ��������� �� ����������� ��� �������;
2 - ��������� �� ����������� ����� ���. �� ���� �������� ���(���������������� �����).
3 - ��� ��������� ���
Return:
0 - ��� ���;
1 - ������ ���.
*/
int CheckSmsRegUserFm911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode)
{
    SMS_INFO sms_cmd;
    memset(&sms_cmd, 0, sizeof(SMS_INFO));

#if(_FAST_REGISTRATION_USER_)
    //�������� �������� ������� + IMEI.
    char strIMEI[SIZE_IMEI];
    GetStrIMEI(strIMEI);
    strcpy(ptrNameDevice, NAME_DEVICE);
    int n = strlen(strIMEI) - (MAX_SIZE_NAME_DEVICE - strlen(ptrNameDevice) - 1);
    for(int i = strlen(ptrNameDevice); i < MAX_SIZE_NAME_DEVICE - 1; i++) {
        ptrNameDevice[i] = strIMEI[n++];
    }
    ptrNameDevice[MAX_SIZE_NAME_DEVICE - 1] = NULL;

    strcpy(ptrNumUserTel, TEL_REG_USER);    //�������� ����� ������������.
    *pCode = CODE_LAT;
    return 1;
#endif

    sms_cmd.txt.buf = (uint8_t*)g_aucOutDataFrameBuffer;
    SMS_RESPONSE ret = PDU_SMGL_FM911(ptrNumUserTel, ptrNameDevice, pCode);
    if(ret == SMS_TRUE) {
        return 1;
    }

    /* �������� �� ������ ������ ��� */
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
                ReStartDmaGsmUsart();     //������������� DMA.
                g_bDmaGsmFail = FALSE;    //������ �������� � DMA - ��������� ����
                PDU_SMGD(6);              // Delete all SMS
                break;
        }
    }
    return 0;
}

/*
�������� ������� ������ �� ����� � ����� ������� FM911.
Input: no.
Return:
  ������� ������ �� �����:
    TYPE_REG_USER =              'R',     // ����������� ������������ �� �������
    TYPE_REG_TO_BASE =           'B',     // ����������� ���������� � ���� ������ ������� � �������� �����������
    TYPE_ERR_POWER =             'D',     // GSM ������ ���������� ��-�� ����������� ���������� �������
    TYPE_WU_START =              'A',     // ��������� �������
    TYPE_WU_BUTTON =             'E',     // ������������ �� ������
    TYPE_WU_TIMER =              'T',     // ����������� �� �������
    TYPE_ERR_COLD =              'H',     // ������ �����������
    //TYPE_ERR_NET =               'O',     // ���� ��������� �� �������, ��������� ����� ������
    //TYPE_ERR_CMDGSM =            'K',     // �� ������� ����� �� �������, ��������� ����� ������
    //TYPE_ERR_CONGPRS =           'G',     // ����������� GPRS ����������, ��������� ����� ������
    TYPE_ERR_CONSRV =            'X',     // �� ������� ����� �� �������, ��������� ����� ������
    TYPE_ERR_TIMER =             'F',     // �� �������� ������
    TYPE_MOVE_START =            'M',     // ������ ��������
    TYPE_MOVE_STOP =             'N',     // ����������
    TYPE_GOODBYE =               'J'      // ���� � ������� 911 �� ����� � ������ ���������
*/
T_TYPE_CONNECT GetTypeConnectFm911(void)
{
    T_TYPE_CONNECT eTypeCon = GetTypeRegBase911();
    if(eTypeCon != TYPE_WU_START) {
        return eTypeCon;
    }

    RESET_STATUS_DEVICE eStatReset = GetStatusReset();    // ���� ������� ������������.

    switch(eStatReset) {
        case BUTTON_RESET:
            return TYPE_WU_BUTTON;    // ������������ �� ������

        case LOW_POWER:
            return TYPE_WU_START;    // ������������ �� ���� �������

        case WAKE_UP_ACCEL:
            return TYPE_MOVE_START;    // ������������ �� �������������(������ ��������)

        case WAKE_UP_STOP:
            return TYPE_MOVE_STOP;    // ����������

        case WAKE_UP_ALARM:
        case WAKE_UP_LOW_PWR1:
        case WAKE_UP_LOW_PWR2:
            return TYPE_WU_TIMER;    // ����������� �� �������
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