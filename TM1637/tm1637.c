
#include "tm1637.h"
#include "includes.h"

void _tm1637Start(void);
void _tm1637Stop(void);
void _tm1637ReadResult(void);
void _tm1637WriteByte(unsigned char b);
void _tm1637DelayUsec(unsigned int i);
void _tm1637ClkHigh(void);
void _tm1637ClkLow(void);
void _tm1637DioHigh(void);
void _tm1637DioLow(void);

const char segmentMap[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,   // 0-7
                            0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,   // 8-9, A-F
                            0x00 };

void tm1637Init(void)
{
   // Init GPIO Structure
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_APB2PeriphClockCmd(CLK_PORT_CLK, ENABLE);
   RCC_APB2PeriphClockCmd(DIO_PORT_CLK, ENABLE);

   GPIO_InitStructure.GPIO_Pin = CLK_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(CLK_PORT, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = DIO_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(DIO_PORT, &GPIO_InitStructure);

   tm1637SetBrightness(8);
}

void tm1637DisplayDecimal(int v, int displaySeparator)
{
   unsigned char digitArr[4];
   for (int i = 0; i < 4; ++i)
   {
      digitArr[i] = segmentMap[v % 10];
      if (i == 2 && displaySeparator)
      {
         digitArr[i] |= 1 << 7;
      }
      v /= 10;
   }

   _tm1637Start();
   _tm1637WriteByte(0x40);
   _tm1637ReadResult();
   _tm1637Stop();

   _tm1637Start();
   _tm1637WriteByte(0xc0);
   _tm1637ReadResult();

   for (int i = 0; i < 4; ++i)
   {
      _tm1637WriteByte(digitArr[3 - i]);
      _tm1637ReadResult();
   }

   _tm1637Stop();
}

// Valid brightness values: 0 - 8.
// 0 = display off.
void tm1637SetBrightness(char brightness)
{
   // Brightness command:
   // 1000 0XXX = display off
   // 1000 1BBB = display on, brightness 0-7
   // X = don't care
   // B = brightness
   _tm1637Start();
   _tm1637WriteByte(0x87 + brightness);
   _tm1637ReadResult();
   _tm1637Stop();
}

void _tm1637Start(void)
{
   _tm1637ClkHigh();
   _tm1637DioHigh();
   _tm1637DelayUsec(200);
   _tm1637DioLow();
}

void _tm1637Stop(void)
{
   _tm1637ClkLow();
   _tm1637DelayUsec(200);
   _tm1637DioLow();
   _tm1637DelayUsec(200);
   _tm1637ClkHigh();
   _tm1637DelayUsec(200);
   _tm1637DioHigh();
}

void _tm1637ReadResult(void)
{
   _tm1637ClkLow();
   _tm1637DelayUsec(500);
   // while (dio); // We're cheating here and not actually reading back the response.
   _tm1637ClkHigh();
   _tm1637DelayUsec(200);
   _tm1637ClkLow();
}

void _tm1637WriteByte(unsigned char b)
{
   for (int i = 0; i < 8; ++i)
   {
      _tm1637ClkLow();
      if (b & 0x01)
      {
         _tm1637DioHigh();
      }
      else
      {
         _tm1637DioLow();
      }
      _tm1637DelayUsec(300);
      b >>= 1;
      _tm1637ClkHigh();
      _tm1637DelayUsec(300);
   }
}

void _tm1637DelayUsec(unsigned int i)
{
   for (; i > 0; i--)
   {
      for (int j = 0; j < 10; ++j)
      {
         __NOP();
      }
   }
}

void _tm1637ClkHigh(void)
{
   GPIO_HIGH(CLK_PORT, CLK_PIN);
}

void _tm1637ClkLow(void)
{
   GPIO_LOW(CLK_PORT, CLK_PIN);
}

void _tm1637DioHigh(void)
{
   GPIO_HIGH(DIO_PORT, DIO_PIN);
}

void _tm1637DioLow(void)
{
   GPIO_LOW(DIO_PORT, DIO_PIN);
}
