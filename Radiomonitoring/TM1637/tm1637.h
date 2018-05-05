
#ifndef _TM1637_H_
#define _TM1637_H_

#define CLK_PIN GPIO_Pin_8
#define CLK_PORT GPIOB
#define CLK_PORT_CLK RCC_APB2Periph_GPIOB

#define DIO_PIN GPIO_Pin_9
#define DIO_PORT GPIOB
#define DIO_PORT_CLK RCC_APB2Periph_GPIOB

void tm1637Init(void);
void tm1637DisplayDecimal(int dec, int displaySeparator);
void tm1637SetBrightness(char brightness);
void tm1637DisplayOff(void);
void tm1637DisplayString(char* pStr, int displaySeparator);

#endif