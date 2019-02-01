
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

const char segmentMap[] = {
    0x3f,
    0x06,
    0x5b,
    0x4f,
    0x66,
    0x6d,
    0x7d,
    0x07,    // 0-7
    0x7f,
    0x6f,
    0x77,
    0x7c,
    0x39,
    0x5e,
    0x79,
    0x71,    // 8-9, A-F
    0x00,    // off
    0x54,    // n
    0x40,    // dash '-'
    0x76,    // X
    0x50,    // r
    0x74,    // h
    0x76,    // H
    0x5C,    // o
    0x63,    // grad
    0x1c,    // u
    0x3e,    // U
    0x38,    // L
    0x78,    // T
};

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

void tm1637DisplayOff(void)
{
    char digitArr[4] = { 0, 0, 0, 0 };
    _tm1637Start();
    _tm1637WriteByte(0x40);
    _tm1637ReadResult();
    _tm1637Stop();

    _tm1637Start();
    _tm1637WriteByte(0xc0);
    _tm1637ReadResult();

    for(int i = 0; i < 4; ++i) {
        _tm1637WriteByte(digitArr[3 - i]);
        _tm1637ReadResult();
    }

    _tm1637Stop();
}

void tm1637DisplayString(char* pStr, int displaySeparator)
{
    char digitArr[4];
    uint8_t index = 0;
    for(int i = 0; i < 4; i++) {
        switch(pStr[i]) {
            case 'a':
            case 'A':
                index = 10;
                break;
            case 'b':
            case 'B':
                index = 11;
                break;
            case 'c':
            case 'C':
                index = 12;
                break;
            case 'd':
            case 'D':
                index = 13;
                break;
            case 'e':
            case 'E':
                index = 14;
                break;
            case 'f':
            case 'F':
                index = 15;
                break;
            case 'n':
            case 'N':
                index = 17;
                break;
            case '-':
                index = 18;
                break;
            case 'x':
            case 'X':
                index = 19;
                break;
            case 'r':
            case 'R':
                index = 20;
                break;
            case 'h':
                index = 21;
                break;
            case 'H':
                index = 22;
                break;
            case 'o':
                index = 23;
                break;
            case 'O':
                index = 0;
                break;
            case 223:
                index = 24;
                break;    // grad
            case 'u':
                index = 25;
                break;
            case 'U':
                index = 26;
                break;
            case 'l':
            case 'L':
                index = 27;
                break;
            case 't':
            case 'T':
                index = 28;
                break;
            case 's':
            case 'S':
                index = 5;
                break;
            default:
                index = 16;
                if(pStr[i] >= '0' && pStr[i] <= '9') {
                    index = pStr[i] - 0x30;
                }
        }
        digitArr[i] = segmentMap[index];
        if(i == 1 && displaySeparator) {
            digitArr[i] |= (1 << 7);
        }
    }

    _tm1637Start();
    _tm1637WriteByte(0x40);
    _tm1637ReadResult();
    _tm1637Stop();

    _tm1637Start();
    _tm1637WriteByte(0xc0);
    _tm1637ReadResult();

    for(int i = 0; i < 4; i++) {
        _tm1637WriteByte(digitArr[i]);
        _tm1637ReadResult();
    }

    _tm1637Stop();
}

void tm1637DisplayDecimal(int dec, int displaySeparator)
{
    char digitArr[4];
    for(int i = 0; i < 4; ++i) {
        digitArr[i] = segmentMap[dec % 10];
        if(i == 2 && displaySeparator) {
            digitArr[i] |= 1 << 7;
        }
        dec /= 10;
    }

    _tm1637Start();
    _tm1637WriteByte(0x40);
    _tm1637ReadResult();
    _tm1637Stop();

    _tm1637Start();
    _tm1637WriteByte(0xc0);
    _tm1637ReadResult();

    for(int i = 0; i < 4; ++i) {
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
    for(int i = 0; i < 8; ++i) {
        _tm1637ClkLow();
        if(b & 0x01) {
            _tm1637DioHigh();
        }
        else {
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
    for(; i > 0; i--) {
        for(int j = 0; j < 10; ++j) {
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
