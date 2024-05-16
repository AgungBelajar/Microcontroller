// Protus Tanuhandaru
// www.progresstech.co.id 

#include <Wire.h> // Memanggil library untuk komunikasi I2C
#define alamatSlave 10 // Alamat Slave untuk Arduino 
int dataSlave = 0; // menampung data komunikasi antara master dan slave 

void setup() {
  Wire.begin(alamatSlave); // Memulai komunikasi I2C pada PIN A4 dan PIN A5
  Wire.onRequest(requestEvents); // Memanggil fungsi ketika Slave menerima nilai dari Master
  Wire.onReceive(receiveEvents); // Memanggil fungsi ketika Master meminta nilai dari Slave
  Serial.println("ProgressTech");
  Serial.begin(9600); // Memulai komunikasi serial pada baud rate 9600
  
}

void requestEvents() {
  dataSlave = dataSlave + 1; // Menambahkan angka satu ke variabel komunikasi antara master dan slave
  Serial.println("Slave menerima permintaan dari Master");
  Serial.print("Mengirimkan nilai : "); // Menampilkan konten pada Serial Monitor 
  Serial.println(dataSlave); // Menampilkan konten pada Serial Monitor 
  Wire.write(dataSlave); // Slave mengirimkan data kepada Master
}

void receiveEvents(int jumlahByte) {
  dataSlave = Wire.read();
  Serial.print("Nilai diterima : "); // Menampilkan konten pada Serial Monitor
  Serial.println(dataSlave); // Menampilkan konten pada Serial Monitor
}

void loop() {
}
