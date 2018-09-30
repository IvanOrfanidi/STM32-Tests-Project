#include "includes.h"
#include "flash_archive.h"

//����� ��� ���������� ������
#define FLASH_DATA_BUFF_LEN (FLASH_PAGE_DATA_LEN + 300)
static u8 data_buff[FLASH_DATA_BUFF_LEN];
u16 indx_data_buff;

//����� �������� ���� ��� ������-������
static TFlashPage stPageBuffer;

TFlash stFlash;
//������ ������, ����� � ���������� ������ (����� ����� � ��������� RTC BACKUP)
TFlash_Pointers stFlash_Pointers;

#define TIME_LEN 4
#define TYPE_LEN 1

void FlashEarse(void);

void SaveFlashPointers()
{
   u32* ptr = (u32*)&stFlash_Pointers;
   u8 rtc_write_protect;

   stFlash_Pointers.uiInitID = FLASH_INIT_ID;
   stFlash_Pointers.reserved = 0xFFFF;

   //��������� ������� write protect ��� ��������� backup
   if (PWR->CR & PWR_CR_DBP)
   {
      rtc_write_protect = 0;
   }
   else
   {
      rtc_write_protect = 1;
   }

   if (rtc_write_protect)
   {
      PWR_RTCAccessCmd(ENABLE);
   }

   //��������� ���������
   for (u8 i = 0; i < sizeof(TFlash_Pointers) / 4; i++)
   {
      RTC_WriteBackupRegister(RTC_BKP_DR1 + i, *ptr++);
   }

   if (rtc_write_protect)
   {
      PWR_RTCAccessCmd(DISABLE);
   }
}

s8 LoadFlashPointers()
{
   u32* ptr = (u32*)&stFlash_Pointers;

   //������ ������
   for (u8 i = 0; i < sizeof(TFlash_Pointers) / 4; i++)
   {
      *ptr++ = RTC_ReadBackupRegister(RTC_BKP_DR1 + i);
   }

   if (stFlash_Pointers.uiInitID != FLASH_INIT_ID)
   {
      memset(&stFlash_Pointers, 0, sizeof(stFlash_Pointers));
      return FAIL;
   }

   return OK;
}

u8 GetDataBuf(u16 iIndex)
{
   if (iIndex > sizeof(data_buff))
      return 0;
   return data_buff[iIndex];
}

u16 GetLenDataBuf(void)
{
   return indx_data_buff;
}

void ReloadDataBuf(char* pOut, u32 Len)
{
   for (int i = 0; i < Len; i++)
   {
      pOut[i] = data_buff[i];
   }
}

//���������� ������� ���� ������ �������� �� ��������
u16 GetDataLen(TFlashPage* page)
{
   u8* pdat = &page->data[page->ucStart];
   u16 len = page->ucStart;
   u16 len_rec;

   while (pdat[0] != 0xFF)
   {
      if (pdat[0] == NAVIGATIONAL_PACKET || pdat[0] == NAVIGATIONAL_PACKET_REAL_TIME)
         len_rec = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
      else
         len_rec = pdat[1] + TYPE_LEN + 1;
      pdat += len_rec;
      len += len_rec;

      if (len >= FLASH_PAGE_DATA_LEN)
      {
         len = FLASH_PAGE_DATA_LEN;
         break;
      }
   }

   return len;
}

