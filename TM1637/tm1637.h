
#ifndef _TM1637_H_
#define _TM1637_H_

#define CLK_PIN GPIO_Pin_5
#define CLK_PORT GPIOB
#define CLK_PORT_CLK RCC_APB2Periph_GPIOB

#define DIO_PIN GPIO_Pin_6
#define DIO_PORT GPIOB
#define DIO_PORT_CLK RCC_APB2Periph_GPIOB

void tm1637Init(void);
void tm1637DisplayDecimal(int v, int displaySeparator);
void tm1637SetBrightness(char brightness);

#endif