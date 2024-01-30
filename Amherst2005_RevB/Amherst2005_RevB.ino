#include <Adafruit_MCP23X17.h>
#include <Timer.h>
#include <BoardPins.h>
#include <KatoSwitches.h>

// #define DEBUG 
#define DELAY_MS          100


/*****************************************************************************************
/
/
/                       MAIN PROGRAM BEGIN - GLOBALS + MAIN
/
/
/
*****************************************************************************************/

// The MCP23017 provides 16 IO pins. The Arduino has pins 2-17 [only using up to 15] easily accessible, then I2C on the top to address additional pins
// Switch reading pins on the MCP23017 breakout board
// Initialize adafruit mcp variable so pin modes know what to write to - these routines could be collapsed into the Switch class, but it's really a global
// for the arduino in this implementation. it shouldn't be controlled by any one switch. Truthfully, it should be provided by the main class as a pointer
// but whatever
Adafruit_MCP23X17 mcp;
Timer strobeTimer = Timer();
BoardPins pins = BoardPins(&mcp);
const int NUM_SWITCHES = 7;

// SwitchTwoPin(*pins, switchpin [INPUT_PULLUP], enable pin [OUTPUT], friendly name)
// input pin can be soemthing laggy because it's a slow switch doing the trigger. The enable pin should be as fast as possible
SwitchTwoPin switches[NUM_SWITCHES] = {SwitchTwoPin(&pins, Pin::MCP23017_0, Pin::ARDUINO_3, "Switch 0"),
                            SwitchTwoPin(&pins, Pin::MCP23017_1, Pin::ARDUINO_4, "Switch 1"),
                            SwitchTwoPin(&pins, Pin::MCP23017_2, Pin::ARDUINO_5, "Switch 2"),
                            SwitchTwoPin(&pins, Pin::MCP23017_3, Pin::ARDUINO_6, "Switch 3"),
                            SwitchTwoPin(&pins, Pin::MCP23017_4, Pin::ARDUINO_7, "Switch 4"),
                            SwitchTwoPin(&pins, Pin::MCP23017_5, Pin::ARDUINO_8, "Switch 5"),
                            SwitchTwoPin(&pins, Pin::MCP23017_6, Pin::ARDUINO_9, "Switch 6"),
                            };

void setup() {
  // Setup each switch
  Serial.begin(9600);
  if (!mcp.begin_I2C()) 
  {
    Serial.println("Error.");
    while(1);
  } 

#ifndef DEBUG
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    switches[i].Initialize(); // Input pins will be pullup enabled, so they will read high by default. 
  }
#else
  pins.SetPinDirection(Pin::MCP23017_13, INPUT_PULLUP);
  pins.SetPinDirection(Pin::MCP23017_4, OUTPUT);
  pins.PinWrite(Pin::MCP23017_4, HIGH);
#endif
}

void loop() 
{
#ifdef DEBUG
  static int readVal = 0;
  readVal = pins.PinRead(Pin::MCP23017_13);
  pins.PinWrite(Pin::MCP23017_4, readVal);
#else
  // Check to see if a button is currently depressed on any of the switches/buttons - if so, change the switch
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    switches[i].Monitor();
  }
#endif
  delay(DELAY_MS);
}