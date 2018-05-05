
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#   define __MAIN_H

#   ifdef __cplusplus
extern "C" {
#   endif

/* Includes ------------------------------------------------------------------*/
#   include <string.h>
#   include <ctype.h>
#   include <stdlib.h>
#   include <limits.h>
#   include <intrinsics.h>
#   include <assert.h>
#   include <stdint.h>

extern void HAL_MspInit(void);
extern void stopDevice(void);
extern void MX_NVIC_Init(void);
extern void MX_IWDG_Init(void);
extern uint32_t GetFlagsControlRegister(void);
void SystemClock_Config(void);

/* Private define ------------------------------------------------------------*/

#   define MCO_Pin GPIO_PIN_0
#   define MCO_GPIO_Port GPIOH

#   define USART_TX_Pin GPIO_PIN_2
#   define USART_TX_GPIO_Port GPIOA

#   define USART_RX_Pin GPIO_PIN_3
#   define USART_RX_GPIO_Port GPIOA

/* Pin Led */
#   define LD2_Pin GPIO_PIN_15
#   define LD2_GPIO_Port GPIOB

/* Pullup for USB connect */
#   define CHECK_USB_Pin GPIO_PIN_2
#   define CHECK_USB_Port GPIOB

/* Power enable communication interface */
#   define Enable_Uc_Pin GPIO_PIN_8
#   define Enable_Uc_Port GPIOA

#   define LedOn() HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET)
#   define LedOff() HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)

#   define UcEnable() HAL_GPIO_WritePin(Enable_Uc_Port, Enable_Uc_Pin, GPIO_PIN_SET)
#   define UcDisable() HAL_GPIO_WritePin(Enable_Uc_Port, Enable_Uc_Pin, GPIO_PIN_RESET)

#   define UsbConnect() HAL_GPIO_WritePin(CHECK_USB_Port, CHECK_USB_Pin, GPIO_PIN_SET)
#   define UsbDisconnect() HAL_GPIO_WritePin(CHECK_USB_Port, CHECK_USB_Pin, GPIO_PIN_RESET)

#   define DBG_UART_BAUDRATE 9600U
#   define DBG_UART_RX_BUF_SIZE 64

#   define COMM_UART_BAUDRATE 4800U
#   define COMM_UART_RX_BUF_SIZE 256

/* Config registers backup */
#   define DEF_REG_CONFIG_RTC RTC_BKP_DR0   // регистр определяет сконфигуриррован RTC
#   define DEF_REG_COUNT_ARCH_EEP RTC_BKP_DR3   // регист счетчик индекса адреса записи архива в eeprom
/* */

#   define CURENT_WEEKDAY RTC_WEEKDAY_WEDNESDAY
#   define CURENT_DATE 14
#   define CURENT_MOUNTH RTC_MONTH_AUGUST
#   define CURENT_YEAR 17

#   define TIME_SAVE_ARCHIVE 3600U
#   define SIZE_BUFFER_MEAS_ADC 100

#   define loop(__a) for (size_t i = 0; i < __a; i++)
#   define myswap(__a, __b) \
      __a ^= __b; \
      __b ^= __a; \
      __a ^= __b

void _Error_Handler(char*, int);

#   define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#   ifdef __cplusplus
}
#   endif

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
