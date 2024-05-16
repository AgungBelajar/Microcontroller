#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>

const char *ssid = "Vivo";
const char *password = "asdf12345";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Inisialisasi koneksi WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  
  // Inisialisasi SD card
  if (!SD.begin()) {
    Serial.println("Failed to initialize SD card");
    return;
  }

  // Mengatur penanganan permintaan root
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><table border='1'>";

    // Baca data dari file dan proses menjadi array
    String fileName = "/data.txt";
    File file = SD.open(fileName);
    if (file) {
      html += "<tr><td colspan='2'>Data from " + fileName + ":</td></tr>";

      // Membaca data dari file
      String data = "";
      while (file.available()) {
        data += (char)file.read();
      }
      file.close();

      // Memotong data menjadi array berdasarkan koma (CSV)
      String delimiter = ",";
      int startIndex = 0;
      int endIndex = data.indexOf(delimiter);

      while (endIndex != -1) {
        String value = data.substring(startIndex, endIndex);
        html += "<tr><td>" + value + "</td></tr>";

        startIndex = endIndex + delimiter.length();
        endIndex = data.indexOf(delimiter, startIndex);
      }

      // Menambahkan sisa data terakhir
      String lastValue = data.substring(startIndex);
      html += "<tr><td>" + lastValue + "</td></tr>";

      html += "</table></body></html>";
      request->send(200, "text/html", html);
    } else {
      html += "<tr><td colspan='2'>Failed to open " + fileName + "</td></tr></table></body></html>";
      request->send(200, "text/html", html);
    }
  });

  // Mulai server
  server.begin();
}

void loop() {
  // Tidak perlu ada yang di-loop untuk server AsyncWebServer
}
