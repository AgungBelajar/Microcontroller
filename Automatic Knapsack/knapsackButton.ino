#define relay 2
const int button1 = 8;
const int button2 = 9;
const int button3 = 10;

int buttonState1, buttonState2, buttonState3 = 0;

int waktuTombol1 = 6000;
int waktuTombol2 = 9000;
int waktuTombol3 = 12000;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(relay,OUTPUT);
pinMode(button1, INPUT);
pinMode(button2, INPUT);
pinMode(button3, INPUT);

}

void loop() {
//  buttonState1 = digitalRead(button1);
//  Serial.println(buttonState1);
  buttonState2 = digitalRead(button2);
  Serial.println(buttonState2);
//  buttonState3 = digitalRead(button3);
//  Serial.println(buttonState3);

  if (buttonState2 == LOW)
  {
    digitalWrite(relay,LOW);
    Serial.println(String(waktuTombol2) + "detik nyala");
    delay(waktuTombol2);
    digitalWrite(relay,HIGH);
    delay(1000);
  }
//  else if (buttonState2 == HIGH)
//  {
//    digitalWrite(relay,LOW);
//    Serial.println(String(waktuTombol2) + "detik nyala");
//    delay(waktuTombol2);
//    digitalWrite(relay,HIGH);
//    delay(1000);
//  }
//  else if (buttonState3 == HIGH)
//  {
//    digitalWrite(relay,LOW);
//    Serial.println(String(waktuTombol3) + "detik nyala");
//    delay(waktuTombol3);
//    digitalWrite(relay,HIGH);
//    delay(1000);
//  }
  else
  {
    digitalWrite(relay,HIGH);
    Serial.println("HARUSNYA MATI");
  }
}
