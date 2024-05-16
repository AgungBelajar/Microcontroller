bool test = true;

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD.h"
#include "SPI.h"
#include <HTTPClient.h>
#include <WiFiMulti.h> // Built-in
#include <string>

bool state = true;
#define compressor 25
#define sol_buka 26
#define sol_tutup 27
#define button_buka 34
#define button_tutup 35
#define selektor_auto 13
#define selektor_manual 15

bool debugMode = true;
// Setup
float sensorD = 320.47; // sensor ke dasar
float sensorN = 190.47; // sensor ke nol blok
float minimum = -50;
float maximum = -39;
float benteng = -115;
float calIn = 10;
float calOut = 138;

String jam;

const int arIn = 10;
const int arOut = 10;
float avgInAr[arIn], avgOutAr[arOut] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float avgIn, avgOut = 0;
int idwl = 55;
int log_time_unit = 15; // 5 menit
int test_time = 1;      // test time

// declare ultrasonic [trig, echo]
const int ultrasonic1[2] = {2, 4};
const int ultrasonic2[2] = {14, 12};

unsigned long test_siklus = 5000;
unsigned long siklusPompa = 7200000;
bool triggerPompa = true;

// setting id water level dan php
String serverPath = "http://srs-ssms.com/post-wl-data.php";
// 172.105.125.184

// define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// pengukuran ultrasonic
float distanceCmIn[10], distanceCmOut[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String distanceStrIn, distanceStrOut = "";
float distanceNowIn, distanceNowOut = 0;

// password wifi1 default
const char *ssid = "asdf";
const char *password = "asdfasdfasdf";
// const char* ssid = "wifi";
// const char* password = "wifi2021";

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

// Jam
#include "RTClib.h"
RTC_DS3231 rtc;
String timeArr[10] = {"null", "null", "null", "null", "null", "null", "null", "null", "null", "null"};
String timeStr, timeNow = "";
// interrupt
int timer_cnt, log_interval, log_count;

long uptimeInt = 0;
long nyalaInt = 90;
long matiInt = 30;
bool seq = false;

void avgInFn(float nilaiInBaru);
void avgOutFn(float nilaiOutBaru);
void debug(String noDebug);
void connectToWifi();
float getDistance(const int *ultrasonic, float cal);
float getDistanceAct(const int *ultrasonic, float cal);
String getTime();
void logging();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void readFile(fs::FS &fs, const char *path);
String readLine(fs::FS &fs, const char *path);
void createDir(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void deleteFile(fs::FS &fs, const char *path);
struct LinesAndPositions FindLinesAndPositions(char *filename);
void CopyFiles(char *ToFile, char *FromFile);
void DeleteLineFromFile(char *filename, int Line);
void DeleteSelectedLinesFromFile(char *filename, char *StrOfLines);
void DeleteMultipleLinesFromFile(char *filename, int SLine, int ELine);
int kirimDataKeServer(String postRequest);

void setup()
{
  delay(1000); // delay nunggu wifi nyala
  if (test)
  {
    siklusPompa = test_siklus;
    log_time_unit = test_time;
  }

  // set awal interrupt
  log_count = 0;
  log_interval = log_time_unit * 10;
  timer_cnt = log_interval + 1;
  Serial.begin(115200);  // Starts the serial communication
  pinMode(compressor, OUTPUT);
  pinMode(sol_buka, OUTPUT);
  pinMode(sol_tutup, OUTPUT);
  pinMode(button_buka, INPUT);
  pinMode(button_tutup, INPUT);
  pinMode(selektor_auto, INPUT);
  pinMode(selektor_manual, INPUT);

  digitalWrite(compressor, HIGH);
  digitalWrite(sol_buka, HIGH);
  digitalWrite(sol_tutup, HIGH);

  // set pin ultrasonic
  pinMode(ultrasonic1[0], OUTPUT); // Sets the trigPin as an Output
  pinMode(ultrasonic1[1], INPUT);  // Sets the echoPin as an Input
  pinMode(ultrasonic2[0], OUTPUT); // Sets the trigPin as an Output
  pinMode(ultrasonic2[1], INPUT);  // Sets the echoPin as an Input

  debug("inisalisasi ultrasonik");

  // set awal jam
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    //    Serial.flush();
    //    abort();
  }
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  debug("inisalisasi RTC");

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  debug("inisalisasi SPIFF");

  // Initialize SD
  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  debug("inisalisasi SPIFF");

  // Initialize ssid wifi
  writeFile(SPIFFS, "/wifi.txt", ssid); // todo
  writeFile(SPIFFS, "/pass.txt", password);
  ssidStr = readLine(SPIFFS, "/wifi.txt"); // todo
  pwdStr = readLine(SPIFFS, "/pass.txt");  // todo
  debug("inisalisasi password");

  String statemSD = readLine(SD, "/state.txt");
  if (statemSD == "true")
  {
    state = true;
  }
  else if (statemSD = "false")
  {
    state = false;
  }
  else
  {
    Serial.println("state fail");
  }
  Serial.println(state);

  // Connect to Wi-Fi
  connectToWifi();
  debug("inisalisasi konek wifi");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/code.min.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/code.min.js", "text/javascript");
  });
  server.on("/distancein", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceNowIn).c_str());
  });
  server.on("/distanceout", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceNowOut).c_str());
  });
  server.on("/timenow", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", timeNow.c_str());
  });
  server.on("/distancein0", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[0]).c_str());
  });
  server.on("/distancein1", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[1]).c_str());
  });
  server.on("/distancein2", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[2]).c_str());
  });
  server.on("/distancein3", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[3]).c_str());
  });
  server.on("/distancein4", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[4]).c_str());
  });
  server.on("/distancein5", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[5]).c_str());
  });
  server.on("/distancein6", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[6]).c_str());
  });
  server.on("/distancein7", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[7]).c_str());
  });
  server.on("/distancein8", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[8]).c_str());
  });
  server.on("/distancein9", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmIn[9]).c_str());
  });
  server.on("/distanceout0", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[0]).c_str());
  });
  server.on("/distanceout1", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[1]).c_str());
  });
  server.on("/distanceout2", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[2]).c_str());
  });
  server.on("/distanceout3", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[3]).c_str());
  });
  server.on("/distanceout4", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[4]).c_str());
  });
  server.on("/distanceout5", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[5]).c_str());
  });
  server.on("/distanceout6", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[6]).c_str());
  });
  server.on("/distanceout7", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[7]).c_str());
  });
  server.on("/distanceout8", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[8]).c_str());
  });
  server.on("/distanceout9", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(distanceCmOut[9]).c_str());
  });
  server.on("/time0", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[0]).c_str());
  });
  server.on("/time1", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[1]).c_str());
  });
  server.on("/time2", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[2]).c_str());
  });
  server.on("/time3", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[3]).c_str());
  });
  server.on("/time4", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[4]).c_str());
  });
  server.on("/time5", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[5]).c_str());
  });
  server.on("/time6", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[6]).c_str());
  });
  server.on("/time7", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[7]).c_str());
  });
  server.on("/time8", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[8]).c_str());
  });
  server.on("/time9", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", String(timeArr[9]).c_str());
  });

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

  createDir(SD, "/LOG");
}

