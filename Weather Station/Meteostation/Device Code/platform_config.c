
// User functions //
#include "includes.h"
#include "platform_config.h"

void InitGPIO(void)
{
   // Init GPIO Structure
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
                          ENABLE);

   // Init Led Light
   RCC_APB2PeriphClockCmd(PORT_LED_LIGHT_LCD_CLK, ENABLE);

   // Init Led Light Pin
   GPIO_InitStructure.GPIO_Pin = LED_LIGHT_LCD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(PORT_LED_LIGHT_LCD, &GPIO_InitStructure);
   // GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);

   // Init Buzzer
   RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);
   // Init Buz
   GPIO_InitStructure.GPIO_Pin = BUZ;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_BUZ, &GPIO_InitStructure);

   // Init USB
   RCC_APB2PeriphClockCmd(PORT_USB_M_CLK, ENABLE);

   // Init USB Pin
   GPIO_InitStructure.GPIO_Pin = USB_M;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_USB_M, &GPIO_InitStructure);

   RCC_APB2PeriphClockCmd(PORT_BUT_CLK, ENABLE);

   // Init But S1
   GPIO_InitStructure.GPIO_Pin = BUT_S1;
   // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(PORT_BUT, &GPIO_InitStructure);

   // Init But S2
   GPIO_InitStructure.GPIO_Pin = BUT_S2;
   // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(PORT_BUT, &GPIO_InitStructure);
}

void InitBKP(void)
{
   /* Enable PWR and BKP clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

   /* Enable write access to Backup domain */
   PWR_BackupAccessCmd(ENABLE);

   /* Clear Tamper pin Event(TE) pending flag */
   BKP_ClearFlag();
}
