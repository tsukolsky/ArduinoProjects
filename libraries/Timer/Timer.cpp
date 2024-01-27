#include "Arduino.h"
#include "Timer.h"

Timer::Timer()
{
  this->Set(0);
}

Timer::Timer(unsigned long duration)
{
  this->Set(duration);
}

bool Timer::Reset()
{
  _startTime = millis();
  _endTime = _startTime + _duration;
  return true;
}

unsigned long Timer::Duration()
{
  return _duration;
}
unsigned long Timer::EndTime()
{
  return _endTime;
}

unsigned long Timer::StartTime()
{
  return _startTime;
}

bool Timer::Set(unsigned long duration)
{
  _startTime = millis();
  _endTime = _startTime + duration;
  _duration = duration;
  return true;
}

bool Timer::Ended()
{
  return millis() >= _endTime;
}