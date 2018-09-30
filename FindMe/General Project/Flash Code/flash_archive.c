#include "includes.h"
#include "flash_archive.h"

//буфер для сохранения архива
#define FLASH_DATA_BUFF_LEN (FLASH_PAGE_DATA_LEN + 300)
static u8 data_buff[FLASH_DATA_BUFF_LEN];
u16 indx_data_buff;

//образ страницы флеш для чтения-записи
static TFlashPage stPageBuffer;

TFlash stFlash;
//адреса начала, конца и переданных данных (копия лежит в регистрах RTC BACKUP)
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

   //сохраняем текущий write protect для регистров backup
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

   //сохраняем структуру
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

   //читаем данные
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

//определяет сколько байт данных записано на странице
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
   //читаем всю флеш и ищем там архив
   //ищем начало архива
   for (u16 i = FLASH_ARCHIVE_START_PAGE; i < FLASH_ARCHIVE_END_PAGE; i++)
   {
      //читаем только id
      EXT_FLASH_Read((u8*)&stPageBuffer.ulPageID, (i << 8) + 249, 4);

      if (min_id > stPageBuffer.ulPageID)
      {
         //проверяем КС страницы
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
   //ищем конец архива
   for (int i = FLASH_ARCHIVE_END_PAGE - 1; i >= FLASH_ARCHIVE_START_PAGE; i--)
   {
      //читаем только id
      EXT_FLASH_Read((u8*)&stPageBuffer.ulPageID, (i << 8) + 249, 4);

      // if (stPageBuffer.ulPageID == FLASH_TMP_PAGE_ID)
      //  tmp_page = i;

      if (max_id < stPageBuffer.ulPageID && stPageBuffer.ulPageID != 0xFFFFFFFF)
      {
         //проверяем КС страницы
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
      //чистая флешка
      min_page = FLASH_ARCHIVE_START_PAGE;
      max_page = FLASH_ARCHIVE_START_PAGE;
      // tmp_page = FLASH_ARCHIVE_START_PAGE+1;
      stFlash.curr_id = 1;
   }
   else
   {
      //флешка с данными
      //номер первой свободной страницы
      if (++max_page >= FLASH_ARCHIVE_END_PAGE)
         max_page = FLASH_ARCHIVE_START_PAGE;

      stFlash.curr_id = max_id + 1;
   }

   //инициализируем адреса
   stFlash_Pointers.start_adr = min_page << 8;
   stFlash_Pointers.read_adr = min_page << 8;
   stFlash_Pointers.write_adr = max_page << 8;

   SaveFlashPointers();
   //}

   //инициализируем временный буфер
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

   u32 write_sub_sector = stop_adr >> 12;   //текущий подсектор, куда пишутся данные
   u32 read_sub_sector =
      write_sub_sector + FLASH_ARCHIVE_FREE_SUB_SEC_NUM + 1;   //минимальный номер подсектора для чтения
   if (read_sub_sector >= ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) / SIZE_SUBSECTOR_FLASH))
   {
      read_sub_sector -= ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) / SIZE_SUBSECTOR_FLASH);
   }

   //проверяем допустимое значение адреса чтения
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

формирует пакет для записи и кладет его в буфер
возврашает FAIL если в буфере нет места. OK
в случае успеха.

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

   //проверим есть ли свободное место в буфере
   if (len > FLASH_DATA_BUFF_LEN - indx_data_buff)
      return FAIL;

   if (Buffer_Take_Semaphore() != OK)
      return FAIL;

   //формируем пакет
   // data_buff[indx_data_buff++] = type;   //тип
   // data_buff[indx_data_buff++] = len + TIME_LEN;    //длина
   // data_buff[indx_data_buff++] = *ptime++;//время
   // data_buff[indx_data_buff++] = *ptime++;
   // data_buff[indx_data_buff++] = *ptime++;
   // data_buff[indx_data_buff++] = *ptime++;
   //данные
   for (u16 i = 0; i < len; i++)
   {
      data_buff[indx_data_buff++] = pdata[i];
   }

   //определяем уменьшилось ли кол-во данных в буфере, если да, тогда мы сохранили страницу во флеш
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
      //генерируем событие сохранения данных во флеш
      stFlash.data_shift = stFlash.data_shift_tmp;
      stFlash.data_shift_tmp = indx_data_buff % FLASH_PAGE_DATA_LEN;
      //команда на запись
      xSemaphoreGive(sBinSemDATA_WRITE);
   }
   else
   {
      //запись во временный буфер
   }
   Buffer_Give_Semaphore();

   return OK;
}

//число байт которые можно вычитать из архива
u32 CurrReadLen()
{
   if (stFlash_Pointers.write_adr >= stFlash.curr_read)
   {
      return stFlash_Pointers.write_adr - stFlash.curr_read;
   }

   return (FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN - stFlash.curr_read) + stFlash_Pointers.write_adr;
}

