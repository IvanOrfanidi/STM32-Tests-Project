
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
   RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);

   // Init Led Light Pin
   GPIO_InitStructure.GPIO_Pin = LED;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_LED, &GPIO_InitStructure);
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
