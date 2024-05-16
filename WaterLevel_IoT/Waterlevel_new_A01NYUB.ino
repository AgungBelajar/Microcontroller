#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <SD.h>
#include <SPI.h>
#include <RTClib.h>
#include <Timer.h>
#include <Ticker.h>
#include <Wire.h>
#include <ArduinoJson.h>

// define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define WATCHDOG_TIMEOUT 60
int watchdogMin = 0;
int watchdogTimer = 11;

Ticker watchdogTicker;
WebServer server(80);
Timer checkLoop, simpanData, kirimDataMSD, checkData, cekWaktu;
RTC_DS3231 rtc;

bool test = false;
bool checkSend = false;
float sensorD = 320.47; // sensor ke dasar
float sensorN = 190.47; // sensor ke nol blok
float minimum = -60;
float maximum = -50;
float calIn = 0;
int idwl = 1;
int delCheckData = 3600000;

//A01NYUB
#define CONTROL_PIN 5   // This is the YELLOW wire, can be any data line
int16_t distance;  // The last measured distance
bool newData = false; // Whether new data is available from the sensor
uint8_t buffer[4];  // our buffer for storing data
uint8_t idx = 0;  // our idx into the storage buffer

String lastValue;
String jam;
String timeArr[10] = {"null", "null", "null", "null", "null", "null", "null", "null", "null", "null"};
String timeStr, timeNow = "";
int delLogging = 600000;
int delCekData = 3600000;
bool seq = false;
DateTime koreksiPHP;

String serverPath = "http://srs-ssms.com/post-wl-data.php";
String ssid = "Pompa_IoT";
String password = "asdf12345";

const int arIn = 10;
float avgInAr[arIn] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float avgIn = 0;

// declare ultrasonic [trig, echo]
const int ultrasonic1[2] = {2, 4};
float distanceCmIn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String distanceStrIn = "";
float distanceNowIn = 0;


void resetWatchdog();
void sendChecker();
void avgInFn(float nilaiInBaru);
float getDistance(const int *ultrasonic, float cal);
float getDistanceAct(const int *ultrasonic, float cal);
String getTime();
void Logging();
int kirimData(String postRequest);
void connectWifi();
void readFile(fs::FS & fs, const char *path);
String readLine(fs::FS & fs, const char *path);
void createDir(fs::FS & fs, const char *path);
void writeFile(fs::FS & fs, const char *path, const char *message);
void listDir(fs::FS & fs, const char *dirname, uint8_t levels);
void deleteFile(fs::FS & fs, const char *path);
void kirimDataMsd();
int countFiles(File folder);
void cekDataFolder();
DateTime stringToDateTime(String waktuString);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);
  watchdogTicker.attach(WATCHDOG_TIMEOUT, resetWatchdog);

  delay(1000);
  if (test)
  {
    delLogging = 30000;
    delCekData = 60000;
  }

  //setup ultrasonic
  pinMode(ultrasonic1[0], OUTPUT); // Sets the trigPin as an Output
  pinMode(ultrasonic1[1], INPUT);  // Sets the echoPin as an Input
  Serial.println("inisialisasi ultrasonik");
  //setup RTC
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    //    abort();
  }
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Serial.println("inisialisasi RTC");
  //setup micro SD
  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    //    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    //    return;
  }
  Serial.println("inisialisasi SD Card");

  //setup internet
  connectWifi();
  Serial.println("inisialisasi connect wifi");
  server.begin();

  int i = 1;
  while (i < arIn + 1)
  {
    distanceNowIn = getDistance(ultrasonic1, calIn);
    avgInFn(distanceNowIn);
    i++;
    String a = "pengukuran awal " + String(i);
    Serial.println(a);
    delay(200);
  }
  createDir(SD, "/LOG");
  simpanData.every(delLogging, Logging);
  kirimDataMSD.every(4000, kirimDataMsd);
  checkData.every(delCekData, cekDataFolder);
}

void loop() {
  simpanData.update();
  kirimDataMSD.update();
  checkData.update();

  // realtime ultrasonic
  distanceNowIn = getDistance(ultrasonic1, calIn);
  avgInFn(distanceNowIn);
  Serial.println("DistanceIn (cm): " + String(distanceNowIn));
  Serial.println("Avg In : " + String(avgIn));
  Serial.println("ukur realtime ultrasonik");
  timeNow = getTime();
  Serial.println(jam);
  delay(500);
  watchdogMin = 0;
}