void InitFlashArchive()
{
   u32 max_page = 0xFFFFFFFF;
   u32 max_id = 0;
   u32 min_page = 0xFFFFFFFF;
   u32 min_id = 0xFFFFFFFF;
   // u32 tmp_page = 0;

   // if (LoadFlashPointers() == FAIL)
   //{
   //������ ��� ���� � ���� ��� �����
   //���� ������ ������
   for (u16 i = FLASH_ARCHIVE_START_PAGE; i < FLASH_ARCHIVE_END_PAGE; i++)
   {
      //������ ������ id
      EXT_FLASH_Read((u8*)&stPageBuffer.ulPageID, (i << 8) + 249, 4);

      if (min_id > stPageBuffer.ulPageID)
      {
         //��������� �� ��������
         FLASH_READ_PAGE(i, (u8*)&stPageBuffer);
         if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)))
            continue;
         if (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0)
            continue;
         min_page = i;
         min_id = stPageBuffer.ulPageID;
      }
      IWDG_ReloadCounter();   // Reload IWDG counter
   }
   //���� ����� ������
   for (int i = FLASH_ARCHIVE_END_PAGE - 1; i >= FLASH_ARCHIVE_START_PAGE; i--)
   {
      //������ ������ id
      EXT_FLASH_Read((u8*)&stPageBuffer.ulPageID, (i << 8) + 249, 4);

      // if (stPageBuffer.ulPageID == FLASH_TMP_PAGE_ID)
      //  tmp_page = i;

      if (max_id < stPageBuffer.ulPageID && stPageBuffer.ulPageID != 0xFFFFFFFF)
      {
         //��������� �� ��������
         FLASH_READ_PAGE(i, (u8*)&stPageBuffer);
         if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)))
         {
            continue;
         }
         if (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0)
         {
            continue;
         }

         max_id = stPageBuffer.ulPageID;
         max_page = i;
      }
      IWDG_ReloadCounter();   // Reload IWDG counter
   }

   if (min_page == 0xFFFFFFFF || max_page == 0xFFFFFFFF)
   {
      //������ ������
      min_page = FLASH_ARCHIVE_START_PAGE;
      max_page = FLASH_ARCHIVE_START_PAGE;
      // tmp_page = FLASH_ARCHIVE_START_PAGE+1;
      stFlash.curr_id = 1;
   }
   else
   {
      //������ � �������
      //����� ������ ��������� ��������
      if (++max_page >= FLASH_ARCHIVE_END_PAGE)
         max_page = FLASH_ARCHIVE_START_PAGE;

      stFlash.curr_id = max_id + 1;
   }

   //�������������� ������
   stFlash_Pointers.start_adr = min_page << 8;
   stFlash_Pointers.read_adr = min_page << 8;
   stFlash_Pointers.write_adr = max_page << 8;

   SaveFlashPointers();
   //}

   //�������������� ��������� �����
   /*FLASH_READ_PAGE(stFlash.tmp_adr >> 8,(u8*)&stPageBuffer);
   if (stPageBuffer.uiCRC == get_cs16((char*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) &&
   stPageBuffer.ulPageID == FLASH_TMP_PAGE_ID)
   {
     u16 len = GetDataLen(&stPageBuffer);
     memcpy(data_buff, stPageBuffer.data, len);
     stFlash.data_shift = stPageBuffer.ucStart;
     indx_data_buff = len;
   }
   else
   {
     stFlash.data_shift = 0;
     indx_data_buff = 0;
   }*/

   stFlash.data_shift = stFlash.data_shift_tmp = 0;
   indx_data_buff = 0;
   stFlash.curr_read = stFlash_Pointers.read_adr;
   stFlash.ucFlashInit = 1;
}

void IncWriteCnt()
{
   u32 stop_adr = stFlash_Pointers.write_adr;

   stop_adr += FLASH_PAGE_LEN;

   if (stop_adr >= FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN)
   {
      stop_adr = FLASH_ARCHIVE_START_PAGE * FLASH_PAGE_LEN;
   }

   u32 write_sub_sector = stop_adr >> 12;   //������� ���������, ���� ������� ������
   u32 read_sub_sector =
      write_sub_sector + FLASH_ARCHIVE_FREE_SUB_SEC_NUM + 1;   //����������� ����� ���������� ��� ������
   if (read_sub_sector >= ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) / SIZE_SUBSECTOR_FLASH))
   {
      read_sub_sector -= ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) / SIZE_SUBSECTOR_FLASH);
   }

   //��������� ���������� �������� ������ ������
   if (write_sub_sector < read_sub_sector)
   {
      if (stFlash_Pointers.write_adr < stFlash_Pointers.read_adr && stFlash_Pointers.read_adr < (read_sub_sector << 12))
         stFlash_Pointers.read_adr = read_sub_sector << 12;
      if (stFlash_Pointers.write_adr < stFlash.curr_read && stFlash.curr_read < (read_sub_sector << 12))
         stFlash.curr_read = read_sub_sector << 12;
   }
   else
   {
      if (stFlash_Pointers.write_adr < stFlash_Pointers.read_adr || stFlash_Pointers.read_adr < (read_sub_sector << 12))
         stFlash_Pointers.read_adr = read_sub_sector << 12;
      if (stFlash_Pointers.write_adr < stFlash.curr_read || stFlash.curr_read < (read_sub_sector << 12))
         stFlash.curr_read = read_sub_sector << 12;
   }

   stFlash_Pointers.write_adr = stop_adr;
   SaveFlashPointers();
}

s8 FLASH_Take_Semaphore(void)
{
   if (xSemaphoreTake(sBinSemFLASH, portMAX_DELAY) != pdTRUE)
      return FAIL;
   else
      return OK;
}

void FLASH_Give_Semaphore(void)
{
   xSemaphoreGive(sBinSemFLASH);
}

s8 Buffer_Take_Semaphore(void)
{
   if (xSemaphoreTake(sBinSemFLASH_BUFF, 1000 / portTICK_PERIOD_MS) != pdTRUE)
      return FAIL;
   else
      return OK;
}

