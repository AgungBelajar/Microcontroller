#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Pompa_IoT";
const char *password = "asdf12345";
const char *repeaterSSID = "repeater1";
const char *repeaterPassword = "asdf12345";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to the existing WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set up the repeater network
  WiFi.softAP(repeaterSSID, repeaterPassword);

  // Set up the web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, WiFi Repeater!");
  });

  // Start the server
  server.begin();
}

void loop() {
  // Nothing to do here for now
  Serial.println("cek sinyal");
  Serial.println(WiFi.status());
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  }
  delay(2000);
}
