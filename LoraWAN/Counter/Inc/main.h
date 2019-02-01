
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

extern void HAL_MspInit(void);
extern void stopDevice(void);
extern void MX_NVIC_Init(void);
extern void MX_IWDG_Init(void);
extern uint32_t GetFlagsControlRegister(void);
void SystemClock_Config(void);

/* Private define ------------------------------------------------------------*/

#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA

#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

#define LD2_Pin GPIO_PIN_15
#define LD2_GPIO_Port GPIOB

#define DBG_UART 9600U       // �������� ����������� UART�
#define RX_BUFFER_SIZE 64    // ����� ����������� UART�

/* Config registers backup */
#define DEF_REG_CONFIG_RTC RTC_BKP_DR0         // ������� ���������� ���������������� RTC
#define DEF_REG_COUNT_VAL_LPTIM RTC_BKP_DR1    // ������� ��� �������� �������� ������� ���������
#define DEF_REG_COUNT_DAILY \
    RTC_BKP_DR2                               // ������ ��� ����� ����� ����������� ����� � ��� �������� �������� � ����� ������������ ������ �� \
                                              // �����������
#define DEF_REG_COUNT_ARCH_EEP RTC_BKP_DR3    // ������ ������� ������� ������ ������ ������ � eeprom
#define DEF_REG_TAMPER_STATE RTC_BKP_DR4      // ������� ��� �������� ��������� ���������� �����
/* */

#define CURENT_WEEKDAY RTC_WEEKDAY_WEDNESDAY
#define CURENT_DATE 14
#define CURENT_MOUNTH RTC_MONTH_AUGUST
#define CURENT_YEAR 17

#define TIME_TAMPER_REACTION 15U    // ���, ������� �� ��������� ���������� ����� (15 ����� �� ����������)

#define TIME_SAVE_ARCHIVE (60 * 60)    // ����� ������ ������ � �������� (1 ��� �� ����������)
#define SIZE_BUFFER_MEAS_ADC 100       // ������ ������ ��� ��� ���������� ���������.

#define loop(__n) for(int i = 0; i < __n; i++)

void _Error_Handler(char*, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
