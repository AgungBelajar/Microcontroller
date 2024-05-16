#include <LoRa.h>
#include <LoRaMesher.h>
#include <LoRaMesherRoutes.h>
#include <HardwareSerial.h>

#define LORA_BAND 915E6 // Sesuaikan dengan frekuensi yang Anda gunakan
#define NODE_NAME "Node1" // Ubah sesuai dengan node yang lain

void setup() {
  Serial.begin(115200);
  
  // Initialize LoRa
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Initialize LoRaMesher
  LoRaMesher.begin(NODE_NAME);
  LoRaMesher.onReceive(receiveMessage);
  
  Serial.println("LoRaMesher started!");
}

void loop() {
  // Handle LoRaMesher tasks
  LoRaMesher.run();

  // Kirim pesan ke node tertentu (misalnya ke Node2)
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    LoRaMesher.sendMessage("Node2", message);
    Serial.println("Message sent: " + message);
  }
}

void receiveMessage(String sender, String message) {
  Serial.print("Received message from ");
  Serial.print(sender);
  Serial.print(": ");
  Serial.println(message);
}
