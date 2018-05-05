#ifndef DHT22_H_
#define DHT22_H_

/* Includes ------------------------------------------------------------------*/
#include "includes.h"

#define MAX_TICS 200   //Настроить таймер в зависимости от частоты МК.
#define MAX_LOG_VAL 50   //Подбирать этот параметр от загруженности МК(самое слабое звено, необходима отладка).

/* Exported constants --------------------------------------------------------*/
#define DHT22_PORT_CLOCK RCC_APB2Periph_GPIOA
#define DHT22_PORT GPIOA
#define DHT22_PIN GPIO_Pin_7

#define DHT22_OK 0
#define DHT22_NO_CONN 1
#define DHT22_CS_ERROR 2

/* Exported functions ------------------------------------------------------- */
int Read_DHT22(uint8_t* pBuf);

#endif
