#include <math.h>
#include <string.h>
#include "includes.h"
#include "gps_parser_ver1.h"

#define BUF_SIZE BSPCFG_SCI5_QUEUE_SIZE

void gprmc_parser(GPS_INFO* const inf, uint8_t* const state, uint8_t byte);
void gpgga_parser(GPS_INFO* const inf, uint8_t* const state, uint8_t byte);

#define GPS_ROW 6
#define GPS_COL 3

char gps_head[GPS_ROW][GPS_COL] = { "RMC", "VTG", "GGA", "GLL", "GSA", "GSV" };

typedef enum {
    GPS_PARSER_START = 0,
    GPS_PARSER_RMC_STEP1,
    GPS_PARSER_RMC_STEP2,
    GPS_PARSER_GGA_STEP1,
    GPS_PARSER_GGA_STEP2,
    GPS_PARSER_COPY,
    GPS_PARSER_COPY_DONE
} GPS_PARSER_STEP;

unsigned char GPSStingToGPS_Parser[100] = "$GP";

uint8_t step = 0;
uint8_t msg_copy_to_index = 0;

void gps_parser_sting(GPS_INFO* const inf, char* buf, int size)
{
    uint16_t index;

    for(index = 0; index < size; index++) {
        if((step == GPS_PARSER_START) && (buf[index] == 'R')) {
            inf->status = 0;
            GPSStingToGPS_Parser[3] = buf[index];
            step = GPS_PARSER_RMC_STEP1;
            continue;
        }

        if((step == GPS_PARSER_RMC_STEP1) && (buf[index] == 'M')) {
            GPSStingToGPS_Parser[4] = buf[index];
            step = GPS_PARSER_RMC_STEP2;
            continue;
        }

        if((step == GPS_PARSER_RMC_STEP2) && (buf[index] == 'C')) {
            GPSStingToGPS_Parser[5] = buf[index];
            step = GPS_PARSER_COPY;
            msg_copy_to_index = 6;
            continue;
        }

        if((buf[index] == 'G') && (step == GPS_PARSER_START)) {
            GPSStingToGPS_Parser[3] = buf[index];
            step = GPS_PARSER_GGA_STEP1;
            continue;
        }

        if((step == GPS_PARSER_GGA_STEP1)) {
            if(buf[index] != 'G') {
                step = GPS_PARSER_START;
                continue;
            }

            GPSStingToGPS_Parser[4] = buf[index];
            step = GPS_PARSER_GGA_STEP2;
            continue;
        }

        if((step == GPS_PARSER_GGA_STEP2)) {
            if(buf[index] != 'A') {
                step = GPS_PARSER_START;
                continue;
            }

            GPSStingToGPS_Parser[5] = buf[index];
            step = GPS_PARSER_COPY;
            msg_copy_to_index = 6;
            continue;
        }

        if(step == GPS_PARSER_COPY) {
            if(msg_copy_to_index < sizeof(GPSStingToGPS_Parser))    // buffer overflow protection
            {
                GPSStingToGPS_Parser[msg_copy_to_index++] = buf[index];
            }
            if(buf[index] == '\n') {
                step = GPS_PARSER_START;

                gps_parser(inf, GPSStingToGPS_Parser, msg_copy_to_index);
            }
            continue;
        }
    }
}

