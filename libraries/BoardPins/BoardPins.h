#ifndef BOARDPINS_H_
#define BOARDPINS_H_

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

enum Pin
{
  ARDUINO_2 = 2,
  ARDUINO_3,
  ARDUINO_4,
  ARDUINO_5,
  ARDUINO_6,
  ARDUINO_7,
  ARDUINO_8,
  ARDUINO_9,
  ARDUINO_10,
  ARDUINO_11,
  ARDUINO_12,
  ARDUINO_13,
  ARDUINO_14,
  ARDUINO_15,
  MCP23017_0,
  MCP23017_1,
  MCP23017_2,
  MCP23017_3,
  MCP23017_4,
  MCP23017_5,
  MCP23017_6,
  MCP23017_7,
  MCP23017_8,
  MCP23017_9,
  MCP23017_10,
  MCP23017_11,
  MCP23017_12,
  MCP23017_13,
  MCP23017_14,
  MCP23017_15,
};


#define MIN_PIN (int) Pin::ARDUINO_2
#define MAX_PIN (int) Pin::MCP23017_15

class BoardPins
{
  public:
    BoardPins();
    BoardPins(Adafruit_MCP23X17* mcp);
    bool SetPinDirection(int pinNumber, int direction);
    int PinRead(int pinNumber);
    bool PinWrite(int pinNumber, int value);
  private:
    Adafruit_MCP23X17* _mcp;
};

#endif