//примерное число данных для передачи на сервер
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
            //обновляем адрес чтения
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

вычитывает данные из флеш заданной длины.
Возврашает FAIL если данных для передачи нет,
возврашает длину буфера в случае успеха

************************************************/
s16 Flash_ReadData(u8* pbuf, u16 max_len)
{
   if (stFlash.ucFlashInit == 0)
   {
      return FAIL;
   }

   FLASH_Take_Semaphore();

   //всегда читаем сначала
   stFlash.curr_read = stFlash_Pointers.read_adr;

   s16 len = 0;
   u16 rec_len = 0;
   u16 indx = 0;
   u8* pdata = 0;
   u32 page_num = stFlash.curr_read / FLASH_PAGE_LEN;   //адрес страницы
   u16 page_shift = stFlash.curr_read % FLASH_PAGE_LEN;   //смещение внутри страницы

   //проверим есть ли данные в архиве
   if (CurrReadLen() == 0)
   {
      FLASH_Give_Semaphore();
      return FAIL;
   }

   //грузим первую целую страницу
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
   //проверим что по данному смещению лежит начало записи
   //смещение может поехать если на флеш вылетят страницы
   while (indx < page_shift - 1)
   {
      if (pdata[indx] == NAVIGATIONAL_PACKET || pdata[indx] == NAVIGATIONAL_PACKET_REAL_TIME)
         rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
      else
         rec_len = pdata[indx + 1] + TYPE_LEN + 1;
      indx += rec_len;
   }
   //неверный адрес для чтения - берем данные с начала страницы
   if (indx != page_shift)
      page_shift = stPageBuffer.ucStart;

   indx = page_shift;
   u8 rec_tp = 0xFF;
   u8 rec_pos = 0;
   rec_len = 0;
   u8 read_len = indx;

   do
   {
      //проверяем нужно ли подгрузить след. страницу
      if (indx >= FLASH_PAGE_DATA_LEN)
      {
         stFlash.curr_read = page_num * FLASH_PAGE_LEN + read_len;

         if (CurrReadLen() < FLASH_PAGE_LEN)
            break;

         indx = 0;
         read_len = 0;
         //подгружаем след. страницу из флешки
         if (++page_num >= FLASH_ARCHIVE_END_PAGE)
            page_num = FLASH_ARCHIVE_START_PAGE;
         stFlash.curr_read = page_num * FLASH_PAGE_LEN;
         if (GetPage(&page_num) != OK)
         {
            //куча невалидных страниц обновим счетчики что бы в след. раз не читать по новой
            stFlash.curr_read = page_num * FLASH_PAGE_LEN;
            stFlash_Pointers.read_adr = stFlash.curr_read;
            SaveFlashPointers();
            break;
         }

         //проверим что на странице есть недоставющие данные (если запись лежит на двух страницах)
         if (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME)
            rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
         else if (rec_pos == 1)
            rec_len = pdata[indx] + TYPE_LEN + 1;
         if (rec_pos && stPageBuffer.ucStart != rec_len - rec_pos)
         {
            //данных нет - удалим первую половину записи
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
            //читаем с начала след. страницы
            indx = stPageBuffer.ucStart;
         }
      }

      //обрабатываем данные
      if (rec_pos == 0)
      {
         rec_tp = pdata[indx];   //тип записи
         if (rec_tp == NAVIGATIONAL_PACKET || rec_tp == NAVIGATIONAL_PACKET_REAL_TIME)
            rec_len = LEN_NAVIGATIONAL_PACKET_REAL_TIME;
         else
            rec_len = pdata[indx + 1] + TYPE_LEN + 1;
      }
      else if (rec_pos == 1 && rec_tp != NAVIGATIONAL_PACKET && rec_tp != NAVIGATIONAL_PACKET_REAL_TIME)
         rec_len = pdata[indx] + TYPE_LEN + 1;   //длина всей записи
      else
      {
         //копируем запись - она помещается в буфер
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
         //прочитали текущую запись - переходим к след.
         if (rec_pos == rec_len - 1)
         {
            len += rec_len;
            rec_len = 0;
            rec_pos = 0;
            rec_tp = 0xFF;
            indx++;
            //сохраняем сколько мы прочитали в тек странице
            read_len = indx;
            continue;
         }
      }

      indx++;
      rec_pos++;
   } while (len + rec_len <= max_len);

   //запомним адрес где мы остановились
   stFlash.curr_read = page_num * FLASH_PAGE_LEN + read_len;

   FLASH_Give_Semaphore();

   return len;
}

