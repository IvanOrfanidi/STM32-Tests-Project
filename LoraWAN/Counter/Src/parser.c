
#include "parser.h"
#include "protocol.h"
#include "rtc.h"
#include "adc.h"

#define SIZE_BUF_PARSER 64

static void Pac2Date(RTC_DateTypeDef*, uint32_t);
static int parseArchive(uint8_t*, uint8_t*, uint8_t);
static int parseConfigured(uint8_t*, uint8_t*, uint8_t);
static void timeOffsetConfig(int);
static uint8_t hex2bin(uint8_t);

/* Парсер запроса из сети LoRA.
@param входной буфер данных запроса.
@param длина входного буфера данных запроса.
@param выходной буфер данных ответа.
@param длина выходной буфера данных ответа(нужна для фикса ошибок).

@retval выходная длина буфера данных ответа.
*/
int ParseRadio(uint8_t* pInData, uint8_t lenIn, uint8_t* pOutData, uint8_t lenOut)
{
    /* парсим тип пакета и его длину */
    if((lenIn == SIZE_ARCIVE_PACKET) && (pInData[0] & 0x80)) {
        /* пакет запрос данных из архива */
        return parseArchive(pInData, pOutData, lenOut);
    }

    if(lenIn == SIZE_CONFIG_PACKET) {
        /* пакет конфигурации */
        return parseConfigured(pInData, pOutData, lenOut);
    }

    return 0;
}

/*
Обработка пакета конфигурации.
@param входной буфер данных запроса.
@param выходной буфер данных ответа.

@retval выходная длина буфера данных ответа.
*/
static int parseConfigured(uint8_t* pInData, uint8_t* pOutData, uint8_t lenOut)
{
    if(lenOut < SIZE_CONFIG_PACKET + 1) {    //проверка корретности длины буфера
        return ERR_SIZE_OUT_BUF;
    }

    /* определяем является пакет запросом на получения конфигурации */
    _Bool reg_true = 1;
    for(uint8_t i = 0; i < SIZE_CONFIG_PACKET; i++) {
        if(pInData[i]) {
            reg_true = 0;
            break;
        }
    }

    if(reg_true) {    // запрос на получение конфигурации(интервал в часах, время в timestamp и напряжение питания батареи)
        // получаем интервал в часах выхода на связь
        extern RTC_HandleTypeDef hrtc;
        uint32_t interval_send = HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_COUNT_DAILY);
        interval_send >>= 16;
        pOutData[0] = (uint8_t)interval_send;

        // получаем текущее время
        packingCurrSecUnix(&pOutData[1]);

        pOutData[5] = (uint8_t)convert_meas(getMeasVin());

        extern _Bool __TEST__;
        if(__TEST__) {
            loop(SIZE_CONFIG_PACKET + 1)
            {
                printf("%02X ", pOutData[i]);
            }
            printf("\r\n");
        }

        return SIZE_CONFIG_PACKET + 2;
    }
    else {    // установка конфигураци
        uint32_t interval_send;
        if(pInData[0]) {
            interval_send = pInData[0];
            if(interval_send) {    //
                interval_send <<= 16;
                extern RTC_HandleTypeDef hrtc;
                uint32_t count_daily = HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_COUNT_DAILY);
                count_daily &= ~0xFFFFFF00;
                count_daily |= interval_send;
                HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_COUNT_DAILY, count_daily);
            }
        }

        int time_offset = 0;
        uint8_t* ptr_time_offset = (uint8_t*)&time_offset;
        uint8_t n = sizeof(time_offset);
        for(uint8_t i = 0; i < sizeof(time_offset); i++) {
            ptr_time_offset[i] = pInData[n];
            n--;
        }

        timeOffsetConfig(time_offset);

        extern _Bool __DEBUG__;
        if(__DEBUG__) {
            printf("Config Interval: = %dh\r\n", (interval_send >>= 16));
            printf("Config Time Offset: = %dsec\r\n", time_offset);
        }
    }
    return 0;
}

