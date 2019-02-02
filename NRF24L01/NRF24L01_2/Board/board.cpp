/**
 ******************************************************************************
 * @file    board.cpp
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    27-01-2019
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
    LedOff();    // Led in OFF
}

void Board::GpioClock(const GPIO_TypeDef* port, FunctionalState state)
{
    if(port == GPIOA) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, state);
    }
    else if(port == GPIOB) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, state);
    }
    else if(port == GPIOC) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, state);
    }
    else if(port == GPIOD) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, state);
    }
    else if(port == GPIOE) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, state);
    }
    else if(port == GPIOF) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, state);
    }
    else if(port == GPIOG) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, state);
    }
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
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;

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
