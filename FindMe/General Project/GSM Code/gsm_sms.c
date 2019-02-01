
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "gsm_sms.h"

u8 ucSmsFormat = 0x91;    //Формат номера откуда пришла СМС и куда её переслать(интернациональный, национальный)

int state_sms = 0;

#define END 0x1A
int SMS_Exec(SMS_INFO* pSMS);

static int my_strcpy(char* dst, const char* src)
{
    int n = 0;

    if(dst != 0 && src != 0) {
        while(src[n] != 0) {
            dst[n] = src[n];
            n++;
        }
        dst[n] = src[n];
    }

    return n;
}

static int koi8_to_ucs2(char* out, u8* inp, u8 len)
{
    u8 n = 0;
    u16 m = 0;
    u16 uc = 0;

    for(; n < len; n++) {
        if(inp[n] == 0)
            break;

        uc = inp[n];

        if(uc == 'ё')
            uc = 0x0451;
        else if(uc == 'Ё')
            uc = 0x0401;
        else if(uc >= 0x00C0) {
            uc = uc + 0x00410 - 0x00C0;
        }

        sprintf(out + m, "%04X", uc);
        m += 4;
    }
    out[m] = 0;

    return m;
}

static u8 get_D16(u8* pBuf)
{
    u8 ret = 0;

    if(pBuf[0] >= '0' && pBuf[0] <= '9') {
        ret = pBuf[0] - '0';
    }
    else if(pBuf[0] >= 'A' && pBuf[0] <= 'F') {
        ret = pBuf[0] - 'A' + 10;
    }

    ret *= 16;

    if(pBuf[1] >= '0' && pBuf[1] <= '9') {
        ret += (pBuf[1] - '0');
    }
    else if(pBuf[1] >= 'A' && pBuf[1] <= 'F') {
        ret += (pBuf[1] - 'A' + 10);
    }

    return ret;
}

static int PDU_UnPackTN(SMS_TN* tn, u8* pInp, u8 size)
{
    u8 n;

    for(n = 0; n < size;) {
        /*if(pInp[n] < '0' || pInp[n] > '9') {
          if(pInp[n] != 'F') return 0;
      } else*/
        {
            tn->buf[n + 1] = pInp[n];
        }
        n++;

        /*if(pInp[n] < '0' || pInp[n] > '9') return 0;*/
        tn->buf[n - 1] = pInp[n];
        n++;
    }

    tn->buf[size] = 0;
    tn->size = size;

    return size;
}

static int PDU_PackTN(char* out, SMS_TN* tn)
{
    uint8_t n = 0;
    for(n = 0; n < tn->size; n += 2) {
        out[n] = tn->buf[n + 1];
        out[n + 1] = tn->buf[n];
    }

    if((tn->size & 1) != 0) {
        out[tn->size - 1] = 'F';
    }

    return n;
}

static int PDU_SetHeader(char* buf, SMS_TN* tn)
{
    if(tn->size <= 0) {
        return -1;
    }
    sprintf(buf, "001100%02X%02X", tn->size, ucSmsFormat);    // header = 8 + tn_size = (2)
    int len = PDU_PackTN(buf + 10, tn);
    if(len <= 0)
        return -1;
    return (len + 10);
}

static int PDU_TextUnCode7(u8* pOut, u8* pInp, int size)
{
    int i = 0;
    u8 m = 0, c0 = 0, c1 = 0;

    for(int n = 0; i < size; i++, n += 2) {
        c1 = get_D16(pInp + n);

        pOut[i] = ((c1 << m) | (c0 >> (8 - m))) & 0x7F;

        if(m < 6) {
            m++;
        }
        else {
            i++;
            pOut[i] = (c1 >> 1) & 0x7F;
            m = 0;
        }

        c0 = c1;
    }
    pOut[i] = 0;

    return i;
}

int PDU_SMGD(uint8_t sms_number)
{
    sprintf(g_asCmdBuf, "%s%d", AT_CMGD, sms_number);
    if(mc(g_asCmdBuf, 4, MC_COUNT) == OK) {
        return 0;
    }
    return -1;
}

