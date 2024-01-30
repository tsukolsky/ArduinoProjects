#ifndef KATO_SWITCHES_H_
#define KATO_SWITCHES_H_

#include <Arduino.h>
#include <Timer.h>
#include <BoardPins.h>


enum SwitchState 
{
  SWITCH_OPEN = 0,
  SWITCH_CLOSED = 1,
};
/*******************************
*
* Two Pin Switch Class
*
********************************/
class SwitchTwoPin
{
  /*
  This class is meant to control the logic and pinout voltage of Kato Switches using an external L298N driver board. It recognizes inputs from an external switch, and
  displays the if the switch is driven straight or turned via separate LEDs that are controlled by the 3 pin switch. 
  Inputs: Switch Digital Reading -- measured with pull down resistor (1k-10k)
  Outputs: Enable Pin -- L298 EN pin to drive the set voltage. 
           Switch Straight Pin -- L298 input A to drive switch straight
           Switch Turn Pin -- L298 input B to drive switch to a turn
  */
  private:
    int _inputPin;
    int _enablePin;
    bool _initialized;
    char _switchName[10];
    SwitchState _switchState; 
    char debugstr[10];
    const int _minDriveTime = 50; // We have to drive the pin for more than this time to get it to actually switch
    unsigned long _driveUntilTime;  // This will roll over when the millis() call also rolls over
    Timer _driveTimer;
    bool _lockout;
    BoardPins* _pins;
    bool _debug;
    
  public:
    SwitchTwoPin(BoardPins* pins, int inputpin, int enablepin, char* friendlyName);
    void Initialize();
    void PrintDebug(const String &s);
    bool IsTurn();
    bool Lockout();
    void Monitor();
    void ChangeSwitchState();
};

#endif