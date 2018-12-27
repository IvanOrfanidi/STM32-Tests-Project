
#ifndef _LCD_DIS24_H_
#define _LCD_DIS24_H_

#include "fsmc_sram.h"

void LCD_Init(void);
void LCD_WR_REG(unsigned int index);
extern void LCD_WR_CMD(unsigned int index,unsigned int val);

void LCD_WR_Data(unsigned int val);
void LCD_test(void);
void LCD_clear(unsigned int p);


unsigned int LCD_RD_data(void);
extern void lcd_rst(void);
extern void _delay_ms(__IO uint32_t nCount);
extern void _delay_us(__IO uint32_t nCount);
void lcd_wr_pixel(unsigned int a, unsigned int b, unsigned int e) ;


#endif