SMS_RESPONSE PDU_SMGL(u64* mask)
{
    SMS_RESPONSE ret = SMS_FALSE;
    GSM_INFO data_gsm_sms;
    *mask = 0;
    // выполняем команду
    mc_send(AT_SMGL, NULL, 10);
    portTickType xLastWakeTimerFreezes = xTaskGetTickCount() + (120 * configTICK_RATE_HZ);

    while(1) {
        int iRxSize = gsm_parser(AT_SMGL, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);

        if(xTaskGetTickCount() > xLastWakeTimerFreezes) {
            DP_GSM("SMGL SMS \"OK\"!\r\n");
            return PARESER_FREEZES;
        }

        if(iRxSize < 0) {
            switch(iRxSize) {
                case -1:
                    return DMA_OVERFLOW;
            }
        }

        if(iRxSize == 0) {
            // timeout
            GSM_DC(AT_SMGL, 't');
            break;
        }

        /* ERRORS  */
        //+CMS ERROR: operation not allowed
        if(data_gsm_sms.m_type == M_STRING || data_gsm_sms.m_type == M_CME_ERROR) {
            char* pFindERROR = strstr(g_asRxBuf, "+CMS ERROR:");

            /* Перечисление распространенных ошибок */
            if(pFindERROR) {    //+CMS ERROR:
                if(strstr(g_asRxBuf, "operation not allowed")) {
                    ret = OPERATION_NOT_ALLOWED;
                }
                break;

                if(strstr(g_asRxBuf, "SIM PIN required")) {
                    ret = SIM_PIN_REQUIRED;
                }
                break;

                ret = UNSPECIFIED_ERROR;
                break;
            }
        }

        if(data_gsm_sms.m_type == M_SMGL) {
            osDelay(1000);
            iRxSize = gsm_parser(AT_SMGL, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);
            if((data_gsm_sms.m_type == M_STRING) && (data_gsm_sms.msg[0].size)) {
                if(g_bDmaGsmFail == TRUE) {
                    return DMA_OVERFLOW;
                }
                if(iRxSize < 0) {
                    return PARESER_FREEZES;
                }

                u16 len;
                u16 len_1;
                u16 len_2;
                SMS_INFO sms;
                sms.number = 1;
                sms.txt.buf = (uint8_t*)g_aucOutDataFrameBuffer;

                // вычисляем смещение до поля с длинной номера пользователя
                len_1 = (get_D16((u8*)g_asRxBuf) + 2) * 2;
                // вычисляем длину номера
                len_2 = get_D16((u8*)g_asRxBuf + len_1);
                // вычисляем смещение до признака кодирования
                len = len_1 + len_2 + 6;
                if((len_2 & 1) != 0)
                    len++;

                // определяем формат номера национальный/интернациональный
                ucSmsFormat = get_D16((uint8_t*)g_asRxBuf + len_1 + 2);

                // декодируем телефонный номер
                if(PDU_UnPackTN(&sms.tn, (u8*)g_asRxBuf + len_1 + 4, len_2) == 0) {
                    // игнорируем не определённый номер
                    DP_GSM("SMS: Error telephone number\r\n");
                    return SMS_FALSE;
                }

                sms.tn.size = len_2;
                sms.txt.size = get_D16((u8*)g_asRxBuf + len + 16);
                if((get_D16((u8*)g_asRxBuf + len) & 0x08) != 0) {
                    // 16-битный код
                    sms.txt.code = 1;
                    sms.txt.size <<= 1;

                    // копируем 16-битный код
                    memcpy(sms.txt.buf, g_asRxBuf + len + 18, sms.txt.size);
                    sms.txt.buf[sms.txt.size] = 0;
                }
                else {
                    // 7-битный код
                    sms.txt.code = 0;

                    // декодируем 7-битный код
                    PDU_TextUnCode7(sms.txt.buf, (u8*)g_asRxBuf + len + 18, sms.txt.size);
                }

                SMS_Exec(&sms);
                PDU_SMGD(6);    // Delete all SMS
                return SMS_TRUE;
            }
        }

        if((data_gsm_sms.m_type == M_SMGL) || (data_gsm_sms.m_type == M_P_CMGL)) {
            if(data_gsm_sms.msg[0].var < 63) {
                *mask |= (u64)1 << (data_gsm_sms.msg[0].var - 1);
            }
            ret++;
        }
        if(data_gsm_sms.m_type == M_OK) {
            GSM_DC(AT_SMGL, '0');
            break;
        }
        if(data_gsm_sms.m_type == M_ERROR || data_gsm_sms.m_type == M_ABORTED) {
            GSM_DC(AT_SMGL, '4');
            break;
        }
        if(data_gsm_sms.m_type == M_CME_ERROR) {
            GSM_DC(AT_SMGL, '5');
            break;
        }
        if(g_bDmaGsmFail == TRUE) {
            return DMA_OVERFLOW;
        }
    }

    return ret;
}