void Buffer_Give_Semaphore(void)
{
   xSemaphoreGive(sBinSemFLASH_BUFF);
}

/************************************************

��������� ����� ��� ������ � ������ ��� � �����
���������� FAIL ���� � ������ ��� �����. OK
� ������ ������.

************************************************/
s8 Flash_QueryPacket(u8* pdata, u16 len)
{
   // RTC_t date;
   // u32 curr_sec;
   // u8 *ptime = (u8*)&curr_sec;

   // getSystemDate(&date);
   // curr_sec = Date2Sec(&date);

   if (stFlash.ucFlashInit == 0)
      return FAIL;

   //�������� ���� �� ��������� ����� � ������
   if (len > FLASH_DATA_BUFF_LEN - indx_data_buff)
      return FAIL;

   if (Buffer_Take_Semaphore() != OK)
      return FAIL;

   //��������� �����
   // data_buff[indx_data_buff++] = type;   //���
   // data_buff[indx_data_buff++] = len + TIME_LEN;    //�����
   // data_buff[indx_data_buff++] = *ptime++;//�����
   // data_buff[indx_data_buff++] = *ptime++;
   // data_buff[indx_data_buff++] = *ptime++;
   // data_buff[indx_data_buff++] = *ptime++;
   //������
   for (u16 i = 0; i < len; i++)
   {
      data_buff[indx_data_buff++] = pdata[i];
   }

   //���������� ����������� �� ���-�� ������ � ������, ���� ��, ����� �� ��������� �������� �� ����
   static u8 wr_cmd = 0;
   static u16 prev_len = 0;

   if (prev_len > indx_data_buff)
   {
      wr_cmd = 0;
   }
   prev_len = indx_data_buff;

   if (indx_data_buff >= FLASH_PAGE_DATA_LEN && wr_cmd == 0)
   {
      wr_cmd = 1;
      //���������� ������� ���������� ������ �� ����
      stFlash.data_shift = stFlash.data_shift_tmp;
      stFlash.data_shift_tmp = indx_data_buff % FLASH_PAGE_DATA_LEN;
      //������� �� ������
      xSemaphoreGive(sBinSemDATA_WRITE);
   }
   else
   {
      //������ �� ��������� �����
   }
   Buffer_Give_Semaphore();

   return OK;
}

//����� ���� ������� ����� �������� �� ������
u32 CurrReadLen()
{
   if (stFlash_Pointers.write_adr >= stFlash.curr_read)
   {
      return stFlash_Pointers.write_adr - stFlash.curr_read;
   }

   return (FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN - stFlash.curr_read) + stFlash_Pointers.write_adr;
}

//��������� ����� ������ ��� �������� �� ������
u32 DataLen()
{
   if (stFlash_Pointers.write_adr >= stFlash_Pointers.read_adr)
   {
      return stFlash_Pointers.write_adr - stFlash_Pointers.read_adr;
   }

   return (FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN - stFlash_Pointers.read_adr) + stFlash_Pointers.write_adr;
}

s8 GetPage(u32* pg_num)
{
   u32 read_num = 0;
   do
   {
      FLASH_READ_PAGE(*pg_num, (u8*)&stPageBuffer);
      if (stPageBuffer.uiCRC != get_cs16((uint8_t*)stPageBuffer.data, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) ||
          (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0))
      {
         if (CurrReadLen() >= FLASH_PAGE_LEN)
         {
            *pg_num = *pg_num + 1;
            if (*pg_num >= FLASH_ARCHIVE_END_PAGE)
               *pg_num = FLASH_ARCHIVE_START_PAGE;
            //��������� ����� ������
            stFlash.curr_read = *pg_num * FLASH_PAGE_LEN;
         }
         else
            return FAIL;
      }
      else
         return OK;
   } while (++read_num < PAGE_WRITE_READ_ATTEMPS);

   return FAIL;
}

