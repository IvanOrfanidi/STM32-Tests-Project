

#include "includes.h"
#include "TIMER.h"

uint32_t TimeStamp;
uint32_t delay_rtos;
uint32_t uiCurSec = 0;

// Timer Led
void TM4Init(void)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   // Enable the TIM4 Clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
   TIM_TimeBaseStructure.TIM_Prescaler = 3200 - 1;
   TIM_TimeBaseStructure.TIM_Period = TIMER4_PERIOD * 1;
   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   // TIM IT enable
   TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

   NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   // IRQ Enable
   NVIC_EnableIRQ(TIM4_IRQn);

   // TIM4 enable counter
   TIM_Cmd(TIM4, ENABLE);
}

void TIM4_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
   {
      if (GetLedEnable())
      {
         LedVisual();   //Обработчик мигания светодиода.
      }
      else
      {
         LED_OFF;
      }

      // Clear TIM_IT_Update Interrupt pending bit
      TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
   }
}

int isEndWaitPeriod(const uint32_t wait)
{
   if (delay_rtos != wait)
   {
      delay_rtos = wait;
   }

   if (TimeStamp >= delay_rtos)
   {
      delay_rtos = 0;
      TimeStamp = 0;
      return 1;
   }

   TimeStamp++;

   portTickType xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
   return 0;
}

// Timer One Sec
void TM7Init(void)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   // Enable the TIM4 Clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
   TIM_TimeBaseStructure.TIM_Prescaler = 3200 - 1;
   TIM_TimeBaseStructure.TIM_Period = TIMER7_PERIOD * 1;
   TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

   // TIM IT enable
   TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

   NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   // IRQ Enable
   NVIC_EnableIRQ(TIM7_IRQn);

   // TIM4 enable counter
   TIM_Cmd(TIM7, ENABLE);
}

uint32_t get_cur_sec(void)
{
   return uiCurSec;
}

void TIM7_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
   {
      uiCurSec++;
      // Clear TIM_IT_Update Interrupt pending bit
      TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
   }
}
