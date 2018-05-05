
// User functions //
#include "includes.h"
#include "LCD.h"

void DelayLcd(uint64_t delay);

void _E(void)
{
   E_HIGH;
   //_delay_ms(10);
   DelayLcd(10);
   E_LOW;
}

//--------Initialize LCD-----------------------------
void LCD_Init(void)
{
   // Init GPIO Structure
   GPIO_InitTypeDef GPIO_InitStructure;
   RCC_APB2PeriphClockCmd(PORT_LCD_DATA_CLK, ENABLE);   // Enable Clock Port.
   RCC_APB2PeriphClockCmd(PORT_LCD_CMD_CLK, ENABLE);   // Enable Clock Port.

   // Init RS
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_RS_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_RS_PORT, &GPIO_InitStructure);

   // Init RW
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_RW_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_RW_PORT, &GPIO_InitStructure);

   // Init E
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_E_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_E_PORT, &GPIO_InitStructure);

   // Init D4
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_D4_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_D4_PORT, &GPIO_InitStructure);

   // Init D5
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_D5_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_D5_PORT, &GPIO_InitStructure);

   // Init D6
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_D6_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_D6_PORT, &GPIO_InitStructure);

   // Init D7
   GPIO_InitStructure.GPIO_Pin = (1 << LCD_D7_PIN);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(LCD_D7_PORT, &GPIO_InitStructure);

   RW_LOW;

   RS_LOW;
   _delay_ms(20);
   D4_HIGH;
   D5_HIGH;
   _E();
   _delay_ms(20);
   D4_HIGH;
   D5_HIGH;
   _E();
   _delay_ms(20);
   D4_HIGH;
   D5_HIGH;
   _E();
   _delay_ms(20);
   D4_LOW;
   D5_HIGH;
   _E();
   D4_LOW;
   D5_LOW;
   D6_LOW;
   D7_LOW;
}

//--------Send command to LCD--------//
void LCD_Send_CMD(uint8_t cmd)
{
   _delay_ms(LCD_CHAR_SPEED);   // Delay for LCD char

   if (cmd & 0x80)
   {
      D7_HIGH;
   }
   if (cmd & 0x40)
   {
      D6_HIGH;
   }
   if (cmd & 0x20)
   {
      D5_HIGH;
   }
   if (cmd & 0x10)
   {
      D4_HIGH;
   }

   RS_LOW;   // set LCD to data mode
   _E();
   D4_LOW;
   D5_LOW;
   D6_LOW;
   D7_LOW;

   if (cmd & 0x8)
   {
      D7_HIGH;
   }
   if (cmd & 0x4)
   {
      D6_HIGH;
   }
   if (cmd & 0x2)
   {
      D5_HIGH;
   }
   if (cmd & 0x1)
   {
      D4_HIGH;
   }

   RS_LOW;   // set LCD to data mode
   _E();
   D4_LOW;
   D5_LOW;
   D6_LOW;
   D7_LOW;
}

//--------Send char to LCD-------//
void LCD_Send_CHAR(const uint8_t data_lcd)
{
   _delay_ms(LCD_CHAR_SPEED);   // Delay for LCD char

   if (data_lcd & 0x80)
      D7_HIGH;
   if (data_lcd & 0x40)
      D6_HIGH;
   if (data_lcd & 0x20)
      D5_HIGH;
   if (data_lcd & 0x10)
      D4_HIGH;

   RS_HIGH;
   _E();
   D4_LOW;
   D5_LOW;
   D6_LOW;
   D7_LOW;

   if (data_lcd & 0x8)
      D7_HIGH;
   if (data_lcd & 0x4)
      D6_HIGH;
   if (data_lcd & 0x2)
      D5_HIGH;
   if (data_lcd & 0x1)
      D4_HIGH;

   RS_HIGH;
   _E();
   D4_LOW;
   D5_LOW;
   D6_LOW;
   D7_LOW;
}

//--------Send string to LCD---------//
void LCD_Send_STR(const char* str)
{
   uint8_t i = 0;
   static uint8_t pointer_lcd = 0;
   static uint8_t total_size_lcd = 0;
   static NUM_LINE_LCD nim_line_lcd = DD_RAM_ADDR1;

   if (total_size_lcd >= COLUMN_LCD * LINE_LCD)
   {
      LCD_Clear();
      total_size_lcd = 0;
      pointer_lcd = 0;
      nim_line_lcd = DD_RAM_ADDR1;
   }

   // loop to the end of the string
   while (str[i] != 0)
   {
      if ((str[i] == '\r') || (str[i] == '\n') || (pointer_lcd > COLUMN_LCD - 1))
      {
         pointer_lcd = 0;
         switch (nim_line_lcd)
         {
         case DD_RAM_ADDR1:
            nim_line_lcd = DD_RAM_ADDR2;
            LCD_Send_CMD(DD_RAM_ADDR2);
            break;
         case DD_RAM_ADDR2:
            nim_line_lcd = DD_RAM_ADDR3;
            LCD_Send_CMD(DD_RAM_ADDR3);
            break;
         case DD_RAM_ADDR3:
            nim_line_lcd = DD_RAM_ADDR4;
            LCD_Send_CMD(DD_RAM_ADDR4);
            break;
         case DD_RAM_ADDR4:
            nim_line_lcd = DD_RAM_ADDR1;
            LCD_Send_CMD(DD_RAM_ADDR1);
            break;
         }

         if ((str[i] == '\r') || (str[i] == '\n'))
         {
            i++;
            continue;
         }
      }

      LCD_Send_CHAR(str[i]);
      pointer_lcd++;
      total_size_lcd++;
      i++;
   }
}

void LCD_Clear(void)
{
   LCD_Send_CMD(CLR_DISP);
   DelayLcd(10000);
}

#pragma optimize = none
void DelayLcd(uint64_t delay)
{
   while (delay)
   {
      delay--;
   }
}
