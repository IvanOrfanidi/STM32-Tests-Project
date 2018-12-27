/**
 ******************************************************************************
 * @file    board.cpp
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    25-March-2018
 * @brief   Ôאיכ הנאיגונא ןכאעפמנלû.
 *
 *
 ******************************************************************************
 * @attention
 *
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "board.hpp"
#include "stm32f10x_conf.h"

uint32_t Board::SysCount = 0;

/*
 * @brief  Set NVIC Priority Group.
 * @retval None.
 */
void Board::SetNvicPriorityGroup(uint32_t priority_group)
{
   /* clang-format off */
    /* The table below gives the allowed values of the pre-emption priority and subpriority according
     to the Priority Grouping configuration performed by NVIC_PriorityGroupConfig function
      ============================================================================================================================
        NVIC_PriorityGroup   | NVIC_IRQChannelPreemptionPriority | NVIC_IRQChannelSubPriority  | Description
      ============================================================================================================================
       NVIC_PriorityGroup_0  |                0                  |            0-15             |   0 bits for pre-emption priority
                             |                                   |                             |   4 bits for subpriority
      ----------------------------------------------------------------------------------------------------------------------------
       NVIC_PriorityGroup_1  |                0-1                |            0-7              |   1 bits for pre-emption priority
                             |                                   |                             |   3 bits for subpriority
      ----------------------------------------------------------------------------------------------------------------------------    
       NVIC_PriorityGroup_2  |                0-3                |            0-3              |   2 bits for pre-emption priority
                             |                                   |                             |   2 bits for subpriority
      ----------------------------------------------------------------------------------------------------------------------------    
       NVIC_PriorityGroup_3  |                0-7                |            0-1              |   3 bits for pre-emption priority
                             |                                   |                             |   1 bits for subpriority
      ----------------------------------------------------------------------------------------------------------------------------    
       NVIC_PriorityGroup_4  |                0-15               |            0                |   4 bits for pre-emption priority
                             |                                   |                             |   0 bits for subpriority                       
      ============================================================================================================================
    */
   /* clang-format on */
   NVIC_PriorityGroupConfig(priority_group);
}

/**
 * @brief  Update and geting System Clock Core.
 * @retval System Clock Core.
 */
uint32_t Board::ClockUpdate()
{
   SystemCoreClockUpdate();
   return SystemCoreClock;
}

/**
 * @brief  This method configures System Tick
 * @param [in]  ticks  Number of ticks between two interrupts.
 *                      / 1000     |   1ms  /
 *                      / 100000   |   10us /
 *                      / 1000000  |   1us  /
 * @retval None.
 */
void Board::InitSysTick(uint32_t ticks_us)
{
   while (SysTick_Config(SystemCoreClock / ticks_us))
   {
      // Wait running System Tick
   }
   __enable_irq();
}

/**
 * @brief  Initialisation Led.
 * @retval None.
 */
void Board::InitLed()
{
   // Init Led Light
   RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);

   // Init Led Light Pin
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = LED_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_LED, &GPIO_InitStructure);
   LedOff();   // Led in OFF
}

/**
 * @brief  Enable Led.
 * @retval None.
 */
void Board::LedOn()
{
   GPIO_ResetBits(PORT_LED, LED_PIN);
}

/**
 * @brief  Disable Led.
 * @retval None.
 */
void Board::LedOff()
{
   GPIO_SetBits(PORT_LED, LED_PIN);
}

/**
 * @brief  Initialisation Backup.
 * @retval None.
 */
void Board::InitBKP()
{
   /* Enable PWR and BKP clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

   /* Enable write access to Backup domain */
   PWR_BackupAccessCmd(ENABLE);

   /* Clear Tamper pin Event(TE) pending flag */
   BKP_ClearFlag();
}

/**
 * @brief  Initialisation Watchdog timer.
 * @retval None.
 */
void Board::InitIWDG()
{
   /* Enable the LSI OSC */
   RCC_LSICmd(ENABLE);

   /* Wait till LSI is ready */
   while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
   {
   }

   /* IWDG timeout equal to 2000 ms (the timeout may varies due to LSI
    frequency dispersion) */
   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

   /* IWDG counter clock: LSI/32 */
   IWDG_SetPrescaler(IWDG_Prescaler_256);

/* Set counter reload value to obtain 250ms IWDG TimeOut.
   Counter Reload Value = 250ms/IWDG counter clock period
                        = 250ms / (LSI/32)
                        = 0.25s / (LsiFreq/32)
                        = LsiFreq/(32 * 4)
                        = LsiFreq/128
 */
#define LSI_FREQ 40000
   IWDG_SetReload(LSI_FREQ / 64);

   /* Reload IWDG counter */
   IWDG_ReloadCounter();

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG_Enable();
}

/**
 * @brief  Sleep Device.
 * @retval None
 */
void Board::SleepDevice()
{
   /* Allow access to BKP Domain */
   PWR_BackupAccessCmd(ENABLE);
   PWR_EnterSTANDBYMode();   //  -_-zZ
}

/**
 * @brief Enable WKUP pin.
 * @retval None
 */
void Board::WakeUpPinEnable()
{
   /* Enable WKUP pin */
   PWR_WakeUpPinCmd(ENABLE);
}

uint32_t Board::GetSysCount()
{
   return SysCount;
}

void Board::DelayMS(uint32_t delay)
{
   delay += SysCount;
   while (delay >= SysCount)
      ;
}

/*
 *       INTERRUPTS
 */
void SysTick_Handler(void)
{
   Board::SysCount++;
}
