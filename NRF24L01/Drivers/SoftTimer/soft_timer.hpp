/**
 ******************************************************************************
 * @file    soft_timer.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    04-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 *
 ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOFT_TIMER_HPP
#define __SOFT_TIMER_HPP

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <time.h>

class SoftTimer {
  public:
    SoftTimer(time_t value, time_t (*callback)())
    {
        _GetClock = callback;
        _Time = _GetClock() + value;
    };

    void Start(time_t value)
    {
        _Time = _GetClock() + value;
    };

    void Stop()
    {
        _Time = NULL;
    };

    bool Match()
    {
        return (_Time <= _GetClock());
    };

    bool Stopped()
    {
        return (NULL == _Time);
    };

    bool Started()
    {
        return (!Started());
    };

    time_t OverTime()
    {
        return (_GetClock() - _Time);
    };

    time_t RemainingTime()
    {
        return (_Time - _GetClock());
    };

  private:
    time_t (*_GetClock)();    ///< Callback handler function

    volatile time_t _Time;
};

#endif