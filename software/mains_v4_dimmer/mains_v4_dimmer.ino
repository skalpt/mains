#include <TimerOne.h>

const int Hz = 50;
const int dimSteps = 100;
const int stepWait = 30;
volatile int i = 0;
volatile boolean isZeroCross = 0;
int ACPin = 10;
int dimValue = 0;
unsigned long lastStepTime = 0;
int dimDirection = 1;
int stepTime = 50000 / Hz / dimSteps;

void setup()
{
  pinMode(ACPin, OUTPUT);
  
  attachInterrupt(0, zeroCross, FALLING);
  Timer1.initialize(stepTime);
  Timer1.attachInterrupt(dimCheck, stepTime);
}

void loop()
{
  if (millis() - lastStepTime >= stepWait)
  {
    lastStepTime = millis();
    if (dimValue + dimDirection < 0 || dimValue + dimDirection > dimSteps)
    {
      dimDirection *= -1;
    }
    dimValue =+ dimDirection;
  }
}

void zeroCross()
{
  isZeroCross = 1;
}

void dimCheck()
{
  if(isZeroCross == 1)
  {
    if(i >= dimValue)
    {
      digitalWrite(ACPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(ACPin, LOW);
      i = 0;
      isZeroCross = 0;
    } else
    {
      i++;
    }
  }
}
