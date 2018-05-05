

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_HIGH(a, b) a->BSRR = b
#define GPIO_LOW(a, b) a->BRR = b
#define GPIO_TOGGLE(a, b) a->ODR ^= b

/* USB CONNECT */
#define USB_M_PIN GPIO_Pin_13
#define PORT_USB_M_PORT GPIOC
#define PORT_USB_M_CLK RCC_APB2Periph_GPIOC

#define USB_CONN_ON GPIO_LOW(PORT_USB_M_PORT, USB_M_PIN)
#define USB_CONN_OFF GPIO_HIGH(PORT_USB_M_PORT, USB_M_PIN)

/* POWER DISPLAY */
#define PWR_OLED_PIN GPIO_Pin_7
#define PWR_OLED_PORT GPIOB
#define PORT_OLED_CLK RCC_APB2Periph_GPIOB

#define DISPLAY_PWR_OFF GPIO_LOW(PWR_OLED_PORT, PWR_OLED_PIN)
#define DISPLAY_PWR_ON GPIO_HIGH(PWR_OLED_PORT, PWR_OLED_PIN)

/* BUZZER */
#define BUZ_PIN GPIO_Pin_6
#define BUZ_PORT GPIOB
#define PORT_BUZ_CLK RCC_APB2Periph_GPIOB

#define BUZ_ON BuzzerOn()
#define BUZ_OFF BuzzerOff()

/* BUTTON BOOT */
#define BUT_BOOT_PIN GPIO_Pin_2
#define BUT_BOOT_PORT GPIOB
#define PORT_BUT_BOOT_CLK RCC_APB2Periph_GPIOB
#define GET_BUT_BOOT (_Bool) GPIO_ReadInputDataBit(BUT_BOOT_PORT, BUT_BOOT_PIN)

/* BUTTON SECOND */
#define BUT_SECOND_PIN GPIO_Pin_1
#define BUT_SECOND_PORT GPIOB
#define PORT_BUT_SECOND_CLK RCC_APB2Periph_GPIOB
#define GET_BUT_SECOND (_Bool) GPIO_ReadInputDataBit(BUT_SECOND_PORT, BUT_SECOND_PIN)

/* USART Interface -----------------------------------------------------------*/
/* Debug UART */
#define DBG_BAUDRATE 256000
#define DBG_UART 1
#define DBG_RX_BUFFER_SIZE 0
#define DBG_TX_BUFFER_SIZE 256

#if (DBG_UART == 1)
#   define UART_DBG_INTRERFACE USART1
#   define USART_DBG_CLK USART1_CLK

#   define UART_DBG_RX_PIN USART1_RX_PIN
#   define UART_DBG_RX_PORT USART1_PORT
#   define UART_DBG_RX_PORT_CLK USART1_PORT_CLK

#   define UART_DBG_TX_PIN USART1_TX_PIN
#   define UART_DBG_TX_PORT USART1_PORT
#   define UART_DBG_TX_PORT_CLK USART1_PORT_CLK

#   define UART_DBG_IRQ_CHANNEL USART1_IRQn
#   define UART_DBG_PREMP_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#   define UART_DBG_SUB_PRIORITY 0
#endif

#define BLEEDER_BAT (float)1.5
#define BLEEDER_WKUP (float)1.69

#define VBAT_FULL 4.00
#define VBAT_HIGH 3.90
#define VBAT_MEDIUM 3.75
#define VBAT_LOW 3.60

#define VBUT_WKP_USB_CONNECT 2500U
#define VBUT_WKP_USB_CONNECT_BUT_ON 5300U
#define VBUT_WKP_BUT_ON 5000U

#define TIM4_PERIOD 1000

typedef enum
{
   BUT_OFF = 0,
   BUT_ON,
   USB_CONNECT
} TStatWkupPin;

void RCC_Configuration(void);
void GPIO_Configuration(void);
void TIM4_Configuration(void);
void BKP_Configuration(void);
void IWDG_Configuration(void);
void SleepDevice(void);
void BuzzerOn(void);
void BuzzerOff(void);
void GetBatMeasAndStatWkupPin(float* const pVbat, TStatWkupPin* const pStatWkupPin);
uint32_t GetFlagsControlRegister(void);
uint16_t GetImageWelcome(void);
void SetImageWelcome(uint16_t count_img_wel);
void SetSound(_Bool sound);
_Bool GetSound(void);
void SetShot(int shot);
int getShot(void);

void vBuzzerTask(void* pvParameters);
void ShutdownDevice();
void usbInitDriver(void);
void usbDeInitDriver(void);

#ifdef __cplusplus
}
#endif

#endif