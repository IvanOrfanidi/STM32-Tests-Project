#include "includes.h"
#include "gsm_general.h"
#include "flash_archive.h"

void set_fm_step_count(uint8_t ucstep);

TGsmStatus stGsmStatus;
void waitSMS(uint32_t);

static _Bool choiceSearch();

static int (*ptrSwitchAddrServer[])(char* pOut) = {
    GetAddrFirstServ,
    GetAddrSecondServ,
};

void (*ptrCallbackGsmScheduler[])(TGsmStatus* stGsmStatus) = {
    /* GSM Config */
    stepGsmOff,
    stepGsmLowOff,
    stepGsmStart,
    stepGsmSettingStart,
    stepGsmInit,
    stepGsmJammingDetection,
    stepGsmSim,
    stepGsmSimPin,
    stepGsmConfig,
    stepGsmMessageEvent,
    stepGsmCellularNetwork,
    stepGsmCheckSMS,

    /* GPRS Config */
    stepGsmProfileGprsConnect,
    stepGsmProfileGprsSocket,
    stepGsmProfileGprsActivate,
    stepGsmProfileGprsDeactivate,
    stepDeactivateCheckSmsActivate,

    /* TRACK ION */
    stepGsmProfileGprsSendDataInit,
    stepGsmSwitchData,
    stepGsmSendAckC,
    stepCheckSMS,

    stepWaitFlashDataReady,
    stepGsmSendAnsOkData,
    stepGsmSendFailC,
    stepGsmProfileGprsAcknowData,

    /* FindMe WEB */
    stepFindMeDataReady,
    stepGsmSleepWaitGps,
    stepGsmSleepWaitAccelStop,

    stepGsmProfileGprsSendDataStatusFirmware,
    stepSynchronizationServerTimeRequest,
    stepGsmProfileGprsSendStatusDevice,
    stepGsmProfileGprsSendLogDevice,
    stepGsmProfileGprsAccelStatus,

    stepGsmProfileGprsSendDataFm,
    stepGsmProfileGprsSendDataION,
    stepGsmProfileGprsSendDataConfig,
    stepGsmProfileGprsSendArchiveData,

    /* FindMe 911 */
    stepGsmFindme911DataReady,
    stepGsmFindme911GprsAckData,
    stepGsmFindme911WaitRegUser,

    stepGsmEndSms,

    /* UPDATE FIRMWARE */
    stepGsmProfileHttpSocket,
    stepGsmProfileHttpDownload,
    stepGsmProfileCheckFirmware,
    /***********************/

    stepConnectError,
    stepSleep,
    stepRestartNow,

    stepDeviceTest
};

// �������� GSM �������.
void vGsmTask(void* pvParameters)
{
    stGsmStatus.eGsmStep = GSM_START;
    stGsmStatus.eNextAckGsmStep = GSM_START;
    stGsmStatus.eCmdGsmStep = GSM_START;
    stGsmStatus.uiGsmStepDelay = 0;

    while(1) {
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (stGsmStatus.uiGsmStepDelay / portTICK_RATE_MS));

        if(stGsmStatus.eCmdGsmStep != GSM_START) {
            stGsmStatus.eGsmStep = stGsmStatus.eCmdGsmStep;
            stGsmStatus.eCmdGsmStep = GSM_START;
        }
        SystemUpdateMonitor();    //��������� ���������� ��������� FreeRTOS

        /* GSM Scheduler */
        (*ptrCallbackGsmScheduler[stGsmStatus.eGsmStep])(&stGsmStatus);
        EepromHandler();
    }
}

void stepSleep(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepSleep\r\n");

    SetTimePwrGsm(g_stRam.stDevDiag.stHard.uiTimePwrGsm + GetTimePwrGsm());    //��������� ����� ������ GSM ������.

    /* ������� ������ ������������� � ���� �� ���������� ��������, �� �� �����, � ����� �� GSM ON */
    if(GetEnableAccelToFind() && AccelState() == ACC_STATE_MOVE && GetModeDevice() == TIMER_FIND) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
        pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_ACCEL_STOP;
        return;
    }

    extern _Bool checkSMS;
    if(GetStatusReset() == BUTTON_RESET && checkSMS == FALSE) {
        SetTimeWaitSms(DEF_TIME_WAIT_SMS);
    }

    GSM_State(OFF);    //���� GSM ������

    vTaskSuspendAll();    // STOP RTOS
    SetFlagSleep();
    SaveConfig();    // Save Config

    _Bool accel_enable = FALSE;
    if(GetModeProtocol() == FINDME_911 && GetEnableAccelToFind()) {    //���� �������� ���������� �� ������������� � ����� ����� ������.
        accel_enable = TRUE;
    }

    if(GetModeProtocol() == ION_FM && GetModeDevice() == TIMER_FIND && GetEnableAccelToFind()) {
        accel_enable = TRUE;
    }

    if(accel_enable) {
        accelInit(GetAccelSensitivity());    // Init Accel
    }
    else {
        Accel_Power_Down();
    }

    DelayResolution100us(100);

    SetTimeSleepDevice();
}

void resetConnectError(void)
{
    g_stRam.stDevice.CountSimFirstError = 0;
    g_stRam.stDevice.CountSimSecondError = 0;
}

void stepGsmOff(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmOff\r\n");
    ledStatus(LOAD_DEVICE);    // �������� ����������
    g_stRam.stSim.eRegStatus = SIM_WAIT;
    g_stRam.stSim.bGprsTrue = FALSE;
    g_stRam.stConnect.bGprsProfActivate = FALSE;
    g_stRam.stSim.bGprsTrue = OFF;

    if(GetModeDevice() == TRACK_ION) {
        if(g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE) {
            pGsmStatus->eGsmStep = GSM_START;
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_3000;
            SimPwr(OFF);
            GSM_State(OFF);
        }
        else {
            pGsmStatus->eGsmStep = GSM_OFF_LOW_PWR;
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
        }
    }
    else {
        pGsmStatus->eGsmStep = GSM_START;
        SimPwr(OFF);
        GSM_State(OFF);
        /* ���������� �������� ������ ��� ������ �� ��������� SIM ������ */
        if(SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD) {
            if(!(getUseSecondSim())) {
                g_stRam.stDevice.CountSimFirstError++;
            }
        }
        else {
            if(getUseSecondSim()) {
                g_stRam.stDevice.CountSimSecondError++;
            }
        }

        /* �������� ��������� � ������ �� �������� ��� */
        if(g_stRam.stDevice.CountSimFirstError > MAX_COUNT_SIM_FIRST_ERROR) {
            if(                                                 /*GetCountReConnect()                                                   // �������� ����������������
             (������ ���� �� �������)
             &&*/
                GetTypeConnectFm911() != TYPE_REG_USER          // ���� �� ����� ����������� ������������
                && GetTypeConnectFm911() != TYPE_REG_TO_BASE    // ���� �� ����� ����������� � ��
                && getFlagSimSecondInstld()) {                  // ���� ���������� ���� ��� ��������� ��� ����� ���� �����������
                /* ����� ������ �� �������� ��� ��������, ������������� �� ���������. */
                setUseSecondSim();    // ������������ �� ������ ���.
            }
            else {
                // ���� ������ ���������������� ��� ������ �����������, �� ����� ��� ������������ �� ������ ���
                pGsmStatus->eGsmStep = GSM_CONNECT_ERROR;
                pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
            }
        }

        if(g_stRam.stDevice.CountSimSecondError > MAX_COUNT_SIM_SECOND_ERROR) {
            /* ����� ������ �� ��������� ��� ��������, ������ � ��� */
            pGsmStatus->eGsmStep = GSM_CONNECT_ERROR;
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
        }
    }
}

