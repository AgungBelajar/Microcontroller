#include <WiFi.h>

const char *ssid = "SRS";
const char *password = "SRS@2023";
const char *loginServer = "https://10.6.1.1/connect/PortalMain";
const int loginPort = 80;
const String loginPath = "";
const String username = "agung.prabowo";
const String userpassword = "1615UmarMuadz!";

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");

  // Simulasi proses login
  if (loginToWiFi()) {
    Serial.println("Login successful!");
  } else {
    Serial.println("Login failed.");
  }
}

void loop() {
  // Your main code goes here
}

bool loginToWiFi() {
  WiFiClient client;

  Serial.print("Connecting to login server: ");
  Serial.println(loginServer);

  if (client.connect(loginServer, loginPort)) {
    Serial.println("Connected to login server");

    String postData = "username=" + username + "&password=" + userpassword;

    // Send a POST request
    client.print("POST " + loginPath + " HTTP/1.1\r\n");
    client.print("Host: " + String(loginServer) + "\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: " + String(postData.length()) + "\r\n");
    client.print("\r\n");
    client.print(postData);

    delay(1000); // Give the server some time to respond

    // Read the response from the server
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    client.stop();

    return true; // If you reached this point, consider the login successful
  } else {
    Serial.println("Connection to login server failed");
    return false;
  }
}
