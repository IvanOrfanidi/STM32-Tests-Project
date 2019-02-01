
#include "includes.h"
#include "protocol_general.h"

// char InpDataBuffer[SIZE_IN_DATA_BUF];

int LenDataGprs = 0;

RET_INFO GprsWaitAcknow(uint8_t nProf)
{
    uint8_t i = 1;    // g_stEepConfig.stGsm.uc_gprs_receive_count;
    RET_INFO ret;
    while(i) {
        ret = wait_rx_ready(nProf, GetGsmGprsTimeout());    //!
        if(ret == RET_GPRS_RECEIVE_OK) {                    //Если данные пришли выходим с OK.
            return RET_GPRS_RECEIVE_OK;
        }
        i--;
    }

    return ERR_SERVER_CONNECT;
}

RET_INFO GprsSendFailC(void)
{
    //На ошибку отправляем пустой пакет с C_FAIL
    uint8_t LenDataPacket = 0;
    uint8_t mask_ready = 0;
    LenDataPacket = FrameGeneralBuildAckC(C_FAIL, "", 0, g_aucOutDataFrameBuffer);
    if(socket_send(0, g_aucOutDataFrameBuffer, LenDataPacket, &mask_ready) == RET_GPRS_SEND_OK) {
        return RET_OK;
    }

    return ERR_GPRS_SEND;
}

RET_INFO GprsSendAckC(void)
{
    uint8_t LenDataPacket = 0;
    uint8_t mask_ready = 0;

    g_stFrame.ucType = C_ACK;
    g_stFrame.ulSerialNumber = GetIMEI();
    g_stFrame.usDevicePaketNumber = g_stFrame.usServerDataPaketNumber;

    LenDataPacket = FrameGeneralBuild(&g_stFrame, "", 0, g_aucOutDataFrameBuffer);
    if(socket_send(0, g_aucOutDataFrameBuffer, LenDataPacket, &mask_ready) == RET_GPRS_SEND_OK) {
        return RET_OK;
    }

    return ERR_GPRS_SEND;
}

RET_INFO GprsSendAnsOkData(void)
{
    int Len;
    uint8_t mask_ready = 0;
    Len = frame_build_ans_cmd_paket(InpDataBuffer, 0, g_asInpDataFrameBuffer);
    memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
    g_stFrame.ucType = C_DATA;
    g_stFrame.ulSerialNumber = GetIMEI();
    g_stFrame.usDevicePaketNumber++;

    LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, Len, g_aucOutDataFrameBuffer);

    if(socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK) {
        DP_GSM("D_DEV ANS: <<<");
        for(uint8_t i = 2; i < Len; i++) {
            DPC(InpDataBuffer[i]);
        }
        DP_GSM(">>>\r\n");
        return RET_OK;
    }
    memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
    GprsAckData();

    DP_GSM("D_SER ANS: OK\r\n");
    return RET_OK;
}

RET_INFO GprsSendAnsErrData(void)
{
    uint8_t mask_ready = 0;
    strcpy(g_aucOutDataFrameBuffer, g_asInpDataFrameBuffer);
    LenDataGprs = strlen(g_aucOutDataFrameBuffer);
    if(socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK) {
        return RET_OK;
    }

    return ERR_GPRS_SEND;
}

RET_INFO GprsSendDataAccelStatus(void)
{
    uint8_t mask_ready = 0;
    LenDataGprs = 0;

    DP_GSM("D_Packet ACCEL\r\n");

    if(GetGpsStatus()) {
        LenDataGprs += GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET, LenDataGprs);
    }
    else {
        //Отправим навигационный пакет GPS с невалидными координатами.
        LenDataGprs += frame_build_navigational_not_valid_packet(InpDataBuffer, LenDataGprs);
    }

    g_stFrame.ucType = C_DATA;
    g_stFrame.ulSerialNumber = GetIMEI();
    g_stFrame.usDevicePaketNumber++;
    LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, LenDataGprs, g_aucOutDataFrameBuffer);

    if(socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK) {
        return RET_OK;
    }
    return ERR_GPRS_SEND;
}

GSM_STEP GprsAckData(void)
{
    int ret;
    int tx_size_ack_data = 0;
    int count = GetAnswerTimeout() / 10;

    while(!(tx_size_ack_data > 0)) {
        //Данные пришли, прочитаем их.
        tx_size_ack_data = socket_read(PROF_FIRST_SERVER, g_aucOutDataFrameBuffer, SIZE_OUT_DATA_BUF);
        count--;
        if(!(count)) {
            return GSM_PROFILE_GPRS_DEACTIVATE;
        }
    }
#if OUT_DEBUG_DATA_SER == 1
    DP_GSM("D_GPRS RX<- ");
    for(int i = 0; i < tx_size_ack_data; i++) {
        DP_GSM("%02X ", g_aucOutDataFrameBuffer[i]);
    }
    DP_GSM("\r\n");
#endif

    //Разбираем ответ.
    ret = ack_data_parser(g_aucOutDataFrameBuffer, tx_size_ack_data);

    if((ret == S_FAIL) || (ret == S_FIN)) {
        if(ret == S_FAIL) {
            DP_GSM("D_SERVER ERR: S_FAIL\r\n");
            return GSM_PROFILE_GPRS_DEACTIVATE;    //Сделаем переконнект сразу
        }
        if(ret == S_FIN) {
            DP_GSM("D_SERVER ERR: S_FIN\r\n");
            return GSM_PROFILE_GPRS_DEACTIVATE;    //Сделаем переконнект на рандомное время
        }
    }
    DPS("D_SERVER: S_ACK\r\n");
    return GSM_SWITCH_DATA;
}

GSM_STEP parsingData(uint8_t* ptr)
{
    GSM_STEP (*pCallbackParsing)
    (uint8_t * arg);

    DPS("D_SRV: ");
    switch(ptr[0])    //Находим команду переданную сервером
    {
        case CONFIG_FIRMWARE_PACKET:    // Получение ссылки новой прошивки.
            DPS("CONFIG_FIRMWARE_PACKET\r\n");
            pCallbackParsing = configFirmware;
            break;

        case CHANGE_FIRMWARE_PACKET:    // Применение прошивки.
            DPS("CHANGE_FIRMWARE_PACKET\r\n");
            pCallbackParsing = changeFirmware;
            break;

        case SYNCHRO_TIME_SERVER_PACKET:    // Синхронизация времени.
            DPS("SYNCHRO_TIME_SERVER_PACKET\r\n");
            pCallbackParsing = configTime;
            break;

        case CONFIG_DEVICE_PACKET:    // Конфигурация девайса сервером.
            DPS("CONFIG_DEVICE_PACKET\r\n");
            pCallbackParsing = configDevice;
            break;

        case COMMAND_SERVER_TO_DEVICE_PACKET:    // Конфигурация по одной команде девайса сервером.
            DPS("COMMAND_SERVER_TO_DEVICE_PACKET\r\n");
            pCallbackParsing = configCmdDevice;
            break;

        default:    // Команда не передназначина для этого девайса
            DP_GSM("ERROR COMMAND: %02X\r\n", ptr[0]);
            return CHECK_SMS;
    }
    return pCallbackParsing(ptr);
}