void stepGsmLowOff(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmLowOff\r\n");
    PWR_STATUS BackPowerStatus;

    uint32_t delay = 0;
    uint32_t SecRTC = 0;
    uint32_t delay_gsm_off = 0;    // ����� ������������ gsm ������ � ��������
    stAccelAxisData stAccStateBack;
    TAcc_state stAcc_state;
    portTickType xLastWakeTimerDelay;
    SimPwr(OFF);

    GSM_State(SLEEP);    //�������� ����� � ���

    /* �������� �������� ������ ���������� ����� ��������� ����� ������ ����� ����� */
    if(GetModeDevice() == TRACK_ION) {
        switch(g_stRam.stDevice.eCurPwrStat) {
            case POWER_RUN_MODE:
                DP_GSM("D_PWR LOW STATUS: ERROR\r\n");
                break;

            case POWER_LOW_PWR1_MODE:
                delay_gsm_off = GsmLowPwr1();
                break;

            case POWER_LOW_PWR2_MODE:
                delay_gsm_off = GsmLowPwr2();
                break;
        }
    }

    BackPowerStatus = g_stRam.stDevice.eCurPwrStat;

    if(delay_gsm_off) {
        SecRTC = time();
        delay = SecRTC + delay_gsm_off;    //�������� �������� ����� �����������.

        ReadAxisDataAccel(&stAcc_state);
        stAccStateBack.X = stAcc_state.X;
        stAccStateBack.Y = stAcc_state.Y;
        stAccStateBack.Z = stAcc_state.Z;

#define DEF_ADD_SENS_ACCEL 0x0500
        uint8_t ucSensitivity = GetAccelSensitivity();
        uint16_t uiDelta = DEF_ADD_SENS_ACCEL + (ucSensitivity << 8);

        while(1) {
            IWDG_ReloadCounter();

            ReadAxisDataAccel(&stAcc_state);
            /*DP_GSM("D_AXIS %i", stAcc_state.X);
         DP_GSM(" %i", stAcc_state.Y);
         DP_GSM(" %i\r\n", stAcc_state.Z);*/

            if((stAcc_state.X > stAccStateBack.X + uiDelta) || (stAcc_state.Y > stAccStateBack.Y + uiDelta) ||
                (stAcc_state.Z > stAccStateBack.Z + uiDelta) || (stAcc_state.X < stAccStateBack.X - uiDelta) ||
                (stAcc_state.Y < stAccStateBack.Y - uiDelta) || (stAcc_state.Z < stAccStateBack.Z - uiDelta)) {
                if(GetModeDevice() == TRACK_ION) {
                    g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
                    break;
                }
            }

            //������� �������� � ������ ��������� ������ ����������������� ��� ���� ���������� ��������.
            if((BackPowerStatus != g_stRam.stDevice.eCurPwrStat) && (GetModeDevice() == TRACK_ION)) {
                if(g_stRam.stAccel.eAccelState == ACC_STATE_MOVE) {
                    g_stRam.stAccel.eAccelState = AccelState();    //������� ������ �������������.
                }
                break;
            }

            SecRTC = time();
            if(delay <= SecRTC) {
                break;    //���������� � �����, ������ ������ ���������� � �����
            }

            if(g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR1_MODE) {
                DP_GSM("D_LOW PWR1: ");
            }
            else {
                if(g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE) {
                    DP_GSM("D_LOW PWR2: ");
                }
                else {
                    DP_GSM("D_ERROR:");
                }
            }
            DP_GSM("%d sec\r\n", delay - SecRTC);

            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
        }
        IWDG_ReloadCounter();
    }

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    pGsmStatus->eGsmStep = GSM_START;
}

void stepGsmStart(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmStart\r\n");

    ledStatus(LOAD_DEVICE);    //�������� ����������

    /* �������� �� ����������� */
    osDelay(SLEEP_MS_1000);
#if(TEMPERATURE_ACCEL)
    int8_t cTemperature = CalculTemperaturAccel();
#else
    int8_t cTemperature = 0;
#endif
    DP_GSM("D_DEVICE TEMPERATURE %+dC\r\n", cTemperature);
    if(cTemperature < GetMinTemperaturWorkDevice()) {
        DP_GSM("-D_ERR: COLD TEMPERATUR-\r\n");
        if(GetTypeRegBase911() == TYPE_WU_START) {
            SetTypeRegBase911(TYPE_ERR_COLD);    // ��������� ����� "������ �����������" � �����
        }
        uint16_t GsmLowPwr, GsmLowTemp;
        GetGsmWorkErr(&GsmLowPwr, &GsmLowTemp);
        GsmLowTemp++;
        SetGsmWorkErr(GsmLowPwr, GsmLowTemp);
        pGsmStatus->eGsmStep = SLEEP_DOWN;
        return;
    }

    if(alarmTrue()) {
        SIM_ON;
    }

    //������ ������� �� GSM ������
    uint16_t usTemp1, usTemp2, usTemp3, usTemp4;
    GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
    usTemp1++;    //����� ����� �������

    RET_INFO eStatusRet = (RET_INFO)(GSM_State(ON));
    if(eStatusRet != RET_OK) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
        pGsmStatus->eGsmStep = GSM_OFF;
        usTemp2++;    //����� ��������� �������� �������� GSM
        SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);
        return;
    }

    osDelay(SLEEP_MS_2000);
    eStatusRet = GsmModemCmdOn();
    if(eStatusRet != RET_OK) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
        pGsmStatus->eGsmStep = GSM_SETTING_START;
        return;
    }
    SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    pGsmStatus->eGsmStep = GSM_CONFIG;
}

void stepGsmSettingStart(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmSettingStart\r\n");

    RET_INFO eStatusRet = ModemSettingStart();
    if(eStatusRet == RET_OK) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
        pGsmStatus->eGsmStep = GSM_CONFIG;
    }
    else {
        uint16_t usTemp1, usTemp2, usTemp3, usTemp4;
        GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
        usTemp2++;    //����� ��������� �������� �������� GSM
        SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);

        pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
        pGsmStatus->eGsmStep = GSM_OFF;
    }
}

void stepGsmInit(TGsmStatus* pGsmStatus)
{
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    DP_GSM("D_GSM step: stepGsmInit\r\n");
    static _Bool jam_ok = FALSE;    //���� ��� ��� ������� �������� �� ��������

    if(initGsmModule() == RET_OK) {
        /* �������� ���� �� ��������� ��� ����������� ��������� ������ */
        _Bool alarm = FALSE;
        if(alarmTrue()) {
            alarm = TRUE;
        }
        if((GetJamDetect() == FALSE) || (alarm == TRUE) || (jam_ok == TRUE)) {
            pGsmStatus->eGsmStep = GSM_SIM;
        }
        else {
            jam_ok = TRUE;
            pGsmStatus->eGsmStep = GSM_JAMMING_DETECTED;
        }
    }
    else {
        pGsmStatus->eGsmStep = GSM_OFF;
    }

    // pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;
}

