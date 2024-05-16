/*
  I2C Slave Demo
  i2c-slave-demo.ino
  Demonstrate use of I2C bus
  Slave receives character from Master and responds
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/
 
// Include Arduino Wire library for I2C

#define CONTROL_PIN 5   // This is the YELLOW wire, can be any data line
int16_t distance;  // The last measured distance
bool newData = false; // Whether new data is available from the sensor
uint8_t buffer[4];  // our buffer for storing data
uint8_t idx = 0;  // our idx into the storage buffer

#include <Wire.h>
// Define Slave I2C Address
#define SLAVE_ADDR 9
// Define Slave answer size
#define ANSWERSIZE 5
// Define string with response to Master
String answer = "Hello";
byte i = 0;
void setup() {
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Adafruit DYP-ME007YS Test");

  // set the data rate for the Serial port, 9600 for the sensor
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

   
  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);
  
  // Function to run when data requested from master
  Wire.onRequest(requestEvent); 
  
  // Function to run when data received from master
  Wire.onReceive(receiveEvent);
  
  // Setup Serial Monitor 
  Serial.begin(115200);
  Serial.println("I2C Slave Demonstration");
  Serial3.begin(9600);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);
}
 
void receiveEvent() {
 
  // Read while data received
  while (0 < Wire.available()) {
    byte x = Wire.read();
  }
  
  // Print to Serial Monitor
  Serial.println("Receive event");
}
 
void requestEvent() {
 
  // Setup byte variable in the correct size
  byte response[ANSWERSIZE];
  
  // Format answer as array
  for (byte i=0;i<ANSWERSIZE;i++) {
    String strDistance = String(distance);
    response[i] = (byte)strDistance.charAt(i);
  }
  
  // Send response back to Master
  Wire.write(response,sizeof(response));
  
  // Print to Serial Monitor
  Serial.print("Request event :   ");
  Serial.write(response,sizeof(response));
  Serial.println("");
}

void loop() { // run over and over
  if (Serial3.available()) {
    uint8_t c = Serial3.read();
//    Serial.println(c, HEX);

    // See if this is a header byte
    if (idx == 0 && c == 0xFF) {
      buffer[idx++] = c;
    }
    // Two middle bytes can be anything
    else if ((idx == 1) || (idx == 2)) {
      buffer[idx++] = c;
    }
    else if (idx == 3) {
      uint8_t sum = 0;
      sum = buffer[0] + buffer[1] + buffer[2];
      if (sum == c) {
        distance = ((uint16_t)buffer[1] << 8) | buffer[2];
        newData = true;
      }
      idx = 0;
    }
  }
  
  if (newData) {
    int jarak = (distance/10);
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" mm");
    Serial.print(jarak);
    Serial.println(" cm");
    newData = false;
  }
}