SMS_RESPONSE PDU_SMGL_FM911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode)
{
    SMS_RESPONSE ret = SMS_FALSE;
    GSM_INFO data_gsm_sms;

    // выполняем команду
    mc_send(AT_SMGL, NULL, 10);
    portTickType xLastWakeTimerFreezes = xTaskGetTickCount() + (120 * configTICK_RATE_HZ);

    while(1) {
        int iRxSize = gsm_parser(AT_SMGL, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);

        if(xTaskGetTickCount() > xLastWakeTimerFreezes) {
            DP_GSM("SMGL SMS \"OK\"!\r\n");
            return PARESER_FREEZES;
        }

        if(iRxSize < 0) {
            switch(iRxSize) {
                case -1:
                    return DMA_OVERFLOW;

                default:
                    return DMA_OVERFLOW;
            }
        }

        if(iRxSize == 0) {
            // timeout
            GSM_DC(AT_SMGL, 't');
            break;
        }

        /* ERRORS  */
        //+CMS ERROR: operation not allowed
        if(data_gsm_sms.m_type == M_STRING) {
            char* pFindERROR = strstr((char*)g_asRxBuf, "+CMS ERROR");

            /* Перечисление распространенных ошибок */
            if(pFindERROR) {    //+CMS ERROR:
                pFindERROR = strstr((char*)g_asRxBuf, "operation not allowed");
                if(pFindERROR) {
                    ret = OPERATION_NOT_ALLOWED;
                }
                break;
            }
        }

        if(data_gsm_sms.m_type == M_SMGL) {
            osDelay(1000);
            iRxSize = gsm_parser(AT_SMGL, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);
            if((data_gsm_sms.m_type == M_STRING) && (data_gsm_sms.msg[0].size)) {
                if(g_bDmaGsmFail == TRUE) {
                    return DMA_OVERFLOW;
                }
                if(iRxSize < 0) {
                    return PARESER_FREEZES;
                }

                u16 len;
                u16 len_1;
                u16 len_2;
                SMS_INFO sms;
                sms.number = 1;
                sms.txt.buf = (uint8_t*)g_aucOutDataFrameBuffer;

                // вычисляем смещение до поля с длинной номера пользователя
                len_1 = (get_D16((u8*)g_asRxBuf) + 2) * 2;
                // вычисляем длину номера
                len_2 = get_D16((u8*)g_asRxBuf + len_1);
                // вычисляем смещение до признака кодирования
                len = len_1 + len_2 + 6;
                if((len_2 & 1) != 0)
                    len++;

                // определяем формат номера национальный/интернациональный
                ucSmsFormat = get_D16((uint8_t*)g_asRxBuf + len_1 + 2);

                // декодируем телефонный номер
                if(PDU_UnPackTN(&sms.tn, (u8*)g_asRxBuf + len_1 + 4, len_2) == 0) {
                    // игнорируем не определённый номер
                    DP_GSM("SMS: Error telephone number\r\n");
                    return SMS_FALSE;
                }

                sms.tn.size = len_2;
                sms.txt.size = get_D16((u8*)g_asRxBuf + len + 16);
                if((get_D16((u8*)g_asRxBuf + len) & 0x08) != 0) {
                    // 16-битный код
                    sms.txt.code = CODE_UCS2;    //
                    sms.txt.size <<= 1;

                    // копируем 16-битный код
                    memcpy(sms.txt.buf, g_asRxBuf + len + 18, sms.txt.size);
                    sms.txt.buf[sms.txt.size] = 0;
                }
                else {
                    // 7-битный код
                    sms.txt.code = CODE_LAT;    // LAT

                    // декодируем 7-битный код
                    PDU_TextUnCode7(sms.txt.buf, (u8*)g_asRxBuf + len + 18, sms.txt.size);

                    char* ptr = strstr((char*)sms.txt.buf, "\r\n");    // фильтруем перевод сроки в тексте СМС
                    if(ptr)
                        *ptr = NULL;
                }

                _Bool err = FALSE;
                /* Фильтруем спамовые номера меньше 5 цифр и пользовательское название девайса не более 15 знаков */
                if(strlen((char*)sms.tn.buf) < MIN_SIZE_NUMBER_SMS) {
                    /* сообщение не прошло проверку */
                    err = TRUE;
                }
                if((sms.txt.code == CODE_LAT) && (strlen((char*)sms.txt.buf) >= MAX_SIZE_NAME_DEVICE)) {
                    /* сообщение не прошло проверку */
                    err = TRUE;
                }
                if((sms.txt.code == CODE_UCS2) && (strlen((char*)sms.txt.buf) >= (MAX_SIZE_NAME_DEVICE * 4))) {
                    /* сообщение не прошло проверку */
                    err = TRUE;
                }

                if(err) {           // сообщение не прошло проверку
                    PDU_SMGD(6);    // Delete all SMS
                    ptrNumUserTel[0] = '\0';
                    ptrNameDevice[0] = '\0';
                    return SMS_FALSE;
                }

                *pCode = sms.txt.code;
                strcpy(ptrNameDevice, (char*)sms.txt.buf);    // Возвращаем имя девайса.
                strcpy(ptrNumUserTel, (char*)sms.tn.buf);     // Возвращаем телефон пользователя.

                /* Сформируем и отправим СМС пользователю о ожидании регистрации */
                sendSmsForUser911((char*)sms.tn.buf);
                // PDU_SMGD(6);        //удалим СМС познее, уже после передачи данных на сервер
                return SMS_TRUE;
            }
        }

        if((data_gsm_sms.m_type == M_SMGL) || (data_gsm_sms.m_type == M_P_CMGL)) {
            if(data_gsm_sms.msg[0].var < 63) {
                //*mask |= (u64)1 << (data_gsm_sms.msg[0].var - 1);
            }
            ret++;
        }
        if(data_gsm_sms.m_type == M_OK) {
            GSM_DC(AT_SMGL, '0');
            break;
        }
        if(data_gsm_sms.m_type == M_ERROR || data_gsm_sms.m_type == M_ABORTED) {
            GSM_DC(AT_SMGL, '4');
            break;
        }
        if(data_gsm_sms.m_type == M_CME_ERROR) {
            GSM_DC(AT_SMGL, '5');
            break;
        }
        if(g_bDmaGsmFail == TRUE) {
            return DMA_OVERFLOW;
        }
    }

    return ret;
}