int gps_parser(GPS_INFO* const inf, unsigned char* buf, int size)
{
    int n;
    char ch;
    uint8_t csum = 0;
    uint8_t csum_stop = 0;
    uint8_t step = 0;
    uint8_t state = 0;
    uint8_t j, row = 0, col = 0;
    const char alarm[15] = "PMTK010,002*2D";

    for(n = 0; n < size; n++) {
        ch = buf[n];
        if(ch == '$') {
            row = 0;
            col = 0;
            step = 0;
            csum = 0;
            csum_stop = 0;
            state = 1;
        }
        else if(state > 0) {
            if(ch == '\r') {
                switch(row) {
                    case GPS_GGA:
                        if(step == 15) {
                            if(csum == inf->cs_gga)
                                return row;
                        }
                        break;
                    case GPS_RMC:
                        if(step == 13) {
                            if(csum == inf->cs_rmc)
                                return row;
                        }
                        break;
                    case GPS_START:
                        if(col != 14)
                            break;
                    case GPS_GLL:
                    case GPS_GSA:
                    case GPS_GSV:
                        return row;
                    case GPS_VTG:
                        // printf("\r\n");
                        return row;
                }
                return -1;
            }
            else {
                if(csum_stop == 0) {
                    if(ch != '*') {
                        csum ^= ch;
                    }
                    else {
                        csum_stop = 1;
                    }
                }
                switch(state) {
                    case 1:
                        if(ch == 'G') {
                            state++;
                        }
                        else if(ch == alarm[col]) {
                            col++;
                            row = GPS_START;
                            state = 5;
                        }
                        else {
                            state = 0;
                        }
                        break;

                    case 2:
                        state++;
                        break;

                    case 3:
                        if(col < GPS_COL) {
                            if(ch != gps_head[row][col]) {
                                for(j = row + 1; j < GPS_ROW; j++) {
                                    if(ch == gps_head[j][col]) {
                                        row = j;
                                        break;
                                    }
                                }
                                if(j == GPS_TOTAL) {
                                    state = 0;
                                }
                            }
                            col++;
                            break;
                        }
                        else {
                            state = 4;
                            switch(row) {
                                case GPS_GGA:
                                    // GPGGA
                                    inf->sat = 0;
                                    inf->hdop = 0;
                                    inf->cs_gga = 0;
                                    inf->status &= ~MASK_GGA;
                                    // printf("GGA");
                                    break;
                                case GPS_RMC:
                                    // GPRMC
                                    inf->time = 0;
                                    inf->latitude = 0;
                                    inf->longitude = 0;
                                    inf->course = 0;
                                    inf->speed = 0;
                                    inf->cs_rmc = 0;
                                    inf->status &= ~MASK_RMC;
                                    // printf("RMC");
                                    break;
                                case GPS_GSA:
                                    // printf("GSA");
                                    break;
                                case GPS_GSV:
                                    // printf("GSV");
                                    break;
                                case GPS_VTG:
                                    // printf("VTG");
                                    break;
                                case GPS_GLL:
                                    // printf("GLL");
                                    break;
                            }
                        }

                    case 4:
                        switch(row) {
                            case GPS_GGA:
                                gpgga_parser(inf, &step, ch); /*printf("%c", ch);*/
                                break;
                            case GPS_GSA:
                                break;    // printf("5-$GPGSA\r\n");
                            case GPS_GSV:
                                break;    // printf("6-$GPGSV\r\n");
                            case GPS_RMC:
                                gprmc_parser(inf, &step, ch); /*printf("%c", ch);*/
                                break;
                            case GPS_VTG:
                                break;    // printf("2-$GPVTG\r\n");
                            case GPS_GLL:
                                break;    // printf("4-$GPGLL\r\n");
                        }
                        break;

                    case 5:
                        if(col < 15) {
                            if(ch == alarm[col]) {
                                col++;
                            }
                            else {
                                state = 0;
                            }
                        }
                        break;
                }
            }
        }
    }

    return -1;
}

