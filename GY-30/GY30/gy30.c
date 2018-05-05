#include "gy30.h"
#include "includes.h"

void BH1750_I2C_Init(void)
{
   I2C_InitTypeDef I2C_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Enable I2C and GPIO clocks */
   RCC_APB1PeriphClockCmd(BH1750_I2C_RCC_Periph, ENABLE);
   RCC_APB2PeriphClockCmd(BH1750_I2C_RCC_Port, ENABLE);

   /* Configure I2C pins: SCL and SDA */
   GPIO_InitStructure.GPIO_Pin = BH1750_I2C_SCL_Pin | BH1750_I2C_SDA_Pin;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
   GPIO_Init(BH1750_I2C_Port, &GPIO_InitStructure);

   /* I2C configuration */
   I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
   I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
   I2C_InitStructure.I2C_OwnAddress1 = 0x00;   // BH1750 7-bit adress = 0x1E;
   I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
   I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
   I2C_InitStructure.I2C_ClockSpeed = BH1750_I2C_Speed;

   /* Apply I2C configuration after enabling it */
   I2C_Init(BH1750_I2C, &I2C_InitStructure);

   I2C_Cmd(BH1750_I2C, ENABLE);
}

void BH1750_Init(void)
{
   while (I2C_GetFlagStatus(BH1750_I2C, I2C_FLAG_BUSY))
      ;
   I2C_GenerateSTART(BH1750_I2C, ENABLE);

   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_MODE_SELECT))
      ;
   I2C_Send7bitAddress(BH1750_I2C, BH1750_ADDRESS, I2C_Direction_Transmitter);   // Send slave address
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
      ;

   I2C_SendData(BH1750_I2C, BH1750_CONTINUOUS_HIGH_RES_MODE);
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
      ;

   I2C_AcknowledgeConfig(I2C1, DISABLE);   // Disable I2C acknowledgement
   I2C_GenerateSTOP(BH1750_I2C, ENABLE);   // Send STOP condition
}

uint32_t BH1750_Read(void)
{
   uint16_t BH_H;
   uint16_t BH_L;
   uint32_t value;

   int iTimeout = DEF_TIMEOUT_I2C;
   I2C_AcknowledgeConfig(BH1750_I2C, ENABLE);   // Enable I2C acknowledge
   I2C_GenerateSTART(BH1750_I2C, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return 0;
      }
   }
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return 0;
      }
   }
   I2C_Send7bitAddress(BH1750_I2C, BH1750_ADDRESS, I2C_Direction_Receiver);   // Send slave address for READ
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return 0;
      }
   }
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return 0;
      }
   }
   BH_H = (uint16_t)I2C_ReceiveData(BH1750_I2C);   // Receive MSB
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(BH1750_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return 0;
      }
   }
   BH_L = (uint32_t)I2C_ReceiveData(BH1750_I2C);   // Receive LSB
   I2C_AcknowledgeConfig(BH1750_I2C, DISABLE);   // Disable I2C acknowledgment
   I2C_GenerateSTOP(BH1750_I2C, ENABLE);   // Send STOP condition

   value = BH_H * 256 + BH_L;
   value = value * 10 / 12;

   return value;
}