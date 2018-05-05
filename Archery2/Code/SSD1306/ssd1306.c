/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>
   ----------------------------------------------------------------------
      Copyright (C) Alexander Lutsai, 2016
    Copyright (C) Tilen Majerle, 2015
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 */

#include "includes.h"

#include "ssd1306.h"

/* Write command */
#define SSD1306_WRITECOMMAND(command) \
   ssd1306_I2C_Write(SSD1306_I2C, SSD1306_I2C_ADDR, 0x00, (command)); \
   loop(100)
/* Write data */
#define SSD1306_WRITEDATA(data) ssd1306_I2C_Write(SSD1306_I2C, SSD1306_I2C_ADDR, 0x40, (data))
/* Absolute value */
#define ABS(x) ((x) > 0 ? (x) : -(x))

/* SSD1306 data buffer */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0,
   0xF8, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x00, 0xFF,
#if (SSD1306_HEIGHT * SSD1306_WIDTH > 96 * 16)
   0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0xFF, 0xFF,
   0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x80, 0x00, 0x00, 0x8C, 0x8E, 0x84, 0x00, 0x00, 0x80, 0xF8, 0xF8, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
   0xE0, 0xE0, 0xC0, 0x80, 0x00, 0xE0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xC7, 0x01, 0x01, 0x01, 0x01,
   0x83, 0xFF, 0xFF, 0x00, 0x00, 0x7C, 0xFE, 0xC7, 0x01, 0x01, 0x01, 0x01, 0x83, 0xFF, 0xFF, 0xFF, 0x00, 0x38, 0xFE,
   0xC7, 0x83, 0x01, 0x01, 0x01, 0x83, 0xC7, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0xFF, 0xFF,
   0x07, 0x01, 0x01, 0x01, 0x00, 0x00, 0x7F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF,
   0xFF, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x03, 0x0F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x8F, 0x8F,
   0x9F, 0xBF, 0xFF, 0xFF, 0xC3, 0xC0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
   0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x03, 0x03, 0x00, 0x00,
   0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03,
   0x01, 0x01, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x03,
   0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#   if (SSD1306_HEIGHT == 64)
   0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x0F, 0x87, 0xC7, 0xF7,
   0xFF, 0xFF, 0x1F, 0x1F, 0x3D, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0x7C, 0x7D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xC0,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFE, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xF8, 0xF8,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xF0, 0xF8,
   0x1C, 0x0E, 0x06, 0x06, 0x06, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFE,
   0xFC, 0x00, 0x18, 0x3C, 0x7E, 0x66, 0xE6, 0xCE, 0x84, 0x00, 0x00, 0x06, 0xFF, 0xFF, 0x06, 0x06, 0xFC, 0xFE, 0xFC,
   0x0C, 0x06, 0x06, 0x06, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0xC0, 0xF8, 0xFC, 0x4E, 0x46, 0x46, 0x46, 0x4E, 0x7C,
   0x78, 0x40, 0x18, 0x3C, 0x76, 0xE6, 0xCE, 0xCC, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F,
   0x1F, 0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00,
   0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0C, 0x18, 0x18, 0x0C,
   0x06, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x0F, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07, 0x01, 0x00, 0x04, 0x0E, 0x0C,
   0x18, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x07, 0x07, 0x0C, 0x0C, 0x18, 0x1C, 0x0C, 0x06, 0x06, 0x00, 0x04, 0x0E, 0x0C,
   0x18, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#   endif
#endif
};

/* Private SSD1306 structure */
typedef struct
{
   uint16_t CurrentX;
   uint16_t CurrentY;
   uint8_t Inverted;
   uint8_t Initialized;
} SSD1306_t;

/* Private variable */
static SSD1306_t SSD1306;

