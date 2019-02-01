

#include "includes.h"
#include "gps_general.h"
#include "UART.h"

//#define _VIRTUAL_GPS_
static int gps_init();
static void configNmeaMessage();
static void configBaudrateGps(uint32_t);
static void calculateGpsCrc(char*);
static void configGpsUart(uint32_t);
static int autoSetBaudrateGps();

/*    Примеры настроечных команд GPS
const char sim_msg_disable[] = "$PMTK314,"; //$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*34\r\n
const char set_nmea_baudrate[] = "$PMTK251,";
      Примеры выходных телеграмм GPS протокола NMEA
const char msg_nmea_rmc[] = "$GNRMC,151228.000,A,6003.3102,N,03018.4906,E,0.00,48.74,051216,,,A";
const char msg_rmc[] = "$GNRMC,151150.000,V,,,,,0.00,47.35,051216,,,N";
const char msg_nmea_gga[] = "$GNGGA,151228.000,6003.3102,N,03018.4906,E,1,4,3.32,132.1,M,18.0,M,,";
const char msg_nmea_accuracy[] = "$GPACCURACY,72.8";
*/

_Bool fLBS_REAL_TIME_DATA_READY = FALSE;
char DataNavigationFrameBuffer[LEN_NAVIGATIONAL_PACKET_REAL_TIME];

uint32_t GpsTimeBack = 0;
uint32_t GpsRealTimeBack = 0;
uint16_t Time_GPS_Activ = 0;

GPS_INFO g_stGpsData;

typedef __packed struct
{
    uint32_t latitude;     // широта
    uint32_t longitude;    // долгота
    _Bool spin;            //направление курса.
    float course;
} T_OLD_GPS_INFO;

T_OLD_GPS_INFO stOld;

char g_aucGpsBuffOut[100];
static void gps_deinit();
static void SetPositionGps(const GPS_INFO*);

void vGpsHandler(void* pvParameters)
{
    uint32_t uiTimeGpsReboot = 0;
    TYPE_MODE_DEV mode_device = STANDART;    //предыдущий режим девайса, для мониора вкл GPS
    osDelay(5000);                           //Обыкновенная задержка на процесс gps handler

    while(1) {
        if(GetModeDevice() == TRACK_ION && mode_device == STANDART) {    //Включение и инициализация GPS модуля при смене режима
            xSemaphoreGive(mINIT_GPS_MODULE);
            mode_device = TRACK_ION;
        }

        if(uiTimeGpsReboot > TIME_NO_VALID_GPS) {
            xSemaphoreGive(mINIT_GPS_MODULE);
        }

        if(osMutexWait(mINIT_GPS_MODULE, 0) == osOK) {
            xSemaphoreTake(mINIT_GPS_MODULE, 0);
            uiTimeGpsReboot = 0;
            if(gps_init()) {
                xSemaphoreGive(mINIT_GPS_MODULE);    // неудалось инициализировать модуль gps
            }
        }

        if(GetModeDevice() != TRACK_ION && mode_device == TRACK_ION) {    //Выключение GPS модуля при смене режима
            xSemaphoreGive(mDEINIT_GPS_MODULE);
            mode_device = STANDART;
        }

        if(osMutexWait(mDEINIT_GPS_MODULE, 0) == osOK) {
            gps_deinit();
            memset(&g_stGpsData, 0, sizeof(g_stGpsData));
            xSemaphoreTake(mDEINIT_GPS_MODULE, 0);
        }

        if(osMutexWait(mGPS_DATA_ARRIVAL, 100) == osOK) {
            xSemaphoreTake(mGPS_DATA_ARRIVAL, 10);

            gps_parser(&g_stGpsData, g_aucGpsBuffOut, strlen(g_aucGpsBuffOut));

            GPS_DPD(g_aucGpsBuffOut, strlen(g_aucGpsBuffOut));
            memset(g_aucGpsBuffOut, 0, sizeof(g_aucGpsBuffOut));

            __disable_interrupt();
            if(GPS_VALID) {
                SetPositionGps(&g_stGpsData);
                __enable_interrupt();
                TimeSynchronizationRTC(g_stGpsData.time);
                if(stOld.course == 0) {
                    stOld.course = g_stGpsData.course + 360;
                }
                Time_GPS_Activ = 0;    //Не требуется синхронизация времени с сервером.
                uiTimeGpsReboot = 0;
            }
            else {
                __enable_interrupt();
                uiTimeGpsReboot++;
                Time_GPS_Activ++;
            }
        }
    }
}

_Bool GpsRealTimeFilterData(void)
{
    if(!(GetGpsRealtime()))
        return 0;

    if(time() >= GpsRealTimeBack) {
        return 1;
    }
    return 0;
}

