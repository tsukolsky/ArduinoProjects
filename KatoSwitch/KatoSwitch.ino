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
    SwitchState switchstate;
    ButtonState buttonstate; 
    char debugstr[10];
    const int drivetime = 25; // We have to drive the pin for more than this time to get it to actually switch
    unsigned long _driveUntilTime;  // This will roll over when the millis() call also rolls over
    
  public:
    SwitchFourPin(int straightswitchpin, int switchturnpin, int buttonpin, int enablepin) 
    {
      this->switchStraightPin = straightswitchpin; 
      this->switchTurnPin = switchturnpin;
      this->buttonpin = buttonpin;
      this->enablePin = enablepin; 
      this->switchstate = SwitchState::SW_TURN;
      this->buttonstate = ButtonState::BUTTON_OFF;
      this->_driveUntilTime = 0;
      pinMode(straightswitchpin, OUTPUT);
      pinMode(switchturnpin, OUTPUT);
      pinMode(buttonpin, INPUT);
      pinMode(enablepin, OUTPUT);
      
      // Initial everyting based off of initial state
      ChangeSwitchState();
    }

    Monitor()
    {
      if (millis() > this->_driveUntilTime)
      {
        // Not in a drive situation - make sure the enable pin is LOW, the input pins should be LEFT AS THEY ARE
        digitalWrite(this->enablePin, LOW);
        
        // First, read in the current button value
        int buttonVal = digitalRead(buttonpin);
        switch (this->buttonstate)
        {
          case ButtonState::BUTTON_OFF:
          {
            if (buttonVal == 1)
            {
              // Rising edge
              this->ChangeSwitchState();
            }
            break;
          }
          default:
            break;
        }

        // Update
        this->buttonstate = buttonVal;
      }
      else
      {
        // In a drive situation - do nothing. there is a race condition if two button presses are detected before this happens.
        // In this situation, the pins would be updated and the drive time would be increased, so maybe it wouldn't happen. Regardless, 
        // don't do anything in this situation
        // Ignore button states as well - this is a lockout situation
        Serial.print("LOCKOUT\n");
      }
    }

    ChangeSwitchState()
    {
      // Switch the digital pins and then drive it
      digitalWrite(this->enablePin, LOW); // just to make sure
      if (this->switchstate == SwitchState::SW_STRAIGHT)
      {
        // It should now be a turn
        this->switchstate = SwitchState::SW_TURN;
        digitalWrite(this->switchStraightPin, LOW);
        digitalWrite(this->switchTurnPin, HIGH);
      }
      else
      {
        this->switchstate = SwitchState::SW_STRAIGHT;
        digitalWrite(this->switchTurnPin, LOW);
        digitalWrite(this->switchStraightPin, HIGH);
      }

      // Now drive it and set the next millis time that this drive action can be stopped
      digitalWrite(this->enablePin, HIGH);
      this->_driveUntilTime = millis() + this->drivetime;
      Serial.println("Current time " + String(millis()));
      Serial.println("Drive Until Time " + String(this->_driveUntilTime));
    }
};