/*
0 - GPS
1 - GSM
*/
static _Bool choiceSearch()
{
    /* ���� ��������� ��������� �����������, �� ��������� ��� ������� GPS */
    if(alarmTrue()) {
        return 1;
    }

    /* � ����������� �� ������� ��������� ��� ������� GPS */
    if(GetTypeConnectFm911() == TYPE_REG_USER || GetTypeConnectFm911() == TYPE_ERR_CONSRV ||
        GetTypeConnectFm911() == TYPE_WU_BUTTON || GetTypeConnectFm911() == TYPE_WU_START ||
        GetTypeConnectFm911() == TYPE_ERR_COLD) {
        return 1;
    }

    if(GetEnableAccelToFind() && GetStatusReset() == WAKE_UP_ACCEL) {
        return 1;
    }

    /* ���� ��������� � ������ ������, �� ����� �������� �� ����� GPS */
    static _Bool selFindGps = TRUE;
    if((GetModeDevice() == TIMER_FIND) && (selFindGps)) {
        selFindGps = FALSE;
        return 0;
    }

    return 1;
}

void stepGsmJammingDetection(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmJammingDetection\r\n");
    TS_MOND base_station;
    GSM_INFO stGsm;
    memset(&stGsm, 0, sizeof(GSM_INFO));
    INIT_GSM_INFO(stGsm);
    stGsm.inf = &base_station;
    static uint8_t ErrJamDetect = 0;
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;

    if(GetJamDetect()) {
        DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");
        //�������� LLS, ��� �� ���� �� ����� ����� ���. ������� GSM.
        getAllLbsInfo(&stGsm, GetLbsFindTimeout());

        DP_GSM("D_JMP LBS COUNT: %d\r\n", stGsm.count);
        if(stGsm.count > 1) {
            setAllLbsData(&stGsm);
            if(choiceSearch()) {
                pGsmStatus->eGsmStep = GSM_SIM;
            }
            else {
                pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;
            }
        }
        else {
#define MAX_COUNT_ERR_JAM_DETECT 1
            if(ErrJamDetect > MAX_COUNT_ERR_JAM_DETECT) {
#ifdef FM3
                DP_GSM("-D_ERR: THERE IS NO CELLULAR CONNECTION-\r\n");
                uint16_t usTemp1, usTemp2, usTemp3, usTemp4;
                GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
                usTemp4++;    //�� ��������� � ����
                SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);
#endif
                if(GetModeDevice() == TRACK_ION) {
                    pGsmStatus->eGsmStep = GSM_OFF;
                }
                else {
                    pGsmStatus->eGsmStep = SLEEP_DOWN;    //�������� ���� �� ����� ������� GSM.
                }
            }
            else {
                ErrJamDetect++;
                pGsmStatus->eGsmStep = GSM_JAMMING_DETECTED;
            }
        }
    }
}

static void selectContactCurrentSim(_Bool fLockSimPin, _Bool fVerifySecSimForLowPwr)
{
    /* �������� �� �������������� ��������� SIM ����� */
    if((alarmTrue())              // ������ ��������� ������
        || (getUseSecondSim())    // ��������� ��������� ����� ��� ������ �� ������
        || (GetStatusReset() == LOW_POWER &&
               fVerifySecSimForLowPwr)       // ���������� ��������� ������� ��������� SIM(��� ������ �������)
        || (expireTimeContactSecSim())) {    // ������� ����� ���������� ������ �� ��������� SIM (1 �����)
        if(GetStatusReset() == LOW_POWER && fVerifySecSimForLowPwr) {
            DPS("D_GetStatusReset() && VerifySecSimForLowPwr == TRUE\r\n");
        }
        if(GetStatusReset() != LOW_POWER) {
            setTimeContactSecondSim(
                time());    // ��������� ����� ����������� �� ��������� ���(����������� ����� ������ ��������� SIM).
        }
        SelectNumSim(SECOND_SIMCARD_OK);    // ������������� �� ��������� ���.
    }
    else {
        if(!(fLockSimPin)) {
            SelectNumSim(FIRST_SIMCARD_OK);
        }
    }
}

/*
�������� IMEI GSM ������ (������ ��� �������� SIM �����)
  TRUE - IMEI �� ���������� ����� �� ���������� GSM ������
  FALSE - IMEI ���������� ��� ������ � ��������� SIM.
*/
static _Bool checkFirstGsmImei(void)
{
    if(SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD) {    // ���� �������� ��� �����, �� ��������� � �������� IMEI ������
        char strIMEI[SIZE_IMEI] = { '\0' };
        if(!(GetImeiFirstGsm(strIMEI))) {
            if(readGsmIMEI(strIMEI)) {    // IMEI GSM-������
                SetImeiFirstGsm(strIMEI);
            }
            else {
                return TRUE;    // �� �������� IMEI
            }
        }
    }
    return FALSE;    // ��� ��, ��� ��������� ���.
}

