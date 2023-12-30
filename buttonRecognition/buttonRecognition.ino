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

struct Switch
{
  private:
    int switchStraightPin;
    int switchTurnPin;
    int buttonpin;
    int ledTurnPin;
    int ledStraightPin;
    SwitchState switchstate;
    ButtonState buttonstate; 
    char debugstr[10];
    //const int drivetime = 100; // I don't know if I need this yet. Can we just drive it quickly and then bring it low?

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
