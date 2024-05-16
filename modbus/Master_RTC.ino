#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include <Wire.h>
#define SLAVE_ADDR 9
#define ANSWERSIZE 5

void Rtc();
void master();
 
void setup () {
  Wire.begin();
  
  Serial.begin(115200);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
}


void loop() {
  // put your main code here, to run repeatedly:
Rtc();
master();
delay(3000);
}

void Rtc()
{
  DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

void master()
{
  Serial.println("Write data to slave");
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(0);
  Wire.endTransmission(); 
  Serial.println("Receive data");
  
  Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  
  String response = "";
  while (Wire.available()) {
      char b = Wire.read();
      response += b;
  } 
  Serial.println(response);
}