int PDU_PartOfThePack7(char* out, SMS_INFO* inp, u8 id, u8 total, u8 part)
{
    u8 txt_size_m = 7;
    int txt_size_n = 0, txt_size_i = 0;

    txt_size_i = PDU_SetHeader(out, &inp->tn);
    /*
   char msg_buf[10];
   sprintf(msg_buf, "TXTSIZEI=%d\r\n", txt_size_i);
   DP_GSM(msg_buf);
   */

    if(txt_size_i <= 0) {
        return -1;
    }

    out[2] = '5';    // признак длинного смс

    sprintf(out + txt_size_i, "0000AD%02X050003%02X%02X%02X", inp->txt.size + 7, id, total, part);
    txt_size_i += 20;

    sprintf(out + txt_size_i,
        "%02X",
        ((inp->txt.buf[txt_size_n] >> (txt_size_m - 1)) | (inp->txt.buf[txt_size_n] << (8 - txt_size_m))) & 0xFF);
    txt_size_i += 2;
    txt_size_m = 1;
    txt_size_n++;

    for(;;) {
        if(txt_size_n >= inp->txt.size)
            break;

        sprintf(out + txt_size_i,
            "%02X",
            ((inp->txt.buf[txt_size_n] >> (txt_size_m - 1)) | (inp->txt.buf[txt_size_n + 1] << (8 - txt_size_m))) &
                0xFF);

        txt_size_i += 2;
        txt_size_n++;

        if(txt_size_m >= 7) {
            txt_size_m = 1;
            txt_size_n++;
        }
        else {
            txt_size_m++;
        }
    }

    out[txt_size_i] = 0;

    return txt_size_i;
}

