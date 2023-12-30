int PIN_ORANGE_LED = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_ORANGE_LED, OUTPUT);
  digitalWrite(2, HIGH);
  Serial.begin(9600);
  Serial.print("initialization complete\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  // this does while (true) {}
  Serial.print("Start of main loop\n");
  uint16_t timedelay = 0;
  for (int i = 0; i < 5; i++)
  {
    timedelay = i*200;
    Serial.println("Pausing for " + String(timedelay) + " ms");
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(PIN_ORANGE_LED, HIGH);
    delay(timedelay);
    digitalWrite(PIN_ORANGE_LED, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    delay(timedelay);
  }
}