void ReloadGpsRealTime(void)
{
    GpsRealTimeBack = time() + (uint32_t)GetGpsRealtime();
}

static float getSpeedKmGps(void)
{
    return g_stGpsData.speed * 1.852;
}

void clearTimeFilterGps(void)
{
    GpsTimeBack = 0;
}

_Bool GpsFilterData(void)
{
    return GpsFilterDataConfig(
        GetGpsRecordMinSpeed(), GetGpsRecordDistanse(), GetRecordAccel(), GetGpsRealtime(), GetGpsRecordCourse());
}

_Bool GpsFilterDataConfig(uint8_t MinSpeed,
    uint16_t GpsDistance,
    _Bool GpsAccelTrue,
    uint16_t GpsTime,
    uint8_t GpsCourse)
{
    uint32_t SecRTC;
    static uint32_t SecBackRTC = 0;

    float Course = 0;
    _Bool spin;
    uint32_t Lon;
    uint32_t Lat;
    static double distance;

    if(g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE) {
        return 0;
    }

    float SpeedGps = getSpeedKmGps();

    SecRTC = get_cur_sec();
    if(SecRTC == SecBackRTC) {
        return 0;
    }
    SecBackRTC = SecRTC;

    if((GpsAccelTrue == ON) && (AccelState() == ACC_STATE_STOP)) {
        return 0;
    }

    Lat = (uint32_t)g_stGpsData.latitude;
    Lon = (uint32_t)g_stGpsData.longitude;
    Course = g_stGpsData.course + 360;

    if(Course > stOld.course) {
        spin = 1;
    }
    else {
        spin = 0;
    }

    //Проверяем на время.
    if((GpsTime != 0) && (SecRTC >= GpsTimeBack)) {
        GpsTimeBack = SecRTC + (uint32_t)GpsTime;

        if(SpeedGps < MinSpeed) {
            distance = GetMapDistanceInM(stOld.latitude, stOld.longitude, Lat, Lon);
            if(distance > 15) {
                stOld.latitude = Lat;
                stOld.longitude = Lon;
                stOld.spin = spin;
                stOld.course = Course;
                return 1;
            }
        }
        else {
            stOld.latitude = Lat;
            stOld.longitude = Lon;
            stOld.spin = spin;
            stOld.course = Course;
            return 1;
        }
        return 0;
    }

    if(SpeedGps >= MinSpeed)    //#1
    {
        if((!((Course + GpsCourse > stOld.course) && (Course - GpsCourse < stOld.course))) && (GpsCourse)) {    //#2
            stOld.latitude = Lat;
            stOld.longitude = Lon;
            stOld.spin = spin;
            stOld.course = Course;
            return 1;
        }

        distance = GetMapDistanceInM(stOld.latitude, stOld.longitude, Lat, Lon);
        if((distance > GpsDistance) && (GpsDistance))    //#3
        {
            stOld.latitude = Lat;
            stOld.longitude = Lon;
            stOld.spin = spin;
            stOld.course = Course;
            return 1;
        }
        return 0;
    }
    else {
        if((!((Course + GpsCourse > stOld.course) && (Course - GpsCourse < stOld.course))) && (GpsCourse)) {    //#4
            if(GpsCourse < 30) {                                                                                //#5
                if((Course + 31 > stOld.course) && (Course - 31 < stOld.course)) {                              //#7
                    distance = GetMapDistanceInM(stOld.latitude, stOld.longitude, Lat, Lon);
                    if((distance > 15) || (spin == stOld.spin)) {    //#6
                        stOld.latitude = Lat;
                        stOld.longitude = Lon;
                        stOld.spin = spin;
                        stOld.course = Course;
                        return 1;
                    }
                }
            }
            else {
                distance = GetMapDistanceInM(stOld.latitude, stOld.longitude, Lat, Lon);
                if((distance > 15) || (spin == stOld.spin)) {    //#6
                    stOld.latitude = Lat;
                    stOld.longitude = Lon;
                    stOld.spin = spin;
                    stOld.course = Course;
                    return 1;
                }
            }
        }
        else {
            distance = GetMapDistanceInM(stOld.latitude, stOld.longitude, Lat, Lon);
            if((distance > GpsDistance) && (GpsDistance))    //#3
            {
                stOld.latitude = Lat;
                stOld.longitude = Lon;
                stOld.spin = spin;
                stOld.course = Course;
                return 1;
            }
        }
    }
    return 0;
}

static void gps_deinit(void)
{
    DPS("-GPS PWR OFF-\r\n");
    GPS_Reference(OFF);
    DeInitUSART(UART_GPS);
}

