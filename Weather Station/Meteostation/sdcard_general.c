

// User functions //
#include "includes.h"
#include "sdcard_general.h"

BYTE BufferSdCard[_MAX_SS];   // file copy buffer

SD_Error Status = SD_OK;

SD_CardInfo SDCardInfo;

#define TIME_WRITE_DATA 60

/* Private functions ---------------------------------------------------------*/
FATFS fs;   // Work area (file system object) for logical drive
// FIL fsrc;
FIL fdst;   // file object
UINT br, bw;   // File R/W count

#define MAX_NUM_FILES 10
#define MAX_NUM_CHAR_FILE 13

void vSD_Card(void* pvParameters)
{
   RTC_t stDate;
   FRESULT res;
   /*
   FILINFO f_info;
   DIR dirs;
   char dir_name[10]={""};
   */
   char strFileName[11];
   uint32_t uiTimeWriteDown = 0;
   uint32_t uiCurentSec;

   TOnewire_Data stTemperaturData;
   char* pSt = (char*)&stTemperaturData;
   unsigned char toSend;

   char a_ucFileData[20];
   // OutPutFile();

   res = f_mount(0, &fs);   // Mount disk
   // res = f_opendir(&dirs, dir_name);

   if (res == FR_OK)
   {
      rtc_gettime(&stDate);
      sprintf(strFileName, "%02d%02d%02d.txt", stDate.month, stDate.mday, stDate.hour);
      res = f_open(&fdst, strFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
      f_read(&fdst, BufferSdCard, sizeof(BufferSdCard), &bw);   // Read File in BufferSdCard
      f_close(&fdst);
      res = f_mount(0, NULL);   // Unmount disk
   }

   while (1)
   {
      rtc_gettime(&stDate);
      uiCurentSec = Date2Sec(&stDate);
      if ((uiCurentSec >= uiTimeWriteDown) && (xQueueOnewireDataToSdcard != 0) &&
          (uxQueueMessagesWaiting(xQueueOnewireDataToSdcard)))
      {
         uiTimeWriteDown = uiCurentSec + TIME_WRITE_DATA;

         f_mount(0, &fs);   // Mount disk
         res = f_open(&fdst, strFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);

         if (res == FR_OK)
         {
            f_read(&fdst, BufferSdCard, sizeof(BufferSdCard), &bw);   // Read File in BufferSdCard
            for (uint8_t i = 0; i < sizeof(stTemperaturData); i++)
            {
               if (xQueueReceive(xQueueOnewireDataToSdcard, (void*)&toSend, (portTickType)10))
               {
                  pSt[sizeof(stTemperaturData) - 1 - i] = toSend;
               }
               else
               {
                  stTemperaturData.bDataValid = 0;
               }
            }

            if (stTemperaturData.bDataValid)
            {
               sprintf(a_ucFileData, "%02d:%02d:%02d ", stDate.hour, stDate.min, stDate.sec);
               sprintf(a_ucFileData + strlen(a_ucFileData), "%i\r\n", stTemperaturData.iRealTemperatur);
            }
            else
            {
               sprintf(a_ucFileData, "%02d:%02d:%02d %s\r\n", stDate.hour, stDate.min, stDate.sec, "ERROR");
            }

            if (bw + strlen(a_ucFileData) >= _MAX_SS)
            {
               f_close(&fdst);
               sprintf(strFileName, "%02d%02d%02d.txt", stDate.month, stDate.mday, stDate.hour);
               uiTimeWriteDown = 0;
            }
            else
            {
               bw = strlen((char*)BufferSdCard);
               f_write(&fdst, a_ucFileData, strlen(a_ucFileData), &bw);
               _delay_ms(100);
               f_close(&fdst);
            }
         }
         else
         {
            uiTimeWriteDown = 0;
         }

         res = f_mount(0, NULL);   // Unmount disk
         _delay_ms(1000);
      }
      else
      {
         if (res != FR_OK)
         {
            uiTimeWriteDown = 0;
            // res = f_mount(0, &fs); // Mount disk
            // res = f_mount(0, NULL); // Unmount disk
         }
         _delay_ms(1000);
      }
   }

   vTaskDelete(NULL);
}

/*
char Files[MAX_NUM_FILES][MAX_NUM_CHAR_FILE];
FRESULT OutPutFile(void)
{
  FRESULT res;
  FILINFO f_info;
  DIR dirs;
   
  int nFiles = 0;
  char dir_name[10]={""};
  _Bool ErrRUN = 0;
  res = f_mount(0, &fs); // Mount disk
  if(res != FR_OK) {
     return res;
  }
  
  res = f_opendir(&dirs, dir_name);
  if(res == FR_OK)
  {
      while((f_readdir(&dirs, &f_info) == FR_OK) && f_info.fname[0])
      {
        strcpy(Files[nFiles],f_info.fname);
        nFiles++;
      }
  }
    else
    {
       return res;
    }
  
  char NameFileRead[] = "config.ini";
  uint8_t QuantityFile = 0;
  
  ErrRUN = 1;
  for(uint8_t i=0; i<nFiles; i++)
  {
    if(!(strcmp(NameFileRead, Files[i])))
    {
      QuantityFile = i;
      ErrRUN = 0;
      break;
    }
  }
  
  if(ErrRUN) {
    return FR_NO_FILE;
  }

  res = f_open(&fsrc, Files[QuantityFile], FA_OPEN_EXISTING | FA_READ);
   
  if(res == FR_OK)
  {
    memset(BufferSdCard, 0, sizeof(BufferSdCard));
    f_read(&fsrc, BufferSdCard, sizeof(BufferSdCard), &br);	//Read File in BufferSdCard
    f_close(&fsrc);
  }
    else
    {
       return res;
    }
  
  
  // סמחהאול פאיכ write.txt
  res = f_open(&fdst, "write.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (res == FR_OK)
  {
     f_write(&fdst, BufferSdCard, br, &bw);
     f_close(&fdst);
  }
  
  return res;
}

*/