uint8_t SSD1306_Init(void)
{
   /* Init I2C */
   ssd1306_I2C_Init();

   /* Check if LCD connected to I2C */
   if (!(ssd1306_I2C_IsDeviceConnected(SSD1306_I2C, SSD1306_I2C_ADDR)))
   {
      /* Return false */
      return 0;
   }

   /* A little delay */
   uint16_t p = 2500;
   while (p > 0)
   {
      p--;
   }

   /* Init LCD */
   SSD1306_WRITECOMMAND(SSD1306_DISPLAYOFF);   // 0xAE
   SSD1306_WRITECOMMAND(SSD1306_SETDISPLAYCLOCKDIV);   // 0xD5
   SSD1306_WRITECOMMAND(0x80);   // the suggested ratio 0x80

   SSD1306_WRITECOMMAND(SSD1306_SETMULTIPLEX);   // 0xA8
   SSD1306_WRITECOMMAND(SSD1306_HEIGHT - 1);

   SSD1306_WRITECOMMAND(SSD1306_SETDISPLAYOFFSET);   // 0xD3
   SSD1306_WRITECOMMAND(SSD1306_SETLOWCOLUMN);   // no offset
   SSD1306_WRITECOMMAND(SSD1306_SETSTARTLINE | 0x0);   // line #0
   SSD1306_WRITECOMMAND(SSD1306_CHARGEPUMP);   // SSD1306_CHARGEPUMP

   SSD1306_WRITECOMMAND(SSD1306_SETHIGHCOLUMN);
   // SSD1306_WRITECOMMAND(0x14);

   SSD1306_WRITECOMMAND(SSD1306_MEMORYMODE);   // 0x20
   SSD1306_WRITECOMMAND(SSD1306_SETLOWCOLUMN);   // 0x0 act like ks0108
   SSD1306_WRITECOMMAND(SSD1306_SEGREMAP | 0x1);
   SSD1306_WRITECOMMAND(SSD1306_COMSCANDEC);

#if defined SSD1306_128_32
   SSD1306_WRITECOMMAND(SSD1306_SETCOMPINS);   // 0xDA
   SSD1306_WRITECOMMAND(0x02);
   SSD1306_WRITECOMMAND(SSD1306_SETCONTRAST);   // 0x81
   SSD1306_WRITECOMMAND(0x8F);

#elif defined SSD1306_128_64
   SSD1306_WRITECOMMAND(SSD1306_SETCOMPINS);   // 0xDA
   SSD1306_WRITECOMMAND(0x12);
   SSD1306_WRITECOMMAND(SSD1306_SETCONTRAST);   // 0x81

   SSD1306_WRITECOMMAND(0x9F);
   // SSD1306_WRITECOMMAND(0xCF);

#elif defined SSD1306_96_16
   SSD1306_WRITECOMMAND(SSD1306_SETCOMPINS);   // 0xDA
   SSD1306_WRITECOMMAND(0x2);   // ada x12
   SSD1306_WRITECOMMAND(SSD1306_SETCONTRAST);   // 0x81
   SSD1306_WRITECOMMAND(SSD1306_SETHIGHCOLUMN);
   // SSD1306_WRITECOMMAND(0xAF);
#endif

   SSD1306_WRITECOMMAND(SSD1306_SETPRECHARGE);   // 0xd9
   SSD1306_WRITECOMMAND(SSD1306_PAGEADDR);
   // SSD1306_WRITECOMMAND(0xF1);

   SSD1306_WRITECOMMAND(SSD1306_SETVCOMDETECT);   // 0xDB
   SSD1306_WRITECOMMAND(SSD1306_SEGREMAP);
   SSD1306_WRITECOMMAND(SSD1306_DISPLAYALLON_RESUME);   // 0xA4
   SSD1306_WRITECOMMAND(SSD1306_NORMALDISPLAY);   // 0xA6

   SSD1306_WRITECOMMAND(SSD1306_DEACTIVATE_SCROLL);

   SSD1306_WRITECOMMAND(SSD1306_DISPLAYON);   //--turn on oled panel

   /* Clear screen */
   SSD1306_Fill(SSD1306_COLOR_BLACK);

   /* Update screen */
   SSD1306_UpdateScreen();

   /* Set default values */
   SSD1306.CurrentX = 0;
   SSD1306.CurrentY = 0;

   /* Initialized OK */
   SSD1306.Initialized = 1;

   /* Return OK */
   return 1;
}