/************************************************

���������� ������ �� ���� �������� �����.
���������� FAIL ���� ������ ��� �������� ���,
���������� ����� ������ � ������ ������

************************************************/
s16 Flash_ReadData(u8* pbuf, u16 max_len)
{
   if (stFlash.ucFlashInit == 0)
   {
      return FAIL;
   }

   FLASH_Take_Semaphore();

   //������ ������ �������
   stFlash.curr_read = stFlash_Pointers.read_adr;

   s16 len = 0;
   u16 rec_len = 0;
   u16 indx = 0;
   u8* pdata = 0;
   u32 page_num = stFlash.curr_read / FLASH_PAGE_LEN;   //����� ��������
   u16 page_shift = stFlash.curr_read % FLASH_PAGE_LEN;   //�������� ������ ��������

   //�������� ���� �� ������ � ������
   if (CurrReadLen() == 0)
   {
      FLASH_Give_Semaphore();
      return FAIL;
   }

   //������ ������ ����� ��������
   if (GetPage(&page_num) != OK)
   {
      stFlash.curr_read = page_num * FLASH_PAGE_LEN;
      stFlash_Pointers.read_adr = stFlash.curr_read;
      SaveFlashPointers();
      FLASH_Give_Semaphore();
      return FAIL;
   }

   indx = stPageBuffer.ucStart;
   pdata = stPageBuffer.data;
   //�������� ��� �� ������� �������� ����� ������ ������
   //�������� ����� ������� ���� �� ���� ������� ��������
   while (indx < page_shift - 1)
   {
      if (pdata[indx] == NAVIGATIONAL_PACKET || pdata[indx] == NAVIGATIONAL_PACKET_REAL_TIME)
         rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
      else
         rec_len = pdata[indx + 1] + TYPE_LEN + 1;
      indx += rec_len;
   }
   //�������� ����� ��� ������ - ����� ������ � ������ ��������
   if (indx != page_shift)
      page_shift = stPageBuffer.ucStart;

   indx = page_shift;
   u8 rec_tp = 0xFF;
   u8 rec_pos = 0;
   rec_len = 0;
   u8 read_len = indx;

   do
   {
      //��������� ����� �� ���������� ����. ��������
      if (indx >= FLASH_PAGE_DATA_LEN)
      {
         stFlash.curr_read = page_num * FLASH_PAGE_LEN + read_len;

         if (CurrReadLen() < FLASH_PAGE_LEN)
            break;

         indx = 0;
         read_len = 0;
         //���������� ����. �������� �� ������
         if (++page_num >= FLASH_ARCHIVE_END_PAGE)
            page_num = FLASH_ARCHIVE_START_PAGE;
         stFlash.curr_read = page_num * FLASH_PAGE_LEN;
         if (GetPage(&page_num) != OK)
         {
            //���� ���������� ������� ������� �������� ��� �� � ����. ��� �� ������ �� �����
            stFlash.curr_read = page_num * FLASH_PAGE_LEN;
            stFlash_Pointers.read_adr = stFlash.curr_read;
            SaveFlashPointers();
            break;
         }

         //�������� ��� �� �������� ���� ������������ ������ (���� ������ ����� �� ���� ���������)
         if (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME)
            rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
         else if (rec_pos == 1)
            rec_len = pdata[indx] + TYPE_LEN + 1;
         if (rec_pos && stPageBuffer.ucStart != rec_len - rec_pos)
         {
            //������ ��� - ������ ������ �������� ������
            if (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME)
            {
               if (rec_pos >= 2)
                  pbuf -= rec_pos;
            }
            else if (rec_pos > 2)
               pbuf -= rec_pos;
            rec_len = 0;
            rec_pos = 0;
            rec_tp = 0xFF;
            //������ � ������ ����. ��������
            indx = stPageBuffer.ucStart;
         }
      }

      //������������ ������
      if (rec_pos == 0)
      {
         rec_tp = pdata[indx];   //��� ������
         if (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME)
            rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
         else
            rec_len = pdata[indx + 1] + TYPE_LEN + 1;
      }
      else if (rec_pos == 1 && rec_tp != NAVIGATIONAL_PACKET && rec_tp != NAVIGATIONAL_PACKET_REAL_TIME)
         rec_len = pdata[indx] + TYPE_LEN + 1;   //����� ���� ������
      else
      {
         //�������� ������ - ��� ���������� � �����
         if (rec_pos == 2 && rec_tp != NAVIGATIONAL_PACKET && rec_tp != NAVIGATIONAL_PACKET_REAL_TIME)
         {
            *pbuf++ = rec_tp;
            *pbuf++ = rec_len - (TYPE_LEN + 1);
         }
         if (rec_pos == 1 && (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME))
         {
            *pbuf++ = rec_tp;
         }

         *pbuf++ = pdata[indx];
         //��������� ������� ������ - ��������� � ����.
         if (rec_pos == rec_len - 1)
         {
            len += rec_len;
            rec_len = 0;
            rec_pos = 0;
            rec_tp = 0xFF;
            indx++;
            //��������� ������� �� ��������� � ��� ��������
            read_len = indx;
            continue;
         }
      }

      indx++;
      rec_pos++;
   } while (len + rec_len <= max_len);

   //�������� ����� ��� �� ������������
   stFlash.curr_read = page_num * FLASH_PAGE_LEN + read_len;

   FLASH_Give_Semaphore();

   return len;
}

