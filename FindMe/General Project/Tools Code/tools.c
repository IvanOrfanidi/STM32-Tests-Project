

#include "includes.h"
#include "tools.h"

int bit_packing(char* pOut, uint32_t inp, uint8_t* bitFree, uint8_t bitUse)
{
   uint8_t n = 0;
   uint8_t mask = 0;

   if (*bitFree == 8)
      pOut[0] = 0;

   do
   {
      if (*bitFree < bitUse)
      {
         // количество оставшихся свободных битов меньше чем имеется для упаковки
         switch (*bitFree)
         {
         case 1:
            mask = 0x01;
            break;
         case 2:
            mask = 0x03;
            break;
         case 3:
            mask = 0x07;
            break;
         case 4:
            mask = 0x0F;
            break;
         case 5:
            mask = 0x1F;
            break;
         case 6:
            mask = 0x3F;
            break;
         case 7:
            mask = 0x7F;
            break;
         case 8:
            mask = 0xFF;
            break;
         }
         pOut[n] |= ((inp & mask) << (8 - *bitFree));
         inp >>= (*bitFree);
         bitUse -= *bitFree;
         *bitFree = 0;
      }
      else
      {
         // количество оставшихся свободных битов больше чем имеется для упаковки
         switch (bitUse)
         {
         case 1:
            mask = 0x01;
            break;
         case 2:
            mask = 0x03;
            break;
         case 3:
            mask = 0x07;
            break;
         case 4:
            mask = 0x0F;
            break;
         case 5:
            mask = 0x1F;
            break;
         case 6:
            mask = 0x3F;
            break;
         case 7:
            mask = 0x7F;
            break;
         case 8:
            mask = 0xFF;
            break;
         }
         pOut[n] |= ((inp & mask) << (8 - *bitFree));
         inp >>= (*bitFree);
         *bitFree -= bitUse;
         bitUse = 0;
      }

      if (*bitFree == 0)
      {
         *bitFree = 8;
         n++;
         if (bitUse > 0)
            pOut[n] = 0;
      }

   } while (bitUse > 0);

   return n;
}

int bit_unpacking(uint8_t* inp, uint32_t* out, uint8_t* bitExist, uint8_t bitTotal)
{
   uint8_t n = 0;
   uint8_t mask = 0;
   uint32_t tmp;
   uint8_t shiftLeft;
   uint8_t shiftRight = 0;

   *out = 0;

   do
   {
      if (*bitExist < bitTotal)
      {
         // количество оставшихся свободных битов меньше чем имеется для упаковки
         switch (*bitExist)
         {
         case 1:
            mask = 0x80;
            break;
         case 2:
            mask = 0xC0;
            break;
         case 3:
            mask = 0xE0;
            break;
         case 4:
            mask = 0xF0;
            break;
         case 5:
            mask = 0xF8;
            break;
         case 6:
            mask = 0xFC;
            break;
         case 7:
            mask = 0xFE;
            break;
         case 8:
            mask = 0xFF;
            break;
         }

         shiftLeft = 8 - *bitExist;
         tmp = inp[n] & mask;

         if (shiftLeft > shiftRight)
         {
            tmp >>= (shiftLeft - shiftRight);
         }
         else if (shiftLeft < shiftRight)
         {
            tmp <<= (shiftRight - shiftLeft);
         }
         *out |= tmp;

         bitTotal -= *bitExist;
         shiftRight += *bitExist;
         *bitExist = 0;
      }
      else
      {
         // количество оставшихся свободных битов больше чем имеется для упаковки
         switch (bitTotal)
         {
         case 1:
            mask = 0x01;
            break;
         case 2:
            mask = 0x03;
            break;
         case 3:
            mask = 0x07;
            break;
         case 4:
            mask = 0x0F;
            break;
         case 5:
            mask = 0x1F;
            break;
         case 6:
            mask = 0x3F;
            break;
         case 7:
            mask = 0x7F;
            break;
         case 8:
            mask = 0xFF;
            break;
         }
         shiftLeft = 8 - *bitExist;
         tmp = (inp[n] >> shiftLeft) & mask;
         tmp <<= shiftRight;

         *out |= tmp;

         *bitExist -= bitTotal;
         shiftRight += bitTotal;
         bitTotal = 0;
      }

      if (*bitExist == 0)
      {
         *bitExist = 8;
         n++;
      }

   } while (bitTotal > 0);

   return n;
}

// Delay 1sec = 10000
#pragma optimize = none
#define DLY_100US 260
void DelayResolution100us(uint32_t Dly)
{
   for (; Dly; Dly--)
   {
      for (volatile uint32_t j = DLY_100US; j; j--)   // 7 operations per cycle
      {
      }
      IWDG_ReloadCounter();
   }
}