void SSD1306_UpdateScreen(void)
{
   for (uint8_t i = 0; i < 8; i++)
   {
      SSD1306_WRITECOMMAND(0xB0 + i);
      SSD1306_WRITECOMMAND(SSD1306_SETLOWCOLUMN);
      SSD1306_WRITECOMMAND(SSD1306_SETHIGHCOLUMN);

      /* Write multi data */
      ssd1306_I2C_WriteMulti(SSD1306_I2C, SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
   }
}

void SSD1306_ToggleInvert(void)
{
   /* Toggle invert */
   SSD1306.Inverted = !SSD1306.Inverted;

   /* Do memory toggle */
   for (uint16_t i = 0; i < sizeof(SSD1306_Buffer); i++)
   {
      SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
   }
}

void SSD1306_Fill(SSD1306_COLOR_t color)
{
   /* Set memory */
   memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color)
{
   if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
   {
      /* Error */
      return;
   }

   /* Check if pixels are inverted */
   if (SSD1306.Inverted)
   {
      color = (SSD1306_COLOR_t)!color;
   }

   /* Set color */
   if (color == SSD1306_COLOR_WHITE)
   {
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y & 7);
   }
   else
   {
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
   }
}

void SSD1306_GotoXY(uint16_t x, uint16_t y)
{
   /* Set write pointers */
   SSD1306.CurrentX = x;
   SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color)
{
   uint32_t i, b, j;

   /* Check available space in LCD */
   if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight))
   {
      /* Error */
      return 0;
   }

   /* Go through font */
   for (i = 0; i < Font->FontHeight; i++)
   {
      b = Font->data[(ch - 32) * Font->FontHeight + i];
      for (j = 0; j < Font->FontWidth; j++)
      {
         if ((b << j) & 0x8000)
         {
            SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)color);
         }
         else
         {
            SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
         }
      }
   }

   /* Increase pointer */
   SSD1306.CurrentX += Font->FontWidth;

   /* Return character written */
   return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color)
{
   /* Write characters */
   while (*str)
   {
      /* Write character by character */
      if (SSD1306_Putc(*str, Font, color) != *str)
      {
         /* Return error */
         return *str;
      }

      /* Increase string pointer */
      str++;
   }

   /* Everything OK, zero should be returned */
   return *str;
}

void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c)
{
   int16_t dx, dy, sx, sy, err, e2, i, tmp;

   /* Check for overflow */
   if (x0 >= SSD1306_WIDTH)
   {
      x0 = SSD1306_WIDTH - 1;
   }
   if (x1 >= SSD1306_WIDTH)
   {
      x1 = SSD1306_WIDTH - 1;
   }
   if (y0 >= SSD1306_HEIGHT)
   {
      y0 = SSD1306_HEIGHT - 1;
   }
   if (y1 >= SSD1306_HEIGHT)
   {
      y1 = SSD1306_HEIGHT - 1;
   }

   dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
   dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
   sx = (x0 < x1) ? 1 : -1;
   sy = (y0 < y1) ? 1 : -1;
   err = ((dx > dy) ? dx : -dy) / 2;

   if (dx == 0)
   {
      if (y1 < y0)
      {
         tmp = y1;
         y1 = y0;
         y0 = tmp;
      }

      if (x1 < x0)
      {
         tmp = x1;
         x1 = x0;
         x0 = tmp;
      }

      /* Vertical line */
      for (i = y0; i <= y1; i++)
      {
         SSD1306_DrawPixel(x0, i, c);
      }

      /* Return from function */
      return;
   }

   if (dy == 0)
   {
      if (y1 < y0)
      {
         tmp = y1;
         y1 = y0;
         y0 = tmp;
      }

      if (x1 < x0)
      {
         tmp = x1;
         x1 = x0;
         x0 = tmp;
      }

      /* Horizontal line */
      for (i = x0; i <= x1; i++)
      {
         SSD1306_DrawPixel(i, y0, c);
      }

      /* Return from function */
      return;
   }

   while (1)
   {
      SSD1306_DrawPixel(x0, y0, c);
      if (x0 == x1 && y0 == y1)
      {
         break;
      }
      e2 = err;
      if (e2 > -dx)
      {
         err -= dy;
         x0 += sx;
      }
      if (e2 < dy)
      {
         err += dx;
         y0 += sy;
      }
   }
}

