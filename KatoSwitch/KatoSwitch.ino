enum SwitchState
{
  SW_STRAIGHT = 0,
  SW_TURN = 1,
};

enum ButtonState 
{
  BUTTON_OFF = 0,
  BUTTON_ON = 1,
};

class Switch
{
  /*
  This class is meant to control the logic and pinout voltage of Kato Switches using an external L298N driver board. It recognizes inputs from an external button, and
  displays the if the switch is driven straight or turned via separate LEDs. 
  Inputs: Button Digital Reading -- measured with pull down resistor (1k-10k)
  Outputs: Turn LED -- is the switch driven for a turn, ON if so, else off -- The LEDs could be replaced if the pins are set high / low and the ENABLE pin is toggled off. This 
                       would allow each switch to require 3 pins only. Should probably do this once we validate it works
           Straight LED -- is the switch driven for a straight line, ON if so, else off
           Switch Straight Pin -- L298 input A to drive switch straight
           Switch Turn Pin -- L298 input B to drive switch to a turn
  */
  private:
    int switchStraightPin;
    int switchTurnPin;
    int buttonpin;
    int ledTurnPin;
    int ledStraightPin;
    SwitchState switchstate;
    ButtonState buttonstate; 
    char debugstr[10];
    //const int drivetime = 100; // I don't know if I need this yet. Can we just drive it quickly and then bring it low? People saying it needs to be thrown for 10ms minimum. will see

  public:
    Switch(int straightswitchpin, int switchturnpin, int buttonpin, int turnledpin, int straightledpin) 
    {
      this->switchStraightPin = straightswitchpin; 
      this->switchTurnPin = switchturnpin;
      this->buttonpin = buttonpin;
      this->ledTurnPin = turnledpin;
      this->ledStraightPin = straightledpin;  
      this->switchstate = SwitchState::SW_STRAIGHT;
      this->buttonstate = ButtonState::BUTTON_OFF;
      pinMode(straightswitchpin, OUTPUT);
      pinMode(switchturnpin, OUTPUT);
      pinMode(buttonpin, INPUT);
      pinMode(turnledpin, OUTPUT);
      pinMode(straightledpin, OUTPUT);
      
      // Initial everyting based off of initial state
      Monitor();
    }

    Monitor()
    {
      // First, read in the current button value
      int buttonVal = digitalRead(buttonpin);
      switch (this->buttonstate)
      {
        case ButtonState::BUTTON_OFF:
        {
          if (buttonVal == 1)
          {
            // Rising edge
            this->UpdateSwitchState();
          }
          break;
        }
        default:
          break;
      }

      // Update
      this->buttonstate = buttonVal;

      // At the end of this state, what needed to be driven was driven, so now drive the pins low
      digitalWrite(this->switchStraightPin, LOW);
      digitalWrite(this->switchTurnPin, LOW);
      SetLED();
    }

    SetLED()
    {
      // Write the current state of the LEDs absed on switch state
      int isTurn = this->switchstate == SwitchState::SW_TURN ? 1 : 0;
      digitalWrite(this->ledStraightPin, !isTurn);
      digitalWrite(this->ledTurnPin, isTurn);
    }

    UpdateSwitchState()
    {
      // Switch the LED's and drive it
      if (this->switchstate == SwitchState::SW_STRAIGHT)
      {
        // It should now be a turn
        this->switchstate = SwitchState::SW_TURN;
        digitalWrite(this->switchStraightPin, HIGH);
      }
      else
      {
        this->switchstate = SwitchState::SW_STRAIGHT;
        digitalWrite(this->switchTurnPin, HIGH);
      }
    }
};

Switch switchOne = Switch(5, 6, 3, 2, 4); // hpin, lpin, button read, turn pin, straight pin

void setup() {
  // Setup each switch
  Serial.begin(9600);
  Serial.println("Initialized");
}

void loop() 
{
  // Read all the switches
  switchOne.Monitor();
  delay(10);
}