void Flash_DataSendOK()
{
   FLASH_Take_Semaphore();
   //проверяем нужно ли стирать блок на флеш
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

//проверяем целостность данных на странице во флеш
s8 CheckFlashPage(u32 page)
{
   FLASH_READ_PAGE(page, (u8*)&stPageBuffer);
   if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) ||
       (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0))
   {
      //перечитаем читаем еще раз
      FLASH_READ_PAGE(page, (u8*)&stPageBuffer);
      if (stPageBuffer.uiCRC != get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC)) ||
          (stPageBuffer.uiCRC == 0 && stPageBuffer.ulPageID == 0))
         return FAIL;
   }

   return OK;
}

void WriteArchivePage()
{
   //заполняем заголовок сраницы
   //счетчик записанных страниц
   stPageBuffer.ulPageID = stFlash.curr_id++;
   //откуда начинается первый блок данных на странице
   stPageBuffer.ucStart = stFlash.data_shift;
   //ставим КС
   stPageBuffer.uiCRC = get_cs16((uint8_t*)&stPageBuffer, FLASH_PAGE_LEN - sizeof(stPageBuffer.uiCRC));

   if ((stFlash_Pointers.write_adr % SIZE_SUBSECTOR_FLASH) == 0)
   {
      //если пишем в след. сектор - стираем его и несколько следующих
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

   //пишем данные
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
  
  //пишем данные
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
      //проверяем пакет
      for (u16 i = 0; i < pdat[1] + 2;i++)
      {
        if (i == 0)//тип
          continue;
        if (i == 1)//длина
          continue;
        if (i <= 5)//время
          continue;
        //данные
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
         //заберем данные из буфера
         memcpy(stPageBuffer.data, data_buff, FLASH_PAGE_DATA_LEN);
         //оставщиеся данные в начало буфера
         indx_data_buff -= FLASH_PAGE_DATA_LEN;
         memcpy(data_buff, data_buff + FLASH_PAGE_DATA_LEN, indx_data_buff);
         Buffer_Give_Semaphore();

         //пишем во флеш.
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
      //обрабатываем команды для флешки
      if (cmd == CMD_FLASH_FULL_ERASE)
      {
         FullFlashErase();
      }
      else if (cmd == CMD_FLASH_ARCHIVE_ERASE)
      {
         ArchiveErase();
      }
      else if (cmd > NULL && cmd < ((FLASH_ARCHIVE_END_PAGE * FLASH_PAGE_LEN) /
                                    SIZE_SUBSECTOR_FLASH))   //проверка на коректный адрес субсектора flash
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
      /* Определяем сколько данных есть */
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
   /* Обработчик переполнения архива */
   if (count_point > MAX_LEN_FLASH)
   {
      DP_GSM("D_ARCH OVERFLOW\r\n");
      count_point = MAX_LEN_FLASH;
      /* Создание копии архива с последними данными */
      uint32_t GetAddrCopy();
      uint32_t address_copy = GetAddrCopy();   //получаем адрес копии архива

      /* отчиска flash копии архива */
      void EraseCopy(uint32_t);
      EraseCopy(address_copy);

      /* Перегрузка всех данных из архивной области flash в копию по переполнению */
      void OverflowArchive(uint32_t);
      OverflowArchive(address_copy);

      /* Полная отчиска архива перед записью  */
      EraseArcive();

      /* Перегрузка данных из копии flash в архив */
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
      /* Полная отчиска архива */
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
   uint32_t address_copy = GetAddrCopy();   //получаем адрес копии архива

   /* отчиска flash копии архива */
   void EraseCopy(uint32_t);
   EraseCopy(address_copy);
   if (gCountFlashData)
   {   //Обработаем архив если в нем есть данные.
      /* Перегрузка данных из архивной области flash в копию */
      void ArchiveToCopy(uint32_t, uint8_t);
      ArchiveToCopy(address_copy, count_point);

      /* Отчиска архива маяка на сервер My iRZ перед записью */
      EraseArcive();

      /* Перегрузка данных из копии flash в архив */
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

/* ret: Адрес во flash копии архива */
static uint32_t GetAddrCopy(void)
{
   uint32_t address_copy = 0;   //Адрес копии архива.
   /* вычисляем адрес копии архива */
   for (uint32_t addr = FLASH_ARCHIVE_START_PAGE;
        addr < (FLASH_ARCHIVE_START_PAGE + MAX_LEN_FLASH * SIZE_RECORD_EXT_FLASH);
        addr += SIZE_SUBSECTOR_FLASH)
   {
      address_copy = addr;
   }
   return address_copy + SIZE_SUBSECTOR_FLASH;
}

/* Перегрузка данных из одной области flash в копию */
static void ArchiveToCopy(uint32_t address_copy, uint8_t count_point)
{
   /* Копирование копии */
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

/* Перегрузка всего архива кроме последней точки архива */
static void OverflowArchive(uint32_t address_copy)
{
   /* Копирование копии */
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

/* отчиска flash копии архива */
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

/* Отчиска архива маяка на сервер My iRZ */
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