class SwitchFivePin
{
  /*
  This class is meant to control the logic and pinout voltage of Kato Switches using an external L298N driver board. It recognizes inputs from an external button, and
  displays the if the switch is driven straight or turned via separate LEDs. It assumees the L298N output ENABLE is fused HIGH, so it is always enabled.
  Inputs: Button Digital Reading -- measured with pull down resistor (1k-10k)
  Outputs: Turn LED -- is the switch driven for a turn, ON if so, else off -- THis is the difference between the 4 and 5 pin implementations. The 4 pin onl
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
    const int drivetime = 25; // We have to drive the pin for more than this time to get it to actually switch
    unsigned long _driveUntilTime;  // This will roll over when the millis() call also rolls over
    
  public:
    SwitchFivePin(int straightswitchpin, int switchturnpin, int buttonpin, int turnledpin, int straightledpin) 
    {
      this->switchStraightPin = straightswitchpin; 
      this->switchTurnPin = switchturnpin;
      this->buttonpin = buttonpin;
      this->ledTurnPin = turnledpin;
      this->ledStraightPin = straightledpin;  
      this->switchstate = SwitchState::SW_TURN;
      this->buttonstate = ButtonState::BUTTON_OFF;
      this->_driveUntilTime = 0;
      pinMode(straightswitchpin, OUTPUT);
      pinMode(switchturnpin, OUTPUT);
      pinMode(buttonpin, INPUT);
      pinMode(turnledpin, OUTPUT);
      pinMode(straightledpin, OUTPUT);
      
      // Initial everyting based off of initial state
      ChangeSwitchState();
      SetLED();
    }

    Monitor()
    {
      if (millis() > this->_driveUntilTime)
      {
        // Not in a drive situation - make sure eveything is low
        digitalWrite(this->switchStraightPin, LOW);
        digitalWrite(this->switchTurnPin, LOW);
        
        // First, read in the current button value
        int buttonVal = digitalRead(buttonpin);
        switch (this->buttonstate)
        {
          case ButtonState::BUTTON_OFF:
          {
            if (buttonVal == 1)
            {
              // Rising edge
              this->ChangeSwitchState();
            }
            break;
          }
          default:
            break;
        }

        // Update
        this->buttonstate = buttonVal;

        // At the end of this state, what needed to be driven was driven, so now drive the pins low
        SetLED();
      }
      else
      {
        // In a drive situation - do nothing. there is a race condition if two button presses are detected before this happens.
        // In this situation, the pins would be updated and the drive time would be increased, so maybe it wouldn't happen. Regardless, 
        // don't do anything in this situation
        // Ignore button states as well - this is a lockout situation
        Serial.print("LOCKOUT\n");
      }
    }

    SetLED()
    {
      // Write the current state of the LEDs absed on switch state
      int isTurn = this->switchstate == SwitchState::SW_TURN ? 1 : 0;
      digitalWrite(this->ledStraightPin, !isTurn);
      digitalWrite(this->ledTurnPin, isTurn);
    }

    ChangeSwitchState()
    {
      // Switch the LED's and drive it
      if (this->switchstate == SwitchState::SW_STRAIGHT)
      {
        // It should now be a turn
        this->switchstate = SwitchState::SW_TURN;
        digitalWrite(this->switchStraightPin, LOW);
        digitalWrite(this->switchTurnPin, HIGH);

      }
      else
      {
        this->switchstate = SwitchState::SW_STRAIGHT;
        digitalWrite(this->switchTurnPin, LOW);
        digitalWrite(this->switchStraightPin, HIGH);
      }

      // Update when these pins can be driven low
      this->_driveUntilTime = millis() + this->drivetime;
      Serial.println("Current time " + String(millis()));
      Serial.println("Drive Until Time " + String(this->_driveUntilTime));
    }
};
enum Direction
{
  FORWARD = 0,
  REVERSE = 1,
};

class KatoPowerController
{
  public:
    KatoPowerController(int pwmPin, int forwardPin, int reversePin)
    {
      this->_pwmPin = pwmPin;
      this->_forwardPin = forwardPin;
      this->_reversePin = reversePin;
      this->_direction = Direction::FORWARD;

      pinMode(forwardPin, OUTPUT);
      pinMode(reversePin, OUTPUT);
      
      SetPower(0);
      SetDirection();
    }

    SetPower(int power)
    {
      if (-255 <= power <= 255)
      {
        // valid power
        if (power < 0 && this->_direction == Direction::FORWARD)
        {
          // Flip the direction
          this->_direction = Direction::REVERSE;
          SetDirection();
        }
        else if (power >= 0 && _direction == Direction::REVERSE)
        {
          // Flip the direction
          this->_direction = Direction::FORWARD;
          SetDirection();
        }

        // WRite the analog power
        Serial.println("Writing analog power to " + String(abs(power)) + " and direction is " + String(this->_direction));
        analogWrite(this->_pwmPin, abs(power));
      
      }
    }

  private:
    int _pwmPin;
    int _forwardPin;
    int _reversePin;
    Direction _direction;
    SetDirection()
    {
      Serial.println("Driving direction pins");
      digitalWrite(this->_forwardPin, _direction == Direction::FORWARD ? 1 : 0);
      digitalWrite(this->_reversePin, _direction == Direction::REVERSE ? 1 : 0);
    }
};

enum SpeedTestState
{
  STOP = 0,
  FORWARD_SLOW = 160,
  FORWARD_FAST = 230,
  REVERSE_SLOW = -120,
  REVERSE_FAST = -175,
};

class SpeedControlButton
{
  public:
    SpeedControlButton(int buttonpin)
    {
      this->buttonPin = buttonpin;
      this->buttonstate = ButtonState::BUTTON_OFF;
      this->debounceTimeout = millis();
      this->speedState = SpeedTestState::STOP;
      this->lastMovingState = SpeedTestState::REVERSE_FAST; // THis is the last one, goes forward slow, revers slow, forward fast, reverse fast
      pinMode(buttonpin, INPUT);
    }

    Monitor()
    {
      if (this->debounceTimeout < millis())
      {
        int buttonVal = digitalRead(this->buttonPin);
        switch (this->buttonstate)
        {
          case ButtonState::BUTTON_OFF:
          {
            if (buttonVal == 1)
            {
              // Rising edge of a button press, go faster
              this->UpdateSpeed();
              this->debounceTimeout = millis() + 100;
            }
            break;
          }
          default:
            break;
        }

        // Update to debounce
        this->buttonstate = buttonVal;
      }
      else
      {
        // Ignore - we are debouncing
        Serial.println("Debouncing");
      }
    }

    UpdateSpeed()
    {
      /* The train will NOT move by default. When a button is pressed, it will go forward slow. Then when a button is pressed, it will stop before doign anything else.
        Between each state, the train will be stopped. The speed is controlled by the values in SpeedTestState. The controls work as expected with a switch.
      */
      switch (speedState)
      {
        case SpeedTestState::STOP:
        {
          // Go forward
          switch(this->lastMovingState)
          {
            case SpeedTestState::FORWARD_FAST:
            {
              speedState = SpeedTestState::REVERSE_FAST;
              break;
            }
            case SpeedTestState::FORWARD_SLOW:
            {
              speedState = SpeedTestState::REVERSE_SLOW;
              break;
            }
            case SpeedTestState::REVERSE_FAST:
            {
              speedState = SpeedTestState::FORWARD_SLOW;
              break;
            }
            case SpeedTestState::REVERSE_SLOW:
            {
              speedState = SpeedTestState::FORWARD_FAST;
              break;
            }
            default:
            {
              speedState = SpeedTestState::FORWARD_SLOW;
              break;
            }
          }
          // Update last moving state so when we stop (next button press), we know what to do next.
          this->lastMovingState = speedState;
          break;
        }
        case SpeedTestState::FORWARD_SLOW:
        case SpeedTestState::REVERSE_SLOW:
        case SpeedTestState::FORWARD_FAST:
        case SpeedTestState::REVERSE_FAST:
        default:
        {
          // If we are doing anything but stopped, go to a stopped positionxs
          speedState = SpeedTestState::STOP;
          break;
        }
      }
      pwController.SetPower((int) speedState);
    }
  private:
    int buttonPin;
    int pwmPin;
    ButtonState buttonstate;
    SpeedTestState speedState;
    SpeedTestState lastMovingState;
    unsigned long debounceTimeout;
};


/******* GLOBAL VARIABLE *********/

//SwitchFivePin switchOne = SwitchFivePin(7, 8, 3, 2, 4); // L298 IN1 straight pin,  L298 IN2 turn pin, button read, turn led pin, straight led pin
SwitchFourPin switchOne = SwitchFourPin(7, 8, 3, 2);
KatoPowerController pwController = KatoPowerController(6, 10,11); // PWM pin (L298 EN), Forward (L298 IN1), Reverse (L298 IN2) (Valid PWM pins are 3, 5, 6, 9, 10, 11 @490Hz, 5 and 6 can go to 980Hz)
SpeedControlButton btnController = SpeedControlButton(5);

void setup() {
  // Setup each switch
  Serial.begin(9600);
  Serial.println("Initialized");
  // Initialize the Power Controller - set power to 0. This variable is global and is used by the Button Controller class
  pwController.SetPower((int) SpeedTestState::STOP);
}

void loop() 
{
  // Read all the switches
  switchOne.Monitor();
  btnController.Monitor();
  delay(1);
}
