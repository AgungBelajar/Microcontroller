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
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <Wire.h>
#include <ArduinoJson.h>

//define selektor buka tutup
#define dir 33
#define pwmTutup 25
#define pwmBuka 26
#define pompa 27
int delKerja = 120000;
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
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 25200);
const int localTime = 7 * 3600;

bool test = false;
bool state = true;
bool checkSend = false;
bool state_pompa = true;
bool triggerPompa = true;

float sensorD = 320.47; // sensor ke dasar
float sensorN = 190.47; // sensor ke nol blok
float minimum = -60;
float maximum = -50;
float calIn = 57;//58//6
float calOut = 42;//188//24
int idwl = 79;
int delCheckData = 3600000;

String lastValue;
String jam;
String timeArr[10] = {"null", "null", "null", "null", "null", "null", "null", "null", "null", "null"};
String timeStr, timeNow = "";
int timer_cnt, log_interval, log_count;
long uptimeInt = 0;
long nyalaInt = 180;
long matiInt = 36;
int delLogging = 600000;
int del = 100000;
int delCekData = 3600000;
bool seq = false;
DateTime koreksiPHP;

String serverPath = "http://srs-ssms.com/post-wl-data.php";
String ssid = "Pompa_IoT";
String password = "asdf12345";

const int arIn = 10;
const int arOut = 10;
float avgInAr[arIn], avgOutAr[arOut] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float avgIn, avgOut = 0;
int log_time_unit = 2; // 5 menit
int test_time = 1;      // test time
unsigned long test_siklus = 5000;
unsigned long siklusPompa = 7200000;

// declare ultrasonic [trig, echo]
const int ultrasonic1[2] = {2, 4};
const int ultrasonic2[2] = {13, 12};
float distanceCmIn[10], distanceCmOut[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String distanceStrIn, distanceStrOut = "";
float distanceNowIn, distanceNowOut = 0;


void resetWatchdog();
void sendChecker();
void avgInFn(float nilaiInBaru);
void avgOutFn(float nilaiOutBaru);
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
  Serial2.begin(115200);
  watchdogTicker.attach(WATCHDOG_TIMEOUT, resetWatchdog);

  pinMode(dir, OUTPUT);
  pinMode(pwmBuka, OUTPUT);
  pinMode(pwmTutup, OUTPUT);
  pinMode(pompa, OUTPUT);
  digitalWrite(dir, LOW);
  digitalWrite(pwmBuka, LOW);
  digitalWrite(pwmTutup, LOW);
  digitalWrite(pompa, LOW);
  delay(1000);
  if (test)
  {
    siklusPompa = test_siklus;
    log_time_unit = test_time;
    del = 1000;
    delLogging = 30000;
    matiInt = 30;
    nyalaInt = 90;
    delCekData = 60000;
  }
  log_count = 0;
  log_interval = log_time_unit * 10;
  timer_cnt = log_interval + 1;

  //setup ultrasonic
  pinMode(ultrasonic1[0], OUTPUT); // Sets the trigPin as an Output
  pinMode(ultrasonic1[1], INPUT);  // Sets the echoPin as an Input
  pinMode(ultrasonic2[0], OUTPUT); // Sets the trigPin as an Output
  pinMode(ultrasonic2[1], INPUT);  // Sets the echoPin as an Input
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

  String statemsd_pompa = readLine(SD, "/statepompa.txt");
  if (statemsd_pompa == "true")
  {
    state_pompa = true;
    digitalWrite(pompa,HIGH);
  }
  else if (statemsd_pompa = "false")
  {
    state_pompa = false;
    digitalWrite(pompa,LOW);
  }
  else
  {
    Serial.println("state pompa fail");
  }
  Serial.println(state_pompa);
  //setup internet
  connectWifi();
  //  if (WiFi.status() == WL_CONNECTED)
  //  {
  //    rtc.adjust(DateTime(timeClient.getEpochTime()));
  //  }

  Serial.println("inisialisasi connect wifi");
  server.begin();

  int i = 1;
  while (i < arIn + 1)
  {
    distanceNowIn = getDistance(ultrasonic1, calIn);
    distanceNowOut = getDistance(ultrasonic2, calOut);
    avgInFn(distanceNowIn);
    avgOutFn(distanceNowOut);
    i++;
    String a = "pengukuran awal " + String(i) + " avgIn: " + String (avgIn) + " avgOut: " + String (avgOut) ;
    Serial.println(a);
    delay(200);
  }
  /*String jarak = readLine(SD, "/jarak.txt");
  if (jarak == "true")
  {
    distanceNowIn = 10;
    distanceNowOut= -10;
  }
  else if (jarak = "false")
  {
    distanceNowIn = -10;
    distanceNowOut= 10;
  }
  else
  {
    Serial.println("state pompa fail");
  }
  Serial.println("jarak: " + String(jarak));
  */
  createDir(SD, "/LOG");
  simpanData.every(delLogging, Logging);
  kirimDataMSD.every(4000, kirimDataMsd);
  checkData.every(delCekData, cekDataFolder);
  //  cekWaktu.every(7200000, cekJam); // cek RTC setiap 2 jam
  //  timeClient.begin();
  //  timeClient.update();

}