void Flash_DataSendOK()
{
   FLASH_Take_Semaphore();
   //��������� ����� �� ������� ���� �� ����
   int sec_read = stFlash.curr_read / SIZE_SUBSECTOR_FLASH;
   int sec_prev_read = stFlash_Pointers.read_adr / SIZE_SUBSECTOR_FLASH;
   if (sec_read != sec_prev_read)
   {
      xQueueSend(xFlashQueue, &sec_prev_read, 500);
   }
   stFlash_Pointers.read_adr = stFlash.curr_read;
   SaveFlashPointers();
   FLASH_Give_Semaphore();
}

u32 Flash_DataLen()
{
   u32 res;
   FLASH_Take_Semaphore();
   res = DataLen();
   FLASH_Give_Semaphore();

   return res;
}

//��������� ����������� ������ �� �������� �� ����
s8 CheckFlashPage(u32 page)
{
   FLASH_READ_PAGE(page, (u8*)&stPageBuffer);
   if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) ||
       (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0))
   {
      //���������� ������ ��� ���
      FLASH_READ_PAGE(page, (u8*)&stPageBuffer);
      if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) ||
          (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0))
         return FAIL;
   }

   return OK;
}

void WriteArchivePage()
{
   //��������� ��������� �������
   //������� ���������� �������
   stPageBuffer.ulPageID = stFlash.curr_id++;
   //������ ���������� ������ ���� ������ �� ��������
   stPageBuffer.ucStart = stFlash.data_shift;
   //������ ��
   stPageBuffer.uiCRC = get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC));

   if ((stFlash_Pointers.write_adr % SIZE_SUBSECTOR_FLASH) == 0)
   {
      //���� ����� � ����. ������ - ������� ��� � ��������� ���������
      FlashSubSectorEarse(stFlash_Pointers.write_adr);
      u32 adr = stFlash_Pointers.write_adr;
      for (u32 i = 0; i < FLASH_ARCHIVE_FREE_SUB_SEC_NUM; i++)
      {
         adr += SIZE_SUBSECTOR_FLASH;
         if (adr >= FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN)
         {
            adr = FLASH_ARCHIVE_START_PAGE * FLASH_PAGE_LEN;
         }
         FlashSubSectorEarse(adr);
      }
   }

   //����� ������
   FLASH_WRITE_PAGE(stFlash_Pointers.write_adr >> 8, stPageBuffer.data);
}

void ArchiveErase(void)
{
   FLASH_Take_Semaphore();
   stFlash_Pointers.start_adr = 0;
   stFlash_Pointers.read_adr = 0;
   stFlash_Pointers.write_adr = 0;
   stFlash.curr_id = 1;
   stFlash.curr_read = stFlash_Pointers.read_adr;
   stFlash.data_shift = stFlash.data_shift_tmp = 0;

   indx_data_buff = 0;
   SaveFlashPointers();

   ArciveFlashEarse();

   FLASH_Give_Semaphore();
}

void ArciveFlashEarse(void)
{
   for (uint64_t j = FLASH_ARCHIVE_START_PAGE; j < (FLASH_ARCHIVE_END_PAGE * 256); j += SIZE_SECTOR_FLASH)
   {
      IWDG_ReloadCounter();   // Reload IWDG counter
      FlashSectorEarse(j);
      osDelay(10);
   }
}

void FullFlashErase(void)
{
   FLASH_Take_Semaphore();
   FlashBulkErase();
   FLASH_Give_Semaphore();
}

