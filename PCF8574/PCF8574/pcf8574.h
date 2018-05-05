#ifndef _PCF8574_H_
#define _PCF8574_H_

#define _I2C_PORT_PCF8574_ 1

#if _I2C_PORT_PCF8574_ == 1
#   define PCF8574_I2C I2C1
#   define PCF8574_I2C_RCC_Periph RCC_APB1Periph_I2C1
#   define PCF8574_I2C_Port GPIOB
#   define PCF8574_I2C_SCL_Pin GPIO_Pin_6
#   define PCF8574_I2C_SDA_Pin GPIO_Pin_7
#   define PCF8574_I2C_RCC_Port RCC_APB2Periph_GPIOB
#elif _I2C_PORT_PCF8574_ == 2
#   define PCF8574_I2C I2C2
#   define PCF8574_I2C_RCC_Periph RCC_APB1Periph_I2C2
#   define PCF8574_I2C_Port GPIOB
#   define PCF8574_I2C_SCL_Pin GPIO_Pin_10
#   define PCF8574_I2C_SDA_Pin GPIO_Pin_11
#   define PCF8574_I2C_RCC_Port RCC_APB2Periph_GPIOB
#endif

#define PCF8574_I2C_Speed 100000

#define PCF8574_ADDRESS 0x07   // this device only has one address

// Раскомментировать нужную строку для своего ЖКИ
//#define LCD_2004
#define LCD_1602

// Указать название вункции задержки
extern void delay(uint32_t t);
#define lcd_pause _delay_ms(1)   // а тут указать значение задержки

// Не нужно редактировать.
// Описание пинов расширителя портов
#define PCF_P0 0
#define PCF_P1 1
#define PCF_P2 2
#define PCF_P3 3
#define PCF_P4 4
#define PCF_P5 5
#define PCF_P6 6
#define PCF_P7 7

// Соответствие пинов ЖКИ и расширителя портов. Возможно тут подредактировать под себя
#define DB4 PCF_P4
#define DB5 PCF_P5
#define DB6 PCF_P6
#define DB7 PCF_P7
#define EN PCF_P2
#define RW PCF_P1
#define RS PCF_P0
#define BL PCF_P3

void PCF8574_I2C_Init(void);

// Функции API
void LcdPcf_Init(void);
void LcdPcf_Backlight(uint8_t state);
void prit_lcd(const char* str);
void LcdPcf_Goto(uint8_t row, uint8_t col);

#endif