void stepGsmSim(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmSim\r\n");
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

    /* ����� �������� ���� ����� ��� ������������, ������ ��� FM3 */
    if(checkTwoSIM()) {
        DP_GSM("D_You must insert the second SIM-card\r\n");
        pGsmStatus->eGsmStep = SLEEP_DOWN;    //�������� ���� ������� ���� ���� ��� �����.
        return;
    }

    /* ����� SIM ����� �� ������� ����� ������������� ����� */
    static _Bool fLockSimPin = 0;                  //���� ��� ����� ��� � ��� �����
    static _Bool fVerifySecSimForLowPwr = TRUE;    // ���� ��� ��������� ������� ��������� ��� ��� ���������� �������
    /* ���� ������ �����������, �� ������ ���� fVerifySecSimForLowPwr �� �������� */
    if(GetTypeConnectFm911() == TYPE_REG_TO_BASE || GetTypeConnectFm911() == TYPE_REG_USER) {
        fVerifySecSimForLowPwr = FALSE;
    }
    selectContactCurrentSim(fLockSimPin, fVerifySecSimForLowPwr);

    /* �������� IMEI GSM ������ (������ ��� �������� SIM �����) */
    if(checkFirstGsmImei()) {    // ��� ��������� SIM ����� ������ FALSE
        // IMEI �� ���������� ����� �� ���������� GSM ������.
        pGsmStatus->eGsmStep = GSM_OFF;
        return;
    }

    /* �������� SCID ��� ����� � � ��������� */
    M_INFO eSimStatus;
    char strFIRST_SCID[SIZE_SCID] = { '\0' };
    char strSECOND_SCID[SIZE_SCID] = { '\0' };

    /* �������� ������� SCID ��� ����� � � ��������� */
    if(SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD) {
        eSimStatus = getSimStatus(strFIRST_SCID);    // First SIM Card
    }
    else {
        eSimStatus = getSimStatus(strSECOND_SCID);    // Second SIM Card
    }

    if(eSimStatus == M_SIM_READY) {
#ifdef FM3
        /* ����� �� ��������� SCID ��� �����, ����� �������� � ��� ������ FM911 */
        if((!(GetScidFirstSim(strFIRST_SCID))) && GetTypeConnectFm911() == TYPE_REG_TO_BASE &&
            (GetNumSim() == FIRST_SIMCARD_OK) && (!(fLockSimPin))) {
            if(LockSimCard()) {                       //����� ������� ��� ���.
                pGsmStatus->eGsmStep = SLEEP_DOWN;    //�� ������� ���������� ��� ���.
                                                      // return;
            }
        }
#endif

#if(TWO_SIMCARD)
        /* ��������� ����� � ������� ��������� SIM ����� */
        if((GetStatusReset() == LOW_POWER)                                 // ���������� �� ����� ��������
            && (GetTypeConnectFm911() != TYPE_REG_TO_BASE)                 // �� � ������ ����������� � ��
            && (fVerifySecSimForLowPwr)                                    // ���� ��� ��������� ������� ��������� ��� ��� ���������� �������
            && (SelectNumSim(GET_NUM_SIM) == CURRENT_SECOND_SIMCARD)) {    // ������ �� ������ �����
            setFlagSimSecondInstld(TRUE);
            fVerifySecSimForLowPwr = FALSE;
            pGsmStatus->eGsmStep = GSM_OFF;
            resetConnectError();
            SaveConfigCMD();
            return;
        }
#endif

        pGsmStatus->eGsmStep = GSM_ADDITIONAL_CONFIG;
    }
    else {
        switch(eSimStatus) {
            case M_SIM_PIN:    // SIM PIN CODE
                fLockSimPin = 1;
                g_stRam.stSim.eRegStatus = SIM_PIN;
                pGsmStatus->eGsmStep = GSM_SIM_PIN;
                break;
            case M_SIM_PUK:    // SIM PUK CODE
                g_stRam.stSim.eRegStatus = SIM_PUK;
                if(GetModeDevice() == TRACK_ION) {
                    pGsmStatus->eGsmStep = GSM_OFF;
                }
                else {
                    pGsmStatus->eGsmStep = SLEEP_DOWN;
                }
                break;
            case M_OTHER:    // SIM OTHER ERROR
                g_stRam.stSim.eRegStatus = SIM_ERROR;
                pGsmStatus->eGsmStep = GSM_OFF;

            default:    // SIM NO READY for default
                g_stRam.stSim.eRegStatus = SIM_NO_READY;
                pGsmStatus->eGsmStep = GSM_OFF;
        }

        /* ����� ����� � ������� ��������� SIM ����� */
        if((GetStatusReset() == LOW_POWER && fVerifySecSimForLowPwr)       // ���������� �� ����� ��������
            && (SelectNumSim(GET_NUM_SIM) == CURRENT_SECOND_SIMCARD)) {    // ������ �� ������ �����
            setFlagSimSecondInstld(FALSE);
            fVerifySecSimForLowPwr = FALSE;
            pGsmStatus->eGsmStep = GSM_OFF;
            resetConnectError();
            SaveConfigCMD();
        }
    }

    if(switchingEventSim(eSimStatus)) {
        pGsmStatus->eGsmStep = GSM_OFF;
    }
}

void stepGsmSimPin(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmSimPin\r\n");

    M_INFO eSimStatus = SimCardPin();
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;    //�� ��������� ���� �������, ������ �� ����������� � ����.
    if(eSimStatus == M_OK) {
        pGsmStatus->eGsmStep = GSM_SIM;
        return;
    }
    else {
        if(eSimStatus == M_SIM_PUK) {
            g_stRam.stSim.eRegStatus = SIM_PIN_GUESSING;
        }
        else {
            g_stRam.stSim.eRegStatus = SIM_PIN_NO_SET;
        }
        if(GetModeDevice() == TRACK_ION) {
            pGsmStatus->eGsmStep = GSM_OFF;
        }
        else {
            pGsmStatus->eGsmStep = SLEEP_DOWN;
        }
    }
}

void stepGsmConfig(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmConfig\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

    if(configGsmModule() != RET_OK) {
        DP_GSM("D_GSM SIM ERROR\r\n");
        pGsmStatus->eGsmStep = GSM_OFF;
        return;
    }

    _Bool get_flag_message(void);
    /* �������� ��������� ���������� */
    if(alarmTrue()) {
        //����� ����������� � ��������.
        pGsmStatus->eGsmStep = GSM_MESSAGE_EVENT;
        return;
    }

    /* ������� ������ GPS � ��������� ����. ���� ��, �� �������� ������, � ���� ���, �� ��������� GSM ����� � ����
    * ������� GPS. */
    if(g_stRam.stConnect.bWaitGpsModeFm == FALSE) {
        if(GetModeProtocol() == FINDME_911) {    //���� �������� FM911
            if(GetTypeConnectFm911() == TYPE_REG_USER || GetTypeConnectFm911() == TYPE_GOODBYE) {
                g_stRam.stSim.eRegStatus = FIND_NET;
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
                return;
            }
            if((GetModeDevice() == STANDART) && (GetTypeConnectFm911() != TYPE_REG_TO_BASE)) {
                g_stRam.stSim.eRegStatus = FIND_NET;
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
            }
            else {
                if(choiceSearch()) {
                    g_stRam.stSim.eRegStatus = FIND_NET;
                    pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
                }
                else {
                    pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;
                }
            }
#if(_FAST_REGISTRATION_TO_BASE_)
            if(GetTypeConnectFm911() == TYPE_REG_TO_BASE) {
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
            }
#endif
            return;
        }

        if(GetModeDevice() == STANDART || GetModeDevice() == TRACK_ION)    // ����������� �����
        {
            /* ����� ������������ �� ������ ��������� ���� ������ �� ������ �� ��������� iON */
            if(GetTypeConnectFm911() == TYPE_WU_BUTTON) {    //���� ������ ������, �� ����� ������ ��������.
#ifdef FM3
                pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;
#endif
#ifdef FM4
                g_stRam.stSim.eRegStatus = FIND_NET;    //�� ����� ������ GPS, � �������� ������ LBS.
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
#endif
            }
            else {
                g_stRam.stSim.eRegStatus = FIND_NET;    //�� ����� ������ GPS, � �������� ������ LBS.
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
            }
        }
        else {
            if(GetModeProtocol() == FINDME_911 &&
                (GetTypeConnectFm911() == TYPE_WU_BUTTON ||
                    GetTypeConnectFm911() ==
                        TYPE_ERR_CONSRV ||                   // ���� ������� ������ ������� �� ������ ��� ������� ��������� ����� ���
                    GetStatusReset() == WAKE_UP_ACCEL)) {    // ���� ������ ������, �� �� ����� ������ ��������
                g_stRam.stSim.eRegStatus = FIND_NET;
                pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
            }
            else {
                pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;    //����� ������ GPS.
            }
        }
    }
    else {
        g_stRam.stSim.eRegStatus = FIND_NET;
        pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
    }
}

