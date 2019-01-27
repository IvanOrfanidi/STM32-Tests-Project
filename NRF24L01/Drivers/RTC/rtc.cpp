/**
 ******************************************************************************
 * @file    rtc.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file provides all the RTC firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc.hpp"


#define leapYear(year) ((year % 4) ? 0 : 1)


const uint8_t mon_len[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


/*
 * Main array pointers of classes Callback
 */
void (*Rtc::Callback)() = nullptr;


/**
 * @brief Empty function
*/
static void Nop()
{
    __NOP;
}


/**
 * @bref Initialisation RTC
 * @param rtc - Data
 */
void Rtc::Init()
{   
    // Enable PWR and BKP clocks 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    
    // Enable the LSI OSC 
    RCC_LSICmd(ENABLE);

    // Wait till LSI is ready 
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    if(BKP_ReadBackupRegister(RTC_BKP_REGISTER) != RTC_FLAG_ENABLE)
    {
        /* Backup data register value is not correct or not yet programmed 
           (when the first time the program is executed) */

        // Allow access to BKP Domain 
        PWR_BackupAccessCmd(ENABLE);

        // Reset Backup Domain 
        BKP_DeInit();

        // Enable LSE 
        RCC_LSEConfig(RCC_LSE_ON);

        // Wait till LSE is ready 
        uint32_t Timeout = 100000;
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
            Timeout--;
            if(!(Timeout))
                break;
        }

        if(Timeout) {   // LSE - OK.
            
            // Select LSE as RTC Clock Source 
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);    // LSE = 32.768 KHz

            // Enable RTC Clock 
            RCC_RTCCLKCmd(ENABLE);

            // Wait for RTC registers synchronization 
            RTC_WaitForSynchro();

            // Wait until last write operation on RTC registers has finished 
            RTC_WaitForLastTask();

            // Set RTC prescaler: set RTC period to 1sec 
            RTC_SetPrescaler(LSE_PRESCALER); // RTC period = RTCCLK/RTC_PR = (32.768KHz)/(32767+1) 
        }
        else {   // LSE - NO.
            
            uint32_t count = 10000;

            // Enable the LSI OSC
            RCC_LSICmd(ENABLE);

            // Wait till LSI is ready
            while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && (--count) > 0)
            {
                __NOP();
                __NOP();
                __NOP();
                __NOP();
            }

            // Select the RTC Clock Source
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

            // Enable RTC Clock
            RCC_RTCCLKCmd(ENABLE);

            // Wait for RTC registers synchronization
            RTC_WaitForSynchro();

            // Wait until last write operation on RTC registers has finished
            RTC_WaitForLastTask();

            // Set RTC prescaler: set RTC period to 1sec
            RTC_SetPrescaler(LSI_PRESCALER);
        }

        // Wait until last write operation on RTC registers has finished 
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(RTC_BKP_REGISTER, RTC_FLAG_ENABLE);

        // Lock access to BKP Domain 
        PWR_BackupAccessCmd(DISABLE);
    }
}


void Rtc::InitAlarm(void (*function)(), uint8_t preemption_priority, uint8_t sub_priority)
{
    if(function) {
        Callback = function;
    }
    else {
        Callback = &Nop;
    }
    
    NVIC_InitTypeDef rtcInitStructure;

    // Enable the RTC Interrupt
    rtcInitStructure.NVIC_IRQChannel = RTC_IRQn;
    rtcInitStructure.NVIC_IRQChannelPreemptionPriority = preemption_priority;
    rtcInitStructure.NVIC_IRQChannelSubPriority = sub_priority;
    rtcInitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&rtcInitStructure);
}


void Rtc::ResetAlarm()
{
    // Enable the RTC Alarm
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
    
    RTC_SetAlarm(0);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
}


void Rtc::SetAlarm(time_t cnt)
{
    RTC_SetAlarm(cnt);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
    
    // Enable the RTC Alarm
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
}


void Rtc::SetAlarm(const RTC_t* const rtc)
{
    // converting date to sec
    const uint32_t cnt = DateToSec(rtc);
    
    RTC_SetAlarm(cnt);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
    
    // Enable the RTC Alarm
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
}


/**
 * @bref  Sets RTC current time and date
 * @param cnt - Time UNIX (по UTC с 31 декабря 1969 года на 1 января 1970)
 */
void Rtc::SetTime(time_t cnt)
{   
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
    RTC_SetCounter(cnt);

    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
}


/**
 * @bref  Sets RTC current time and date
 * @param [in] rtc - Time and date in formate RTC_t
 */
void Rtc::SetTime(const RTC_t* const rtc)
{
    // converting date to sec
    const uint32_t cnt = DateToSec(rtc);
    
    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
    RTC_SetCounter(cnt);

    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
}


/**
 * @bref Geting current time
 * @retval Current time
 */
time_t Rtc::GetTime()
{
    return RTC_GetCounter();
}


/**
 * @brief Geting current time
 * @param [out] rtc - Current time
 */
void Rtc::GetTime(RTC_t* rtc)
{
    SecToDate(rtc, RTC_GetCounter());
}


/**
 * @bref Enable the LSI OSC
 * @retval true - success, false - fail
 */
bool Rtc::LseEnable()
{
    uint32_t count = 0xFFFFFFFF;

    // LSE Enable
    RCC_LSEConfig(RCC_LSE_ON);

    // Wait till LSE is ready
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && (--count) > 0) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
    
    if(0 == count) {
        return false;
    }

    // LCD Clock Source Selection
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    // Enable RTC Clock 
    RCC_RTCCLKCmd(ENABLE);

    // Wait for RTC registers synchronization 
    RTC_WaitForSynchro();

    // Wait until last write operation on RTC registers has finished 
    RTC_WaitForLastTask();

    // Set RTC prescaler: set RTC period to 1sec 
    RTC_SetPrescaler(32767);    // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)

    // Wait until last write operation on RTC registers has finished 
    RTC_WaitForLastTask();

    // Wait for RTC registers synchronization 
    RTC_WaitForSynchro();

    return true;
}


