#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Variables for logging:
const int logSize = 100;
unsigned long thisZeroCross = 0;
unsigned long lastZeroCross = 0;
unsigned long zeroCrossLogCount = 0;
unsigned long zeroCrossLog[logSize];
unsigned long totalZeroCrossCycle = 0;
unsigned int minZeroCrossCycle = 0;
unsigned int maxZeroCrossCycle = 0;

// Variables for analysing:
byte fullSet;
unsigned int avgZeroCrossCycle = 0;
unsigned int Hz = 0;

// Variables for reporting:
const int refreshWaitTime = 2000;
unsigned long lastDisplayRefresh = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Detecting ZC...");

  attachInterrupt(0, zeroCross, FALLING);
}

void loop()
{
  if (millis() - lastDisplayRefresh > refreshWaitTime && fullSet)
  {
    lastDisplayRefresh = millis();
    avgZeroCrossCycle = totalZeroCrossCycle / logSize;
    Hz = 500000 / avgZeroCrossCycle;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Z:");
    lcd.print(avgZeroCrossCycle);
    lcd.print((char)228);
    lcd.print("s, F:");
    lcd.print(Hz);
    lcd.print("Hz");

    lcd.setCursor(0, 1);
    lcd.print("R:");
    lcd.print(minZeroCrossCycle);
    lcd.print("-");
    lcd.print(maxZeroCrossCycle);
    lcd.print((char)228);
    lcd.print("s");
  }
}

void zeroCross()
{
  thisZeroCross = micros();
  if (lastZeroCross != 0)
  {
    if (fullSet)
    {
      if (minZeroCrossCycle == 0 || minZeroCrossCycle > thisZeroCross - lastZeroCross)
      {
        minZeroCrossCycle = thisZeroCross - lastZeroCross;
      }
      if (maxZeroCrossCycle < thisZeroCross - lastZeroCross)
      {
        maxZeroCrossCycle = thisZeroCross - lastZeroCross;
      }
    }
    totalZeroCrossCycle -= zeroCrossLog[zeroCrossLogCount];
    zeroCrossLog[zeroCrossLogCount] = thisZeroCross - lastZeroCross;
    totalZeroCrossCycle += zeroCrossLog[zeroCrossLogCount];
    zeroCrossLogCount++;
    if (zeroCrossLogCount == logSize)
    {
      fullSet = 1;
      zeroCrossLogCount = 0;
    }
  }
  lastZeroCross = thisZeroCross;
}
