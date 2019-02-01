
#include "includes.h"
#include "sdcard_general.h"

/* Private define ------------------------------------------------------------*/
#define BlockSize _MAX_SS /* Block Size in Bytes */
//#define BlockSize            128 /* Block Size in Bytes */
#define BufferWordsSize (BlockSize >> 2)

#define NumberOfBlocks 2 /* For Multi Blocks operation (Read/Write) */
#define MultiBufferWordsSize ((BlockSize * NumberOfBlocks) >> 2)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Buffer_Block_Tx[BufferWordsSize], Buffer_Block_Rx[BufferWordsSize];
uint32_t Buffer_MultiBlock_Tx[MultiBufferWordsSize], Buffer_MultiBlock_Rx[MultiBufferWordsSize];
/* Private typedef -----------------------------------------------------------*/
typedef enum { FAILED = 0,
    PASSED = !FAILED } TestStatus;
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;

SD_CardInfo SDCardInfo;

#define TIME_WRITE_DATA 5

/* Private functions ---------------------------------------------------------*/
FATFS fs;          // Work area (file system object) for logical drive
FIL fsrc, fdst;    // file objects
//BYTE BufferSdCard[_MAX_SS]; // file copy BufferSDCard
FRESULT res;    // FatFs function common result code
UINT br, bw;    // File R/W count

#define MAX_NUM_FILES 10
#define MAX_NUM_CHAR_FILE 13

char Files[MAX_NUM_FILES][MAX_NUM_CHAR_FILE];

