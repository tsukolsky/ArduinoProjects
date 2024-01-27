#include "BoardPins.h"
#include <Adafruit_MCP23X17.h>


BoardPins::BoardPins()
{
  _mcp = 0;
}

BoardPins::BoardPins(Adafruit_MCP23X17* mcp)
{
  this->_mcp = mcp;
}

// Pin helper functions to deconflict UNO GPIO pins and MCP23017 GPIO pins (via I2C)
bool BoardPins::SetPinDirection(int pinNumber, int direction)
{
  bool ret = false;
  if (pinNumber >= MIN_PIN && pinNumber <= MAX_PIN)
  {
    if (pinNumber >= (int) Pin::MCP23017_0)
    {
      // This should be written over I2C bus to digital IO pin
      pinNumber -= Pin::MCP23017_0;
      _mcp->pinMode(pinNumber, direction);
    }
    else
    {
      // This is an UNO pin, set over normal bus
      pinMode(pinNumber, direction);
    }
    ret = true;
  }
  return ret;
}

int BoardPins::PinRead(int pinNumber)
{
  int ret = 0;  // default 0, this is not valid if there is a pull-up so that is the constraint on this class/implementation
  if (pinNumber >= MIN_PIN && pinNumber <= MAX_PIN)
  {
    if (pinNumber >= (int) Pin::MCP23017_0)
    {
      // This should be read over I2C bus to digital IO pin
      pinNumber -= Pin::MCP23017_0;
      ret = _mcp->digitalRead(pinNumber);
    }
    else
    {
      // This is an Arduino pin, get over normal bus
      ret = digitalRead(pinNumber);
    }
  }
  return ret;
}

bool BoardPins::PinWrite(int pinNumber, int value)
{
  bool ret = false;
  // Must be valid pin
  if (pinNumber >= MIN_PIN && pinNumber <= MAX_PIN)
  {
    // If 0 -> 13, then its on the UNO, otherwise its on the MCP and needs to be written over I2C
    if (pinNumber >= (int) Pin::MCP23017_0)
    {
      pinNumber -= Pin::MCP23017_0;
      // This should be written over I2C bus to digital IO pin
      _mcp->digitalWrite(pinNumber, value);
    }
    else
    {
      // This is an Arduino pin, set over normal bus
      digitalWrite(pinNumber, value);
    }
    ret = true;
  }
  return ret;
}