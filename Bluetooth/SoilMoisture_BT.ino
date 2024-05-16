#include <OneWire.h>
#include <DallasTemperature.h>
#include "BluetoothSerial.h"

#define AOUT_PIN 35 // Tiang 2
#define BOUT_PIN 34 // Tiang 1
float kelembabanTanahT2; //Tiang 2
float kelembabanTanahT1; // Tiang 1
const int oneWireBus = 4;  
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
char incomingChar;
int calIn=0;
String message = "";
BluetoothSerial SerialBT;

void setup() {

  Serial.begin(9600);
  SerialBT.begin("Moisture Soil"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  sensors.begin();
}

void loop() {
  //2 Sensor Kelembaban Tanah
  int valueT2 = analogRead(AOUT_PIN); // read the analog value from sensor
  int valueT1 = analogRead(BOUT_PIN); // read the analog value from sensor
  kelembabanTanahT2 = (100-((valueT2/1023.00)*100));
  kelembabanTanahT1 = (100-((valueT1/1023.00)*100));
  
  // Sensor DS18B20
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  
  // Print Sensor Suhu
  Serial.print(temperatureC);
  Serial.print("ºC\t");
  SerialBT.print("Suhu : ");
  SerialBT.print(temperatureC);
  SerialBT.println("ºC  ");
  
  // Print Sensor Kelembaban Tanah
  Serial.print("ADC kelembaban 1: ");
  Serial.print(valueT2);
  Serial.print("\tkelembaban 1: ");
  Serial.print(kelembabanTanahT2);
  SerialBT.print("ADC kelembaban 1: ");
  SerialBT.print(valueT2);
  SerialBT.print("  kelembaban 1: ");
  SerialBT.println(kelembabanTanahT2);
  Serial.print("\tADC kelembaban 2: ");
  Serial.print(valueT1);
  Serial.print("\tkelembaban 2: ");
  Serial.println(kelembabanTanahT1);
  SerialBT.print("ADC kelembaban 2: ");
  SerialBT.print(valueT2);
  SerialBT.print("  kelembaban 2: ");
  SerialBT.println(kelembabanTanahT2);
  delay(1000);
}
