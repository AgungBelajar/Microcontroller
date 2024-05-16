#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD.h"
#include "SPI.h"
#include <HTTPClient.h>
#include <WiFiMulti.h> // Built-in
#include <string>

// setting id water level dan php
/* String serverPath = "http://srs-ssms.com/post-wl-data-test.php"; */
String serverPath = "http://srs-ssms.com/post-wl-data.php";
// 172.105.125.184

// password wifi1 default
const char *ssid = "asdf";
const char *password = "asdfasdfasdf";
String ssidStr, pwdStr; // password wifi2 dari SPIFF
AsyncWebServer server(80); // inisialisasi webserver
WiFiMulti wifiMulti; // inisialisasi wifi

// Delete line di TXT
#define isSpace(x) (x == ' ')
#define isComma(x) (x == ',')
template <class T>
inline Print &operator<<(Print &str, T arg)
{
  str.print(arg);
  return str;
}

struct LinesAndPositions
{
  int NumberOfLines; // number of lines in file
  int SOL[50];       // start of line in file
  int EOL[50];       // end of line in file
};

void connectToWifi();
int kirimDataKeServer(String postRequest);

void setup()
{
  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  debug("inisalisasi SPIFF");

  // Initialize ssid wifi
  ssidStr = readLine(SPIFFS, "/wifi.txt"); // todo
  pwdStr = readLine(SPIFFS, "/pass.txt");  // todo
  debug("inisalisasi password");

  // Connect to Wi-Fi
  connectToWifi();
  debug("inisalisasi konek wifi");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  server.on("/code.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/code.min.js", "text/javascript"); });
  server.on("/distancein", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceNowIn).c_str()); });
  server.on("/distanceout", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceNowOut).c_str()); });
  server.on("/timenow", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", timeNow.c_str()); });
  server.on("/distancein0", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[0]).c_str()); });
  server.on("/distancein1", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[1]).c_str()); });
  server.on("/distancein2", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[2]).c_str()); });
  server.on("/distancein3", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[3]).c_str()); });
  server.on("/distancein4", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[4]).c_str()); });
  server.on("/distancein5", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[5]).c_str()); });
  server.on("/distancein6", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[6]).c_str()); });
  server.on("/distancein7", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[7]).c_str()); });
  server.on("/distancein8", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[8]).c_str()); });
  server.on("/distancein9", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmIn[9]).c_str()); });
  server.on("/distanceout0", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[0]).c_str()); });
  server.on("/distanceout1", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[1]).c_str()); });
  server.on("/distanceout2", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[2]).c_str()); });
  server.on("/distanceout3", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[3]).c_str()); });
  server.on("/distanceout4", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[4]).c_str()); });
  server.on("/distanceout5", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[5]).c_str()); });
  server.on("/distanceout6", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[6]).c_str()); });
  server.on("/distanceout7", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[7]).c_str()); });
  server.on("/distanceout8", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[8]).c_str()); });
  server.on("/distanceout9", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(distanceCmOut[9]).c_str()); });
  server.on("/time0", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[0]).c_str()); });
  server.on("/time1", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[1]).c_str()); });
  server.on("/time2", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[2]).c_str()); });
  server.on("/time3", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[3]).c_str()); });
  server.on("/time4", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[4]).c_str()); });
  server.on("/time5", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[5]).c_str()); });
  server.on("/time6", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[6]).c_str()); });
  server.on("/time7", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[7]).c_str()); });
  server.on("/time8", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[8]).c_str()); });
  server.on("/time9", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(timeArr[9]).c_str()); });

  // Start server
  server.begin();
  debug("inisalisasi server");

  int i = 1;
  while (i < arIn + 1)
  {
    distanceNowIn = getDistance(ultrasonic1, calIn);
    distanceNowOut = getDistance(ultrasonic2, calOut);
    avgInFn(distanceNowIn);
    avgOutFn(distanceNowOut);
    i++;
    String a = "pengukuran awal " + String(i);
    debug(a);
    delay(200);
  }
}

void loop()
{

  // kirim data aktual ke webserver
  if (timeNow != "1970-1-1 00:00:00" and timer_cnt >= log_interval)
  {
    timer_cnt = 0;  // log_interval values are 10=15secs 40=1min 200=5mins 400=10mins 2400=1hr
    log_count += 1; // Increase loggin event count
    debug("interrupt");

    logging();
    debug("logging data");
  }

  File file = root.openNextFile();
  if (file)
  {
    String fileName = file.name();
    int z = fileName.length();
    char fileNmArr[z + 1];
    strcpy(fileNmArr, fileName.c_str());
    String payload = readLine(SD, fileNmArr);

    int code = kirimDataKeServer(payload);
    if (code == 1)
    {
      deleteFile(SD, fileNmArr);
      Serial.println("Sukses");
    }
    else
    {
      Serial.println("Gagal");
    }
  }

  delay(500);
}

