#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include "Base64.h"  // Include Base64 library

const char* ssid = "Pompa_IoT";
const char* password = "asdf12345";
const char* serverUrl = "http://srs-ssms.com/aws_misol/upload-txt-wl.php";
int fileCount = 0;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize SPIFFS
  if (!SD.begin(5)) {
    Serial.println("An error has occurred while mounting microSD");
    return;
  }
  fileCount = countFiles();
  Serial.print("Jumlah file di kartu SD: ");
  Serial.println(fileCount);
}

void sendFileToServer(const char* namaFile) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverUrl);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Open the file for reading
    File file = SD.open(namaFile, FILE_READ);
    if (!file) {
      Serial.println("Failed to open file for reading");
      return;
    }

    // Read the file and encode it in Base64
    size_t fileSize = file.size();
    uint8_t* buffer = (uint8_t*) malloc(fileSize);
    file.read(buffer, fileSize);

    String base64Data = base64::encode(buffer, fileSize);
    free(buffer);
    file.close();

    // Prepare the data to be sent
    String postData = "fileData=" + base64Data;
    Serial.println(postData);

    // Send the request
    int httpResponseCode = http.POST(postData);

    // Print the response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      deleteFile(SD, namaFile);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    // End the HTTP connection
    http.end();
  }
}

void loop() {
  // Nothing to do here
  if (fileCount > 0) {
    for (int i = 1; i <= fileCount; i++) {
      String nama = "/data" + String(i) + ".txt";
      const char* namaFile = nama.c_str();
      sendFileToServer(namaFile);
      delay(10000);
    }
  }
  else {
    Serial.println("DATA HABIS BOS!");
  }
}

int countFiles() {
  int fileCount = 0;
  File root = SD.open("/");

  // Loop melalui semua file di direktori
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // Tidak ada lagi file di direktori
      break;
    }
    if (!entry.isDirectory()) {
      // Jika file bukan direktori, tambahkan hitungan file
      fileCount++;
    }
    entry.close();
  }
  root.close();
  return fileCount;
}

void deleteFile(fs::FS & fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