void stepGsmMessageEvent(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmMessageEvent\r\n");
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

    setEventTrue();

    ledStatus(REG_GSM);    //�������� ����������� � GSM ����
    //����� ��������� ���, ����� ����� GSM ����.
    RET_INFO eStatusRet = GetCellularNetwork();
    if(eStatusRet != RET_HOME_NET) {    // ���� ������� ���� ��� GSM ���� �� �������, �� ������.
        DP_GSM("D_GSM NETWORK FAIL(message event)\r\n");
        pGsmStatus->eGsmStep = GSM_OFF;
        return;
    }

    //������ �����
    char strTelUser[SIZE_TEL];
    if(!(GetUserTel(strTelUser))) {
        pGsmStatus->eGsmStep = GSM_SIM;
        return;
    }

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));    //�������� �������, ����� ���� ������������

    if(GetStatusReset() == BUTTON_RESET) {
        if(GetMaskMessageUser() & fBUTTON_MESS_TEL) {    /// SMS
            SendMessage(strTelUser);
        }

        if(GetMaskMessageUser() & fBUTTON_CALL_TEL) {    /// CALL
            //������� ������� �� ���������������� �����
            uint8_t attempt = 2;    //������� �� ������
            while(attempt) {
                if(!(CallEventTel(strTelUser)))
                    break;
                attempt--;
            }
        }
    }

    /* �������� ����� ��� ���� ����� */
    osDelay(SLEEP_MS_1000);
    pGsmStatus->eGsmStep = GSM_SIM;
}

void stepGsmCellularNetwork(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmCellularNetwork\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    ledStatus(REG_GSM);    //�������� ����������� � GSM ����
    RET_INFO eStatusRet = GetCellularNetwork();

    // mc("at+ceng=4", 3, 3);
    // memset(&stGsm, 0, sizeof(stGsm));
    // mc_get(AT_CENG, M_CENG, &stGsm, 3, 4);

    /* ���� �� ����� SCID ������� ��� �����, �� ��������� ��� ��� ��������� */
    char strFIRST_SCID[SIZE_SCID];
    if((!(GetScidFirstSim(strFIRST_SCID))) && ((GetNumSim() == FIRST_SIMCARD_OK))) {
        /* ���� ������ FM3, �� �������� �� ��������� FM911 � �� ������ www.911.fm */
#ifdef FM3
        SetModeProtocol(FINDME_911);
        SetUseTypeServ(SECOND_SERVER);
#endif
        /* ���� ������ FM4, �� �������� �� ��������� iON FM � �� ������ web.irz */
#ifdef FM4
        SetModeProtocol(ION_FM);
        SetUseTypeServ(FIRST_SERVER);
        SetTypeRegBase911(TYPE_WU_START);    //����� ��������� ��� ������� �����, ��� ����������� � �� � �����.
#endif
        if(!(GetScidCurentFirstSim(strFIRST_SCID))) {
            pGsmStatus->eGsmStep = SLEEP_DOWN;
            return;
        }
        DP_GSM("D_SET SCID FIRST SIM CARD: %s\r\n", strFIRST_SCID);
        SetScidFirstSim(strFIRST_SCID);
        SaveConfigCMD();
#ifdef FM3
#if(_FAST_REGISTRATION_TO_BASE_)
        if(GetTypeConnectFm911() == TYPE_REG_TO_BASE) {
            pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
            return;
        }
#endif
        pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;    //����� ������ GPS.
        return;
#endif
    }

    if(eStatusRet == RET_HOME_NET)    // ���� �������� ����
    {
        g_stRam.stSim.eRegStatus = HOME_NET;
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;    // GSM_CHECK_SMS;
        return;
    }

    if(eStatusRet == RET_ROAMING_NET)    // ���� ������� ����
    {
        g_stRam.stSim.eRegStatus = ROAMING_NET;
        // �������� ������ � ��������
        if(GetRoamingGprs()) {                                  //�������� ������ ���������.
            pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;    // GSM_CHECK_SMS;
            return;
        }
    }

    /* ������ ��������� � �������� ���� */
    if(GetModeDevice() == TRACK_ION) {
        pGsmStatus->eGsmStep = GSM_OFF;
    }
    else {
        DP_GSM("D_GSM NETWORK FAIL(celluar network)\r\n");
        pGsmStatus->eGsmStep = GSM_OFF;
    }
}

void stepGsmCheckSMS(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmCheckSMS\r\n");
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

    uint32_t CountReadSms = GetTimeWaitSms();
    char strNameSer[SIZE_SERV_FTP] = { 0 };
    if(GetModeDevice() == TRACK_ION || GetAddrFirmSer(strNameSer)) {
        CountReadSms = 0;
    }    //���� ����� �������, �� �� ����� ��������� ��� �����

    if(GetModeProtocol() == FINDME_911) {
        if(GetTypeConnectFm911() == TYPE_REG_USER || GetTypeConnectFm911() == TYPE_REG_TO_BASE ||
            GetTypeConnectFm911() == TYPE_GOODBYE) {
            CountReadSms = 0;
        }
    }

    CheckSmsCommand();

    waitSMS(CountReadSms);
    pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
}

void stepGsmProfileGprsConnect(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileGprsConnect\r\n");

    RET_INFO eStatusRet = profile_gprs_connect(0);
    ledStatus(SERVER_CONNECT);    //�������� ����������� � �������

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    if(eStatusRet == RET_GPRS_OK) {
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SOCKET;
        return;
    }

    static uint8_t err_socket_init = MAX_COUNT_SIM_FIRST_ERROR;
    if(!(err_socket_init)) {
        err_socket_init = MAX_COUNT_SIM_FIRST_ERROR;
        SetGsmGprsErr(GetGsmGprsErr() + 1);
        pGsmStatus->eGsmStep = GSM_OFF;
        if(!(GetModeDevice() == TRACK_ION)) {
            setUseSecondSim();    // ������������ �� ������ ���.
        }
        return;
    }
    else {
        err_socket_init--;
    }
}

_Bool deb_srv_err_conn = FALSE;

void stepGsmProfileGprsSocket(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileGprsSocket\r\n");

    char strAddrServ[SIZE_SERV] = { '\0' };
    (*ptrSwitchAddrServer[GetUseTypeServ()])(strAddrServ);
    if(deb_srv_err_conn) {
        strcpy(strAddrServ, "911.fm:20001");
    }

    RET_INFO eStatusRet = ProfileSocketInit(strAddrServ, 0);
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
    if(eStatusRet == RET_OK) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACTIVATE;
        // pGsmStatus->eGsmStep = GSM_PROFILE_HTTP_SOCKET;
        return;
    }

    static uint8_t err_socket_init = MAX_COUNT_SIM_FIRST_ERROR;
    if(!(err_socket_init)) {
        err_socket_init = MAX_COUNT_SIM_FIRST_ERROR;
        SetGsmGprsErr(GetGsmGprsErr() + 1);
        pGsmStatus->eGsmStep = GSM_OFF;
        if(!(GetModeDevice() == TRACK_ION)) {
            setUseSecondSim();    // ������������ �� ������ ���.
        }
        return;
    }
    else {
        err_socket_init--;
    }
}