/*
void TestFlash()
{
  u8 *ptr = (u8*)&stPageBuffer;
  u8 id[30];
  FlashReadID(id);
  
  FLASH_ERASE_SUBSECTOR(0);
  FLASH_ERASE_SUBSECTOR(1);
  FLASH_READ_PAGE(0,(u8*)&stPageBuffer);
  for (u16 i = 0; i < 256; i++)
    if (ptr[i] != 0xFF)
      while(1);
  
  //����� ������
  for (u16 i = 0; i < 256; i++)
  {
    ptr[i] = i;
    //EXT_FLASH_Write(&ptr[i], i,1);
  }
  FLASH_WRITE_PAGE(0, stPageBuffer.data);
  FLASH_WRITE_PAGE(16, stPageBuffer.data);
  
  FLASH_READ_PAGE(0,(u8*)&stPageBuffer);
  FLASH_READ_PAGE(16,(u8*)&stPageBuffer);
  
  FLASH_ERASE_SUBSECTOR(1);
  
  FLASH_READ_PAGE(0,(u8*)&stPageBuffer);
  FLASH_READ_PAGE(16,(u8*)&stPageBuffer);
  
  FLASH_READ_PAGE(0,(u8*)&stPageBuffer);
  //FLASH_WRITE_PAGE(16, stPageBuffer.data);
  FLASH_WRITE_PAGE(32, stPageBuffer.data);
  
  FLASH_READ_PAGE(0,(u8*)&stPageBuffer);
  FLASH_READ_PAGE(16,(u8*)&stPageBuffer);
  FLASH_READ_PAGE(32,(u8*)&stPageBuffer);
  
  for (u16 i = 0; i < 256; i++)
    if (ptr[i] != 0xFF)
      while(1);
}

void Flash_FS_Test()
{
  u8 type = 0;
  u8 data[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
  u8 len = 1;
  u32 dat_num = 0;
  u32 total_write = 0;
  u32 total_read = 0;
  u8 tmp_buf[900];
  
  while (stFlash.ucFlashInit != 1)
  {
    osDelay(1000);
  }
  //ArchiveErase();
  while (dat_num++ < 60000)
  {
    if (type == 0xFF)
      type++;
    while (Flash_QueryPacket(type,len, data) != OK)
    osDelay(3);
    total_write += len + 4 + 2;
    type++;
    len++;
    if (len > 30)
      len = 1;
  }
  
  osDelay(1000);
  s16 len_read;
  u16 read_len = 768;
  do
  {
    len_read = Flash_ReadData(read_len, tmp_buf);
    read_len -= 15;
    if (read_len < 67)
      read_len = 768;
    u8 *pdat = tmp_buf;
    if (len_read > 0)
    {
      Flash_DataSendOK();
      total_read += len_read;
    }
    while (len_read > 0)
    {
      u8 val = 0;
      //��������� �����
      for (u16 i = 0; i < pdat[1] + 2;i++)
      {
        if (i == 0)//���
          continue;
        if (i == 1)//�����
          continue;
        if (i <= 5)//�����
          continue;
        //������
        if (val++ != pdat[i])
          while(1)
            {i--;}
      }
      len_read -= pdat[1] + 2;
      pdat += pdat[1] + 2;
    }
  } while(Flash_DataLen() >= 128);
    
  if (total_write != total_read + indx_data_buff)//171802//167817//163797
    while (1);
}
*/

void FlashHandler()
{
   if (osMutexWait(sBinSemDATA_WRITE, NULL) == osOK)
   {
      while (indx_data_buff >= FLASH_PAGE_DATA_LEN)
      {
         FLASH_Take_Semaphore();
         while (Buffer_Take_Semaphore() != OK)
            ;
         //������� ������ �� ������
         memcpy(stPageBuffer.data, data_buff, FLASH_PAGE_DATA_LEN);
         //���������� ������ � ������ ������
         indx_data_buff -= FLASH_PAGE_DATA_LEN;
         memcpy(data_buff, data_buff + FLASH_PAGE_DATA_LEN, indx_data_buff);
         Buffer_Give_Semaphore();

         //����� �� ����.
         u16 cnt = 0;
         do
         {
            WriteArchivePage();
            if (CheckFlashPage(stFlash_Pointers.write_adr >> 8) == OK)
            {
               IncWriteCnt();
               break;
            }
            else
               IncWriteCnt();
         } while (cnt++ < PAGE_WRITE_READ_ATTEMPS);

         FLASH_Give_Semaphore();

         // DPS("-WRITE FLASH PAGE-\r\n");
      }
   }

   int cmd;
   if (xQueueReceive(xFlashQueue, &cmd, 0))
   {
      //������������ ������� ��� ������
      if (cmd == CMD_FLASH_FULL_ERASE)
      {
         FullFlashErase();
      }
      else if (cmd == CMD_FLASH_ARCHIVE_ERASE)
      {
         ArchiveErase();
      }
      else if (cmd > NULL && cmd < ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) /
                                    SIZE_SUBSECTOR_FLASH))   //�������� �� ��������� ����� ���������� flash
      {
         FLASH_Take_Semaphore();
         FlashSubSectorEarse(cmd * SIZE_SUBSECTOR_FLASH);
         FLASH_Give_Semaphore();
      }
   }
}

uint8_t gCountFlashData = 0;

void FlashInit()
{
#ifdef FM3
   gCountFlashData = 0;
   _Bool data_end = 0;

   for (uint32_t i = FLASH_ARCHIVE_START_PAGE; i < (FLASH_ARCHIVE_START_PAGE + MAX_LEN_FLASH * SIZE_RECORD_EXT_FLASH);
        i += SIZE_RECORD_EXT_FLASH)
   {
      /* ���������� ������� ������ ���� */
      FLASH_Take_Semaphore();
      EXT_FLASH_Read(data_buff, i, SIZE_RECORD_EXT_FLASH);
      FLASH_Give_Semaphore();

      data_end = 1;
      for (uint8_t n = 0; n < sizeof(uint64_t); n++)
      {
         if (data_buff[n] != 0xFF)
         {
            data_end = 0;
            gCountFlashData++;
            break;
         }
      }
      if (data_end)
         break;
   }
   DP_GSM("D_ARCHIVE POINT: %i\r\n", gCountFlashData);
#endif
#ifdef FM4
   InitFlashArchive();
#endif
}

