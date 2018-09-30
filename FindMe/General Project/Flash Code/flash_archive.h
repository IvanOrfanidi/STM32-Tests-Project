#ifndef __FLASH_ACHIVE_H_
#define __FLASH_ACHIVE_H_

#include "includes.h"

#define MAX_LEN_FLASH 30

typedef enum
{
   CMD_FLASH_FULL_ERASE = -2,   // ������� ��� ����
   CMD_FLASH_ARCHIVE_ERASE = -1,   // ������� ����
   //������������� �������� - ������� ��� �������� ������������� �������
} CMD_FLASH;

//������ ������ � ��
#define FLASH_ARCHIVE_SZ_MBIT 3

//��������� �������� � ������ �������!
#define FLASH_ARCHIVE_START_PAGE 0
//����� ������ � ������ �������!
#define FLASH_ARCHIVE_END_PAGE (FLASH_ARCHIVE_SZ_MBIT * (32 * FLASH_SUB_SECTOR_PG_NUM))
//����������� ����� ������ �������� ����� ������� � ������ ������
#define FLASH_ARCHIVE_FREE_SUB_SEC_NUM 2   // 8�� ������� �����

/****************************************
�������� �������� ������ �� �������� �� ������
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

�������� � ������ ��� �������� ���� (����� ��������� ����� �
backup ��������� RTC)

****************************************/
typedef __packed struct
{
   u32 start_adr;   //������ ������ (���� �� ������������)
   u32 write_adr;   //����� ��� ������ ������
   u32 read_adr;   //����� � �������� ���� ������ �� ������

   u16 reserved;
   u16 uiInitID;   //���� ������������� ������
#define FLASH_INIT_ID 0x12AB
#define FLASH_START_ADRESS RTC_BKP_DR1
} TFlash_Pointers;

typedef __packed struct
{
   u32 curr_read;   //������� ������� ������
   u32 curr_id;   //������� ������� �� ����, ����� ���������� ����� ������ ��������
#define FLASH_TMP_PAGE_ID 0xFFFFFF00   // id ���������� �����

   //���������� ��� ������� ������ ����� ����������
   u8 data_shift;   //�������� � ������ ������ � ������� ��������
   u8 data_shift_tmp;   //�������� � ������ ������ � ��������� ��������

   //���� ������������� �����
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