uint32_t GetDataNavigationalGpsPacket(char* pOutDataFrameBuffer, uint8_t TypePacket, uint32_t OffsetData)
{
    uint32_t LenDataNavigationalPacket = 0;
    LenDataNavigationalPacket = frame_build_navigational_packet_realtime(&g_stGpsData, DataNavigationFrameBuffer);
    for(uint32_t i = 0; i < LenDataNavigationalPacket; i++) {
        pOutDataFrameBuffer[i + OffsetData] = DataNavigationFrameBuffer[i];
    }
    pOutDataFrameBuffer[OffsetData] = TypePacket;    //Подставляем тип пакета(RealTime или Track)

    return LenDataNavigationalPacket;
}

/*
ret:
0 - Not Gps Valid
1 - Gps Valid
*/
int FindPositionGps(void)
{
    int iResult = 0;
    static _Bool bMoveAccel = 0;
    portTickType xLastWakeTimerDelay;

    DP_GSM("D_FIND GPS, PLEASE WAIT\r\n");
    xSemaphoreGive(mINIT_GPS_MODULE);    //Вкл. GPS
    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_2000 / portTICK_RATE_MS));
    static uint32_t wait_sms = 0;
    wait_sms = GetTimeWaitSms();

    static uint32_t wait_gps = 0;
    if(!(wait_gps)) {
        wait_gps = GetGpsWait();
    }

    // wait_gps = 0;
    setDebugGps(1);
    while(wait_gps) {
        DP_GSM("\r\nD_WAIT GPS: %i\r\n", wait_gps);
        if(GetGpsStatus()) {
            /* Проверим HDOP */
            if((getHdopGps() < GetGpsHdopFixCoordinates()) || (wait_gps == 1)) {
                DP_GSM("\r\nD_GPS VALID\r\n");
                iResult = 1;
                break;    //Выходим если нашли спутники.
            }
            else {
                DP_GSM("D_GPS BAD HDOP\r\n");
            }
        }
        __enable_interrupt();

        _Bool accel_enable = FALSE;
        if(GetModeProtocol() == FINDME_911 && GetEnableAccelToFind()) {    //Если включено просыпание от акселерометра и стоит режим поиска.
            accel_enable = TRUE;
        }

        if(GetModeProtocol() == ION_FM && GetModeDevice() == TIMER_FIND && GetEnableAccelToFind()) {
            accel_enable = TRUE;
        }

        if(accel_enable) {
            if(AccelState() != ACC_STATE_STOP) {
                if(!(bMoveAccel)) {
                    bMoveAccel = 1;
                    iResult = -1;
                    DP_GSM("\r\nD_ACCEL MOVE BREAK\r\n");
                    setDebugGps(0);
                    return iResult;    //Выходим если было движение.
                }
            }
        }

        wait_gps--;
        g_stRam.stDevDiag.stHard.uiTimePwrGps++;
        if(wait_sms)
            wait_sms--;
        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
        SystemUpdateMonitor();    //Обновляем мониторинг зависание FreeRTOS
    }

    setDebugGps(0);
    xSemaphoreGive(mDEINIT_GPS_MODULE);

    SetTimeWaitSms(wait_sms);    // Сохраняем время сколько ждали смс.

    uint16_t aucGpsFind[2] = { 0 };
    GetGpsFind(&aucGpsFind[0], &aucGpsFind[1]);
    if(iResult == 1) {
        aucGpsFind[0]++;
    }
    else if(iResult == 0) {
        aucGpsFind[0]++;
        aucGpsFind[1]++;
    }
    SetGpsFind(aucGpsFind[0], aucGpsFind[1]);

    SetTimePwrGps(GetTimeAllPwrGps());

    return iResult;
}

static GPS_INFO stGpsData;
// 1 - Valid, 0 - No
_Bool GetGpsStatus(void)
{
    return (stGpsData.latitude && stGpsData.longitude && stGpsData.time && stGpsData.sat && stGpsData.status);
}
//Структура с последними GPS данными.
void SetPositionGps(const GPS_INFO* const pstGpsData)
{
    memcpy(&stGpsData, pstGpsData, sizeof(GPS_INFO));
}

/* return:
 1 - GPS Save Data Valid
 0 - GPS Save Data No Valid
*/
_Bool GetPositionGps(GPS_INFO* ptrGpsData)
{
    if(GetGpsStatus()) {
        memcpy(ptrGpsData, &stGpsData, sizeof(GPS_INFO));
        return 1;
    }
    return 0;
}

float getHdopGps(void)
{
    return stGpsData.hdop;
}

void CpyGpsBuf(const char* pBuf, int Len)
{
    memset(g_aucGpsBuffOut, 0, sizeof(g_aucGpsBuffOut));
    memcpy(g_aucGpsBuffOut, pBuf, Len);
}

