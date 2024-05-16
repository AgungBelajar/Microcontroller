#include <SD.h>

int variable1;
int variable2;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize SD card
  if (!SD.begin()) {
    Serial.println("SD card initialization failed.");
    return;
  }
  Serial.println("SD Card initialization success!");

  // Open the configuration file
  File configFile = SD.open("/config.txt");
  if (!configFile) {
    Serial.println("Failed to open configuration file.");
    return;
  }

  // Read the configuration parameters from the file
  while (configFile.available()) {
    String line = configFile.readStringUntil('\n');
    int equalsIndex = line.indexOf('=');
    if (equalsIndex != -1) {
      String variableName = line.substring(0, equalsIndex);
      String variableValue = line.substring(equalsIndex + 1);
      if (variableName == "variable1") {
        variable1 = variableValue.toInt();
        Serial.print("variable1 = ");
        Serial.println(variable1);
      
      } else if (variableName == "variable2") {
        variable2 = variableValue.toInt();
        Serial.print("variable2 = ");
        Serial.println(variable2);
      }
    }
  }

  // Close the configuration file
  configFile.close();
}

void loop() {
  // Main loop code goes here
//  Serial.print("variable1 = ");
//  Serial.println(variable1);
//  Serial.print("variable2 = ");
//  Serial.println(variable2);
//  delay(5000);
}