void stepGsmProfileGprsActivate(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileGprsActivate\r\n");

    char strAddrServ[SIZE_SERV] = { '\0' };
    (*ptrSwitchAddrServer[GetUseTypeServ()])(strAddrServ);
    if(deb_srv_err_conn) {
        strcpy(strAddrServ, "911.fm:20001");
    }
#ifndef USE_TEST_DEVICE
    USE_TEST_DEVICE 0
#endif
#if(USE_TEST_DEVICE)
        /* �������� ���������� ����� ������� �� ������������ */
        if(GetCheckTestDevice())
    {
        /* ���� �� �������. �������� �� ������� �������� ������� */
        pGsmStatus->eGsmStep = DEVICE_TEST;
        return;
    }
#endif

    /* ���� ����� ����������� ������������, �� �������� �� �������� ��� � �� ���������� ������� */
    if(GetModeProtocol() == FINDME_911 && GetTypeConnectFm911() == TYPE_REG_USER) {
        pGsmStatus->eGsmStep = GSM_PROFILE_WAIT_REG_USER_911;
        return;
    }

    /* �������� �� ����� �������� */
    char strNameSer[SIZE_SERV_FTP] = { '\0' };
    RET_INFO eStatusRet = RET_GPRS_OK;
    if(GetAddrFirmSer(strNameSer) == 0) {
        GSM_INFO stGsm;
        memset(&stGsm, 0, sizeof(stGsm));
        if(!(getInfoLbsData(&stGsm))) {
            DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");    // ���� LBS
            getActiveLbsInfo(&stGsm, GetLbsFindTimeout());
            if(stGsm.count) {
                setInfoLbsData(&stGsm);    //�������� ��������� ������ LBS
            }
            else {
                DP_GSM("D_GSM NO LBS\r\n");
                pGsmStatus->eGsmStep = GSM_OFF;
            }
        }
        eStatusRet = profile_activate(PROF_FIRST_SERVER, strAddrServ);
    }
    else {
        char strNewFirmware[SIZE_NAME_FIRMWARE] = { '\0' };
        char* ptr = strstr(strNameSer, "=");
        if(ptr) {
            ptr++;
            strcpy(strNewFirmware, ptr);
        }

        char strCurFirmware[SIZE_NAME_FIRMWARE] = { '\0' };
        sprintf(strCurFirmware, "00%d", flash_read_word(__CONST_FIRM_VER));

        if(strstr(strNewFirmware, strCurFirmware) == NULL) {
            //� ��� ����� ��������, �������� �� �� ����������.
            DP_GSM("D_NEW FIRMWARE\r\n");
            pGsmStatus->eGsmStep = GSM_PROFILE_HTTP_SOCKET;
            return;
        }
        else {
            if(GetUseTypeServ() == SECOND_SERVER) {    //���� ������ �������� �� web.irzonline.com
                ResetFirmware();
            }
            else {
                g_stRam.stFirmware.bNewFirmware = TRUE;    //������ ����, ��� ������ ������� �� ���������� ��������.
                SetFlagsStatusFirmware(FIRMWARE_OK);
                SaveConfigCMD();
            }
        }
    }

    ledStatus(NORMAL_RUN);    //����������� � ���� GSM ��������, ���������� ���������� � �������
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    static uint8_t err_socket_init = MAX_GSM_CONNECT_FAIL - 1;

    /* ����� ��������� ������(ION_FM ��� FM911) */
    if(GetModeProtocol() == ION_FM) {
        /* �������� ION FM */
        if(eStatusRet == RET_GPRS_OK) {
            pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_INIT;
            uint16_t ServConnOk, ServConnErr;
            GetServerConnect(&ServConnOk, &ServConnErr);
            ServConnOk++;
            SetServerConnect(ServConnOk, ServConnErr);
        }
        else {
            pGsmStatus->eGsmStep = GSM_OFF;
            if(!(err_socket_init)) {
                SetGsmGprsErr(GetGsmGprsErr() + 1);
                if(!(GetModeDevice() == TRACK_ION)) {
                    setUseSecondSim();    // ������������ �� ������ ���.
                }
                return;
            }
            else {
                err_socket_init--;
            }
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
        }
    }
    else {
        /* �������� FM911 */
        if(eStatusRet == RET_GPRS_OK) {
            pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_911;
            uint16_t ServConnOk, ServConnErr;
            GetServerConnect(&ServConnOk, &ServConnErr);
            ServConnOk++;
            SetServerConnect(ServConnOk, ServConnErr);
        }
        else {
            if(!(err_socket_init)) {
                SetGsmGprsErr(GetGsmGprsErr() + 1);
                pGsmStatus->eGsmStep = GSM_OFF;
                if(!(GetModeDevice() == TRACK_ION)) {
                    setUseSecondSim();    // ������������ �� ������ ���.
                }
                return;
            }
            else {
                err_socket_init--;
            }
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
        }
    }
}

void stepGsmProfileGprsDeactivate(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileGprsDeactivate\r\n");

    RET_INFO eStatusRet = profile_deactivate(PROF_FIRST_SERVER);
    ledStatus(SERVER_CONNECT);    //�������� ����������� � �������
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    static uint8_t ucCntConFailSer = MAX_GSM_CONNECT_FAIL - 1;

    if(GetModeDevice() != TRACK_ION) {    // ���� ����� �����, �� ������ ������ ���������������.
        DP_GSM("D_SERVER FAIL: %i\r\n", ucCntConFailSer);
        if(GetModeProtocol() == FINDME_911) {    // ���� � ��� �������� FM911
            pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACTIVATE;
        }
        else {
            pGsmStatus->eGsmStep = GSM_DEACTIVATE_CHECK_SMS_ACTIVATE;
        }
        if(ucCntConFailSer) {
            ucCntConFailSer--;
        }
        uint16_t ServConnOk, ServConnErr;
        GetServerConnect(&ServConnOk, &ServConnErr);
        if(!(ucCntConFailSer)) {
            ucCntConFailSer = MAX_COUNT_SIM_FIRST_ERROR;
            pGsmStatus->eGsmStep = GSM_OFF;
            if(!(GetModeDevice() == TRACK_ION)) {
                setUseSecondSim();    // ������������ �� ������ ���.
            }
        }
        ServConnErr++;
        SetServerConnect(ServConnOk, ServConnErr);
    }
    else {
        if(eStatusRet == RET_OK) {
            if(g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE) {
                pGsmStatus->eGsmStep = GSM_DEACTIVATE_CHECK_SMS_ACTIVATE;
            }
            else {
                pGsmStatus->eGsmStep = GSM_OFF_LOW_PWR;
            }
        }
        else {
            pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
            pGsmStatus->eGsmStep = GSM_OFF;
        }
    }
}

void stepDeactivateCheckSmsActivate(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepDeactivateCheckSmsActivate\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    GSM_INFO out_check;
    check_csq(&out_check);
    check_creg(&out_check);
    if((out_check.msg[1].var + 0x30) == RET_ROAMING_NET) {
        g_stRam.stSim.eRegStatus = ROAMING_NET;
        // ��������� �������� ������ � ��������
        if(!(GetRoamingGprs())) {
            pGsmStatus->eGsmStep = GSM_OFF;
            return;
        }
    }
    pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACTIVATE;
}

