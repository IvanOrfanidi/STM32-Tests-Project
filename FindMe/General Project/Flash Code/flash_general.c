

#include "includes.h"
#include "flash_general.h"
#include "flash_archive.h"
//#define FULL_FLASH_ERASE
//#define ARCHIVE_FLASH_ERASE

char flash_common_buf[SIZE_FLASH_BUF];    //����� ������ ��� ������ ������

void ReloadFlashData(uint32_t);
extern int GetDataPeripheryPacket(TPortInputCFG*, char*, int);

void vFlashTask(void* pvParameters)
{
    /* ������������� ������������� ������ ���� ����� updateStatusReset() ��� ��� ����� ������� ����������� ��
     ������������� � ������ ���� � �������� RTOS ��� ������������ ��������� ������������� */
    accelInit(GetAccelSensitivity());

#ifdef FULL_FLASH_ERASE
    FillFlashErase();
    if(osKernelRunning()) {    //��������� �������� �� RTOS.
        vTaskSuspendAll();
    }
    while(1) {
        IWDG_ReloadCounter();
    }
#endif    // FULL_FLASH_ERASE

#ifdef ARCHIVE_FLASH_ERASE
    ArchiveErase();
#endif    // ARCHIVE_FLASH_ERASE

    FlashInit();
    if(GetCountDataFlash() && GetModeProtocol() == FINDME_911) {
        DP_GSM("D_ARCHIVE FM ERASE\r\n");
        EraseArcive();
    }

    int len_data = 0;

    while(1) {
        len_data = 0;
#ifdef FM4
        if(g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE) {
            // len_data = 0;
            // ������ �� GPS
            //�������� ����� ������ GPS ������ ��� ���//
            if((GetGpsStatus()) && (GpsFilterData())) {
                len_data += GetDataNavigationalGpsPacket(flash_common_buf, NAVIGATIONAL_PACKET, len_data);
            }

            // ������ �� GPIO
            if((GetModeDevice() == TRACK_ION) && (GpioFilterData())) {
                g_stInput.SecADC = time();
                extern int GetDataPeripheryPacket(TPortInputCFG * pAds, char* pOut, int OffsetData);
                len_data += GetDataPeripheryPacket(&g_stInput, flash_common_buf, len_data);
            }

            // ������ �� ������� ��������.
#ifdef _DRIVE_SIMPLE_
            if(DriverFilterData()) {
                len_data += DS_Write_Violation((u8*)flash_common_buf + len_data);
            }
#endif    //_DRIVE_SIMPLE_

            if((GpsRealTimeFilterData()) && (GetModeDevice() == TRACK_ION)) {
                g_stRam.stFlash.bRealtimeReady = TRUE;
            }
        }
        else {
            clearTimeFilterGps();
        }

        uint32_t ComparLenDataFlashReady;    //������ ������ ��� ��������� ����� ���������� Flash.
                                             //������������ � ����������� �� ������ ����������������.
        ComparLenDataFlashReady = SIZE_FLASH_BUF - 32;
        if(GetModeDevice() == TRACK_ION) {
            // ComparLenDataFlashReady = GetLenDataFlashReady();
        }
        else {
            ComparLenDataFlashReady = SIZE_RECORD_EXT_FLASH / 2;    // 256
        }

        g_stRam.stFlash.uiFlashDataLen = Flash_DataLen();
        if((g_stRam.stFlash.uiFlashDataLen >= ComparLenDataFlashReady) &&
            (g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE) && (g_stRam.stDevice.eCurPwrStat != POWER_SLEEP_MODE)) {
            g_stRam.stFlash.bFlashReady = TRUE;    // ������ ���� ��� ����� ������ � ��������.
        }
        else {
            g_stRam.stFlash.bFlashReady = FALSE;
        }

        if(len_data) {
            while(Flash_QueryPacket((u8*)flash_common_buf, (u16)len_data) != OK)
                ;

            memset(flash_common_buf, 0, sizeof(flash_common_buf));
            len_data = 0;
        }
        FlashHandler();
        ReloadFlashData(ComparLenDataFlashReady);    // ���������� �������� ������
#endif
#ifdef FM3
        /* ���� �������� ���, �� ����� ��������� ������ � ����������� � ��������� ������ */
        if(GetModeProtocol() == ION_FM) {
            if(osMutexWait(mFLASH_SAVE_DATA_FM, 0) == osOK) {    // ���� ������� �� ���������� ������
                xSemaphoreTake(mFLASH_SAVE_DATA_FM, 10);
                GPS_INFO stGpsSaveData;
                GSM_INFO stGsmSaveData;
                /* ���� ���� �������� */
                if(GetPositionGps(&stGpsSaveData)) {
                    DP_GSM("D_SAVE GPS DATA\r\n");
                    len_data = frame_build_navigational_packet_realtime(&stGpsSaveData, flash_common_buf);
                }
                /*********************/

                /* ���� ���� ������ GSM ������� */
                if(getInfoLbsData(&stGsmSaveData)) {
                    DP_GSM("D_SAVE GSM DATA\r\n");
                    len_data += frame_build_lbs_packet(&stGsmSaveData, flash_common_buf, len_data);
                }
                /********************/

                /* �������� ����� ��������� */
                // DP_GSM("D_SAVE PRPH DATA\r\n");
                // len_data += GetDataPeripheryPacket(&g_stInput, flash_common_buf, len_data);
                /********************/
                if(len_data) {    //���� ���� GSM ��� GPS, �� �������� ������ �� ������.
                    uint8_t len_data_fls = GetCountDataFlash();
                    /* Save Data */
                    SaveDataFm(len_data_fls, flash_common_buf, len_data);
                }

                xSemaphoreGive(mFLASH_COMPLETE);    //�������� ������� ��� ������ ���������.
            }
        }
#endif

        AccelHandler();

        portTickType xLastWakeTimerDelay;
        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_100 / portTICK_RATE_MS));
    }
}

