// User functions //
#include "includes.h"
#include "pcf8574.h"

// Внутрення переменная
uint8_t ucBacklightState;

void lcd_send(uint8_t data);
void lcd_command(uint8_t com);
void lcd_data(uint8_t com);

void PCF8574_I2C_Init(void)
{
    I2C_InitTypeDef I2C_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable I2C and GPIO clocks */
    RCC_APB1PeriphClockCmd(PCF8574_I2C_RCC_Periph, ENABLE);
    RCC_APB2PeriphClockCmd(PCF8574_I2C_RCC_Port, ENABLE);

    /* Configure I2C pins: SCL and SDA */
    GPIO_InitStructure.GPIO_Pin = PCF8574_I2C_SCL_Pin | PCF8574_I2C_SDA_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(PCF8574_I2C_Port, &GPIO_InitStructure);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;    // PCF8574 7-bit adress = 0x1E;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = PCF8574_I2C_Speed;

    /* Apply I2C configuration after enabling it */
    I2C_Init(PCF8574_I2C, &I2C_InitStructure);

    I2C_Cmd(PCF8574_I2C, ENABLE);
}

void LcdPcf_Init(void)
{
    lcd_command(0x33);
    lcd_pause;
    lcd_command(0x32);    // установка режима: 4 линии
    lcd_command(0x28);    // 2 строки и 5*8 точек
    lcd_command(0x08);    // выключить отображение
    lcd_command(0x01);    // очистка экрана
    lcd_pause;
    lcd_command(0x06);    // направление сдвига курсора
    lcd_command(0x0C);    // включить отображение
}

void prit_lcd(const char* str)
{
    uint8_t i;
    while(i = *str++) {
        lcd_data(i);
    }
}

void LcdPcf_Goto(uint8_t row, uint8_t col)
{
    switch(row) {
        case 1:
            lcd_command(0x80 + col);
            break;
        case 2:
            lcd_command(0x80 + col + 0x40);
            break;
        case 3:
            lcd_command(0x80 + col + 0x14);
            break;
        case 4:
            lcd_command(0x80 + col + 0x54);
            break;
    }
}

void LcdPcf_Backlight(uint8_t state)
{
    ucBacklightState = (state & 0x01) << BL;
    lcd_send(ucBacklightState);
}

void lcd_data(uint8_t com)
{
    uint8_t data = 0;

    data |= (1 << EN);
    data |= (1 << RS);
    data |= ucBacklightState;

    data |= (((com & 0x10) >> 4) << DB4);
    data |= (((com & 0x20) >> 5) << DB5);
    data |= (((com & 0x40) >> 6) << DB6);
    data |= (((com & 0x80) >> 7) << DB7);
    lcd_send(data);
    lcd_pause;

    data &= ~(1 << EN);
    lcd_send(data);
    lcd_pause;

    data = 0;

    data |= (1 << EN);
    data |= (1 << RS);
    data |= ucBacklightState;

    data |= (((com & 0x01) >> 0) << DB4);
    data |= (((com & 0x02) >> 1) << DB5);
    data |= (((com & 0x04) >> 2) << DB6);
    data |= (((com & 0x08) >> 3) << DB7);
    lcd_send(data);
    lcd_pause;

    data &= ~(1 << EN);
    lcd_send(data);
    lcd_pause;
}

void lcd_command(uint8_t com)
{
    uint8_t data = 0;

    data |= ucBacklightState;

    data |= (((com & 0x10) >> 4) << DB4);
    data |= (((com & 0x20) >> 5) << DB5);
    data |= (((com & 0x40) >> 6) << DB6);
    data |= (((com & 0x80) >> 7) << DB7);
    lcd_send(data);

    data |= (1 << EN);
    lcd_send(data);
    lcd_pause;

    data &= ~(1 << EN);
    lcd_send(data);
    lcd_pause;

    data = 0;

    data |= ucBacklightState;

    data |= (((com & 0x01) >> 0) << DB4);
    data |= (((com & 0x02) >> 1) << DB5);
    data |= (((com & 0x04) >> 2) << DB6);
    data |= (((com & 0x08) >> 3) << DB7);
    lcd_send(data);

    data |= (1 << EN);
    lcd_send(data);
    lcd_pause;

    data &= ~(1 << EN);
    lcd_send(data);
    lcd_pause;
}

void lcd_send(uint8_t data)
{
    while(I2C_GetFlagStatus(PCF8574_I2C, I2C_FLAG_BUSY))
        ;
    I2C_GenerateSTART(PCF8574_I2C, ENABLE);

    while(!I2C_CheckEvent(PCF8574_I2C, I2C_EVENT_MASTER_MODE_SELECT))
        ;
    I2C_Send7bitAddress(PCF8574_I2C, ((0x20 + PCF8574_ADDRESS) << 1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(PCF8574_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    I2C_SendData(PCF8574_I2C, data);
    while(!I2C_CheckEvent(PCF8574_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    I2C_GenerateSTOP(PCF8574_I2C, ENABLE);
}