/*
Обработка пакета запроса данных из архива.
@param входной буфер данных запроса.
@param выходной буфер данных ответа.

@retval выходная длина буфера данных ответа.
*/
static int parseArchive(uint8_t* pInData, uint8_t* pOutData, uint8_t lenOut)
{
    uint8_t Type = pInData[0];
    Type &= ~0x80;

    RTC_TimeTypeDef stTime;
    memset(&stTime, 0, sizeof(stTime));

    /* Час */
    stTime.Hours = Type;
    stTime.Hours >>= 1;

    /* Дата */
    uint32_t date = (Type & 1);
    date <<= 8;
    RTC_DateTypeDef stDate;
    date |= pInData[1];
    date <<= 8;
    date |= pInData[2];
    Pac2Date(&stDate, date);

    /* кол-во пакетов */
    uint8_t size = (pInData[3] & 0x07);
    if(!(size))
        size = 1;    //значение 0 — интерпретируется как 1.
    if(size > MAX_SIZE_PACK_ARCH)
        size = MAX_SIZE_PACK_ARCH;    // max 6 pack.

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Request Date: %ld = %02dh ", date, stTime.Hours);
        printf("%02dm %02dd 20%02dy\r\n", stDate.Month, stDate.Date, stDate.Year);
        printf("Request Size: %d\r\n", size);
    }

    return buildDataArchive(&stTime, &stDate, size, pOutData, lenOut);
}

static void timeOffsetConfig(int offset)
{
    if(!(offset)) {
        return;
    }

    /* Get current date */
    RTC_TimeTypeDef stTime;
    RTC_DateTypeDef stDate;

    extern RTC_HandleTypeDef hrtc;
    HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);

    uint32_t cur_sec = Date2Sec(&stTime, &stDate);
    cur_sec += offset;

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Synchronization Unix Timestamp: %ld\r\n", cur_sec);
    }

    Sec2Date(&stTime, &stDate, cur_sec);
    HAL_RTC_SetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &stDate, RTC_FORMAT_BIN);
}

/*
Преобразование даты в формате протокола к формату типа RTC HAL.
@param выходная структура даты формата RTC.
@param входная структура даты в форме протокола.

@retval None.
 */
static void Pac2Date(RTC_DateTypeDef* ptrDate, uint32_t date)
{
    ptrDate->Month = date / 10000;
    date -= ptrDate->Month * 10000;
    ptrDate->Date = date / 100;
    date -= ptrDate->Date * 100;
    ptrDate->Year = date;
}

/*
Главная функция обработки запросов из сети LoRaWAN.
Вызывается из функции UART для отладки и функции приема данных из радиомодуля.
@param входные данные из сети LoRa.
@param длина полученных данных.

@retval None.
*/
void ParsingCallback(uint8_t* pData, uint16_t len)
{
    uint8_t* pInData = malloc(SIZE_BUF_PARSER);

    extern _Bool __DEBUG__;
    if(!(pInData)) {
        if(__DEBUG__) {
            printf("ERROR: NO MEMORY!\r\n");
        }
        return;
    }

    _Bool msd = 0;
    uint16_t f_len = 0;    // длина отфильтрованного от некорректных символов
    /* удаляем некорректные символы */
    loop(len)
    {
        if(!((pData[i] < '0' || (pData[i] > '9' && pData[i] < 'A') || (pData[i] > 'Z' && pData[i] < 'a') ||
               pData[i] > 'z'))) {
            uint8_t temp = pData[i];
            temp = hex2bin(temp);
            if(!(msd)) {
                pInData[f_len] = temp;
            }
            else {
                pInData[f_len] <<= 4;
                pInData[f_len] |= temp;
                f_len++;
            }
            msd ^= 1;
        }
    }

    uint8_t* pOutData = malloc(SIZE_BUF_PARSER);
    if(!(pOutData)) {
        if(__DEBUG__) {
            printf("ERROR: NO MEMORY!\r\n");
        }
        free(pInData);
        return;
    }

    int len_ans = ParseRadio(pInData, f_len, pOutData, SIZE_BUF_PARSER);
    if(len_ans) {
        //Отправляем ответ
        // SendRadio(pOutData, len_ans)
    }

    free(pInData);
    free(pOutData);
}

/* преобразование формата ASCII посылки */
static uint8_t hex2bin(uint8_t c)
{
    if((c >= 'A') && (c <= 'F')) {
        return c - 'A' + 0xA;
    }
    else if((c >= 'a') && (c <= 'f')) {
        return c - 'a' + 0xA;
    }
    else {
        return c - '0';
    }
}