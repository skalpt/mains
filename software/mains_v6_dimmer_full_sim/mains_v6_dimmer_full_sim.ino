#include <TimerOne.h> // Software timer with interrupt for TRIAC firing
#include <LiquidCrystal.h> // LCD library

LiquidCrystal lcd(8, 7, 6, 5, 4, 3); // LCD pin setup

// Variables for zero-cross detection:
byte isZeroCross = 0;
byte realZeroCross = 1;
unsigned int simulateZeroCross = 0;

// Variables for dimming:
const unsigned int dimSteps = 99; // The number of dim levels between (but exclusive of) "On" and "Off"
unsigned int Hz = 50;
const unsigned int stepTime = 500000 / Hz / (dimSteps + 1);
int dimValue = 0; // 0 = always on, 1 to [dimSteps] = dimmed
unsigned int timerCount = 0;
unsigned int ACPin = 10;
unsigned int LEDPin = 9;
unsigned long lastTriacFire = 0; // Timestamp (in microseconds) of last TRIAC fire, used to prevent false zero-cross detections when firing the TRIAC at low voltages
const unsigned int stepWait = 50; // Number of milliseconds the program will wait betweeen incrementing/decrementing dimValue
unsigned long lastStepTime = 0; // Timestamp (in milliseconds) of last increment/decrement of dimValue
int dimDirection = 1; // -1 = decrementing (dimming), 1 = incrementing (brightening)

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

// Variables for reporting:
const int refreshWaitTime = 1000;
unsigned long lastDisplayRefresh = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Waiting for ZC. ");

  pinMode(ACPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  
  attachInterrupt(0, zeroCross, FALLING);
  Timer1.initialize();
  Timer1.attachInterrupt(dimCheck, stepTime);
}

void loop()
{
  if (millis() - lastStepTime >= stepWait)
  {
    lastStepTime = millis();
    if (dimValue + dimDirection < 0 || dimValue + dimDirection > (dimSteps + 1) * 0.9) // Never go above 90% dimmed or you will risk the TRIAC staying on through the next half-cycle!
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

    lcd.setCursor(0, 0);
    if (fullSet)
    {
      lcd.print("                ");
      lcd.setCursor(0, 0);
      if (realZeroCross)
      {
        lcd.print("T:");
      } else
      {
        lcd.print("S:");
      }
      lcd.print(avgZeroCrossCycle);
      lcd.print((char)228);
      lcd.print("s, F:");
      lcd.print(Hz);
      lcd.print("Hz");
    } else
    {
      if (micros() - thisZeroCross >= 4000000)
      {
        lcd.print("Simulating...   ");
      } else if (realZeroCross)
      {
        lcd.print("Waiting for ZC. ");
      }
    }
    
    if (micros() - thisZeroCross >= 1000000)
    {
      zeroCrossLogCount = 0;
      fullSet = 0;
      if (micros() - thisZeroCross >= 5000000)
      {
        realZeroCross = 0;
      }
    }
  }
}

void zeroCross()
{
  realZeroCross = 1;
  processZeroCross();
}

void processZeroCross()
{
  thisZeroCross = micros();

//  if (thisZeroCross - lastTriacFire >= stepTime && thisZeroCross - lastZeroCross > 8000) // When firing close to a ZC, it seems that a resistive load can pull the voltage low enough to trigger an erroneous ZC!!
//  {
    isZeroCross = 1;
    timerCount = 0;
  
    digitalWrite(ACPin, LOW);
    digitalWrite(LEDPin, LOW);
  
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
//        stepTime = 500000 / avgZeroCrossCycle / (dimSteps + 1);
      }
    }
    lastZeroCross = thisZeroCross;
//  }
}

void dimCheck()
{
  if(isZeroCross == 1)
  {
    if(timerCount >= dimValue)
    {
      if (realZeroCross)
      {
        lastTriacFire = micros();
        digitalWrite(ACPin, HIGH);
        delayMicroseconds(5);
        digitalWrite(ACPin, LOW);
      }
      digitalWrite(LEDPin, HIGH);
      timerCount = 0;
      isZeroCross = 0;
    } else
    {
      timerCount++;
    }
  }

  if (!realZeroCross)
  {
    simulateZeroCross++;
    if (simulateZeroCross > dimSteps)
    {
      simulateZeroCross = 0;
      processZeroCross();
    }
  }
}