void gprmc_parser(GPS_INFO* const inf, uint8_t* const state, uint8_t byte)
{
    unsigned char n;
    static int8_t len;
    static uint8_t status;
    static DATE_STRUCT date;
    TIME_STRUCT time;

    static uint16_t deg_lt;
    static uint16_t min_lt;
    static float sec_lt;
    static uint16_t deg_lg;
    static uint16_t min_lg;
    static float sec_lg;
    static uint16_t sec_cnt;

    if(*state == 0) {
        if(',' == byte) {
            len = 0;
            status = 0;

            date.DAY = 0;
            date.MONTH = 0;
            date.YEAR = 0;
            date.HOUR = 0;
            date.MINUTE = 0;
            date.SECOND = 0;
            date.MILLISEC = 0;

            (*state)++;
        }
    }
    else {
        switch(*state) {
            case 1:    // время
                deg_lt = 0;
                min_lt = 0;
                sec_lt = 0;
                deg_lg = 0;
                min_lg = 0;
                sec_lg = 0;
                switch(byte) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        n = byte - '0';
                        switch(len) {
                            case 0:
                                date.HOUR = n;
                                break;
                            case 1:
                                date.HOUR *= 10;
                                date.HOUR += n;
                                break;

                            case 2:
                                date.MINUTE = n;
                                break;
                            case 3:
                                date.MINUTE *= 10;
                                date.MINUTE += n;
                                break;

                            case 4:
                                date.SECOND = n;
                                break;
                            case 5:
                                date.SECOND *= 10;
                                date.SECOND += n;
                                break;

                            case 7:
                                date.MILLISEC = n;
                                break;
                            case 8:
                            case 9:
                                date.MILLISEC *= 10;
                                date.MILLISEC += n;
                                break;
                            default:
                                *state = 14;
                        }
                        len++;
                        break;
                    case '.':
                        if(len == 6) {
                            len++;
                        }
                        else {
                            (*state) = 14;
                        }
                        break;
                    case ',':
                        if(len == 9 + GPS_PACKET_LEN) {
                            len = 0;
                            (*state) += 1;
                            break;
                        }
                    default:
                        (*state) = 14;
                }
                break;

            case 2:    // статус данных
                if(len < 2) {
                    if(byte == ',') {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                    if(len == 0) {
                        if(byte == 'A') {
                            status = MASK_RMC;
                        }
                        else {
                        }
                        len++;
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 3:    // широта
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 2) {
                        deg_lt *= 10;
                        deg_lt += n;
                        len++;
                        break;
                    }
                    else if(len < 4) {
                        min_lt *= 10;
                        min_lt += n;
                        sec_cnt = 0;
                        len++;
                        break;
                    }
                    else if(len < 10 - GPS_PACKET_LEN) {
                        len++;
                        if(++sec_cnt > 2)
                            sec_lt += n / pow(10, sec_cnt - 2);
                        else {
                            sec_lt *= 10;
                            sec_lt += n;
                        }
                        break;
                    }
                }
                else if(byte == '.') {
                    if(len == 4) {
                        len++;
                        break;
                    }
                }
                else if(byte == ',') {
                    if(len == 10 - GPS_PACKET_LEN || len == 0) {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 4:    // P-N:северная  S:южная
                if(len < 2) {
                    if(byte == ',') {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                    if(len == 0) {
                        switch(byte) {
                            case 'N':
                            case 'P':
                                deg_lt += 90.000000;
                                break;
                            case 'S':
                                len++;
                                break;
                            case ',':
                                len = 0;
                                deg_lt = 0;
                                min_lt = 0;
                                sec_lt = 0;
                                inf->latitude = 0;
                                (*state) += 1;
                                break;
                            default:
                                (*state) = 14;
                        }
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 5:    // долгота
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 3) {
                        deg_lg *= 10;
                        deg_lg += n;
                        len++;
                        break;
                    }
                    else if(len < 5) {
                        min_lg *= 10;
                        min_lg += n;
                        sec_cnt = 0;
                        len++;
                        break;
                    }
                    else if(len < 11 - GPS_PACKET_LEN) {
                        len++;
                        if(++sec_cnt > 2)
                            sec_lg += n / pow(10, sec_cnt - 2);
                        else {
                            sec_lg *= 10;
                            sec_lg += n;
                        }
                        break;
                    }
                }
                else if(byte == '.') {
                    if(len == 5) {
                        len++;
                        break;
                    }
                }
                else if(byte == ',') {
                    if(len == 11 - GPS_PACKET_LEN || len == 0) {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 6:    // J-E:восточная W:западная
                if(len < 2) {
                    if(byte == ',') {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                    if(len == 0) {
                        switch(byte) {
                            case 'E':
                            case 'J':
                                deg_lg += 180.000000;
                                break;
                            case 'W':
                                len++;
                                break;
                            case ',':
                                len = 0;
                                deg_lg = 0;
                                min_lg = 0;
                                sec_lg = 0;
                                inf->longitude = 0;
                                (*state) += 1;
                                break;
                            default:
                                (*state) = 14;
                        }
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 7:    // скорость
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 2) {
                        if(len == 0) {
                            inf->speed = n;
                        }
                        else {
                            inf->speed *= 10;
                            inf->speed += n;
                        }
                        len = 1;
                    }
                    else {
                        inf->speed += n / pow(10, len - 1);
                        len++;
                    }
                }
                else if(byte == '.') {
                    len = 2;
                }
                else if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                }
                else {
                    status = 0;
                    (*state) = 14;
                }
                break;

            case 8:    // угол
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 2) {
                        if(len == 0) {
                            inf->course = n;
                        }
                        else {
                            inf->course *= 10;
                            inf->course += n;
                        }
                        len = 1;
                    }
                    else {
                        inf->course += n / pow(10, len - 1);
                        len++;
                    }
                }
                else if(byte == '.') {
                    len = 2;
                }
                else if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                }
                else {
                    status = 0;
                    (*state) = 14;
                }
                break;

            case 9:    // дата
                if(len < 7) {
                    if(byte >= '0' && byte <= '9') {
                        n = byte - '0';
                        switch(len) {
                            case 0:
                                date.DAY = n * 10;
                                break;
                            case 1:
                                date.DAY += n;
                                break;
                            case 2:
                                date.MONTH = n * 10;
                                break;
                            case 3:
                                date.MONTH += n;
                                break;
                            case 4:
                                date.YEAR = n * 10;
                                break;
                            case 5:
                                date.YEAR += n + 2000;
                                if(date.HOUR < 24 && date.MINUTE < 60 && date.SECOND < 60 && date.DAY > 0 && date.DAY < 32 &&
                                    date.MONTH > 0 && date.MONTH < 13) {
                                    _time_from_date(&date, &time);
                                    inf->time = time.SEC;
                                }
                                else {
                                    status = 0;
                                    (*state) = 14;
                                }
                        }
                        len++;
                        break;
                    }
                    else if(byte == ',') {
                        len = 0;
                        (*state) += 1;
                        break;
                    }
                }
                status = 0;
                (*state) = 14;
                break;

            case 10:    // магнитное склонение
                if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                    // break;
                }
                /*
         if((byte < '0' || byte > '9') && byte != '.') {
             status = 0;
             (*state) = 14;
         }
         */
                break;

            case 11:    // направление магнитного склонения
                if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                    // break;
                }
                /*
         if(len == 0) {
             if(byte == 'E' || byte == 'W') {
                 len++;
                 break;
             }
         }
         status = 0;
         (*state) = 14;
         */
                break;

            case 12:    // Индикатор режима
                if(byte == '*') {
                    len = 0;
                    (*state) += 1;
                    // break;
                }
                /*
         if(len == 0) {
             if(byte == 'A' || byte == 'N' || byte == 'D' || byte == 'E') {
                 len++;
                 break;
             }
         }
         status = 0;
         (*state) = 14;
         */
                break;

            case 13:    // контрольная сумма
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                }
                else if(byte >= 'A' && byte <= 'F') {
                    n = byte - 'A' + 10;
                }
                else {
                    status = 0;
                    (*state) = 14;
                    break;
                }

                switch(len) {
                    case 0:
                        len++;
                        inf->cs_rmc = n;
                        break;
                    case 1:
                        len++;
                        inf->cs_rmc *= 16;
                        inf->cs_rmc += n;
                        //пересчитываем широту и долготу
                        /* Версия афтофона
            inf->latitude = deg_lt + min_lt/60.0 + sec_lt/6000.0;
            inf->longitude = deg_lg + min_lg/60.0 + sec_lg/6000.0;
            */
                        inf->latitude = (deg_lt * 100 + min_lt) * 10000 + sec_lt * 100;
                        inf->longitude = (deg_lg * 100 + min_lg) * 10000 + sec_lg * 100;
                        inf->status |= status;
                        ulGpsPacket[0]++;
                        break;
                    default:
                        status = 0;
                        (*state) = 14;
                        break;
                }
                break;

            case 14:    // произошла ошибка в разборе
                break;
        }
    }
}

