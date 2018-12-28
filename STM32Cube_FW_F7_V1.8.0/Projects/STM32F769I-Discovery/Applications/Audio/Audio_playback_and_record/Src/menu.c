/**
 ******************************************************************************
 * @file    Audio/Audio_playback_and_record/Src/menu.c
 * @author  MCD Application Team
 * @brief   This file implements Menu Functions
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "waverecorder.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TOUCH_RECORD_XMIN 300
#define TOUCH_RECORD_XMAX 340
#define TOUCH_RECORD_YMIN 212
#define TOUCH_RECORD_YMAX 252

#define TOUCH_PLAYBACK_XMIN 125
#define TOUCH_PLAYBACK_XMAX 165
#define TOUCH_PLAYBACK_YMIN 212
#define TOUCH_PLAYBACK_YMAX 252

/* Private macro -------------------------------------------------------------*/
/* Global extern variables ---------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
AUDIO_DEMO_StateMachine AudioDemo;
AUDIO_PLAYBACK_StateTypeDef AudioState;

/* Private function prototypes -----------------------------------------------*/
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode);
static void LCD_ClearTextZone(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Manages AUDIO Menu Process.
 * @param  None
 * @retval None
 */
void AUDIO_MenuProcess(void)
{
   AUDIO_ErrorTypeDef status;
   TS_StateTypeDef TS_State;
   Point PlaybackLogoPoints[] = { { TOUCH_PLAYBACK_XMIN, TOUCH_PLAYBACK_YMIN },
                                  { TOUCH_PLAYBACK_XMAX, (TOUCH_PLAYBACK_YMIN + TOUCH_PLAYBACK_YMAX) / 2 },
                                  { TOUCH_PLAYBACK_XMIN, TOUCH_PLAYBACK_YMAX } };

   if (appli_state == APPLICATION_READY)
   {
      switch (AudioDemo.state)
      {
      case AUDIO_DEMO_IDLE:

         AudioDemo.state = AUDIO_DEMO_WAIT;

         BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);
         BSP_LCD_ClearStringLine(13); /* Clear touch screen buttons dedicated zone */
         BSP_LCD_ClearStringLine(14);
         BSP_LCD_ClearStringLine(15);
         BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
         BSP_LCD_FillPolygon(PlaybackLogoPoints, 3); /* Playback sign */
         BSP_LCD_FillCircle((TOUCH_RECORD_XMAX + TOUCH_RECORD_XMIN) / 2, /* Record circle */
                            (TOUCH_RECORD_YMAX + TOUCH_RECORD_YMIN) / 2,
                            (TOUCH_RECORD_XMAX - TOUCH_RECORD_XMIN) / 2);
         BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
         BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
         BSP_LCD_DisplayStringAtLine(15, (uint8_t*)"Use touch screen to enter playback or record menu");
         break;

      case AUDIO_DEMO_WAIT:

         BSP_TS_GetState(&TS_State);
         if (TS_State.touchDetected == 1)
         {
            if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX) &&
                (TS_State.touchY[0] > TOUCH_RECORD_YMIN) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX))
            {
               AudioDemo.state = AUDIO_DEMO_IN;
            }
            else if ((TS_State.touchX[0] > TOUCH_PLAYBACK_XMIN) && (TS_State.touchX[0] < TOUCH_PLAYBACK_XMAX) &&
                     (TS_State.touchY[0] > TOUCH_PLAYBACK_YMIN) && (TS_State.touchY[0] < TOUCH_PLAYBACK_YMAX))
            {
               AudioDemo.state = AUDIO_DEMO_PLAYBACK;
            }
            else
            {
               AudioDemo.state = AUDIO_DEMO_EXPLORE;
            }

            /* Wait for touch released */
            do
            {
               BSP_TS_GetState(&TS_State);
            } while (TS_State.touchDetected > 0);
         }
         break;

      case AUDIO_DEMO_EXPLORE:
         if (appli_state == APPLICATION_READY)
         {
            if (AUDIO_ShowWavFiles() > 0)
            {
               LCD_ErrLog("There is no WAV file on the USB Key.\n");
               AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
               AudioDemo.state = AUDIO_DEMO_IDLE;
            }
            else
            {
               AudioDemo.state = AUDIO_DEMO_WAIT;
            }
         }
         else
         {
            AudioDemo.state = AUDIO_DEMO_WAIT;
         }
         break;

      case AUDIO_DEMO_PLAYBACK:
         if (appli_state == APPLICATION_READY)
         {
            if (AudioState == AUDIO_STATE_IDLE)
            {
               if (AUDIO_ShowWavFiles() > 0)
               {
                  LCD_ErrLog("There is no WAV file on the USB Key.\n");
                  AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
                  AudioDemo.state = AUDIO_DEMO_IDLE;
               }
               else
               {
                  /* Start Playing */
                  AudioState = AUDIO_STATE_INIT;
               }
               /* Clear the LCD */
               LCD_ClearTextZone();

               if (AUDIO_PLAYER_Start(0) == AUDIO_ERROR_IO)
               {
                  AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
                  AudioDemo.state = AUDIO_DEMO_IDLE;
               }
            }
            else /* Not idle */
            {
               if (AUDIO_PLAYER_Process() == AUDIO_ERROR_IO)
               {
                  /* Clear the LCD */
                  LCD_ClearTextZone();

                  AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
                  AudioDemo.state = AUDIO_DEMO_IDLE;
               }
            }
         }
         else
         {
            AudioDemo.state = AUDIO_DEMO_WAIT;
         }
         break;

      case AUDIO_DEMO_IN:
         if (appli_state == APPLICATION_READY)
         {
            if (AudioState == AUDIO_STATE_IDLE)
            {
               /* Start Playing */
               AudioState = AUDIO_STATE_INIT;

               /* Clear the LCD */
               LCD_ClearTextZone();

               /* Init storage */
               AUDIO_StorageInit();

               /* Configure the audio recorder: sampling frequency, bits-depth, number of channels */
               if (AUDIO_REC_Start() == AUDIO_ERROR_IO)
               {
                  AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
                  AudioDemo.state = AUDIO_DEMO_IDLE;
               }
            }
            else /* Not idle */
            {
               status = AUDIO_REC_Process();
               if ((status == AUDIO_ERROR_IO) || (status == AUDIO_ERROR_EOF))
               {
                  /* Clear the LCD */
                  LCD_ClearTextZone();

                  AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
                  AudioDemo.state = AUDIO_DEMO_IDLE;
               }
            }
         }
         else
         {
            AudioDemo.state = AUDIO_DEMO_WAIT;
         }
         break;

      default:
         break;
      }
   }

   if (appli_state == APPLICATION_DISCONNECT)
   {
      appli_state = APPLICATION_IDLE;
      AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
   }
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
 * @brief  Changes the selection mode.
 * @param  select_mode: Selection mode
 * @retval None
 */
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode)
{
   if (select_mode == AUDIO_SELECT_MENU)
   {
      LCD_LOG_UpdateDisplay();
      AudioDemo.state = AUDIO_DEMO_IDLE;
   }
   else if (select_mode == AUDIO_PLAYBACK_CONTROL)
   {
      LCD_ClearTextZone();
   }
}

/**
 * @brief  Clears the text zone.
 * @param  None
 * @retval None
 */
static void LCD_ClearTextZone(void)
{
   uint8_t i = 0;

   for (i = 0; i < 13; i++)
   {
      BSP_LCD_ClearStringLine(i + 3);
   }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/