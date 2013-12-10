#include <TimerOne.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Variables for dimming:
const int dimSteps = 99;
const int stepWait = 100;
unsigned int stepTime;
byte isZeroCross = 0;
unsigned int timerCount = 0;
unsigned int ACPin = 10;
unsigned int LEDPin = 9;
unsigned int dimValue = 0;
unsigned long lastStepTime = 0;
int dimDirection = 1;

// Variables for logging:
const int logSize = 100;
unsigned long thisZeroCross = 0;
unsigned long lastZeroCross = 0;
unsigned long zeroCrossLogCount = 0;
unsigned long zeroCrossLog[logSize];
unsigned long totalZeroCrossCycle = 0;

// Variables for analysing:
byte fullSet;
unsigned int avgZeroCrossCycle = 0;
unsigned int Hz = 50;

// Variables for reporting:
const int refreshWaitTime = 1000;
unsigned long lastDisplayRefresh = 0;

// Variables for simulating a zero-cross:
unsigned int simulateZeroCross = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Detecting ZC...");

  pinMode(ACPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  
  stepTime = 500000 / Hz / (dimSteps + 1);
  
  attachInterrupt(0, zeroCross, FALLING);
  Timer1.initialize();
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
    dimValue += dimDirection;
  }

  if (millis() - lastDisplayRefresh > refreshWaitTime)
  {
    lastDisplayRefresh = millis();

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("D:");
    lcd.print(dimValue);
    lcd.print(", W:");
    lcd.print(dimValue * stepTime);
    lcd.print((char)228);
    lcd.print("s");

    if (fullSet)
    {
      lcd.setCursor(0, 0);
      lcd.print("                ");
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(avgZeroCrossCycle);
      lcd.print((char)228);
      lcd.print("s, F:");
      lcd.print(Hz);
      lcd.print("Hz");
    }
  }
}

void zeroCross()
{
  isZeroCross = 1;

  timerCount = 0;
  digitalWrite(ACPin, LOW);
  digitalWrite(LEDPin, LOW);

  thisZeroCross = micros();
  if (lastZeroCross != 0)
  {
    totalZeroCrossCycle -= zeroCrossLog[zeroCrossLogCount];
    zeroCrossLog[zeroCrossLogCount] = thisZeroCross - lastZeroCross;
    totalZeroCrossCycle += zeroCrossLog[zeroCrossLogCount];
    zeroCrossLogCount++;
    if (zeroCrossLogCount == logSize)
    {
      zeroCrossLogCount = 0;
      fullSet = 1;

      avgZeroCrossCycle = totalZeroCrossCycle / logSize;
      Hz = 500000 / avgZeroCrossCycle;
      stepTime = 500000 / avgZeroCrossCycle / (dimSteps + 1);
    }
  }
  lastZeroCross = thisZeroCross;
}

void dimCheck()
{
  if(isZeroCross == 1)
  {
    if(timerCount >= dimValue)
    {
      digitalWrite(ACPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(ACPin, LOW);
      digitalWrite(LEDPin, HIGH);
      timerCount = 0;
      isZeroCross = 0;
    } else
    {
      timerCount++;
    }
  }

  if (thisZeroCross == 0)
  {
    simulateZeroCross++;
    if (simulateZeroCross > dimSteps)
    {
      simulateZeroCross = 0;
      isZeroCross = 1;
      digitalWrite(LEDPin, LOW);
    }
  }
}