typedef enum {
    STEP_RELOAD_FIX_ACCEL_MOVE = 0,
    STEP_RELOAD_WAIT_ACCEL_STOP = 1,
    STEP_RELOAD_WAIT_TIME_STOP = 2,
    STEP_RELOAD_SEND_DATA = 3,
} E_STEP_RELOAD;

/* ���������� �������� ������ */
void ReloadFlashData(uint32_t ComparLenDataFlashReady)
{
    static E_STEP_RELOAD eStepReloadDataFlash = STEP_RELOAD_FIX_ACCEL_MOVE;
    u32 uiFlashDataLen = Flash_DataLen();

    /* ���� ������ ����� ����� �������, �� ������� ��������� ����� flash */
    if((eStepReloadDataFlash == STEP_RELOAD_FIX_ACCEL_MOVE) && (AccelState() == ACC_STATE_MOVE) &&
        (uiFlashDataLen < ComparLenDataFlashReady)) {    //���� ������ ��� � ��������.
        eStepReloadDataFlash = STEP_RELOAD_WAIT_ACCEL_STOP;
    }
    if(eStepReloadDataFlash == STEP_RELOAD_WAIT_ACCEL_STOP && AccelState() == ACC_STATE_STOP) {    //������ ���������
        eStepReloadDataFlash = STEP_RELOAD_WAIT_TIME_STOP;
    }

#define MIN_STOP_TIME 60
    if(eStepReloadDataFlash == STEP_RELOAD_WAIT_TIME_STOP && SecStateStop() > MIN_STOP_TIME) {
        eStepReloadDataFlash = STEP_RELOAD_SEND_DATA;
    }
    else {
        if(eStepReloadDataFlash == STEP_RELOAD_WAIT_TIME_STOP && AccelState() == ACC_STATE_MOVE) {
            eStepReloadDataFlash = STEP_RELOAD_FIX_ACCEL_MOVE;
        }
    }

    if(eStepReloadDataFlash == STEP_RELOAD_SEND_DATA && uiFlashDataLen >= LEN_NAVIGATIONAL_PACKET_REAL_TIME &&
        g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE) {
        g_stRam.stFlash.bTempBufReady = TRUE;    // Set flag reload data
        eStepReloadDataFlash = STEP_RELOAD_FIX_ACCEL_MOVE;
    }
    else {
        if(eStepReloadDataFlash == STEP_RELOAD_SEND_DATA)
            eStepReloadDataFlash = STEP_RELOAD_FIX_ACCEL_MOVE;
    }
    /***************************************************/
}

int GetFlashDataToSend(char* pOutDataFrameBuffer, uint32_t SizeOutBuf)
{
    s16 len_flash_data;
    len_flash_data = Flash_ReadData((u8*)&pOutDataFrameBuffer[14], SizeOutBuf - 16);
    if(len_flash_data < 0) {
        return 0;
    }

    void PacDbgInfo(uint8_t * pFramBuf, uint32_t uiSum);
    if(getDebug()) {
        PacDbgInfo((uint8_t*)pOutDataFrameBuffer, len_flash_data);
    }

    g_stFrame.ucType = C_DATA;
    g_stFrame.ulSerialNumber = GetIMEI();
    g_stFrame.usDevicePaketNumber++;
    g_stFrame.usDeviceDataPaketNumber = g_stFrame.usDevicePaketNumber;
    return FrameGeneralBuild(&g_stFrame, &pOutDataFrameBuffer[14], (uint16_t)len_flash_data, &pOutDataFrameBuffer[0]);
}