int PDU_SMGR(SMS_INFO* sms, u8 second)
{
    i8 count = 1;
    state_sms = 0;
    GSM_INFO data_gsm_sms;
    int iRxSize = 0;

    sprintf(g_asCmdBuf, "at^smgr=%d", sms->number);

    for(; count > 0; count--) {
        switch(state_sms) {
            case 0:
                // выполняем команду
                mc_send(g_asCmdBuf, NULL, 10);
                state_sms = 1;

            case 1:
                // ждём заголовок
                iRxSize = gsm_parser(g_asCmdBuf, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, second);
                if(iRxSize <= 0) {
                    // timeout
                    GSM_DC(g_asCmdBuf, 't');
                    state_sms = 0;
                    break;
                }

                if(data_gsm_sms.m_type == M_SMGR) {
                    u16 len;
                    u16 len_1;
                    u16 len_2;

                    // принимаем daнные
                    iRxSize = gsm_gets(g_asRxBuf, RX_BUFFER_SIZE, second);
                    if(iRxSize <= 0) {
                        // timeout
                        GSM_DC(g_asCmdBuf, 't');
                        state_sms = 0;
                        break;
                    }

                    // вычисляем смещение до поля с длинной номера пользователя
                    len_1 = (get_D16((u8*)g_asRxBuf) + 2) * 2;
                    // вычисляем длину номера
                    len_2 = get_D16((u8*)g_asRxBuf + len_1);
                    // вычисляем смещение до признака кодирования
                    len = len_1 + len_2 + 6;
                    if((len_2 & 1) != 0)
                        len++;

                    // определяем формат номера национальный/интернациональный
                    ucSmsFormat = get_D16((uint8_t*)g_asRxBuf + len_1 + 2);
                    DP_GSM("SMS: Format %02X\r\n", ucSmsFormat);

                    // декодируем телефонный номер
                    if(PDU_UnPackTN(&sms->tn, (u8*)g_asRxBuf + len_1 + 4, len_2) == 0) {
                        // игнорируем не определённый номер
                        DP_GSM("SMS: Error telephone number\r\n");
                        state_sms = 2;
                        count = 0;
                        break;
                    }

                    sms->tn.size = len_2;
                    sms->txt.size = get_D16((u8*)g_asRxBuf + len + 16);
                    if((get_D16((u8*)g_asRxBuf + len) & 0x08) != 0) {
                        // 16-битный код
                        sms->txt.code = 1;
                        sms->txt.size <<= 1;

                        // копируем 16-битный код
                        memcpy(sms->txt.buf, g_asRxBuf + len + 18, sms->txt.size);
                        sms->txt.buf[sms->txt.size] = 0;
                    }
                    else {
                        // 7-битный код
                        sms->txt.code = 0;

                        // декодируем 7-битный код
                        PDU_TextUnCode7(sms->txt.buf, (u8*)g_asRxBuf + len + 18, sms->txt.size);
                    }
                    state_sms = 2;
                }
                else if(data_gsm_sms.m_type == M_OK) {
                    GSM_DC(g_asCmdBuf, '0');
                    return 0;
                }
                else if(data_gsm_sms.m_type == M_ERROR || data_gsm_sms.m_type == M_ABORTED) {
                    GSM_DC(g_asCmdBuf, '4');
                    // return -1;
                    osDelay(SLEEP_MS_1000);
                }
                break;

            case 2:
                // ждём  OK
                gsm_parser(g_asCmdBuf, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, second);
                if(data_gsm_sms.m_type == M_OK) {
                    GSM_DC(g_asCmdBuf, '0');
                    return 0;
                }

                GSM_DC(g_asCmdBuf, 't');
                state_sms = 0;
                break;
        }
    }

    return -1;
}

static u16 GSM_specialSymbol(u8 sym)
{
    if(sym == '{')
        return 0x1B28;
    if(sym == '}')
        return 0x1B29;
    if(sym == '[')
        return 0x1B3C;
    if(sym == ']')
        return 0x1B3E;
    if(sym == '^')
        return 0x1B14;
    if(sym == '\\')
        return 0x1B2F;
    if(sym == '|')
        return 0x1B40;
    if(sym == '~')
        return 0x1B3D;

    return sym;
}

