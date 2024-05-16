#include <Wire.h>
const int I2C_ADDRESS = 0x68;

const char* days[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
const char* months[] = {"Januari", "Februari", "Maret", "Maret", "April", "Mei", "Juni", "Juli", "Agustus","September", "Oktober", "November", "Desember"};

byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 0;
byte monthday = 0;
byte month = 0;
byte year = 0;

void setup(){
  Wire.begin();
  Serial.begin(115200);
  mengaturWaktu();
 }
void loop(){ 
  printTime();
  delay(1000);
}
void printTime() {
  char buffer[3];
  const char* AMPM = 0;
  membacaWaktu();
    Serial.print(days[weekday-1]);
  Serial.print(" ");
  Serial.print(monthday);
  Serial.print(" ");
  Serial.print(months[month-1]);
  Serial.print(" 20");
  Serial.print(year);
  Serial.print(" ");
  if (hour > 12) {
    hour -= 12;
    AMPM = " PM";}
  else AMPM = " AM";
  Serial.print(hour);
  Serial.print(":");
//  Serial.print("%02d");
//  Serial.print(minute);
  sprintf(buffer, "%02d", minute);
  Serial.print(buffer);
  Serial.print(":");
//  Serial.print("%02d");
//  Serial.print(second);
  sprintf(buffer, "%02d", second);
  Serial.print(buffer);
  Serial.println(AMPM);  
}
void membacaWaktu() {
  
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDRESS, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
  
}
void mengaturWaktu() {

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(byte(0));
  Wire.write(decToBcd(30)); // second
  Wire.write(decToBcd(51)); // minute
  Wire.write(decToBcd(8)); // hour
  Wire.write(decToBcd(5));  // weekday
  Wire.write(decToBcd(5)); // date
  Wire.write(decToBcd(1));  // month
  Wire.write(decToBcd(23)); // year
  Wire.write(byte(0));
  Wire.endTransmission();
  
}
byte decToBcd(byte val) {
  return ((val/10*16) + (val%10));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val%16));
}
