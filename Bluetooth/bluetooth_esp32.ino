#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
char incomingChar;
int calIn=0;
String message = "";
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
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
  if (message =="add_1"){
    calIn=calIn+1;
    String printdata = "Ditambah 1 jadi : " +String (calIn);
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
  else if (message =="add_5"){
    calIn=calIn+5;
    String printdata = "Ditambah 5 jadi : " +String (calIn);
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
    else if (message =="min_1"){
    calIn=calIn-1;
    String printdata = "Dikurang 1 jadi : " +String (calIn);
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
    else if (message =="min_5"){
    calIn=calIn-5;
    String printdata = "Dikurang 5 jadi : " +String (calIn);
    SerialBT.println(printdata);
    Serial.println(printdata);
  }
 delay(20);
}