uint8_t GetCountDataFlash(void)
{
   return gCountFlashData;
}

void SaveDataFm(uint8_t count_point, char* ptr, int len_data)
{
   /* ���������� ������������ ������ */
   if (count_point > MAX_LEN_FLASH)
   {
      DP_GSM("D_ARCH OVERFLOW\r\n");
      count_point = MAX_LEN_FLASH;
      /* �������� ����� ������ � ���������� ������� */
      uint32_t GetAddrCopy();
      uint32_t address_copy = GetAddrCopy();   //�������� ����� ����� ������

      /* ������� flash ����� ������ */
      void EraseCopy(uint32_t);
      EraseCopy(address_copy);

      /* ���������� ���� ������ �� �������� ������� flash � ����� �� ������������ */
      void OverflowArchive(uint32_t);
      OverflowArchive(address_copy);

      /* ������ ������� ������ ����� �������  */
      EraseArcive();

      /* ���������� ������ �� ����� flash � ����� */
      void CopyToArchive(uint32_t, uint8_t);
      CopyToArchive(address_copy, (count_point - 1));
   }

   uint32_t address = FLASH_ARCHIVE_START_PAGE + count_point * SIZE_RECORD_EXT_FLASH;
   uint8_t bitFree = 8;
   /* Save length */
   bit_packing(&ptr[SIZE_RECORD_EXT_FLASH - sizeof(uint8_t)], (uint8_t)len_data, &bitFree, 8);
   /* Save date */
   uint32_t SecRTC = time();
   bit_packing(&ptr[SIZE_RECORD_EXT_FLASH - sizeof(uint8_t) - sizeof(uint32_t)], SecRTC, &bitFree, 32);   // for debug

   RTC_t stDateArch;
   DP_GSM("D_ARCH SAVE DATE POIN: ");
   Sec2Date(&stDateArch, SecRTC);
   DP_GSM("%02d/", stDateArch.mday);
   DP_GSM("%02d/", stDateArch.month);
   DP_GSM("%02d ", stDateArch.year);
   DP_GSM("%02d:", stDateArch.hour);
   DP_GSM("%02d:", stDateArch.min);
   DP_GSM("%02d\r\n", stDateArch.sec);

   FLASH_Take_Semaphore();
   EXT_FLASH_Write((uint8_t*)ptr, address, SIZE_RECORD_EXT_FLASH);
   FLASH_Give_Semaphore();
}

int ReadDataFm(uint8_t count_point, char* ptr)
{
   if (count_point)
      count_point--;
   FLASH_Take_Semaphore();
   EXT_FLASH_Read((uint8_t*)ptr, FLASH_ARCHIVE_START_PAGE, SIZE_RECORD_EXT_FLASH);
   FLASH_Give_Semaphore();

   gCountFlashData--;
   if (!(gCountFlashData))
   {
      /* ������ ������� ������ */
      EraseArcive();
   }

   uint32_t len_data = 0;
   uint8_t bitFree = 8;
   /* Read length */
   bit_unpacking((uint8_t*)&ptr[SIZE_RECORD_EXT_FLASH - sizeof(uint8_t)], &len_data, &bitFree, 8);
   /* Read date */
   uint32_t SecRTC = 0;
   RTC_t stDateArch;
   bit_unpacking((uint8_t*)&ptr[SIZE_RECORD_EXT_FLASH - sizeof(uint8_t) - sizeof(uint32_t)], &SecRTC, &bitFree, 32);
   DP_GSM("D_ARCH DATE POIN: ");
   Sec2Date(&stDateArch, SecRTC);
   DP_GSM("%02d/", stDateArch.mday);
   DP_GSM("%02d/", stDateArch.month);
   DP_GSM("%02d ", stDateArch.year);
   DP_GSM("%02d:", stDateArch.hour);
   DP_GSM("%02d:", stDateArch.min);
   DP_GSM("%02d\r\n", stDateArch.sec);

   uint32_t GetAddrCopy();
   uint32_t address_copy = GetAddrCopy();   //�������� ����� ����� ������

   /* ������� flash ����� ������ */
   void EraseCopy(uint32_t);
   EraseCopy(address_copy);
   if (gCountFlashData)
   {   //���������� ����� ���� � ��� ���� ������.
      /* ���������� ������ �� �������� ������� flash � ����� */
      void ArchiveToCopy(uint32_t, uint8_t);
      ArchiveToCopy(address_copy, count_point);

      /* ������� ������ ����� �� ������ My iRZ ����� ������� */
      EraseArcive();

      /* ���������� ������ �� ����� flash � ����� */
      void CopyToArchive(uint32_t, uint8_t);
      CopyToArchive(address_copy, count_point);
   }
   return len_data;
}