void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c)
{
   /* Check input parameters */
   if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
   {
      /* Return error */
      return;
   }

   /* Check width and height */
   if ((x + w) >= SSD1306_WIDTH)
   {
      w = SSD1306_WIDTH - x;
   }
   if ((y + h) >= SSD1306_HEIGHT)
   {
      h = SSD1306_HEIGHT - y;
   }

   /* Draw 4 lines */
   SSD1306_DrawLine(x, y, x + w, y, c); /* Top line */
   SSD1306_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
   SSD1306_DrawLine(x, y, x, y + h, c); /* Left line */
   SSD1306_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}

void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c)
{
   uint8_t i;

   /* Check input parameters */
   if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
   {
      /* Return error */
      return;
   }

   /* Check width and height */
   if ((x + w) >= SSD1306_WIDTH)
   {
      w = SSD1306_WIDTH - x;
   }
   if ((y + h) >= SSD1306_HEIGHT)
   {
      h = SSD1306_HEIGHT - y;
   }

   /* Draw lines */
   for (i = 0; i <= h; i++)
   {
      /* Draw lines */
      SSD1306_DrawLine(x, y + i, x + w, y + i, c);
   }
}

void SSD1306_DrawTriangle(uint16_t x1,
                          uint16_t y1,
                          uint16_t x2,
                          uint16_t y2,
                          uint16_t x3,
                          uint16_t y3,
                          SSD1306_COLOR_t color)
{
   /* Draw lines */
   SSD1306_DrawLine(x1, y1, x2, y2, color);
   SSD1306_DrawLine(x2, y2, x3, y3, color);
   SSD1306_DrawLine(x3, y3, x1, y1, color);
}

