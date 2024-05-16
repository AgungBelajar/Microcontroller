#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
char incomingChar;
int calIn=3800;
String message = "";
BluetoothSerial SerialBT;
#define relay 25

void setup() {
  pinMode(relay,OUTPUT);
  Serial.begin(115200);
  SerialBT.begin("Cam Trig Agrotech"); //Bluetooth device name
  Serial.println("\The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) 
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{
      message = "";
    }
    Serial.write(incomingChar);  
  }
  if (message =="4800"){
    calIn=3800;
    String printdata = "Waktu adalah = 4,8 s";
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
  else if (message =="5800"){
    calIn=4800;
    String printdata = "Waktu adalah = 5,8 s";
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
    else if (message =="3800"){
    calIn=2800;
    String printdata = "Waktu adalah = 3,8 s";
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
digitalWrite(relay,HIGH);
Serial.println("Nyala");
SerialBT.println("Nyala");
delay(1000);
digitalWrite(relay,LOW);
Serial.println("Mati");
SerialBT.println("Mati");
delay(calIn);  
}