static void CopyToArchive(uint32_t address_copy, uint8_t count_point)
{
   uint32_t address = FLASH_ARCHIVE_START_PAGE;
   for (uint8_t i = 0; i < count_point; i++)
   {
      uint8_t TempBuf[SIZE_RECORD_EXT_FLASH] = { 0 };
      FLASH_Take_Semaphore();
      EXT_FLASH_Read(TempBuf, address_copy, SIZE_RECORD_EXT_FLASH);
      EXT_FLASH_Write(TempBuf, address, SIZE_RECORD_EXT_FLASH);
      FLASH_Give_Semaphore();
      address += SIZE_RECORD_EXT_FLASH;
      address_copy += SIZE_RECORD_EXT_FLASH;
   }
}

/* ret: ����� �� flash ����� ������ */
static uint32_t GetAddrCopy(void)
{
   uint32_t address_copy = 0;   //����� ����� ������.
   /* ��������� ����� ����� ������ */
   for (uint32_t addr = FLASH_ARCHIVE_START_PAGE;
        addr < (FLASH_ARCHIVE_START_PAGE + MAX_LEN_FLASH * SIZE_RECORD_EXT_FLASH);
        addr += SIZE_SUBSECTOR_FLASH)
   {
      address_copy = addr;
   }
   return address_copy + SIZE_SUBSECTOR_FLASH;
}

/* ���������� ������ �� ����� ������� flash � ����� */
static void ArchiveToCopy(uint32_t address_copy, uint8_t count_point)
{
   /* ����������� ����� */
   uint32_t address = FLASH_ARCHIVE_START_PAGE + SIZE_RECORD_EXT_FLASH;
   for (uint8_t i = 0; i < count_point; i++)
   {
      uint8_t TempBuf[SIZE_RECORD_EXT_FLASH] = { 0 };
      FLASH_Take_Semaphore();
      EXT_FLASH_Read(TempBuf, address, SIZE_RECORD_EXT_FLASH);
      EXT_FLASH_Write(TempBuf, address_copy, SIZE_RECORD_EXT_FLASH);
      FLASH_Give_Semaphore();
      address += SIZE_RECORD_EXT_FLASH;
      address_copy += SIZE_RECORD_EXT_FLASH;
   }
}

/* ���������� ����� ������ ����� ��������� ����� ������ */
static void OverflowArchive(uint32_t address_copy)
{
   /* ����������� ����� */
   for (uint8_t addr = 1; addr < MAX_LEN_FLASH; addr++)
   {
      uint32_t address = FLASH_ARCHIVE_START_PAGE + SIZE_RECORD_EXT_FLASH * addr;
      uint8_t TempBuf[SIZE_RECORD_EXT_FLASH] = { 0 };
      FLASH_Take_Semaphore();
      EXT_FLASH_Read(TempBuf, address, SIZE_RECORD_EXT_FLASH);
      EXT_FLASH_Write(TempBuf, address_copy, SIZE_RECORD_EXT_FLASH);
      FLASH_Give_Semaphore();
      address += SIZE_RECORD_EXT_FLASH;
      address_copy += SIZE_RECORD_EXT_FLASH;
   }
}

/* ������� flash ����� ������ */
static void EraseCopy(uint32_t address_copy)
{
   for (uint32_t addr = address_copy; addr < (address_copy + MAX_LEN_FLASH * SIZE_RECORD_EXT_FLASH);
        addr += SIZE_SUBSECTOR_FLASH)
   {
      FLASH_Take_Semaphore();
      FlashSubSectorEarse(addr);
      FLASH_Give_Semaphore();
      osDelay(10);
   }
}

/* ������� ������ ����� �� ������ My iRZ */
void EraseArcive(void)
{
   for (uint32_t addr = FLASH_ARCHIVE_START_PAGE;
        addr < (FLASH_ARCHIVE_START_PAGE + MAX_LEN_FLASH * SIZE_RECORD_EXT_FLASH);
        addr += SIZE_SUBSECTOR_FLASH)
   {
      IWDG_ReloadCounter();   // Reload IWDG counter
      FlashSubSectorEarse(addr);
      osDelay(10);
   }
}

void OnChangeArchiveErase(void)
{
   if (xFlashQueue != NULL)
   {
      int tmp = CMD_FLASH_ARCHIVE_ERASE;
      xQueueSend(xFlashQueue, &tmp, 500);
   }
}

void OnChangeFlashErase()
{
   if (xFlashQueue != NULL)
   {
      int tmp = CMD_FLASH_FULL_ERASE;
      xQueueSend(xFlashQueue, &tmp, 500);
   }
}