static int PDU_Pack7(char* out, SMS_INFO* inp)
{
    char m = 1;
    int i = PDU_SetHeader(out, &inp->tn);
    if(i <= 0)
        return -1;

    sprintf(out + i, "0000AD%02X", inp->txt.size);
    i += 8;
    int n = 0;

    for(;;) {
        if(n >= inp->txt.size)
            break;

        sprintf(out + i, "%02X", ((inp->txt.buf[n] >> (m - 1)) | (inp->txt.buf[n + 1] << (8 - m))) & 0xFF);

        i += 2;
        n++;

        if(m >= 7) {
            m = 1;
            n++;
        }
        else {
            m++;
        }
    }

    out[i] = 0;

    return i;
}

int PDU_PartOfThePackUcs2(char* out, SMS_INFO* inp, u8 id, u8 total, u8 part)
{
    u16 header_len = 0;
    u16 size = 0;
    u8 tmp;

    header_len = PDU_SetHeader(out, &inp->tn);
    if(header_len <= 0)
        return -1;

    size = koi8_to_ucs2(out + header_len + 20, inp->txt.buf, 67);
    if(size <= 0)
        return -1;

    out[2] = '5';    // признак длинного смс

    //допишем заголовок
    tmp = out[header_len + 20];
    sprintf(out + header_len, "0008AD%02X050003%02X%02X%02X", size / 2 + 6, id, total, part);
    out[header_len + 20] = tmp;

    out[header_len + 20 + size] = 0;

    return (header_len + 20 + size);
}

int PDU_CMGS(char* buf, int ms_size)
{
    int res = -1;
    char ch;
    char state = 0;
    GSM_INFO data_gsm_sms1;

    ReStartDmaGsmUsart();

    sprintf(g_asCmdBuf, "at+cmgs=%d", ((ms_size - 2) >> 1));

    // выполняем команду
    mc_send(g_asCmdBuf, NULL, 10);

    portTickType xLastWakeTimerFreezes = xTaskGetTickCount() + (60 * configTICK_RATE_HZ);

    // ждём приглашение "> "
    while(1) {
        if(xTaskGetTickCount() > xLastWakeTimerFreezes) {
            DP_GSM("No invitation SMS \">\"!\r\n");
            return -1;
        }

        if(gsm_read(&ch, 1, 5) == 0) {
            return -1;
        }
        switch(ch) {
            case '\r':
                DPS("\r\n");
                break;
            case '\n':
                DPS("\r\n");
                break;
            default:
                DPC(ch);
        }
        if(state == 0) {
            if(ch == '>')
                state = 1;
        }
        else {
            if(ch == ' ')
                break;
        }
    }

    // передаем текст SMS-сообщения
    buf[ms_size] = END;
    ms_size++;
    gsm_write(buf, ms_size, 1);

    memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
    memset(g_asRxBuf, 0, sizeof(g_asRxBuf));

    xLastWakeTimerFreezes = xTaskGetTickCount() + (60 * configTICK_RATE_HZ);

    // игнорим информационное сообщение и ждём OK
    for(;;) {
        osDelay(100);
        int iRxSize = gsm_parser(g_asCmdBuf, &data_gsm_sms1, g_asRxBuf, RX_BUFFER_SIZE, 10);

        if(xTaskGetTickCount() > xLastWakeTimerFreezes || iRxSize < 0) {
            DP_GSM("No answer SMS \"OK\"!\r\n");
            return -1;
        }

        if(iRxSize <= 0) {
            // timeout
            DPS("SMS t");
            state = 0;
            break;
        }

        if(data_gsm_sms1.m_type == M_OK) {
            DPS("SMS 0");
            break;
        }
        if(data_gsm_sms1.m_type == M_ERROR || data_gsm_sms1.m_type == M_ABORTED) {
            DPS("SMS 4");
            break;
        }
    }

    return res;
}

int PDU_PackUcs2(char* out, SMS_INFO* inp)
{
    u8 i = 0;
    u8 size = 0;
    char tmp[4];

    i = PDU_SetHeader(out, &inp->tn);
    if(i <= 0)
        return -1;

    strcpy(out + i, "0008AD");    // header              4
    i += 6;
    size = koi8_to_ucs2(out + i + 2, inp->txt.buf, 70);    // txt_conv_to_utf     12
    if(size <= 0)
        return -1;

    sprintf(tmp, "%02X", (size >> 1));    // txt_size            2
    memcpy(out + i, tmp, 2);

    return (size + i + 2);
}