void sendChecker() {
  if (checkSend == false) {
    //    digitalWrite(relayPin, LOW);
    //    delay(4000);
    //    digitalWrite(relayPin, HIGH);
    ESP.restart();
  } else {
    checkSend = false;
  }
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

void Logging()
{
  for (int j = 9; j > 0; j--)
  {
    distanceCmIn[j] = distanceCmIn[j - 1];
    timeArr[j] = timeArr[j - 1];
  }
  distanceCmIn[0] = getDistance(ultrasonic1, calIn);
  timeArr[0] = getTime();

  // geser array nilai string
  distanceStrIn = "[";
  timeStr = "[";
  for (int i = 0; i < 10; i++)
  {
    distanceStrIn += distanceCmIn[i];
    timeStr += timeArr[i];
    if (i < 9)
    {
      distanceStrIn += ", ";
      timeStr += ",";
    }
  }
  distanceStrIn += "]";
  timeStr += "]";

  String appendPayload = "lvl_in=" + String(avgIn) + "&d=" + String(timeArr[0]) + "&idwl=" + idwl;
  Serial.println(appendPayload);
  // kirim ke microSD
  int payloadLength = appendPayload.length();
  char httpArr[payloadLength + 1];
  strcpy(httpArr, appendPayload.c_str());
  String fileNm = "/LOG/" + jam + ".txt";
  int fileNmLg = fileNm.length();
  char fileNmArr[fileNmLg + 1];
  strcpy(fileNmArr, fileNm.c_str());
  Serial.println(appendPayload);
  writeFile(SD, fileNmArr, httpArr);
}

int kirimData(String postRequest)
{
  int success = 0;
  WiFiClient client;
  HTTPClient http; // Declare object of class HTTPClient
  Serial.println("send begin..");
  http.begin(client, serverPath);
  Serial.println("http begin");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  Serial.println("masukkan header");
  String isi = postRequest + "&idwl=" + idwl;
  Serial.println(isi);
  int httpCode = http.POST(isi);
  Serial.println("eksekusi post");
  String payload = http.getString();
  http.end();
  Serial.println("http code:" + String(httpCode));
  if (httpCode == 200)
  {
    Serial.println(String(httpCode) + " - " + payload); // Print HTTP return code
    String datetime = getDatetimeFromJSON(payload.c_str());
    DateTime koreksiPHP = stringToDateTime(datetime);
    rtc.adjust(koreksiPHP);
    success = 1;
  }
  else
    success = 0;
  Serial.println("send end..");
  return success;
}

void connectWifi()
{
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(750);
    Serial.print(".");
    i++;
    if (i >= 15)
    {
      Serial.println("fail connect to : " + String(ssid));
      break;
    }
  }
  Serial.print("connected to ssid: ");
  Serial.println(ssid);
  Serial.println(WiFi.localIP());
}


void readFile(fs::FS & fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    //    return;
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
    return line;
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

void kirimDataMsd()
{
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
    checkSend = false;
    String fileName = file.name();
    int z = fileName.length();
    char fileNmArr[z + 1];
    strcpy(fileNmArr, fileName.c_str());
    String payload = readLine(SD, fileNmArr);
    Serial.println(payload);
    int code = kirimData(payload);
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
  else if (!file)
  {
    checkSend = true;
  }
}
int countFiles(File folder)
{
  int count = 0;
  while (true) {
    File entry = folder.openNextFile();
    if (!entry) {
      // Tidak ada file lagi dalam folder
      break;
    }

    if (entry.isDirectory()) {
      // Jika file adalah folder, rekursif untuk menghitung jumlah file dalam folder
      count += countFiles(entry);
    } else {
      // Jika file adalah file, tambahkan satu ke jumlah total
      count++;
    }
    entry.close();
  }
  return count;
}

void cekDataFolder()
{
  File root = SD.open("/LOG");
  if (!root)
  {
    Serial.println("gagal buka folder");
    return;
  }
  int fileCount = countFiles(root);
  Serial.println("Isi Folder : " + String (fileCount) + " data");
  if (fileCount >= 5)
  {
    ESP.restart();
  }
  root.close();
}

void resetWatchdog() {
  watchdogMin++;
  if (watchdogMin >= watchdogTimer) {
    esp_cpu_reset(0);
    ESP.restart();
  }
}

DateTime stringToDateTime(String waktuString) {
  int tahun = waktuString.substring(0, 4).toInt();
  int bulan = waktuString.substring(5, 7).toInt();
  int hari = waktuString.substring(8, 10).toInt();
  int jam = waktuString.substring(11, 13).toInt();
  int menit = waktuString.substring(14, 16).toInt();
  int detik = waktuString.substring(17, 19).toInt();

  return DateTime(tahun, bulan, hari, jam, menit, detik);
}

String getDatetimeFromJSON(const char *jsonString) {
  // Parse the JSON string
  DynamicJsonDocument doc(256); // Adjust the size based on your JSON string
  DeserializationError error = deserializeJson(doc, jsonString);

  // Check for parsing errors
  if (error) {
    Serial.print(F("Error parsing JSON: "));
    Serial.println(error.c_str());
    return String(); // Return an empty string to indicate an error
  }

  // Check if the "datetime" key exists and is a string
  if (doc.containsKey("datetime") && doc["datetime"].is<String>()) {
    // Get the datetime value
    return doc["datetime"].as<String>();
  } else {
    Serial.println(F("Error: 'datetime' key not found or is not a string."));
    return String(); // Return an empty string to indicate an error
  }
}

void A01NYUB()
{
  if (Serial2.available()) {
    uint8_t c = Serial2.read();
    if (idx == 0 && c == 0xFF) {
      buffer[idx++] = c;
    }
    // Two middle bytes can be anything
    else if ((idx == 1) || (idx == 2)) {
      buffer[idx++] = c;
    }
    else if (idx == 3) {
      uint8_t sum = 0;
      sum = buffer[0] + buffer[1] + buffer[2];
      if (sum == c) {
        distance = ((uint16_t)buffer[1] << 8) | buffer[2];
        newData = true;
      }
      idx = 0;
    }
  }

  if (newData) {
    int jarak = (distance / 10);
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" mm");
    Serial.print(jarak);
    Serial.println(" cm");
    newData = false;
  }
}
