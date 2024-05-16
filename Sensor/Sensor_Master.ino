// Protus Tanuhandaru
// www.progresstech.co.id 

#include <Wire.h> // Memanggil library untuk komunikasi I2C
#define alamatSlave 10 // Alamat Slave untuk Arduino 
int dataMaster = 0; // Variabel untuk menampung data komunikasi antara master dan slave 

void setup() {
  Wire.begin(); // Memulai komunikasi I2C pada PIN A4 dan PIN A5
  Serial.begin(9600); // Memulai komunikasi serial pada baud rate 9600
  Serial.println("ProgressTech");
}

void loop() {
  // Master mengirimkan nilai ke Slave
  Wire.beginTransmission(alamatSlave); // Master memulai pengiriman data ke alamat slave
  Wire.write(dataMaster); // Master menuliskan data
  Serial.print("Master mengirimakn nilai Integer : "); // Menampilkan konten pada Serial Monitor 
  Serial.println(dataMaster); // Menampilkan konten pada Serial Monitor 
  Wire.endTransmission(); // Mengakiri sesi pengiriman data 
  delay(1000);

  // Meminta nilai dari slave 
  Wire.requestFrom(alamatSlave, 1); // Master meminta data dari Slave
  dataMaster = Wire.read(); // Master membaca data yang dikirimkan oleh Slave
  Serial.print("Slave mengirimkan nilai : "); // Menampilkan konten pada Serial Monitor 
  Serial.println(dataMaster); // Menampilkan konten pada Serial Monitor
  delay(1000);
}
