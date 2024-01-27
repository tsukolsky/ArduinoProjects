#include "Arduino.h"
#include "KatoSwitches.h"


SwitchTwoPin::SwitchTwoPin(BoardPins* pins, int inputpin, int enablepin, char* friendlyName)
{
  /*
  Enable pin is output
  One pin is INPUT_PULLUP --> the circuit needs to have LED diode with resistor goign to ground and our connection is ABOVE that. When switch is 5V, L298 gets 5V above diode as well
  as this pin, so the current is not impacted.
  When pin goes LOW, then a toggle happened and the ENABLE pin shoudl be driven
  When pin goes HIGH, then a toggle happened and the ENABLE pin should be driven
  */
  _pins = pins;
  _initialized = false;
  _inputPin = inputpin;
  _enablePin = enablepin;
  _switchState = SwitchState::SWITCH_CLOSED;
  _lockout = false;
  _debug = false;
  strncpy(this->_switchName, friendlyName, 10);
}

void SwitchTwoPin::Initialize()
{
  _pins->SetPinDirection(_inputPin, INPUT_PULLUP);
  _pins->SetPinDirection(_enablePin, OUTPUT);
  
  // Initial everyting based off of initial state
  ChangeSwitchState();
  Serial.println(String(this->_switchName) + ": Initialized");
  _initialized = true;
}

void SwitchTwoPin::PrintDebug(const String &s)
{
  if (_debug)
  		Serial.println(String(this->_switchName) + ": " + s);
}

bool SwitchTwoPin::IsTurn()
{
  PrintDebug(String(this->_switchName) + ": Current switch state is " + String(this->_switchState));
  // TODO: Identify the correct pin correlation between L298 Output and switch, and L298 IN and OUT
  return this->_switchState;  // If the switch is closed, it is in turn position * This requires the correct wiring --> XXX high makes it turn, which is XXX on L298N
}

bool SwitchTwoPin::Lockout()
{
  return this->_lockout;
}

void SwitchTwoPin::Monitor()
{
  //Serial.println(String(this->_switchName) + ": Current state " + String(this->switchstate));
  if (this->_driveTimer.Ended())
  {
    // Not in a drive situation - make sure the enable pin is LOW, the input pins should be LEFT AS THEY ARE
    this->_lockout = false;
    _pins->PinWrite(_enablePin, LOW);
    
    // First, read in the current button value
    int buttonVal = _pins->PinRead(_inputPin);
    this->PrintDebug("Button Val is " + String(buttonVal));
    switch (this->_switchState)
    {
      case SwitchState::SWITCH_CLOSED:
      {
        // Switch is on (close = 1), so look for falling edge
        if (buttonVal == SwitchState::SWITCH_OPEN)
        {
          // Falling edge
          PrintDebug("Switch toggle - opened!");
          this->ChangeSwitchState();
        }
        break;
      }
      case SwitchState::SWITCH_OPEN:
      {
        if (buttonVal == SwitchState::SWITCH_CLOSED)
        {
          // Rising edge
          PrintDebug("Switch toggle - closed!");
          this->ChangeSwitchState();
        }
        break;
      }
      default:
        break;
    }

    // Update button state - 
    this->_switchState = buttonVal;
  }
  else
  {
    // In a drive situation - do nothing. there is a race condition if two button presses are detected before this happens.
    // In this situation, the pins would be updated and the drive time would be increased, so maybe it wouldn't happen. Regardless, 
    // don't do anything in this situation
    // Ignore button states as well - this is a lockout situation
    PrintDebug("LOCKOUT");
  }
}

void SwitchTwoPin::ChangeSwitchState()
{
  if (!_initialized)
  {
    PrintDebug("Not initialized");
    return;
  }

  // Switch the digital pins and then drive it
  _pins->PinWrite(_enablePin, LOW); // just to make sure

  // Now drive it and set the next millis time that this drive action can be stopped
  this->_lockout = true;
  _pins->PinWrite(_enablePin, HIGH);
  this->_driveTimer.Set(this->_minDriveTime);
  PrintDebug("Current time " + String(millis()));
  PrintDebug("Drive Until Time " + String(this->_driveTimer.EndTime()));
}
