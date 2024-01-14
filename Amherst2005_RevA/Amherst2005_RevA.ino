#include <Adafruit_MCP23X17.h>

//#define DEBUG 
#define TEST_TURN_BLINK
#define TURN_BLINK_HZ     5
#define DELAY_MS          5

void DPRINTLN(const String &s)
{
#ifdef DEBUG
  Serial.println(s);
#endif
}

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

enum SwitchState
{
  SW_STRAIGHT = 0,
  SW_TURN = 1,
};

enum ButtonState 
{
  BUTTON_PRESSED = 0,
  BUTTON_OPEN = 1,
};

// Initialize adafruit mcp variable so pin modes know what to write to - these routines could be collapsed into the Switch class, but it's really a global
// for the arduino in this implementation. it shouldn't be controlled by any one switch. Truthfully, it should be provided by the main class as a pointer
// but whatever
Adafruit_MCP23X17 mcp;



/*******************************
*
* Pin Helper Classes (Arduino vs MCP23017)
*
********************************/

class BoardPins
{
  public:
    BoardPins();
    BoardPins(Adafruit_MCP23X17* mcp);
    bool BSetPinDirection(int pinNumber, int direction);
    int BPinRead(int pinNumber);
    bool BPinWrite(int pinNumber, int value);
  private:
    Adafruit_MCP23X17* mcp;
};

BoardPins::BoardPins()
{
  mcp = 0;
}

BoardPins::BoardPins(Adafruit_MCP23X17* mcp)
{
  this->mcp = mcp;
}