int GetBufDataToSend(char* pOutDataFrameBuffer)
{
    u32 len_buf_data = (u32)GetLenDataBuf();
    u32 len_flash_data = Flash_DataLen();
    DP_GSM("SIZE DATA TEMP BUF AND FLASH BUF: (%i), (%i)\r\n", len_buf_data, len_flash_data);
    ReloadDataBuf(&pOutDataFrameBuffer[14], len_buf_data);
    Flash_ReadData((u8*)&pOutDataFrameBuffer[14 + len_buf_data], len_flash_data);

    if(getDebug()) {
        void PacDbgInfo(uint8_t * pFramBuf, uint32_t uiSum);
        PacDbgInfo((uint8_t*)pOutDataFrameBuffer, (len_buf_data + len_flash_data));
    }
    g_stFrame.ucType = C_DATA;
    g_stFrame.ulSerialNumber = GetIMEI();
    g_stFrame.usDevicePaketNumber++;
    g_stFrame.usDeviceDataPaketNumber = g_stFrame.usDevicePaketNumber;

    return FrameGeneralBuild(
        &g_stFrame, &pOutDataFrameBuffer[14], (uint16_t)(len_buf_data + len_flash_data), &pOutDataFrameBuffer[0]);
}

void PacDbgInfo(uint8_t* pFramBuf, uint32_t uiSum)
{
    uint32_t temp;
    uint8_t bitFree = 8;
    RTC_t date_pac;
    for(u32 i = 14; i < uiSum;) {
        switch(pFramBuf[i]) {
            case PERIPHERY_PACKET:
                DP_GSM("D_GPIO ");
                bit_unpacking(&pFramBuf[i + 2], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += pFramBuf[i + 1] + 2;
                break;

            case NAVIGATIONAL_PACKET:
                DP_GSM("D_GPS ");
                bit_unpacking(&pFramBuf[i + 1], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += LEN_NAVIGATIONAL_PACKET_REAL_TIME;
                break;

            case DT_DRIVE_STYLE:
                DP_GSM("D_DRIVE STYLE ");
                bit_unpacking(&pFramBuf[i + 2], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += pFramBuf[i + 1] + 2;
                break;

            case LOG_DEVICE_PACKET:
                DP_GSM("D_LOG DEV");
                bit_unpacking(&pFramBuf[i + 2], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += pFramBuf[i + 1] + 2;
                break;

            case WIALON_PACKET:
                DP_GSM("D_WIALON ");
                bit_unpacking(&pFramBuf[i + 3], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += pFramBuf[i + 1] + 2;
                break;

            case NAVIGATIONAL_GSM_PACKET:
                DP_GSM("D_GSM ");
                bit_unpacking(&pFramBuf[i + 4], &temp, &bitFree, 32);
                Sec2Date(&date_pac, temp);
                DP_GSM("%02d/", date_pac.mday);
                DP_GSM("%02d/", date_pac.month);
                DP_GSM("%02d ", date_pac.year);
                DP_GSM("%02d:", date_pac.hour);
                DP_GSM("%02d:", date_pac.min);
                DP_GSM("%02d\r\n", date_pac.sec);
                i += pFramBuf[i + 1] + 3;
                uiSum = i - 14;
                break;

            default:    //������� �� �����.
                DP_GSM("D_ERR DATA %i\r\n", pFramBuf[i]);
                uiSum = i - 14;
                i++;
                break;
        }
    }
}

void EraseFirmwareFlash(void)
{
    uint32_t StartAddressExtFlash = ADDR_EXT_FLASH_NEW_FIRMWARE;

    //�������� ���������� ������� flash ��� ������ ��������.
    FLASH_Take_Semaphore();
    for(uint32_t i = StartAddressExtFlash; i < (StartAddressExtFlash + TOTAL_SIZE_FIRMWARE); i += SIZE_SUBSECTOR_FLASH) {
        IWDG_ReloadCounter();    // Reload IWDG counter
        // DP_GSM("D_ERASE SUBSECTOR EXT FLASH ADDRESS: 0x%.X\r\n", i);
        FlashSubSectorEarse(i);
        isEndWaitPeriod(100);
    }
    FLASH_Give_Semaphore();
}