int PDU_SendSms(SMS_INFO* sms, char* sms_buf)
{
    u16 sms_len_max;
    u16 sms_part_len;
    u8 sms_block_id = 1;
    u16 sep_symb_num = 0;
    int sms_size;
    int inp_size;
    int res;
    res = -1;
    SMS_INFO inp;

    sms->txt.code = 0;
    //проверяем есть ли русские символы в смс
    for(u16 i = 0; i < sms->txt.size; i++)
        if(sms->txt.buf[i] > 0x7F && GSM_specialSymbol(sms->txt.buf[i]) < 0x1b00) {
            sms->txt.code = 1;
            break;
        }

    if(sms->txt.code != 0) {
        // 16-битный код
        sms_len_max = 70;
        sms_part_len = 67;
    }
    else {
        // 7-битный код
        sms_part_len = 153;
        sms_len_max = 160;
        //считаем число спец. символов
        for(u16 i = 0; i < sms->txt.size; i++)
            if(GSM_specialSymbol(sms->txt.buf[i]) >= 0x1b00)
                sep_symb_num++;
        sms->txt.buf[sms->txt.size + sep_symb_num] = 0;
        //вставляем спец. символы gsm (начинаются с 0x1b)
        for(i16 i = sms->txt.size - 1 + sep_symb_num; sep_symb_num; i--) {
            u16 code = GSM_specialSymbol(sms->txt.buf[i - sep_symb_num]);
            if(code >= 0x1b00) {
                sms->txt.buf[i--] = (u8)(code & 0x00FF);
                sms->txt.buf[i] = code >> 8;
                sep_symb_num--;
                sms->txt.size++;
            }
            else
                sms->txt.buf[i] = sms->txt.buf[i - sep_symb_num];
        }
    }

    if(sms->txt.size > sms_len_max)    //Если СМС длинное
    {                                  //Отпарвка длинного СМС
        char part = 1;
        char total = sms->txt.size / sms_part_len + ((sms->txt.size % sms_part_len) != 0);

        memcpy(&inp.tn, &sms->tn, sizeof(SMS_TN));
        inp.txt.buf = sms->txt.buf;
        inp.txt.size = sms_part_len;

        for(inp_size = sms->txt.size; inp_size > 0; inp.txt.buf += sms_part_len, inp_size -= sms_part_len) {
            if(inp_size <= sms_part_len)
                inp.txt.size = inp_size;
            if(sms->txt.code != 0)
                sms_size = PDU_PartOfThePackUcs2(sms_buf, &inp, sms_block_id, total, part++);
            else
                sms_size = PDU_PartOfThePack7(sms_buf, &inp, sms_block_id, total, part++);
            /*
         char msg_buf[10];
         sprintf(msg_buf, "SMSSIZE=%d\r\n", sms_size);
         DP_GSM(msg_buf);
         */
            if(sms_size > 0) {
                PDU_CMGS(sms_buf, sms_size);
            }
        }
    }
    else    //Если СМС короткое
    {       //Отпарвка короткого СМС
        if(sms->txt.code != 0) {
            sms_size = PDU_PackUcs2(sms_buf, sms);
        }
        else {
            sms_size = PDU_Pack7(sms_buf, sms);
        }
        if(sms_size > 0) {
            PDU_CMGS(sms_buf, sms_size);
            res = 0;
        }
    }

    return res;
}

int SendTXTSMS(char* pbuf, char* pTelSMS)
{
    SMS_INFO sms;
    if(strlen(pbuf) == 0 || strlen(pTelSMS) == 0) {
        return OK;
    }

    my_strcpy((char*)sms.tn.buf, pTelSMS);
    sms.tn.size = strlen((char*)sms.tn.buf);
    sms.txt.buf = (u8*)g_asInpDataFrameBuffer;
    my_strcpy((char*)sms.txt.buf, (char const*)pbuf);
    sms.txt.size = strlen((char*)sms.txt.buf);
    if(sms.txt.size && sms.tn.size > 7) {
        PDU_SendSms(&sms, g_aucOutDataFrameBuffer);
        return OK;
    }
    return FAIL;
}