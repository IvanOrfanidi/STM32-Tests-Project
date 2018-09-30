#ifndef __FLASH_ACHIVE_H_
#define __FLASH_ACHIVE_H_

#include "includes.h"

#define MAX_LEN_FLASH 30

typedef enum
{
   CMD_FLASH_FULL_ERASE = -2,   // стереть всю флеш
   CMD_FLASH_ARCHIVE_ERASE = -1,   // стереть трек
   //положительные значения - команда для стирания определенного сектора
} CMD_FLASH;

//размер архива в Мб
#define FLASH_ARCHIVE_SZ_MBIT 3

//стартовая страница в начале сектора!
#define FLASH_ARCHIVE_START_PAGE 0
//конец архива в начале сектора!
#define FLASH_ARCHIVE_END_PAGE (FLASH_ARCHIVE_SZ_MBIT * (32 * FLASH_SUB_SECTOR_PG_NUM))
//минимальное число пустых секторов между началом и концом архива
#define FLASH_ARCHIVE_FREE_SUB_SEC_NUM 2   // 8кб пустого места

/****************************************
разметка хранения данных на странице во флешке
****************************************/
#define PAGE_WRITE_READ_ATTEMPS 64

typedef __packed struct
{
#define FLASH_PAGE_DATA_LEN 249
#define FLASH_PAGE_LEN 256

   u8 data[FLASH_PAGE_DATA_LEN];
   u32 ulPageID;
   u8 ucStart;
   u16 uiCRC;
} TFlashPage;

/****************************************

счетчики и адреса для драйвера флеш (копия структуры лежит в
backup регистрах RTC)

****************************************/
typedef __packed struct
{
   u32 start_adr;   //начало архива (пока не используется)
   u32 write_adr;   //адрес для записи архива
   u32 read_adr;   //адрес с которого шлем данные на сервер

   u16 reserved;
   u16 uiInitID;   //флаг инициализации данных
#define FLASH_INIT_ID 0x12AB
#define FLASH_START_ADRESS RTC_BKP_DR1
} TFlash_Pointers;

typedef __packed struct
{
   u32 curr_read;   //текущая позиция чтения
   u32 curr_id;   //счетчик записей во флеш, также уникальный номер каждой страницы
#define FLASH_TMP_PAGE_ID 0xFFFFFF00   // id временного блока

   //переменные для склейки данных между страницами
   u8 data_shift;   //смещение у первой записи в текущей странице
   u8 data_shift_tmp;   //смещение у первой записи в следующей странице

   //флаг инициализации флеши
   u8 ucFlashInit;

} TFlash;

void FlashInit();
void FlashHandler();

s8 Flash_QueryPacket(u8* pdata, u16 len);
s16 Flash_ReadData(u8* pbuf, u16 max_len);
s8 FLASH_Take_Semaphore(void);
void FLASH_Give_Semaphore(void);

void Flash_DataSendOK();
u32 Flash_DataLen();
void ArchiveErase(void);
void OnChangeFlashErase();
void OnChangeArchiveErase();
void ArciveFlashEarse(void);
u16 GetLenDataBuf(void);
u8 GetDataBuf(u16 iIndex);
void FullFlashErase(void);
void ArciveEarse(void);
void EraseArcive(void);

u16 GetLenDataBuf(void);
void ReloadDataBuf(char* pOut, u32 Len);

/* For FM3 for iON */
uint8_t GetCountDataFlash(void);
void SaveDataFm(uint8_t count_point, char* ptr, int len_data);
int ReadDataFm(uint8_t count_point, char* ptr);

#endif
