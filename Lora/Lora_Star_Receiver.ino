#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  // Initialize LoRa module
  if (!LoRa.begin(915E6)) { // Sesuaikan dengan frekuensi yang Anda gunakan
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // Cek apakah ada data yang masuk
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Baca paket
    String receivedText = "";
    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }

    // Tampilkan pesan yang diterima
    Serial.print("Received message: ");
    Serial.println(receivedText);
  }
}