void stepCheckSMS(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepCheckSMS\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    CheckSmsCommand();
    GSM_INFO out_check;
    check_csq(&out_check);
    check_creg(&out_check);
    if((out_check.msg[1].var + 0x30) == RET_ROAMING_NET) {
        g_stRam.stSim.eRegStatus = ROAMING_NET;
        // ��������� �������� ������ � ��������
        if(!(GetRoamingGprs())) {
            pGsmStatus->eGsmStep = GSM_OFF;
            return;
        }
    }

    /* �������� �������� ���������� ������ */
    if(GetModeDevice() == TRACK_ION) {
        pGsmStatus->eGsmStep = WAIT_FLASH_DATA_READY;
    }
    else {
        pGsmStatus->eGsmStep = FIND_ME_DATA_READY;
    }

    /* ���� ������ ����� �� ��������� iON FM, �� ����� ��������� � ���������� ������������ */
    extern char g_aucBufDownHttpFirm[];
#define SERVER_TRUE_VALUE g_aucBufDownHttpFirm
    if(GetModeProtocol() == ION_FM) {
        // memset(SERVER_TRUE_VALUE, USER_CONFIG_DEVICE_PACKET, MAX_PARAM_CONFIG_VALUE);
        /* ���� ���� �� ����������� ������������, �� �������� � */
        for(int i = 0; i < MAX_PARAM_CONFIG_VALUE; i++) {
            if(SERVER_TRUE_VALUE[i]) {
                pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_CONFIG;
                break;
            }
        }

        if(getFlagReset()) {
            pGsmStatus->eGsmStep = RESTART_NOW;
        }
    }
}

/* ���� ������� GPS */
void stepGsmSleepWaitGps(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmSleepWaitGps\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    ledStatus(LOW_PWR1);
    g_stRam.stConnect.bWaitGpsModeFm = TRUE;    // ������, ��� �� ��� ��������, ����� ���������� ������, � �� ������ GPS

    /* ���� ���� ������ ������, �� �� ��������� GSM, � ���������� ���������� ���� ��� ������ ��� */
    _Bool gsm_power_off_true = (_Bool)GetTimeWaitSms();
    if(!(gsm_power_off_true)) {
        GSM_State(SLEEP);
        SimPwr(OFF);
    }

    /* Wait GPS */
    int bResult = FindPositionGps();
    if(bResult < 0 && GetEnableAccelToFind() && GetModeDevice() == TIMER_FIND) {
        //���������� ����� ����������� � �������� � ������ ������ �� �������������.
        SetStatusReset(WAKE_UP_ACCEL);
        g_stRam.stConnect.bWaitGpsModeFm = FALSE;
    }

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    uint8_t count = MC_COUNT;
    while(count) {
        GSM_State(WAKE_UP);    // ����� gsm ������ cfun
        osDelay(100);
        if(mc("AT", 5, 1) == RET_OK)
            break;
        count--;
    }

    if(count) {
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
        pGsmStatus->eGsmStep = GSM_CONFIG;
    }
    else {
        pGsmStatus->eGsmStep = GSM_OFF;
    }
}

/* ���� ��������� �� ����� �������� ���� � ��������� ����� � ������(� ������ ACCEL FIND) */
void stepGsmSleepWaitAccelStop(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmSleepWaitAccelStop\r\n");
    portTickType xLastWakeTimerDelay;
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    g_stRam.stConnect.bWaitGpsModeFm = TRUE;    // ������, ��� �� ��� ��������, ����� ���������� ������, � �� ������ GPS

    GSM_State(SLEEP);    //�������� ����� � ���
    SimPwr(OFF);

    /* Wait accel stop */
    DP_GSM("D_WAIT STOP ACCEL, PLEASE WAIT\r\n");
    while(AccelState() == ACC_STATE_MOVE) {    //�������� � �����, ���� ��������� �� �������������.
        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
        DP_GSM("\r\n-D_ACCEL STOP WAIT-\r\n");
    }

    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));

    /* WAIT GPS */
    int bResult = FindPositionGps();
    if(bResult >= 0) {
        SetStatusReset(WAKE_UP_STOP);
    }

    set_fm_step_count(DATA_FM - 1);    //��������� ��� �������� ������ �� �������� ������� � ���������.

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    GSM_State(WAKE_UP);    //���� ������!
    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
    pGsmStatus->eGsmStep = GSM_SETTING_START;
}

/* UPDATE FIRMWARE ************************************************************/
/* ���� ��������� HTTP ������� */
void stepGsmProfileHttpSocket(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileHttpSocket\r\n");

    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
    ledStatus(LOW_PWR1);
    profile_deactivate(PROF_HTTP_GET_SERVER);    // Close GPRS Connect

    static uint8_t CountErrLoadFirm = 0;
//��������� ������������ ��������
#define MAX_COUNT_DOWNLOAD_FIRMWARE 3
    if(CountErrLoadFirm >= MAX_COUNT_DOWNLOAD_FIRMWARE) {
        CountErrLoadFirm = 0;
        ResetFirmware();
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
        return;
    }
    CountErrLoadFirm++;

    RET_INFO eStatusRet = ProfileSocketInit("", PROF_HTTP_GET_SERVER);
    if(eStatusRet == RET_OK) {
        pGsmStatus->uiGsmStepDelay = SLEEP_MS_500;
        pGsmStatus->eGsmStep = GSM_PROFILE_HTTP_DOWNLOAD;
    }
    else {
        g_stRam.stFirmware.bNewFirmware =
            TRUE;    //������ ����, ��� ������ ������� �� ���������� ��������, �� ������� �� ����������.
        SetFlagsStatusFirmware(ERR_CONNECT_FTP_OR_HTTP);
        SaveConfigCMD();
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
    }
}

/* �������� �� HTTP ������ */
/* ������ �������� � HTTP ������� */
void stepGsmProfileHttpDownload(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileHttpDownload\r\n");

    FRAME_FIRMWARE_TYPE eFirmUpdataStatus;

    // SetAddrFirmSer("web.irzonline.com/f.php?f=1151250");

    g_stRam.stFirmware.bNewFirmware = TRUE;                                              //������ ����, ��� ������ ������� �� ���������� ��������.
    eFirmUpdataStatus = (FRAME_FIRMWARE_TYPE)profile_http_read(PROF_HTTP_GET_SERVER);    // Download Firmware
    profile_deactivate(PROF_HTTP_GET_SERVER);                                            // Close GPRS Connect
    if(eFirmUpdataStatus == FIRMWARE_OK) {
        pGsmStatus->eGsmStep = GSM_PROFILE_CHECK_FIRMWARE;
    }
    else {
        SetFlagsStatusFirmware(eFirmUpdataStatus);    //������ Err ��������� ��������.
        /*switch(eFirmUpdataStatus) {
        case ERR_FIRMWARE_BAD_REQ_400:
        case ERR_FIRMWARE_UNAUTHORIZED_401:
        case ERR_FIRMWARE_NOT_FOUND_404:
        default:
      }*/
        ResetFirmware();    // ����� ������ �� ����� �������� ��� ����� ������
        SaveConfigCMD();
        pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
    }
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
}

/* �������� CRC ��������� �������� */
void stepGsmProfileCheckFirmware(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmProfileCheckFirmware\r\n");
    static FRAME_FIRMWARE_TYPE eBackErrFirm = FIRMWARE_OK;
    FRAME_FIRMWARE_TYPE eFlagsStatusDownloadFirmware = check_firmware();
    SetFlagsStatusFirmware(eFlagsStatusDownloadFirmware);
    SaveConfigCMD();
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;

    if(eFlagsStatusDownloadFirmware == FIRMWARE_OK) {
        resetConnectError();
        ResetFirmware();
        if(GetUseTypeServ() == SECOND_SERVER) {    //���� ������ �������� �� web.irzonline.com, �� ����� ��������������
            // SendSmsStatusLoadFirmware(FIRMWARE_OK);
            pGsmStatus->eGsmStep = RESTART_NOW;
            return;
        }
    }
    else {
        if(eFlagsStatusDownloadFirmware != eBackErrFirm) {
            eBackErrFirm = eFlagsStatusDownloadFirmware;
            if(GetUseTypeServ() == SECOND_SERVER) {
                // SendSmsStatusLoadFirmware(eFlagsStatusDownloadFirmware);
            }
        }
    }
    pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
}
/******************************************************************************/

