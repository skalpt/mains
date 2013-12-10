void setup()
{
  Serial.begin(57600);
  Serial.print("\nZero-cross detection commencing...\n");

  attachInterrupt(1, zeroCross, FALLING);
}

void loop()
{
}

void zeroCross()
{
  Serial.print("ZC@");
  Serial.print(micros());
  Serial.print("us\n");
}