void SSD1306_DrawFilledTriangle(uint16_t x1,
                                uint16_t y1,
                                uint16_t x2,
                                uint16_t y2,
                                uint16_t x3,
                                uint16_t y3,
                                SSD1306_COLOR_t color)
{
   int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0, den = 0, num = 0,
           numadd = 0, numpixels = 0, curpixel = 0;

   deltax = ABS(x2 - x1);
   deltay = ABS(y2 - y1);
   x = x1;
   y = y1;

   if (x2 >= x1)
   {
      xinc1 = 1;
      xinc2 = 1;
   }
   else
   {
      xinc1 = -1;
      xinc2 = -1;
   }

   if (y2 >= y1)
   {
      yinc1 = 1;
      yinc2 = 1;
   }
   else
   {
      yinc1 = -1;
      yinc2 = -1;
   }

   if (deltax >= deltay)
   {
      xinc1 = 0;
      yinc2 = 0;
      den = deltax;
      num = deltax / 2;
      numadd = deltay;
      numpixels = deltax;
   }
   else
   {
      xinc2 = 0;
      yinc1 = 0;
      den = deltay;
      num = deltay / 2;
      numadd = deltax;
      numpixels = deltay;
   }

   for (curpixel = 0; curpixel <= numpixels; curpixel++)
   {
      SSD1306_DrawLine(x, y, x3, y3, color);

      num += numadd;
      if (num >= den)
      {
         num -= den;
         x += xinc1;
         y += yinc1;
      }
      x += xinc2;
      y += yinc2;
   }
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c)
{
   int16_t f = 1 - r;
   int16_t ddF_x = 1;
   int16_t ddF_y = -2 * r;
   int16_t x = 0;
   int16_t y = r;

   SSD1306_DrawPixel(x0, y0 + r, c);
   SSD1306_DrawPixel(x0, y0 - r, c);
   SSD1306_DrawPixel(x0 + r, y0, c);
   SSD1306_DrawPixel(x0 - r, y0, c);

   while (x < y)
   {
      if (f >= 0)
      {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      SSD1306_DrawPixel(x0 + x, y0 + y, c);
      SSD1306_DrawPixel(x0 - x, y0 + y, c);
      SSD1306_DrawPixel(x0 + x, y0 - y, c);
      SSD1306_DrawPixel(x0 - x, y0 - y, c);

      SSD1306_DrawPixel(x0 + y, y0 + x, c);
      SSD1306_DrawPixel(x0 - y, y0 + x, c);
      SSD1306_DrawPixel(x0 + y, y0 - x, c);
      SSD1306_DrawPixel(x0 - y, y0 - x, c);
   }
}

void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c)
{
   int16_t f = 1 - r;
   int16_t ddF_x = 1;
   int16_t ddF_y = -2 * r;
   int16_t x = 0;
   int16_t y = r;

   SSD1306_DrawPixel(x0, y0 + r, c);
   SSD1306_DrawPixel(x0, y0 - r, c);
   SSD1306_DrawPixel(x0 + r, y0, c);
   SSD1306_DrawPixel(x0 - r, y0, c);
   SSD1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

   while (x < y)
   {
      if (f >= 0)
      {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      SSD1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
      SSD1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

      SSD1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
      SSD1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
   }
}

void SSD1306_Enable(void)
{
   SSD1306_WRITECOMMAND(SSD1306_CHARGEPUMP);
   SSD1306_WRITECOMMAND(0x14);
   SSD1306_WRITECOMMAND(SSD1306_DISPLAYON);
}
void SSD1306_Disable(void)
{
   SSD1306_WRITECOMMAND(SSD1306_CHARGEPUMP);
   SSD1306_WRITECOMMAND(SSD1306_SETHIGHCOLUMN);
   SSD1306_WRITECOMMAND(SSD1306_DISPLAYOFF);
}

/* Private variables */
static uint32_t ssd1306_I2C_Timeout;

/* Private defines */
#define I2C_TRANSMITTER_MODE 0
#define I2C_RECEIVER_MODE 1
#define I2C_ACK_ENABLE 1
#define I2C_ACK_DISABLE 0

void ssd1306_I2C_Init()
{
   GPIO_InitTypeDef PORT;

   // Init I2C
   RCC_APB2PeriphClockCmd(I2C_CLOCK_PORT, ENABLE);
   PORT.GPIO_Pin = I2C_SDA_PIN | I2C_SCL_PIN;
   PORT.GPIO_Mode = GPIO_Mode_AF_OD;
   PORT.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(I2C_GPIO_PORT, &PORT);

   I2C_InitTypeDef I2CInit;
   RCC_APB1PeriphClockCmd(I2C_CLOCK, ENABLE);   // Enable I2C clock
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   I2C_DeInit(SSD1306_I2C);   // I2C reset to initial state
   I2CInit.I2C_Mode = I2C_Mode_I2C;   // I2C mode is I2C
   I2CInit.I2C_DutyCycle = I2C_DutyCycle_2;   // I2C fast mode duty cycle (WTF is this?)
   I2CInit.I2C_OwnAddress1 = 1;   // This device address (7-bit or 10-bit)
   I2CInit.I2C_Ack = I2C_Ack_Enable;   // Acknowledgment enable
   I2CInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;   // choose 7-bit address for acknowledgment
   I2CInit.I2C_ClockSpeed = I2C_SPEED;
   I2C_Cmd(SSD1306_I2C, ENABLE);   // Enable I2C
   I2C_Init(SSD1306_I2C, &I2CInit);   // Configure I2C
}

void ssd1306_I2C_WriteMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
   ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
   ssd1306_I2C_WriteData(I2Cx, reg);
   for (uint8_t i = 0; i < count; i++)
   {
      ssd1306_I2C_WriteData(I2Cx, data[i]);
      loop(100);
   }
   loop(100);

   ssd1306_I2C_Stop(I2Cx);
}