void gpgga_parser(GPS_INFO* const inf, uint8_t* const state, uint8_t byte)
{
    unsigned char n;
    static uint8_t status;
    //    static uint8_t end;
    static int8_t len;

    // printf("%c", byte);

    if(*state == 0) {
        if(',' == byte) {
            len = 0;
            //            end = 0;
            status = 0;
            (*state)++;
        }
    }
    else {
        switch(*state) {
            case 1:    // время
            case 2:    // широта
            case 3:    // P-N:северная  S:южная
            case 4:    // долгота
            case 5:    // J-E:восточная W:западная
            case 9:    // HGEO
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                if(',' == byte) {
                    (*state)++;
                }
                else if('*' == byte) {
                    (*state)++;
                    inf->status |= status;
                }
                break;

            case 6:    // Качество GPS сигнала
                if(byte == '0' || byte == '1' || byte == '2' || byte == '3') {
                    status = MASK_GGA;
                }
                else if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                }
                else {
                    status = 0;
                    (*state) = 16;
                }
                break;

            case 7:    // количество спутников
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 2) {
                        if(len == 0) {
                            inf->sat = n;
                        }
                        else {
                            inf->sat *= 10;
                            inf->sat += n;
                        }
                        len = 1;
                    }
                }
                else if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                }
                else {
                    status = 0;
                    (*state) = 16;
                }
                break;

            case 8:    // HDOP
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                    if(len < 2) {
                        if(len == 0) {
                            inf->hdop = n;
                        }
                        else {
                            inf->hdop *= 10;
                            inf->hdop += n;
                        }
                        len = 1;
                    }
                    else {
                        inf->hdop += n / pow(10, len - 1);
                        len++;
                    }
                }
                else if(byte == '.') {
                    len = 2;
                }
                else if(byte == ',') {
                    len = 0;
                    (*state) += 1;
                }
                else {
                    status = 0;
                    (*state) = 16;
                }
                break;

            case 15:    // контрольная сумма
                if(byte >= '0' && byte <= '9') {
                    n = byte - '0';
                }
                else if(byte >= 'A' && byte <= 'F') {
                    n = byte - 'A' + 10;
                }
                else {
                    status = 0;
                    (*state) = 16;
                    break;
                }

                switch(len) {
                    case 0:
                        len++;
                        inf->cs_gga = n;
                        break;
                    case 1:
                        len++;
                        inf->cs_gga *= 16;
                        inf->cs_gga += n;
                        inf->status |= status;
                        ulGpsPacket[1]++;
                        break;
                    default:
                        status = 0;
                        (*state) = 16;
                        break;
                }
                break;

            case 16:    // произошла ошибка в разборе
                break;
        }
    }
}
