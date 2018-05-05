#include "includes.h"
#include "I2C.h"

uint16_t Tx_Idx1;
uint16_t Rx_Idx1;

__IO uint32_t I2CDirection;
uint8_t Address;

void I2C1EvIrq(void)
{
   __IO uint32_t SR1Register = 0;
   __IO uint32_t SR2Register = 0;

   /* Read the I2C1 SR1 and SR2 status registers */
   SR1Register = I2C1->SR1;
   SR2Register = I2C1->SR2;

   /* If I2C1 is slave (MSL flag = 0) */
   if ((SR2Register & 0x0001) != 0x0001)
   {
      /* If ADDR = 1: EV1 */
      if ((SR1Register & 0x0002) == 0x0002)
      {
         /* Clear SR1Register and SR2Register variables to prepare for next IT */
         SR1Register = 0;
         SR2Register = 0;
         /* Initialize the transmit/receive counters for next transmission/reception
         using Interrupt  */
         Tx_Idx1 = 0;
         Rx_Idx1 = 0;
      }
      /* If TXE = 1: EV3 */
      if ((SR1Register & 0x0080) == 0x0080)
      {
         /* Write data in data register */
         I2C1->DR = BufferI2C_Tx1[Tx_Idx1++];
         SR1Register = 0;
         SR2Register = 0;
      }
      /* If RXNE = 1: EV2 */
      if ((SR1Register & 0x0040) == 0x0040)
      {
         /* Read data from data register */
         BufferI2C_Rx1[Rx_Idx1++] = I2C1->DR;
         SR1Register = 0;
         SR2Register = 0;
      }
      /* If STOPF =1: EV4 (Slave has detected a STOP condition on the bus */
      if ((SR1Register & 0x0010) == 0x0010)
      {
         I2C1->CR1 |= CR1_PE_Set;
         SR1Register = 0;
         SR2Register = 0;
      }
   } /* End slave mode */

   // If SB = 1, I2C1 master sent a START on the bus: EV5) //
   if ((SR1Register & 0x0001) == 0x0001)
   {
      // Send the slave address for transmssion or for reception (according to the configured value
      // in the write master write routine
      I2C1->DR = Address;
      SR1Register = 0;
      SR2Register = 0;
   }
}

void I2C1ErIrq(void)
{
   __IO uint32_t SR1Register = 0;

   /* Read the I2C1 status register */
   SR1Register = I2C1->SR1;
   /* If AF = 1 */
   if ((SR1Register & 0x0400) == 0x0400)
   {
      I2C1->SR1 &= 0xFBFF;
      SR1Register = 0;
   }
   /* If ARLO = 1 */
   if ((SR1Register & 0x0200) == 0x0200)
   {
      I2C1->SR1 &= 0xFBFF;
      SR1Register = 0;
   }
   /* If BERR = 1 */
   if ((SR1Register & 0x0100) == 0x0100)
   {
      I2C1->SR1 &= 0xFEFF;
      SR1Register = 0;
   }

   /* If OVR = 1 */

   if ((SR1Register & 0x0800) == 0x0800)
   {
      I2C1->SR1 &= 0xF7FF;
      SR1Register = 0;
   }
}

int I2C_Rx_Len(void)
{
   return Rx_Idx1;
}

void Clear_Rx_Buffer(void)
{
   for (uint16_t i = 0; i < I2C_RX_BUFFER_SIZE1; i++)
      BufferI2C_Rx1[i] = 0;

   Rx_Idx1 = 0;
}

void Clear_Tx_Buffer(void)
{
   for (uint16_t i = 0; i < I2C_TX_BUFFER_SIZE1; i++)
      BufferI2C_Tx1[i] = 0;

   Tx_Idx1 = 0;
}

void BufferI2C_Rx1_Debug(char* pAT_Cmd)
{
   uint16_t i = 0;
   //Вычисляем длину сообщения.
   uint16_t Len;
   while (pAT_Cmd[i] != 0)
   {
      i++;
   }
   Len = i;

   for (i = 0; i < Len; i++)
   {
      BufferI2C_Rx1[i] = pAT_Cmd[i];
   }
   BufferI2C_Rx1[Len] = 0;
   Rx_Idx1 = Len + 1;
}

void BufferI2C_Tx1_Debug(char* pAT_Cmd)
{
   uint16_t i = 0;
   //Вычисляем длину сообщения.
   uint16_t Len;
   while (pAT_Cmd[i] != 0)
   {
      IWDG_ReloadCounter();   // Reload IWDG counter //
      i++;
   }
   Len = i;

   for (i = 0; i < Len; i++)
   {
      BufferI2C_Tx1[i] = pAT_Cmd[i];
   }
   BufferI2C_Tx1[Len] = 0;
   Tx_Idx1 = Len + 1;
}