void loop()
{
  int val_buka = digitalRead(button_buka);
  int val_tutup = digitalRead(button_tutup);
  int sel_au = digitalRead(selektor_auto);
  int sel_man = digitalRead(selektor_manual);
  
  if (sel_au == HIGH)
  {
    Serial.println("auto");
  }
  else if (sel_man == HIGH)
  {
    Serial.println("manual");
  }
  else
  {
    Serial.println("off");
  }
  
  // kirim data aktual ke webserver
  distanceNowIn = getDistance(ultrasonic1, calIn);
  distanceNowOut = getDistance(ultrasonic2, calOut);
  Serial.println("DistanceIn (cm): " + String(distanceNowIn));
  Serial.println("DistanceOut (cm) : " + String(distanceNowOut));
  debug("ukur realtime ultrasonik");

  avgInFn(distanceNowIn);
  avgOutFn(distanceNowOut);
  debug("Avg in : " + String(avgIn));
  debug("Avg out : " + String(avgOut));
  timeNow = getTime();
  debug("dapatkan waktu");

  // implementasi interrupt
  if (timeNow != "1970-1-1 00:00:00" and timer_cnt >= log_interval)
  {
    timer_cnt = 0;  // log_interval values are 10=15secs 40=1min 200=5mins 400=10mins 2400=1hr
    log_count += 1; // Increase loggin event count
    debug("interrupt");
    if (sel_man == HIGH)
    {
      if (val_buka == HIGH && val_tutup == LOW)
      {
        digitalWrite(compressor, LOW);
        delay(2000);
        digitalWrite(compressor, LOW);
        digitalWrite(sol_buka, LOW);
        Serial.println("BUKA");
        delay(20000);
        digitalWrite(sol_buka, HIGH);
        digitalWrite(compressor, HIGH);
        state = true;
        writeFile(SD, "/state.txt", "true");
      }

      else if (val_tutup == HIGH && val_buka == LOW)
      {
        digitalWrite(compressor, LOW);
        delay(2000);
        digitalWrite(compressor, LOW);
        digitalWrite(sol_tutup, LOW);
        Serial.println("TUTUP");
        delay(20000);
        digitalWrite(sol_tutup, HIGH);
        digitalWrite(compressor, HIGH);
        state = false;
        writeFile(SD, "/state.txt", "false");
      }
      else
      {
        digitalWrite(compressor, HIGH);
        digitalWrite(sol_tutup, HIGH);
        Serial.println("TIDAK BERGERAK");
      }
    }
    else if (sel_au == HIGH)
    {
      if (distanceNowIn > distanceNowOut && distanceNowOut <= minimum )
      {
        if (distanceNowIn > maximum && state == false)
        {
          digitalWrite(compressor, LOW);
          delay(2000);
          digitalWrite(compressor, LOW);
          digitalWrite(sol_buka, LOW);
          Serial.println("BUKA");
          delay(20000);
          digitalWrite(sol_buka, HIGH);
          digitalWrite(compressor, HIGH);
          state = true;
          writeFile(SD, "/state.txt", "true");
        }

        else if (distanceNowIn < minimum && state == true)
        {
          digitalWrite(compressor, LOW);
          delay(2000);
          digitalWrite(compressor, LOW);
          digitalWrite(sol_tutup, LOW);
          Serial.println("TUTUP");
          delay(20000);
          digitalWrite(sol_tutup, HIGH);
          digitalWrite(compressor, HIGH);
          state = false;
          writeFile(SD, "/state.txt", "false");
        }
        else
        {
          Serial.println("TIDAK BERGERAK");
        }
      }
      else if (distanceNowIn <= distanceNowOut && state == true)
      {
        digitalWrite(compressor, LOW);
        delay(2000);
        digitalWrite(compressor, LOW);
        digitalWrite(sol_tutup, LOW);
        Serial.println("TUTUP");
        delay(20000);
        digitalWrite(sol_tutup, HIGH);
        digitalWrite(compressor, HIGH);
        state = false;
        writeFile(SD, "/state.txt", "false");
      }
      else
      {
        Serial.println("TIDAK BERGERAK");
      }
    }
    else
    {
      Serial.println("OFF");
    }
    logging();
    debug("logging data");
  }
  timer_cnt += 1; // Readings set by value of log_interval each 40 = 1min

  // periksa ada data di dalam txt atau ga

  File root = SD.open("/LOG");
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
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


void avgInFn(float nilaiInBaru)
{
  float totalIn = 0;
  for (int i = arIn - 1; i > 0; i--)
  {
    avgInAr[i] = avgInAr[i - 1];
    totalIn += avgInAr[i];
  }
  avgInAr[0] = nilaiInBaru;
  totalIn += nilaiInBaru;
  avgIn = totalIn / arIn;
}

void avgOutFn(float nilaiOutBaru)
{
  float totalOut = 0;
  for (int i = arOut - 1; i > 0; i--)
  {
    avgOutAr[i] = avgOutAr[i - 1];
    totalOut += avgOutAr[i];
  }
  avgOutAr[0] = nilaiOutBaru;
  totalOut += nilaiOutBaru;
  avgOut = totalOut / arOut;
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
  {

    // mengubah ssid dan pwd dari string ke char array
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
      if (i > 5)
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

float getDistance(const int *ultrasonic, float cal)
{
  float dt = 0;
  dt = (getDistanceAct(ultrasonic, cal) - sensorN) * -1;
  return dt;
}

float getDistanceAct(const int *ultrasonic, float cal)
{
  long duration;
  float dt = 0;
  // Clears the trigPin
  digitalWrite(ultrasonic[0], LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(ultrasonic[0], HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonic[0], LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ultrasonic[1], HIGH);
  duration *SOUND_SPEED / 2;

  dt = duration * SOUND_SPEED / 2;
  dt += cal;
  return dt;
}

String getTime()
{
  DateTime now = rtc.now();
  String timeS = String(now.year(), DEC) + "-" + String(now.month(), DEC) + "-" + String(now.day(), DEC) + " " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  jam = String(now.year(), DEC) + "_" + String(now.month(), DEC) + "_" + String(now.day(), DEC) + " " + String(now.hour(), DEC) + "_" + String(now.minute(), DEC) + "_" + String(now.second(), DEC);
  return timeS;
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

void listDir(fs::FS & fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS & fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

String readLine(fs::FS & fs, const char *path)
{
  String line = "";
  Serial.printf("Reading line: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    //    deactivate();
    //    ESP.restart();
    //    return line;
  }

  Serial.println("- read from line:");
  line = file.readStringUntil('\n');
  Serial.println(line);

  file.close();
  return line;
}

void createDir(fs::FS & fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (!fs.exists(path))
  {
    if (fs.mkdir(path))
    {
      Serial.println("Dir created");
    }
    else
    {
      Serial.println("mkdir failed");
    }
  }
  else
  {
    Serial.println("dir udah ada");
  }
}

void writeFile(fs::FS & fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    //    deactivate();
    //    ESP.restart();
    //    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
    //    deactivate();
    //    ESP.restart();
  }
  file.close();
}

void appendFile(fs::FS & fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- message appended");
  }
  else
  {
    Serial.println("- append failed");
  }
  file.close();
}

void deleteFile(fs::FS & fs, const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
}

struct LinesAndPositions FindLinesAndPositions(char *filename)
{
  File myFile;
  LinesAndPositions LNP;

  myFile = SD.open(filename);
  if (myFile)
  {
    LNP.NumberOfLines = 0;
    LNP.SOL[0] = 0; // the very first start-of-line index is always zero
    int i = 0;

    while (myFile.available())
    {
      if (myFile.read() == '\n') // read until the newline character has been found
      {
        LNP.EOL[LNP.NumberOfLines] = i;     // record the location of where it is in the file
        LNP.NumberOfLines++;                // update the number of lines found
        LNP.SOL[LNP.NumberOfLines] = i + 1; // the start-of-line is always 1 character more than the end-of-line location
      }
      i++;
    }
    LNP.EOL[LNP.NumberOfLines] = i; // record the last locations
    LNP.NumberOfLines += 1;         // record the last line

    myFile.close();
  }

  return LNP;
}

void CopyFiles(char *ToFile, char *FromFile)
{
  File myFileOrig;
  File myFileTemp;

  File file = SD.open(ToFile);

  if (!file || file.isDirectory())
  {
    SD.remove(ToFile);
  }

  myFileTemp = SD.open(ToFile, FILE_WRITE);
  myFileOrig = SD.open(FromFile);
  if (myFileOrig)
  {
    while (myFileOrig.available())
    {
      myFileTemp.write(myFileOrig.read()); // make a complete copy of the original file
    }
    myFileOrig.close();
    myFileTemp.close();
    // Serial.println("done.");
  }
}

void DeleteLineFromFile(char *filename, int Line)
{
  DeleteMultipleLinesFromFile(filename, Line, Line);
}

void DeleteSelectedLinesFromFile(char *filename, char *StrOfLines)
{
  byte offset = 0;
  Serial << "Deleting multiple lines, please wait";
  for (unsigned short i = 0, j = strlen(StrOfLines), index = 0; i <= j; i++)
  {
    char C = (*StrOfLines++);
    if (isComma(C) || (i == j))
    {
      DeleteLineFromFile(filename, index - offset);
      offset++;
      index = 0;
    }
    else if (isSpace(C))
      continue;
    else
      index = (index * 10) + C - '0';
    if ((i % 2) == 0)
      Serial << ".";
  }
  Serial.println();
}

void DeleteMultipleLinesFromFile(char *filename, int SLine, int ELine)
{
  File myFileOrig;
  File myFileTemp;

  // If by some chance it exists, remove the temp file from the card
  File file = SD.open("tempFile.txt");
  if (!file || file.isDirectory())
  {
    SD.remove("tempFile.txt");
  }

  // Get the start and end of line positions from said file
  LinesAndPositions FileLines = FindLinesAndPositions(filename);

  if ((SLine > FileLines.NumberOfLines) || (ELine > FileLines.NumberOfLines))
  {
    return;
  }

  myFileTemp = SD.open("tempFile.txt", FILE_WRITE);
  myFileOrig = SD.open(filename);
  int position = 0;

  if (myFileOrig)
  {
    while (myFileOrig.available())
    {
      char C = myFileOrig.read();

      // Copy the file but exclude the entered lines by user
      if ((position < FileLines.SOL[SLine - 1]) || (position > FileLines.EOL[ELine - 1]))
        myFileTemp.write(C);

      position++;
    }
    myFileOrig.close();
    myFileTemp.close();
  }

  // copy the contents of tempFile back to the original file
  CopyFiles(filename, "tempFile.txt");

  // Remove the tempfile from the FS card
  File fl = SD.open("tempFile.txt");
  if (!fl || fl.isDirectory())
  {
    SD.remove("tempFile.txt");
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
