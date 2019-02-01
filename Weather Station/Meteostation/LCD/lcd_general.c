
#include "lcd_general.h"
#include "includes.h"

const char strMsgTemperatur[] = "“≈Ãœ≈–¿“”–¿:    ";
const char strMsgError[] = "ERROR";

void vLcdTask(void* pvParameters)
{
    RTC_t stDate;
    char strMsgstDate[20];
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t mday = 0;
    TOnewire_Data stTemperaturData;
    char* pSt = (char*)&stTemperaturData;
    unsigned char toSend;
    _Bool TemperaturDataValid;

    LCD_Init();

    while(1) {
        // LCD_Send_CMD(DISP_OFF);
        // LCD_Send_CMD(DISP_ON);

        rtc_gettime(&stDate);

        if((stDate.year != year) || (stDate.month != month) || (stDate.mday != mday)) {
            year = stDate.year;
            month = stDate.month;
            mday = stDate.mday;

            LCD_Send_CMD(DD_RAM_ADDR1);

            sprintf(strMsgstDate, "%02d/%02d/%02d  ", stDate.mday, stDate.month, stDate.year);
            for(uint8_t i = 0; i < strlen(strMsgstDate); i++) {
                LCD_Send_CHAR(strMsgstDate[i]);
            }
        }

        LCD_Send_CMD(DD_RAM_ADDR1 + 12);

        sprintf(strMsgstDate, "%02d:%02d:%02d", stDate.hour, stDate.min, stDate.sec);
        for(uint8_t i = 0; i < strlen(strMsgstDate); i++) {
            LCD_Send_CHAR(strMsgstDate[i]);
        }

        LCD_Send_CMD(DD_RAM_ADDR2);

        if(xQueueOnewireDataToLcd != 0) {
            TemperaturDataValid = 1;
            for(uint8_t i = 0; i < sizeof(stTemperaturData); i++) {
                if(xQueueReceive(xQueueOnewireDataToLcd, (void*)&toSend, (portTickType)10)) {
                    pSt[sizeof(stTemperaturData) - 1 - i] = toSend;
                }
                else {
                    TemperaturDataValid = 0;
                }
            }

            if(TemperaturDataValid) {
                for(uint8_t i = 0; i < strlen(strMsgTemperatur); i++) {
                    LCD_Send_CHAR(strMsgTemperatur[i]);
                }

                if(stTemperaturData.bDataValid) {
                    //«‰ÂÒ¸ Ó·‡·ÓÚ˜ËÍ.
                }
                else {
                    for(uint8_t i = 0; i < strlen(strMsgError); i++) {
                        LCD_Send_CHAR(strMsgError[i]);
                    }
                }
            }
        }

        /*
      LCD_Send_CMD(DD_RAM_ADDR3);
      for(uint8_t i=0; i<COLUMN_LCD; i++) {
                 LCD_Send_CHAR(0x20);
      }
      
      LCD_Send_CMD(DD_RAM_ADDR4);
      for(uint8_t i=0; i<COLUMN_LCD; i++) {
                 LCD_Send_CHAR(0x20);
      }
       */
        _delay_ms(1000);
    }

    vTaskDelete(NULL);
}