/* Сохранение координат при неудачном выходе */
void saveOldPositionGps(void)
{
    GPS_INFO stGpsData;
    if(GetPositionGps(&stGpsData)) {
        setOldPositionGps(&stGpsData);
    }
}

#pragma optimize = none
int gps_init(void)
{
    DPS("-GPS PWR ON-\r\n");
    GPS_Reference(ON);

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));

    if(autoSetBaudrateGps()) {
        return 1;
    }

    DPS("Configuration of GPS\r\n");
    configNmeaMessage();

    configBaudrateGps(GPS_BAUDRATE);

    configGpsUart(GPS_BAUDRATE);

    DPS("The configuration of the GPS is completed successfully\r\n");
    return 0;
}

/*
Реализация автонастройки скорости UART GPS.
@param  None
@retval:
  1 - Fail
  0 - OK
*/
static int autoSetBaudrateGps(void)
{
    const int baudrate[] = { 115200, 9600 };    //массив с возможными скоростями uart gps
    int count_cr[sizeof(baudrate) / sizeof(int)] = { 0 };

    /* Цикл перебора скоростей */
    loop(sizeof(baudrate) / sizeof(int))
    {
        configGpsUart(baudrate[i]);
        portTickType WakeTick = xTaskGetTickCount() + SLEEP_MS_1000;
        while(xTaskGetTickCount() < WakeTick) {
            IWDG_ReloadCounter();
            extern _Bool NMEA_CR_END;
            if(NMEA_CR_END) {    // ловим окончание строки
                NMEA_CR_END = FALSE;
                count_cr[i]++;
            }
        }
        if(count_cr[i] > 3) {
            break;
        }
    }

    int baudrate_true = 0, count = 0;    // найденная скорость
    loop(sizeof(baudrate) / sizeof(int))
    {
        if(count_cr[i] > count) {
            count = count_cr[i];
            baudrate_true = baudrate[i];
        }
    }

    DPS("Autoset baudrate GPS UART is ");
    if(count) {
        DPS("OK\r\n");    // скорость подобранна правильно
        configGpsUart(baudrate_true);
    }
    else {
        DPS("FAIL\r\n");    // не одна скорость из возможных не подходит
        return 1;
    }
    return 0;
}

static void configGpsUart(uint32_t baudrate)
{
    /* Init UART 9600 */
    DeInitUSART(UART_GPS);
    InitUSART(UART_GPS, baudrate);
    InitDMA(UART_GPS);
    ReStartDmaGpsUsart();
}

static void configNmeaMessage(void)
{
    char strConfigGps[64];
    /* Set NMEA GPS Module*/
    //$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*34\r\n
    sprintf(strConfigGps, "%s", "$PMTK314,");

#if GPS_GLL_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif
#if GPS_RMC_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif
#if GPS_VTG_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif
#if GPS_GGA_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif
#if GPS_GSV_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif
#if GPS_ZDA_ENABLE
    strcat(strConfigGps, "1,");
#else
    strcat(strConfigGps, "0,");
#endif

    strcat(strConfigGps, "0,0,0,0,0,0,0,0,0,0,0,0");

    calculateGpsCrc(strConfigGps);

    extern _Bool NMEA_CR_END;
    loop(15)
    {
        /* перед отправкой команды конфигурации необходимо дождаться конца сообщения */
        NMEA_CR_END = FALSE;
        int timeout_exit = 10000;
        timeout_exit--;
        IWDG_ReloadCounter();
        if(!(timeout_exit)) {
            break;
        }
        USART_Write(UART_GPS, strConfigGps, strlen(strConfigGps));
    }
}

static void configBaudrateGps(uint32_t baudrate)
{
    char strConfigGps[32];
    /* Set Baudrate UART GPS */
    sprintf(strConfigGps, "%s%d", "$PMTK251,", baudrate);

    calculateGpsCrc(strConfigGps);
    loop(5)
    {
        int timeout_exit = 10000;
        /* перед отправкой команды конфигурации необходимо дождаться конца сообщения */
        extern _Bool NMEA_CR_END;
        while(!(NMEA_CR_END)) {
            timeout_exit--;
            IWDG_ReloadCounter();
            if(!(timeout_exit)) {
                break;
            }
        }
        USART_Write(UART_GPS, strConfigGps, strlen(strConfigGps));
    }
}

static void calculateGpsCrc(char* ptr)
{
    uint8_t calc_crc = 0;
    for(int i = 1; i < strlen(ptr); i++) {
        calc_crc ^= ptr[i];
    }
    sprintf(((char*)(ptr) + strlen(ptr)), "*%02X\r\n", calc_crc);
}