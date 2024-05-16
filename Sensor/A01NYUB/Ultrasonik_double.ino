
#define RXD1 27
#define TXD1 14

#define mySerialIn Serial1
#define mySerialOut Serial
#define CONTROL_PIN 5

int16_t distanceIn, distanceOut;  // The last measured distance
bool newDataIn, newDataOut = false; // Whether new data is available from the sensor
uint8_t bufferIn[4], bufferOut[4];  // our buffer for storing data
uint8_t idxIn, idxOut = 0;  // our idx into the storage buffer
int CalIn, calOut;
void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  mySerialIn.begin(9600, SERIAL_8N1, RXD1, TXD1);
  mySerialOut.begin(9600);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);
}

void loop() { //Choose Serial1 or Serial2 as required
 if (mySerialIn.available()) {
    uint8_t c = mySerialIn.read();
//    Serial.println(c, HEX);

    // See if this is a header byte
    if (idxIn == 0 && c == 0xFF) {
      bufferIn[idxIn++] = c;
    }
    // Two middle bytes can be anything
    else if ((idxIn == 1) || (idxIn == 2)) {
      bufferIn[idxIn++] = c;
    }
    else if (idxIn == 3) {
      uint8_t sumIn = 0;
      sumIn = bufferIn[0] + bufferIn[1] + bufferIn[2];
      if (sumIn == c) {
        distanceIn = ((uint16_t)bufferIn[1] << 8) | bufferIn[2];
        newDataIn = true;
      }
      idxIn = 0;
    }
  }
  if (mySerialOut.available()) {
    uint8_t d = mySerialOut.read();
//    Serial.println(c, HEX);

    // See if this is a header byte
    if (idxOut == 0 && d == 0xFF) {
      bufferOut[idxOut++] = d;
    }
    // Two middle bytes can be anything
    else if ((idxOut == 1) || (idxOut == 2)) {
      bufferOut[idxOut++] = d;
    }
    else if (idxOut == 3) {
      uint8_t sumOut = 0;
      sumOut = bufferOut[0] + bufferOut[1] + bufferOut[2];
      if (sumOut == d) {
        distanceOut = ((uint16_t)bufferOut[1] << 8) | bufferOut[2];
        newDataOut = true;
      }
      idxOut = 0;
    }
  }

  int distanceNowIn = (distanceIn + calIn) * -1;
  int distanceNowOut = (distanceOut + calOut) * -1;
  if (newDataIn || newDataOut) {
    Serial.print("Distance: ");
    Serial.print(distanceIn);
    Serial.print(" mm\t");
    Serial.print(distanceNowIn);
    Serial.print(" mm\t");
    Serial.print("Distance: ");
    Serial.print(distanceOut);
    Serial.print(" mm\t");
    Serial.print(distanceNowOut);
    Serial.println(" mm");
    newDataIn = false;
    newDataOut = false;
  }
}
