#ifndef _LCD_H_
#define _LCD_H_

#define LINE_LCD 2
#define COLUMN_LCD 16

#define LCD_CHAR_SPEED 1

#define PORT_LCD_CLK RCC_APB2Periph_GPIOC

#define LCD_RS_PORT GPIOC
#define LCD_RS_PIN 0

#define LCD_RW_PORT GPIOC
#define LCD_RW_PIN 1

#define LCD_E_PORT GPIOC
#define LCD_E_PIN 2

#define LCD_D4_PORT GPIOC
#define LCD_D4_PIN 4

#define LCD_D5_PORT GPIOC
#define LCD_D5_PIN 5

#define LCD_D6_PORT GPIOC
#define LCD_D6_PIN 6

#define LCD_D7_PORT GPIOC
#define LCD_D7_PIN 7

/////////////////////////////////////////////////////////

#define E_HIGH LCD_E_PORT->BSRR = (1 << LCD_E_PIN)
#define E_LOW LCD_E_PORT->BRR = (1 << LCD_E_PIN)

#define RW_HIGH LCD_RW_PORT->BSRR = (1 << LCD_RW_PIN)
#define RW_LOW LCD_RW_PORT->BRR = (1 << LCD_RW_PIN)

#define RS_HIGH LCD_RS_PORT->BSRR = (1 << LCD_RS_PIN)
#define RS_LOW LCD_RS_PORT->BRR = (1 << LCD_RS_PIN)

#define D4_HIGH LCD_D4_PORT->BSRR = (1 << LCD_D4_PIN)
#define D4_LOW LCD_D4_PORT->BRR = (1 << LCD_D4_PIN)

#define D5_HIGH LCD_D5_PORT->BSRR = (1 << LCD_D5_PIN)
#define D5_LOW LCD_D5_PORT->BRR = (1 << LCD_D5_PIN)

#define D6_HIGH LCD_D6_PORT->BSRR = (1 << LCD_D6_PIN)
#define D6_LOW LCD_D6_PORT->BRR = (1 << LCD_D6_PIN)

#define D7_HIGH LCD_D7_PORT->BSRR = (1 << LCD_D7_PIN)
#define D7_LOW LCD_D7_PORT->BRR = (1 << LCD_D7_PIN)

#define DISP_ON 0x0C     // LCD control constants
#define DISP_OFF 0x08    //

#define DISP_CUR1 0x0E
#define DISP_CUR2 0x0d

#define CLR_DISP 0x01     // Clear LCD
#define CUR_HOME 0x02     // 0x0D & 0x0A
#define ENTRY_INC 0x06    //

typedef enum {
    DD_RAM_ADDR1 = 128,
    DD_RAM_ADDR3 = 148,
    DD_RAM_ADDR2 = 192,
    DD_RAM_ADDR4 = 212,
} NUM_LINE_LCD;

#define CG_RAM_ADDR1 0x40    //

//--------Initialize LCD-----------------------------
void LCD_Init(void);

//--------Send char to LCD-----------------------------
void LCD_Send_CHAR(const uint8_t data_lcd);

//--------Send command to LCD-----------------------------
void LCD_Send_CMD(uint8_t cmd);

//--------Send string to LCD-----------------------------
void LCD_Send_STR(const char* str);

void LCD_Clear(void);

#endif
