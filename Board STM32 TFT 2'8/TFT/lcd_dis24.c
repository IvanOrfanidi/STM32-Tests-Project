
#include "lcd_dis24.h"

#define Bank1_LCD_D ((uint32_t)0x60020000)    //disp Data ADDR
#define Bank1_LCD_C ((uint32_t)0x60000000)    //disp Reg ADDR

//void MUC_Init();
void LCD_Init(void);
void LCD_WR_REG(unsigned int index);
void LCD_WR_CMD(unsigned int index, unsigned int val);

void LCD_WR_Data(unsigned int val);
void LCD_WR_Data_8(unsigned int val);
void LCD_test(void);
void LCD_clear(unsigned int p);

unsigned int IndexDataADDR;
unsigned int ValRegADDR;

unsigned int LCD_RD_data(void);
extern void lcd_rst(void);
extern void Delay(__IO uint32_t nCount);

void lcd_wr_pixel(unsigned int a, unsigned int b, unsigned int e);
unsigned char* num_pub(unsigned int a);

extern const unsigned int a3[];

//#pragma optimize=none

void LCD_WR_Data(unsigned int val)
{
    ValRegADDR = val;
    *(__IO uint16_t*)(Bank1_LCD_D) = ValRegADDR;
}

void LCD_WR_Data_8(unsigned int val)
{
    ValRegADDR = val;
    *(__IO uint16_t*)(Bank1_LCD_D) = ValRegADDR;
}

void LCD_Init(void)
{
    lcd_rst();

    Delay(950);    //950
    LCD_WR_CMD(0x0001, 0x0100);
    LCD_WR_CMD(0x0002, 0x0700);
    LCD_WR_CMD(0x0003, 0x1030);
    LCD_WR_CMD(0x0004, 0x0000);
    LCD_WR_CMD(0x0008, 0x0207);
    LCD_WR_CMD(0x0009, 0x0000);
    LCD_WR_CMD(0x000A, 0x0000);
    LCD_WR_CMD(0x000C, 0x0000);
    LCD_WR_CMD(0x000D, 0x0000);
    LCD_WR_CMD(0x000F, 0x0000);
    //power on sequence VGHVGL
    LCD_WR_CMD(0x0010, 0x0000);
    LCD_WR_CMD(0x0011, 0x0007);
    LCD_WR_CMD(0x0012, 0x0000);
    LCD_WR_CMD(0x0013, 0x0000);
    //vgh
    LCD_WR_CMD(0x0010, 0x1290);
    LCD_WR_CMD(0x0011, 0x0227);
    Delay(100);
    //vregiout
    LCD_WR_CMD(0x0012, 0x001d);    //0x001b
    Delay(100);
    //vom amplitude
    LCD_WR_CMD(0x0013, 0x1500);
    Delay(100);
    //vom H
    LCD_WR_CMD(0x0029, 0x0018);
    LCD_WR_CMD(0x002B, 0x000D);

    //gamma
    LCD_WR_CMD(0x0030, 0x0004);
    LCD_WR_CMD(0x0031, 0x0307);
    LCD_WR_CMD(0x0032, 0x0002);    // 0006
    LCD_WR_CMD(0x0035, 0x0206);
    LCD_WR_CMD(0x0036, 0x0408);
    LCD_WR_CMD(0x0037, 0x0507);
    LCD_WR_CMD(0x0038, 0x0204);    //0200
    LCD_WR_CMD(0x0039, 0x0707);
    LCD_WR_CMD(0x003C, 0x0405);    // 0504
    LCD_WR_CMD(0x003D, 0x0F02);
    //ram
    LCD_WR_CMD(0x0050, 0x0000);
    LCD_WR_CMD(0x0051, 0x00EF);
    LCD_WR_CMD(0x0052, 0x0000);
    LCD_WR_CMD(0x0053, 0x013F);
    LCD_WR_CMD(0x0060, 0xA700);
    LCD_WR_CMD(0x0061, 0x0001);
    LCD_WR_CMD(0x006A, 0x0000);
    //
    LCD_WR_CMD(0x0080, 0x0000);
    LCD_WR_CMD(0x0081, 0x0000);
    LCD_WR_CMD(0x0082, 0x0000);
    LCD_WR_CMD(0x0083, 0x0000);
    LCD_WR_CMD(0x0084, 0x0000);
    LCD_WR_CMD(0x0085, 0x0000);
    //
    LCD_WR_CMD(0x0090, 0x0010);
    LCD_WR_CMD(0x0092, 0x0600);
    LCD_WR_CMD(0x0093, 0x0003);
    LCD_WR_CMD(0x0095, 0x0110);
    LCD_WR_CMD(0x0097, 0x0000);
    LCD_WR_CMD(0x0098, 0x0000);
    LCD_WR_CMD(0x0007, 0x0133);

    //	Write_Cmd_Data(0x0022);//

    LCD_WR_CMD(32, 0);
    LCD_WR_CMD(33, 0x013F);
    *(__IO uint16_t*)(Bank1_LCD_C) = 34;

    unsigned long color;
    for(color = 0; color < 76800; color++) {
        LCD_WR_Data(0xffff);
    }
}

void lcd_wr_pixel(unsigned int a, unsigned int b, unsigned int e)
{
    LCD_WR_CMD(0x20, a);
    LCD_WR_CMD(0x21, b);
    LCD_WR_Data(e);
}

void LCD_test(void)
{
    unsigned long n = 0;

    LCD_WR_CMD(0x0003, 0x1018);    //
    LCD_WR_CMD(0x0050, 0);         // Horizontal GRAM Start Address
    LCD_WR_CMD(0x0051, 239);       // Horizontal GRAM End Address
    LCD_WR_CMD(0x0052, 0);         // Vertical GRAM Start Address
    LCD_WR_CMD(0x0053, 319);       // Vertical GRAM Start Address
    LCD_WR_CMD(32, 0);
    LCD_WR_CMD(33, 0);
    //*(__IO uint16_t *) (Bank1_LCD_C)= 34;
    LCD_WR_REG(34);

    //76800//153600
    while(n < 76800) {
        //temp=(uint16_t)( a3[n]<<8)+a3[n+1];
        //temp++;
        LCD_WR_Data(a3[n]);
        n++;
    }
}

void lcd_rst(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_1);
    Delay(0xAFFFf);
    GPIO_SetBits(GPIOE, GPIO_Pin_1);
    Delay(0xAFFFf);
}

void Delay(__IO uint32_t nCount)
{
    for(; nCount != 0; nCount--)
        ;
}

void LCD_WR_REG(unsigned int index)
{
    IndexDataADDR = index;
    *(__IO uint16_t*)(Bank1_LCD_C) = IndexDataADDR;
}

void LCD_WR_CMD(unsigned int index, unsigned int val)
{
    IndexDataADDR = index;
    ValRegADDR = val;
    *(__IO uint16_t*)(Bank1_LCD_C) = IndexDataADDR;
    *(__IO uint16_t*)(Bank1_LCD_D) = ValRegADDR;
}

unsigned int LCD_RD_data(void)
{
    unsigned int a = 0;
    //a=(*(__IO uint16_t *) (Bank1_LCD_D)); 	//Dummy
    //a= *(__IO uint16_t *) (Bank1_LCD_D);  	//H
    //a=a<<8;
    a = *(__IO uint16_t*)(Bank1_LCD_D);    //L

    return (a);
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
