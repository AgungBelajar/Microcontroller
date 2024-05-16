int waktu = 4300;
#define trig 8
#define vcc 9

void setup() {
  Serial.begin(115200);
//  SerialBT.begin("ESP32"); //Bluetooth device name
//  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(trig,OUTPUT);
  pinMode(vcc,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop() {
  digitalWrite(vcc,HIGH);
  digitalWrite(trig,HIGH);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("Take");
  delay(3000);
  digitalWrite(trig,LOW);
  digitalWrite(vcc,LOW);
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("off");
  delay(1800);
}