void vSdCardTask(void* pvParameters)
{
    RTC_t stDate;
    FRESULT res;
    /*
  FILINFO f_info;
  DIR dirs;
  char dir_name[10]={""};
  */
    char strFileName[20];
    uint32_t uiTimeWriteDown = 0;
    uint32_t uiCurentSec;

    tTEMPERATUR_DATA stTemperaturData;
    char* pSt = (char*)&stTemperaturData;
    unsigned char toSend;

    char a_ucFileData[22];
    //OutPutFile();

    res = f_mount(0, &fs);    // Mount disk
    //res = f_opendir(&dirs, dir_name);

    if(res == FR_OK) {
        rtc_gettime(&stDate);
        sprintf(strFileName, "%02d.%02d.%02d %02d.%02d.txt", stDate.mday, stDate.month, stDate.year, stDate.hour, stDate.min);
        res = f_open(&fdst, strFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
        //f_read(&fdst, BufferSdCard, sizeof(BufferSdCard), &bw);	//Read File in BufferSdCard
        f_close(&fdst);
    }

    while(1) {
        rtc_gettime(&stDate);
        uiCurentSec = Date2Sec(&stDate);
        while((uiCurentSec >= uiTimeWriteDown) /* && (xQueueOnewireDataToSdcard !=0) && (uxQueueMessagesWaiting(xQueueOnewireDataToSdcard))*/) {
            uiTimeWriteDown = uiCurentSec + TIME_WRITE_DATA;

            f_mount(0, &fs);    // Mount disk
            res = f_open(&fdst, strFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);

            if(res == FR_OK) {
                GPIO_HIGH(PORT_LED, LED_D1);
                //f_read(&fdst, BufferSdCard, sizeof(BufferSdCard), &bw);	//Read File in BufferSdCard

                for(uint8_t i = 0; i < sizeof(stTemperaturData); i++) {
                    if(xQueueReceive(xQueueOnewireDataToSdcard, (void*)&toSend, (portTickType)10)) {
                        pSt[sizeof(stTemperaturData) - 1 - i] = toSend;
                    }
                    else {
                        stTemperaturData.bDataValid = 0;
                    }
                }

                if(stTemperaturData.bDataValid) {
                    sprintf(a_ucFileData, "%02d:%02d:%02d ", stDate.hour, stDate.min, stDate.sec);
                    sprintf(a_ucFileData + strlen(a_ucFileData), "%s\r\n", stTemperaturData.strTemperatur);
                }
                else {
                    sprintf(a_ucFileData, "%02d:%02d:%02d %s\r\n", stDate.hour, stDate.min, stDate.sec, "ERROR");
                }
                //bw=strlen((char*)BufferSdCard);
                res = f_lseek(&fdst, f_size(&fdst));

                if(res == FR_OK) {
                    f_puts(a_ucFileData, &fdst);
                    f_close(&fdst);
                    GPIO_LOW(PORT_LED, LED_D1);
                    break;
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }

        f_mount(0, NULL);    // Unmount disk
        if(res != FR_OK) {
            GPIO_HIGH(PORT_LED, LED_D1);
            uiTimeWriteDown = 0;
            //res = f_mount(0, &fs); // Mount disk
            //res = f_mount(0, NULL); // Unmount disk
        }
        osDelay(1000);
    }
}

#if 0
void OutPutFile(void)
{ 
  FRESULT res;
  FILINFO f_info;
  DIR dirs;
	
  int nFiles = 0;
  char path[10]={""};
  _Bool ErrRUN = 0;    
  res = f_mount(0, &fs); // Mount disk
  
  res = f_opendir(&dirs, path);

  if(res == FR_OK)
  {
      while((f_readdir(&dirs, &f_info) == FR_OK) && f_info.fname[0])
      {
        strcpy(Files[nFiles],f_info.fname);
        nFiles++;
      }
  }
  
  char NameFileRead[] = "read.txt";
  uint8_t NumFile = 0;
  
  ErrRUN = 1;
  for(uint8_t i=0; i<nFiles; i++)
  {
    if(!(strcmp(NameFileRead, Files[i])))
    {
      NumFile = i;
      ErrRUN = 0;
      break;
    }
  }
  
  if(ErrRUN) return;

  res = f_open(&fsrc, Files[NumFile], FA_OPEN_EXISTING | FA_READ);
	
  if(res == FR_OK)
  {
    for(uint16_t i=0; i<sizeof(BufferSdCard); i++) 
      BufferSdCard[i]=0;                                  
    f_read(&fsrc, BufferSdCard, sizeof(BufferSdCard), &br);	//Read File in BufferSDCard
    f_close(&fsrc);    
  }
  
  // סמחהאול פאיכ write.txt
  res = f_open(&fdst, "write.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (res == FR_OK)
  {
     f_write(&fdst, BufferSdCard, br, &bw);
     f_close(&fdst);
  }
}
#endif

int SDIO_Fats_Init(void)
{
    SD_Error Status = SD_OK;
    NVIC_SDIO_Configuration();
    /*-------------------------- SD Init ----------------------------- */
    Status = SD_Init();

    if(Status == SD_OK) {
        /*----------------- Read CSD/CID MSD registers ------------------*/
        Status = SD_GetCardInfo(&SDCardInfo);
    }

    if(Status == SD_OK) {
        /*----------------- Select Card --------------------------------*/
        Status = SD_SelectDeselect((u32)(SDCardInfo.RCA << 16));
    }

    if(Status == SD_OK) {
        //Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
    }

    /* Set Device Transfer Mode to DMA */
    if(Status == SD_OK) {
        Status = SD_SetDeviceMode(SD_DMA_MODE);
        //Status = SD_SetDeviceMode(SD_POLLING_MODE);
        //Status = SD_SetDeviceMode(SD_INTERRUPT_MODE);
    }

    if(Status == SD_OK) {
        //Status = SD_ReadBlock(0x00, Buffer_Block_Rx, BlockSize);
        //Status = SD_WriteBlock(0x00, Buffer_Block_Tx, BlockSize);
        Status = SD_ReadMultiBlocks(0x00, Buffer_MultiBlock_Rx, BlockSize, NumberOfBlocks);
    }

    if(Status == SD_OK) {
        /* Read block of 512 bytes from address 0 */
        Status = SD_ReadBlock(0x00, Buffer_Block_Rx, BlockSize);
    }

    if(Status == SD_OK) {
        /* Read block of many bytes from address 0 */
        Status = SD_ReadMultiBlocks(0x00, Buffer_MultiBlock_Rx, BlockSize, NumberOfBlocks);
    }

    return Status;
}

void NVIC_SDIO_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
