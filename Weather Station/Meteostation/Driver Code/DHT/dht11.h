#ifndef DHT11_H_
#define DHT11_H_

#include "includes.h"
/* configuration macros: */
/* enable timer setup - optional and implementation defined */
#define ENABLE_TIMER TIM2->CR1 |= TIM_CR1_CEN
#define DISABLE_TIMER TIM2->CR1 &= ~TIM_CR1_CEN

/* micro seconds delay function */
#define DELAY_US(__time__) delay_us(TIM2, __time__)

/* GPIO port name and number for 1-wire data input/output */
#define GPIO_PORT_NAME GPIOC
#define GPIO_PORT_NUMBER 13

/* GPIO configuration defines */
#define SET_GPIO_AS_OUTPUT gpio_cfg(GPIO_PORT_NAME, GPIO_PORT_NUMBER, GPIO_OUTPUT_2MHZ, GPIO_GP_OUTPUT_PUSH_PULL)
#define SET_GPIO_AS_INPUT gpio_cfg(GPIO_PORT_NAME, GPIO_PORT_NUMBER, GPIO_INPUT, GPIO_FLOATING_IN)

/* GPIO actions */
#define GPIO_CLEAR gpio_clr(GPIO_PORT_NAME, GPIO_PORT_NUMBER)           /* clear port state */
#define GPIO_SET gpio_set(GPIO_PORT_NAME, GPIO_PORT_NUMBER)             /* set port state to 1 */
#define GPIO_GET_INPUT gpio_get_input(GPIO_PORT_NAME, GPIO_PORT_NUMBER) /* should return 0 or 1 */

/* optional critical section (because of delays slow as 30us) */
#define CRITICAL_SECTION_DECL
#define CRITICAL_SECTION_PROTECT
#define CRITICAL_SECTION_UNPROTECT

/* optional timeouts for while() loops (in case of hardware failure) */
#define ENABLE_TIMEOUTS /* comment to not perform timeout checks */
#define TIMEOUT_VALUE 100000

/* return codes : */
typedef enum {
    DHT11_WRONG_CHCKSUM = -2,
    DHT11_TIMEOUT = -1,
    DHT11_OK = 0,
} EDHT_STATUS;

/* mode */
#define GPIO_INPUT 0x00        /* GPIOF Input mode */
#define GPIO_OUTPUT_10MHZ 0x01 /* GPIOF Output mode, max speed 10 MHz */
#define GPIO_OUTPUT_2MHZ 0x02  /* GPIOF Output mode, max speed 2 MHz */
#define GPIO_OUTPUT_50MHZ 0x03 /* GPIOF Output mode, max speed 50 MHz */

/* output mode */
#define GPIO_GP_OUTPUT_PUSH_PULL 0x00  /* General purpose output push-pull */
#define GPIO_GP_OUTPUT_OPEN_DRAIN 0x01 /* General purpose output Open-drain */
#define GPIO_AF_OUTPUT_PUSH_PULL 0x02  /* Alternate function output Push-pull */
#define GPIO_AF_OUT_OPEN_DRAIN 0x03    /* Alternate function output Open-drain */

/* in mode */
#define GPIO_ANALOG_MODE 0x00 /* Analog mode */
#define GPIO_FLOATING_IN 0x01 /* Floating input */
#define GPIO_PU_PD_INPUT 0x02 /* Input with pull-up / pull-down */

/* prototypes: */
int dht11_read(uint8_t* p);
void tim_init(TIM_TypeDef* p);

#endif