void loop() {
  simpanData.update();
  kirimDataMSD.update();
  checkData.update();
  //  cekWaktu.update();

// realtime ultrasonic
  distanceNowIn = getDistance(ultrasonic1, calIn);
  distanceNowOut = getDistance(ultrasonic2, calOut);
  avgInFn(distanceNowIn);
  avgOutFn(distanceNowOut);
  Serial.println("DistanceIn (cm): " + String(distanceNowIn) + "\nDistanceOut (cm) : " + String(distanceNowOut));
  Serial.println("Avg In : " + String(avgIn) + "\nAvg Out : " + String(avgOut));
  Serial.println("ukur realtime ultrasonik");
  Serial.println("state pintu : " + String(state) + "\tstate Pompa : " + String(state_pompa));
  Serial2.println(String(distanceNowIn) + "," + String(distanceNowOut));
  timeNow = getTime();
  Serial.println(jam);

  if (avgIn > avgOut + 2 )
  {
    digitalWrite(pompa, LOW);
    state_pompa = false;
    writeFile(SD, "/statepompa.txt", "false");
    if (avgIn > maximum && state == false)
    {
      digitalWrite(pwmBuka, HIGH);
      digitalWrite(dir, HIGH);
      Serial.println("BUKA");
      state = true;
      writeFile(SD, "/state.txt", "true");
      delay(delKerja);
      digitalWrite(pwmBuka, LOW);       
      digitalWrite(dir, LOW);    
      }
    else if (avgIn < minimum && state == true)
    {
      digitalWrite(pwmTutup, HIGH);
      Serial.println("TUTUP");
      state = false;
      writeFile(SD, "/state.txt", "false");
      delay(delKerja);
      digitalWrite(pwmTutup, LOW);
    }
    else
    {
      digitalWrite(pwmTutup, LOW);
      digitalWrite(pwmBuka, LOW);
      digitalWrite(dir, LOW);     
      Serial.println("TIDAK BERGERAK");
    }
  }
  else if (avgIn <= avgOut && state == true)
  {
    digitalWrite(pwmTutup, HIGH);
    Serial.println("TUTUP");
    state = false;
    writeFile(SD, "/state.txt", "false");
    delay(delKerja);
    digitalWrite(pwmTutup, LOW);
  }

  if (timeNow != "1970-1-1 00:00:00" and timer_cnt >= log_interval)
  {
    timer_cnt = 0;  // log_interval values are 10=15secs 40=1min 200=5mins 400=10mins 2400=1hr
    log_count += 1; // Increase loggin event count
    Serial.println("interrupt");
    if (avgIn <= avgOut)
    {
      if (avgIn > maximum)
      {
        Serial.println("pompa Nyala");
        digitalWrite(pompa, HIGH);
        state_pompa = true;
        writeFile(SD, "/statepompa.txt", "true");
        long currentMillis = millis();
        if (pompa == HIGH)
        {
          digitalWrite(pompa, HIGH);
          writeFile(SD, "/statepompa.txt", "true");
          uptimeInt = (currentMillis / del) + nyalaInt;
          state_pompa = true;
        }
        if ((currentMillis / del) > uptimeInt && !seq)
        {
          seq = true;
          uptimeInt = (currentMillis / del) + matiInt;
          digitalWrite(pompa, LOW);
          writeFile(SD, "/statepompa.txt", "false");
          state_pompa = false;
        }
        else if ((currentMillis / del) > uptimeInt && seq)
        {
          seq = false;
          uptimeInt = (currentMillis / del) + nyalaInt;
          digitalWrite(pompa, HIGH);
          state_pompa = true;
          writeFile(SD, "/statepompa.txt", "true");
        }
        else if (pompa == HIGH && avgIn < minimum)
        {
          digitalWrite(pompa, LOW);
          state_pompa = false;
          writeFile(SD, "/statepompa.txt", "false");
          // mati
          Serial.println("pompa mati");
        }
      }
    }

  }
  timer_cnt += 1; // Readings set by value of log_interval each 40 = 1min*/
  delay(500);
  watchdogMin = 0;
}

void sendChecker() {
  if (checkSend == false) {
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
  digitalWrite(ultrasonic[0], HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonic[0], LOW);

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

  String appendPayload = "lvl_in=" + String(avgIn) + "&lvl_out=" + String(avgOut) + "&d=" + String(timeArr[0]) + "&idwl=" + idwl;
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

void cekJam()
{
  timeClient.update();
  rtc.adjust(DateTime(timeClient.getEpochTime()));
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
