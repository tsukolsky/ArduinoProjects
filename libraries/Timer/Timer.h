#ifndef TIMER_H_
#define TIMER_H_

#include <Arduino.h>


/*******************************
*
* Timer Class
*
********************************/

class Timer
{
  public:
    Timer();
    Timer(unsigned long duration);
    bool Set(unsigned long duration);
    bool Reset();
    bool Ended();
    unsigned long EndTime();
    unsigned long Duration();
    unsigned long StartTime();

  private:
    unsigned long _startTime;
    unsigned long _endTime;
    unsigned long _duration;
};

#endif