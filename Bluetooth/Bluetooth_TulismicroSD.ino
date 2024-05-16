#include "BluetoothSerial.h"
#include <SPI.h>
#include <SD.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
char incomingChar;;
String message = "";
BluetoothSerial SerialBT;
float calIn;
File myFile;
char fileCIArr;
void writeFile(fs::FS &fs, const char *path, const char *message);

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP KALIBRASI"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  String filecalIn = String(calIn);
  int z = filecalIn.length();
  char fileCIArr[z + 1];
  strcpy(fileCIArr, filecalIn.c_str());
  File file = SD.open("/calIn.txt");
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return;
  }
  while (file.available())
  {
    String line = "";
    Serial.printf("Reading line: %s\r\n", "/calIn");
    Serial.println("- read from line:");
    line = file.readStringUntil('\n');
    Serial.println(line);
    calIn = line.toFloat();
    file.close();
  }

}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available())
    if (SerialBT.available()) {
      char incomingChar = SerialBT.read();
      if (incomingChar != '\n') {
        message += String(incomingChar);
      }
      else {
        message = "";
      }
      Serial.write(incomingChar);
    }
  if (message == "baca_jarak") {
    myFile = SD.open("/distanceNowIn.txt");
    if (myFile) {
      Serial.println("distanceNowIn.txt");
      while (myFile.available()) {
        String distance = myFile.readStringUntil('\n');
        Serial.println(distance);
        SerialBT.println("ketinggian = ");
        SerialBT.println(distance);
      }
    }
  }
  else if (message == "baca_calin") {
    myFile = SD.open("/calIn.txt");
    if (myFile) {
      Serial.println("calIn.txt");
      while (myFile.available()) {
        String line = myFile.readStringUntil('\n');
        Serial.println(line);
        calIn = line.toFloat();
        Serial.println(calIn);
        SerialBT.println("angka kalibrasi = ");
        SerialBT.println(calIn);
      }
    }
  }
  else if (message == "t010") {
    calIn = calIn + 10;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }
  else if (message == "t5") {
    calIn = calIn + 5;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }
  else if (message == "t1") {
    calIn = calIn + 1;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }
  else if (message == "-010") {
    calIn = calIn - 10;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }
  else if (message == "-5") {
    calIn = calIn - 5;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }
  else if (message == "-1") {
    calIn = calIn - 1;
    SerialBT.println(calIn);
    Serial.println(calIn);
    String filecalIn = String(calIn);
    int z = filecalIn.length();
    char fileCIArr[z + 1];
    strcpy(fileCIArr, filecalIn.c_str());
    writeFile(SD, "/calIn.txt", fileCIArr);
  }

  delay(20);
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    //    deactivate();
    //    ESP.restart();
    //    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
    //    deactivate();
    //    ESP.restart();
  }
  file.close();
}
