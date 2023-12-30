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
      this->_currentPwmPower = 0;

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
        if (power < 0 && _direction == Direction::FORWARD)
        {
          // Flip the direction
          SetDirection();
        }
        else if (power >=0 and _direction == Direction::REVERSE)
        {
          // Flip the direction
          SetDirection();
        }

        // WRite the analog power
        analogWrite(_pwmPin, power);
        _currentPwmPower = power;
      }
    }

  private:
    int _pwmPin;
    int _forwardPin;
    int _reversePin;
    Direction _direction;
    int _currentPwmPower;
    SetDirection()
    {
      digitalWrite(this->_forwardPin, _direction == Direction::FORWARD ? 1 : 0);
      digitalWrite(this->_reversePin, _direction == Direction::REVERSE ? 1 : 0);
    }
};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
