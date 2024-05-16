#include <WiFi.h>
#include <HTTPClient.h>
const char *ssid = "IoT_SRS";  //Nama Wifi
const char *password = "DigitalArchitect2022"; // pass wifi
int idwl = 1;
String serverPath = "http://srs-ssms.com/post-ws-data.php";
String timeArr = "2022-08-11 10:37:55";

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void loop() {
      kirim_data();
      Serial.print("sudah dikirim coy");
      delay(5000);
 }

void kirim_data() {

  int tegangan, arus, suhu, kelembaban, avgIn;
  avgIn = random(0, 12); // ubah dengan data dari sensor
  arus = random(0, 3); // ubah dengan data dari sensor
  suhu = random(28, 32); // ubah dengan data dari sensor
  kelembaban = random(50, 95); // ubah dengan data dari sensor

  String postData = (String)"hum1=" + String(avgIn) +
                    "&d=" + timeArr + 
                    "&idsm=" + String(idwl);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverPath);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  auto httpCode = http.POST(postData);
  String payload = http.getString();

  Serial.println(postData);
  Serial.println(payload);

  http.end();
}