// Pin helper functions to deconflict UNO GPIO pins and MCP23017 GPIO pins (via I2C)
bool SetPinDirection(int pinNumber, int direction)
{
  bool ret = false;
  if (pinNumber >= MIN_PIN && pinNumber <= MAX_PIN)
  {
    if (pinNumber >= (int) Pin::MCP23017_0)
    {
      // This should be written over I2C bus to digital IO pin
      mcp.pinMode(pinNumber, direction);
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

int PinRead(int pinNumber)
{
  int ret = 0;  // default 0, this is not valid if there is a pull-up so that is the constraint on this class/implementation
  if (pinNumber >= MIN_PIN && pinNumber <= MAX_PIN)
  {
    if (pinNumber >= (int) Pin::MCP23017_0)
    {
      // This should be read over I2C bus to digital IO pin
      pinNumber -= Pin::MCP23017_0;
      ret = mcp.digitalRead(pinNumber);
    }
    else
    {
      // This is an Arduino pin, get over normal bus
      ret = digitalRead(pinNumber);
    }
  }
  return ret;
}

bool PinWrite(int pinNumber, int value)
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
      mcp.digitalWrite(pinNumber, value);
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

/*******************************
*
* Four Pin Switch Class
*
********************************/
class SwitchFourPin
{
  /*
  This class is meant to control the logic and pinout voltage of Kato Switches using an external L298N driver board. It recognizes inputs from an external button, and
  displays the if the switch is driven straight or turned via separate LEDs. 
  Inputs: Button Digital Reading -- measured with pull down resistor (1k-10k)
  Outputs: Enable Pin -- L298 EN pin to drive the set voltage. 
           Switch Straight Pin -- L298 input A to drive switch straight
           Switch Turn Pin -- L298 input B to drive switch to a turn
  */
  private:
    int switchStraightPin;
    int switchTurnPin;
    int buttonpin;
    int enablePin;
    char _switchName[10];
    SwitchState switchstate;
    ButtonState buttonstate; 
    char debugstr[10];
    const int _minDriveTime = 25; // We have to drive the pin for more than this time to get it to actually switch
    unsigned long _driveUntilTime;  // This will roll over when the millis() call also rolls over
    Timer _driveTimer;
    bool _lockout;

  public:
    SwitchFourPin(int straightswitchpin, int switchturnpin, int buttonpin, int enablepin, char* friendlyName) 
    {
      this->switchStraightPin = straightswitchpin; 
      this->switchTurnPin = switchturnpin;
      this->buttonpin = buttonpin;
      this->enablePin = enablepin; 
      this->switchstate = SwitchState::SW_TURN;
      this->buttonstate = ButtonState::BUTTON_OPEN;
      this->_lockout = false;
      strncpy(this->_switchName, friendlyName, 10);
      SetPinDirection(straightswitchpin, OUTPUT);
      SetPinDirection(switchturnpin, OUTPUT);
      SetPinDirection(buttonpin, INPUT_PULLUP);
      SetPinDirection(enablepin, OUTPUT);
      
      // Initial everyting based off of initial state
      ChangeSwitchState();
      Serial.println(String(this->_switchName) + ": Initialized");
    }

    void PrintDebug(const String &s)
    {
      DPRINTLN(String(this->_switchName) + ": " + s);
    }

    bool IsTurn()
    {
      PrintDebug(String(this->_switchName) + ": Current switch state is " + String(this->switchstate));
      return this->switchstate == SwitchState::SW_TURN;
    }

    bool Lockout()
    {
      return this->_lockout;
    }

    void WriteStraightPin(int value)
    {
      if (!this->_lockout)
      {
        PinWrite(this->switchStraightPin, value);
      }
    }

    void Monitor()
    {
      //Serial.println(String(this->_switchName) + ": Current state " + String(this->switchstate));
      if (this->_driveTimer.Ended())
      {
        // Not in a drive situation - make sure the enable pin is LOW, the input pins should be LEFT AS THEY ARE
        this->_lockout = false;
        PinWrite(this->enablePin, LOW);
        
        // First, read in the current button value
        int buttonVal = PinRead(buttonpin);
        switch (this->buttonstate)
        {
          case ButtonState::BUTTON_OPEN:
          {
            // Button is off (high), so look for falling edge
            if (buttonVal == ButtonState::BUTTON_PRESSED)
            {
              // Falling edge
              PrintDebug("Button Press!");
              this->ChangeSwitchState();
            }
            break;
          }
          default:
            // IN scenario when button is pressed, we already know it was pressed so do nothing. if depressed, still do nothing
            break;
        }

        // Update button state - 
        this->buttonstate = buttonVal;
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

    void ChangeSwitchState()
    {
      // Switch the digital pins and then drive it
      PinWrite(this->enablePin, LOW); // just to make sure
      if (this->switchstate == SwitchState::SW_STRAIGHT)
      {
        // It should now be a turn
        this->switchstate = SwitchState::SW_TURN;
        PinWrite(this->switchStraightPin, LOW);
        PinWrite(this->switchTurnPin, HIGH);
      }
      else
      {
        this->switchstate = SwitchState::SW_STRAIGHT;
        PinWrite(this->switchTurnPin, LOW);
        PinWrite(this->switchStraightPin, HIGH);
      }

      // Now drive it and set the next millis time that this drive action can be stopped
      this->_lockout = true;
      PinWrite(this->enablePin, HIGH);
      this->_driveTimer.Set(this->_minDriveTime);
      PrintDebug("Current time " + String(millis()));
      PrintDebug("Drive Until Time " + String(this->_driveUntilTime));
    }
};
/*****************************************************************************************
/
/
/                       MAIN PROGRAM BEGIN - GLOBALS + MAIN
/
/
/
*****************************************************************************************/

// The MCP23017 provides 16 IO pins. The Arduino has pins 0-13 easily accessible, then I2C on the top to address the remaining 16. Therefore, for 7 switches all
// logic turn/straight pins will be on the arduino 0-13, with button input and enable pins on the MCP23017 breakout board
// int straightswitchpin, int switchturnpin, int buttonpin, int enablepin) 
const int NUM_SWITCHES = 7;

SwitchFourPin switches[7] = {SwitchFourPin(Pin::ARDUINO_14, Pin::ARDUINO_15, Pin::MCP23017_0, Pin::MCP23017_1, "Switch 0"),
                             SwitchFourPin(Pin::ARDUINO_2, Pin::ARDUINO_3, Pin::MCP23017_2, Pin::MCP23017_3, "Switch 1"),
                             SwitchFourPin(Pin::ARDUINO_4, Pin::ARDUINO_5, Pin::MCP23017_4, Pin::MCP23017_5, "Switch 2"),
                             SwitchFourPin(Pin::ARDUINO_6, Pin::ARDUINO_7, Pin::MCP23017_6, Pin::MCP23017_7, "Switch 3"),
                             SwitchFourPin(Pin::ARDUINO_8, Pin::ARDUINO_9, Pin::MCP23017_8, Pin::MCP23017_9, "Switch 4"),
                             SwitchFourPin(Pin::ARDUINO_10, Pin::ARDUINO_11, Pin::MCP23017_10, Pin::MCP23017_11, "Switch 5"),
                             SwitchFourPin(Pin::ARDUINO_12, Pin::ARDUINO_13, Pin::MCP23017_12, Pin::MCP23017_13, "Switch 6"),
                            };

Timer strobeTimer = Timer(0);
unsigned short turnBlinkLightOn;
unsigned long testTime;

#ifdef TEST_TURN_BLINK
Timer testTimer = Timer(0);
#endif

void setup() {
  // Setup each switch
  Serial.begin(9600);
  // Initialize the Power Controller - set power to 0. This variable is global and is used by the Button Controller class
  turnBlinkLightOn = 0; // by default, turn them off on first check (it will toggle from this)
  strobeTimer.Set((unsigned long) (1000/TURN_BLINK_HZ));
#ifdef TEST_TURN_BLINK
  strobeTimer.Set(10*1000); // 10 seconds
#endif
}

void loop() 
{
  // Check to see if a button is currently depressed on any of the switches/buttons - if so, change the switch
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    switches[i].Monitor();
  }

  // All of the switches that are currently in the TURN position should blink at 1 Hz (# defined somewhere else).
  // Any switch that is straight should be SOLID. So...how to synchronize...you basically have to borrow the pins to write to them, but only if they are not in an active drive situation
  // If they are in an active drive situation, you need to wait until they are not then drive them along with the others
  // Option 1 - Gather all the switches right here that are in each state, then drive the pins as they should be together...do this every single time...this might cause an overwork
  {
    // Or do it every 10 milliseconds; that seems reasonable? hint hint it's not really every 10 ms with this implementation...
    // For solids, it's easy - leave the straight pin as it is (THIS REQUIRES THE STRAIGHT PIN TO BE CONNECTED TO THE LED!!!)
    // If turned (STRAIGHT PIN WILL BE LOW), we need to change the straight pin to high for X milliseconds (corresponding to HZ)
    //  The X milliseconds is baesd on the last clock edge, so we need to make a clock with variables
    //  Beacuse our switch implementation sets the TURN and STARIGHT pins with enable off, then brings enable high, we can mess with them as long as the switch is not in LOCKOUT
    if (strobeTimer.Ended())
    {
      turnBlinkLightOn = !turnBlinkLightOn;
      for (int i = 0; i < NUM_SWITCHES; i++)
      {
        // IF not in lockout, write the straight pin value to be what all the others are
        if (!switches[i].Lockout() && switches[i].IsTurn())
        {
          DPRINTLN("Switch " + String(i) + " is not in lockout and is turn");
          DPRINTLN("Writing " + String(turnBlinkLightOn) + " to straight pin");
          switches[i].WriteStraightPin(turnBlinkLightOn);
        }
      }
      strobeTimer.Reset();
      DPRINTLN("Next blink time is " + String(strobeTimer.EndTime()));
    }
  }

  delay(DELAY_MS);

#ifdef TEST_TURN_BLINK
  // Tester
  if (testTimer.Ended())
  {
    // Force change in switch state
    switches[1].ChangeSwitchState();
    testTimer.Reset();
  }
#endif
}
