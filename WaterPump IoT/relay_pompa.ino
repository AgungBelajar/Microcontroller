//#include <SoftwareSerial.h>
//SoftwareSerial Serial3(10,11);

const int relay[10] = { 52,53, 2,3,4,5, 6,7,8,9,};
bool testRelay =  false;
bool debugMode = true;


void setup() {
  Serial.begin(115200);
 Serial3.begin(115200);
  pinMode(relay[0], OUTPUT);
  pinMode(relay[1], OUTPUT);
  pinMode(relay[2], OUTPUT);
  pinMode(relay[3], OUTPUT);
  pinMode(relay[4], OUTPUT);
  pinMode(relay[5], OUTPUT);
  pinMode(relay[6], OUTPUT);
  pinMode(relay[7], OUTPUT);
  pinMode(relay[8], OUTPUT);
  pinMode(relay[9], OUTPUT);
  pinMode(relay[10], OUTPUT);
  
  for (int i = 0; i < sizeof(relay); i++) {
    digitalWrite(relay[i], HIGH);    // turn the LED off by making the voltage LOW
  }

}

void loop() {
  //test relay
  int i = 1;

  //fixed
  if (testRelay) {
    for (int i = 0; i < sizeof(relay); i++) {
      digitalWrite(relay[i], LOW);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                    // wait for a second
      digitalWrite(relay[i], HIGH);    // turn the LED off by making the voltage LOW
    }
  }
  String content = "";
  char character;

  while (Serial3.available()) {
    delay(3);
    character = Serial3.read();
    content.concat(character);
  }

  if (content != "") {
    int pin = content.toInt();
    Serial.print("Pin " + String(pin));
    if (pin > 0) {
      Serial.println(" nyala");
      digitalWrite(pin, LOW);
    } else if (pin < 0) {
      Serial.println(" mati");
      digitalWrite(pin * -1, HIGH);
    } else {
      Serial.println(" BUILT IN");
    }
  }
}

void debug(String noDebug) {
  if (debugMode) {
    Serial.println("debug " + noDebug);
  }
}