/* Private functions */
int16_t ssd1306_I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack)
{
   /* Generate I2C start pulse */
   I2Cx->CR1 |= I2C_CR1_START;

   /* Wait till I2C is busy */
   ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
   while (!(I2Cx->SR1 & I2C_SR1_SB))
   {
      if (--ssd1306_I2C_Timeout == 0x00)
      {
         return 1;
      }
   }

   /* Enable ack if we select it */
   if (ack)
   {
      I2Cx->CR1 |= I2C_CR1_ACK;
   }

   /* Send write/read bit */
   if (direction == I2C_TRANSMITTER_MODE)
   {
      /* Send address with zero last bit */
      I2Cx->DR = address & ~I2C_OAR1_ADD0;

      /* Wait till finished */
      ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
      while (!(I2Cx->SR1 & I2C_SR1_ADDR))
      {
         if (--ssd1306_I2C_Timeout == 0x00)
         {
            return 1;
         }
      }
   }
   if (direction == I2C_RECEIVER_MODE)
   {
      /* Send address with 1 last bit */
      I2Cx->DR = address | I2C_OAR1_ADD0;

      /* Wait till finished */
      ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
      while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
      {
         if (--ssd1306_I2C_Timeout == 0x00)
         {
            return 1;
         }
      }
   }

   /* Read status register to clear ADDR flag */
   I2Cx->SR2;

   /* Return 0, everything ok */
   return 0;
}

void ssd1306_I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data)
{
   /* Wait till I2C is not busy anymore */
   ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
   while (!(I2Cx->SR1 & I2C_SR1_TXE) && ssd1306_I2C_Timeout)
   {
      ssd1306_I2C_Timeout--;
   }

   /* Send I2C data */
   I2Cx->DR = data;
}

void ssd1306_I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
   ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
   ssd1306_I2C_WriteData(I2Cx, reg);
   ssd1306_I2C_WriteData(I2Cx, data);
   ssd1306_I2C_Stop(I2Cx);
}

uint8_t ssd1306_I2C_Stop(I2C_TypeDef* I2Cx)
{
   /* Wait till transmitter not empty */
   ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
   while (((!(I2Cx->SR1 & I2C_SR1_TXE)) || (!(I2Cx->SR1 & I2C_SR1_BTF))))
   {
      if (--ssd1306_I2C_Timeout == 0x00)
      {
         return 1;
      }
   }

   /* Generate stop */
   I2Cx->CR1 |= I2C_CR1_STOP;

   /* Return 0, everything ok */
   return 0;
}

uint8_t ssd1306_I2C_IsDeviceConnected(I2C_TypeDef* I2Cx, uint8_t address)
{
   uint8_t connected = 0;
   /* Try to start, function will return 0 in case device will send ACK */
   if (!ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE))
   {
      connected = 1;
   }

   /* STOP I2C */
   ssd1306_I2C_Stop(I2Cx);

   /* Return status */
   return connected;
}

void ssd1306_I2C_FillDisplay(uint16_t x, uint16_t y, uint16_t height, uint16_t width, const char* ptr)
{
   uint16_t index = 0;
   uint8_t j = 8;
   for (uint16_t i = 0; i < height; i++)
   {
      for (uint16_t n = 0; n < width; n++)
      {
         SSD1306_COLOR_t eColor = SSD1306_COLOR_WHITE;
         if (ptr[index] & (1 << (j - 1)))
         {
            eColor = SSD1306_COLOR_BLACK;
         }
         SSD1306_DrawPixel(x + n, y + i, eColor);
         j--;
         if (!(j))
         {
            j = 8;
            index++;
         }
      }
      if (j != 8)
      {
         j = 8;
         index++;
      }
   }
}