void debug(String noDebug)
{
  if (debugMode)
  {
    Serial.println("debug " + noDebug);
  }
}

void connectToWifi()
{
  if (wifiMulti.run() != WL_CONNECTED)
  { // mengubah ssid dan pwd dari string ke char array
    int p = ssidStr.length();
    char ssidArr[p + 1];
    strcpy(ssidArr, ssidStr.c_str());
    int l = pwdStr.length();
    char pwdArr[l + 1];
    strcpy(pwdArr, pwdStr.c_str());

    wifiMulti.addAP(ssid, password);  // add Wi-Fi networks you want to connect to, it connects strongest to weakest
    wifiMulti.addAP(ssidArr, pwdArr); // Adjust the values in the Network tab
    Serial.println("Connecting ...");
    int i = 0;
    while (wifiMulti.run() != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
      delay(250);
      Serial.print('.');
      i++;
      if (i > 15)
      {
        Serial.println("");
        break;
      }
    }
    if (wifiMulti.run() == WL_CONNECTED)
    {
      // Print ESP32 Local IP Address
      Serial.println(WiFi.localIP());
    }
  }
}


// log data ke dalam SPIFFS
void logging()
{

  // geser array nilai aktual
  for (int j = 9; j > 0; j--)
  {
    distanceCmIn[j] = distanceCmIn[j - 1];
    distanceCmOut[j] = distanceCmOut[j - 1];
    timeArr[j] = timeArr[j - 1];
  }
  distanceCmIn[0] = getDistance(ultrasonic1, calIn);
  distanceCmOut[0] = getDistance(ultrasonic2, calOut);
  timeArr[0] = getTime();

  // geser array nilai string
  distanceStrIn = "[";
  distanceStrOut = "[";
  timeStr = "[";
  for (int i = 0; i < 10; i++)
  {
    distanceStrIn += distanceCmIn[i];
    distanceStrOut += distanceCmOut[i];
    timeStr += timeArr[i];
    if (i < 9)
    {
      distanceStrIn += ", ";
      distanceStrOut += ", ";
      timeStr += ",";
    }
  }
  distanceStrIn += "]";
  distanceStrOut += "]";
  timeStr += "]";

  // Prints the distance in the Serial Monitor
  Serial.println("DistanceIn (cm): " + String(distanceCmIn[0]));
  Serial.println("Distance ArrayIn: " + String(distanceStrIn));
  Serial.println("DistanceOut (cm): " + String(distanceCmOut[0]));
  Serial.println("Distance ArrayOut: " + String(distanceStrOut));
  Serial.println("Time: " + String(timeArr[0]));
  Serial.println("Time Array: " + String(timeStr));

  if (wifiMulti.run() == WL_CONNECT_FAILED)
  {
    connectToWifi();
  }

  //  String payload = "lvl_in=" + String(distanceCmIn[0]) + "&lvl_out=" + String(distanceCmOut[0]) + "&d=" + String(timeArr[0]);
  String payload = "lvl_in=" + String(avgIn) + "&lvl_out=" + String(avgOut) + "&d=" + String(timeArr[0]);
  String appendPayload = payload + "&idwl=" + idwl;
  int code = kirimDataKeServer(payload);
  if (code == 1)
  {
    Serial.println("Sukses");
  }
  else
  {
    // convert httpRequest to Char
    int payloadLength = appendPayload.length();
    char httpArr[payloadLength + 1];
    strcpy(httpArr, appendPayload.c_str());
    Serial.println("Gagal");
    String fileNm = "/LOG/" + jam + ".txt";
    int fileNmLg = fileNm.length();
    char fileNmArr[fileNmLg + 1];
    strcpy(fileNmArr, fileNm.c_str());
    writeFile(SD, fileNmArr, httpArr);
  }
}

int kirimDataKeServer(String postRequest)
{
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  int success = 0;
  WiFiClient client;
  HTTPClient http; // Declare object of class HTTPClient
  Serial.println("send begin..");
  http.begin(client, serverPath);
  debug("http begin");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  debug("masukkan header");
  String isi = postRequest + "&idwl=" + idwl;
  debug("isi:" + isi);
  int httpCode = http.POST(isi); // Send the request
  debug("ekseskusi post");
  String payload = http.getString(); // Get the response payload
  debug("ambil payload");
  Serial.println(String(httpCode) + " - " + payload); // Print HTTP return code
  http.end();
  Serial.println("http code:" + String(httpCode));
  if (httpCode == 200)
    success = 1;
  else
    success = 0;
  Serial.println("send end..");
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  return success;
}