/**
 * @bref Enable the LSI OSC
 * @retval true - success, false - fail
 */
bool Rtc::LsiEnable()
{
    uint32_t count = 10000;

    // Enable the LSI OSC
    RCC_LSICmd(ENABLE);

    // Wait till LSI is ready
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && (--count) > 0) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
    
    if(0 == count) {
        return false;
    }

    // Select the RTC Clock Source
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    // Enable RTC Clock 
    RCC_RTCCLKCmd(ENABLE);

    // Wait for RTC registers synchronization 
    RTC_WaitForSynchro();

    // Wait until last write operation on RTC registers has finished 
    RTC_WaitForLastTask();

    // Set RTC prescaler: set RTC period to 1sec 
    RTC_SetPrescaler(40000);    // RTC period = RTCCLK/RTC_PR = (~40 KHz)

    // Wait until last write operation on RTC registers has finished 
    RTC_WaitForLastTask();

    // Wait for RTC registers synchronization 
    RTC_WaitForSynchro();

    return true;
}


/**
 * @brief Converting date to sec
 * @param [in] rtc - date
 * @retval Time
 */
time_t Rtc::DateToSec(const RTC_t* rtc)
{
    uint32_t time = 0;
    uint32_t tmp = 0;
    uint16_t year;

    year = rtc->Year - 2000;

    if(year < 70)
    {
        time += 946684800L;    //Дата позже 1 января 2000 года
        for(tmp = 0; tmp < year; time += 86400L * (365L + leapYear(tmp)), tmp++);
    }
    else
    {
        for(tmp = 70; tmp < year; time += 86400L * (365L + leapYear(tmp)), tmp++);
    }

    for(tmp = 0; tmp < rtc->Month; time += (86400L * (mon_len[tmp] + ((tmp == 2) ? leapYear(year) : 0))), tmp++);

    time += (86400L) * (rtc->Mday - 1);
    time += 3600L * rtc->Hour;
    time += 60L * rtc->Min;
    time += rtc->Sec;

    return time;
}


/**
 * @brief Convert sec to date
 * @param [out] rtc - date
 * @param [in] sec - time
 */
void Rtc::SecToDate(RTC_t* rtc, time_t sec)
{
    uint32_t dl = 0;

    if(sec >= 946684800L)
    {
        sec -= 946684800L;    //Дата позже 1 января 2000 года
        rtc->Year = 0;
    }
    else
    {
        rtc->Year = 70;    //Дата от 1970 года до 1999 года
    }

    for(dl = 365L; sec >= (dl = 86400L * (365L + leapYear(rtc->Year))); sec -= dl, rtc->Year++);

    for(rtc->Month = 1;
        sec >= (dl = 86400L * (mon_len[rtc->Month] + ((rtc->Month == 2) ? leapYear(rtc->Year) : 0)));
        sec -= dl, rtc->Month++);

    rtc->Year += 2000;
    rtc->Mday = sec / (86400L) + 1;
    sec %= 86400L;
    rtc->Hour = sec / 3600L;
    sec %= 3600L;
    rtc->Min = sec / 60L;
    sec %= 60L;
    rtc->Sec = sec;
    rtc->Wday = GetWeekDayNum(rtc->Year, rtc->Month, rtc->Mday);
}


/**
 * @brief Determines the week number, the day number and the week day number
 * @param [in] year - year to check
 * @param [in] month - month to check
 * @param [in] day - day to check
 * @note   Day is calculated with hypothesis that year > 2000
 * @retval Week day (0 - SUNDAY)
 */
uint8_t Rtc::GetWeekDayNum(uint32_t year, uint8_t month, uint8_t day)
{
    uint32_t weekday = 0;

    if(month < 3) {
        //D = { [(23 x month)/9] + day + 4 + year + [(year-1)/4] - [(year-1)/100] + [(year-1)/400] } mod 7
        weekday = (((23 * month)/9) + day + 4 + year + ((year-1)/4) - ((year-1)/100) + ((year-1)/400)) % 7;
    }
    else {
        //D = { [(23 x month)/9] + day + 4 + year + [year/4] - [year/100] + [year/400] - 2 } mod 7
        weekday = (((23 * month)/9) + day + 4 + year + (year/4) - (year/100) + (year/400) - 2 ) % 7; 
    }

    return weekday;
}


/**
 * @brief Converts a 2 digit decimal to BCD format
 * @param [in] value - Byte to be converted
 * @retval Converted byte
 */
uint8_t Rtc::ByteToBcd2(uint8_t value)
{
    uint32_t bcdhigh = 0;

    while(value >= 10) {
        bcdhigh++;
        value -= 10;
    }

    return ((uint8_t)(bcdhigh << 4) | value);
}


/**
 * @brief Converts from 2 digit BCD to Binary
 * @param [in] value - BCD value to be converted
 * @retval Converted word
 */
uint8_t Rtc::Bcd2ToByte(uint8_t value)
{
    uint32_t tmp = ((value & 0xF0) >> 0x04) * 10;
    return (tmp + (value & 0x0F));
}


/**
 * @bref Private interrupt handler
 */
void Rtc::Handler()
{
   (*Callback)();
}


/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
    {
        // Clear the RTC Second interrupt
        RTC_ClearITPendingBit(RTC_IT_ALR);

        Rtc::Handler();

        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();

    }
}