GSM_STEP GetGsmStep(void)
{
    return stGsmStatus.eGsmStep;
}

//������� �������� GSM ������ � ������������1. ���������� ����� �������� GSM ������.
uint32_t GsmLowPwr1(void)
{
    uint32_t delay_gsm_off = GetTimeLowPwrMode1() * 60;

    DP_GSM("D_PWR LOW STATUS: PWR1\r\n");

    DP_GSM("D_GSM OFF DELAY: %isec.\r\n", delay_gsm_off);

    ledStatus(LOW_PWR1);    //������ ��� ������ � ������� ������ LOW_PWR1
    SetStatusReset(WAKE_UP_LOW_PWR1);
    SetStatusDeviceReset(GetStatusReset());
    return delay_gsm_off;
}

//������� �������� GSM ������ � ������������2. ���������� ����� �������� GSM ������.
uint32_t GsmLowPwr2(void)
{
    uint32_t delay_gsm_off = GetTimeLowPwrMode2() * 60;

    DP_GSM("D_PWR LOW STATUS: PWR2\r\n");
    DP_GSM("D_GSM OFF DELAY: %isec.\r\n", delay_gsm_off);
    ledStatus(LOW_PWR2);    //������ ��� ������ � ������� ������ LOW_PWR2
    SetStatusReset(WAKE_UP_LOW_PWR2);
    SetStatusDeviceReset(GetStatusReset());
    return delay_gsm_off;
}

void setGsmStep(GSM_STEP eCmdGsmStep)
{
    if(eCmdGsmStep == GSM_OFF) {
        resetConnectError();
    }
    if(eCmdGsmStep == GSM_PROFILE_GPRS_CONNECT && GetModeProtocol() == ION_FM) {
        resetFmStepCount();
    }

    stGsmStatus.eCmdGsmStep = eCmdGsmStep;
}

_Bool checkSMS = FALSE;
;
void stepConnectError(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepConnectError\r\n");
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;

    if(GetModeProtocol() == ION_FM) {
        xSemaphoreGive(mFLASH_SAVE_DATA_FM);    //�������� ������� ���������� ������
    }

    if(GetTypeRegBase911() != TYPE_REG_USER && GetTypeRegBase911() != TYPE_REG_TO_BASE) {
        //���� GSM ������ �������, � ���� ������ ������, �� �������� ���.
        if(GSM_STATUS_ON && GetStatusReset() == BUTTON_RESET) {
            //������ ���� ��� ��� ��������� ���.
            checkSMS = TRUE;
            waitSMS(DEF_TIME_WAIT_SMS);
        }
        /* ��������� ���������� ���������� ��� ������������ ������ �� ������ */
        saveOldPositionGps();
        SetTypeRegBase911(TYPE_ERR_CONSRV);    // �� ������� ����� �� ������� ��� �������� � GPRS
        SaveConfigCMD();
    }

    if(GetTypeRegBase911() != TYPE_REG_TO_BASE) {
        SetCountReConnect(GetCountReConnect() + 1);
        DP_GSM("D_COUNT RECONNECT: %i\r\n", GetCountReConnect());
        DP_GSM("D_CONNECT FAIL\r\n");
    }

#ifdef FM3
    if(GetModeProtocol() == ION_FM) {
        DP_GSM("D_SAVE DATA FLASH: ");
        /* �������� �������� ��� ������ ��������� */
        if(osMutexWait(mFLASH_COMPLETE, 5000) == osOK) {
            DP_GSM("OK\r\n");
        }
        else {
            DP_GSM("FAIL\r\n");
        }
    }
    FlashInit();
#endif

    pGsmStatus->eGsmStep = SLEEP_DOWN;
}

void stepGsmEndSms(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepGsmEndSms\r\n");
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

    uint32_t CountReadSms = GetTimeWaitSms();    // �������� ������� ���. ����� ����� ���.

    // ���� ������ ����������� ��� ����� �������� �� ����� ������, �� ���������� �����
    if(GetTypeConnectFm911() == TYPE_REG_USER || GetTypeConnectFm911() == TYPE_REG_TO_BASE ||
        GetTypeConnectFm911() == TYPE_GOODBYE || SelectNumSim(GET_NUM_SIM) == CURRENT_SECOND_SIMCARD) {
        CountReadSms = 0;
    }

    CheckSmsCommand();
    waitSMS(CountReadSms);
    pGsmStatus->eGsmStep = SLEEP_DOWN;
    if(getFlagReset()) {
        pGsmStatus->eGsmStep = RESTART_NOW;
    }
}

void stepRestartNow(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepRestartNow\r\n");
    DPS("\r\n-D_DEVICE RESTART NOW-\r\n");
    osDelay(100);
    RebootDevice();
}

void stepDeviceTest(TGsmStatus* pGsmStatus)
{
    DP_GSM("D_GSM step: stepDeviceTest\r\n");

#if(USE_TEST_DEVICE)
    static uint8_t CountErrSendDataTestDevice = 0;
    pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
    ledStatus(LOAD_DEVICE);
    _Bool flag_test_dev_ok = FALSE;
    if(!(testDevice(g_aucOutDataFrameBuffer))) {
        flag_test_dev_ok = TRUE;    // Set flag test device OK:)
    }
    /* Send data test in server */
    if(sendDataTestDevice(PROF_HTTP_GET_SERVER, g_aucOutDataFrameBuffer)) {
        CountErrSendDataTestDevice++;
        pGsmStatus->eGsmStep = GSM_OFF;    // Send Data Error
    }
    else {
        if(flag_test_dev_ok) {
            SetCheckTestDevice();    // Test Device Ok
        }
        pGsmStatus->eGsmStep = SLEEP_DOWN;    // Send Data Ok
    }

#define MAX_COUNT_SEND_DATA_TEST_DEVICE 2
    if(CountErrSendDataTestDevice > MAX_COUNT_SEND_DATA_TEST_DEVICE) {
        pGsmStatus->eGsmStep = SLEEP_DOWN;
    }
#else
    pGsmStatus->eGsmStep = SLEEP_DOWN;    // Send Data Ok
    return;
#endif
}

void waitSMS(uint32_t CountReadSms)
{
    uint32_t wait_sms = time() + CountReadSms;
    while(wait_sms > time()) {
        DP_GSM("D_WAIT SMS: %d\r\n", (wait_sms - time()));
        SystemUpdateMonitor();    //��������� ���������� ��������� FreeRTOS
        CheckSmsCommand();
        if(stGsmStatus.eCmdGsmStep != GSM_START) {
            break;    // ���� �������� ������ ��� ���������� ���������
        }
        if(getFlagReset()) {
            stGsmStatus.eCmdGsmStep = RESTART_NOW;
            break;
        }
        CountReadSms--;
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
    }
    SetTimeWaitSms(0);
}