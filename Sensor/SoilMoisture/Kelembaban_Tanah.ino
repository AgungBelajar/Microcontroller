#define AOUT_PIN 35 // Tiang 2
#define BOUT_PIN 34 // Tiang 1
float kelembabanTanahT2; //Tiang 2
float kelembabanTanahT1; // Tiang 1

void setup() {
  Serial.begin(9600);
}

void loop() {
  int valueT2 = analogRead(AOUT_PIN); // read the analog value from sensor
  int valueT1 = analogRead(BOUT_PIN); // read the analog value from sensor
  kelembabanTanahT2 = (100-((valueT2/1023.00)*100));
  kelembabanTanahT1 = (100-((valueT1/1023.00)*100));
  
  Serial.print("ADC Tiang 2: ");
  Serial.print(valueT2);
  Serial.print("\tkelembaban Tiang 2: ");
  Serial.print(kelembabanTanahT2);
  Serial.print("ADC Tiang 1: ");
  Serial.print(valueT1);
  Serial.print("\tkelembaban Tiang 1: ");
  Serial.println(kelembabanTanahT1);
  delay(500);
}
