/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus

/* Standart functions */
#   include <stdint.h>

#   undef CRC
class CRC
{
 public:
   static uint16_t Modbus(const void* data, uint32_t cnt, uint16_t init = 0xFFFFU);

 private:
};

#endif /*__cplusplus */

#endif /* __CRC_H */