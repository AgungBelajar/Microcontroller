#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <SD.h>
#include <SPI.h>
#include <RTClib.h>
#include <Timer.h>

#define WATCHDOG_TIMEOUT 1200
// define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

WebServer server(80);
Timer checkLoop, simpanData, kirimDataMSD;
Ticker watchdogTicker;
RTC_DS3231 rtc;

bool test = true;
bool checkSend = false;
bool debugMode = true;
bool pumpMode = false;
bool pumpModeOut = false;
bool triggerPompa = true;
bool starterGenset = false;
bool seq = false;

const int pompaCons = 4;
const int waterPump[pompaCons] = {2, 3, 4, 5};
const int pompaConsOut = 2;
const int waterPumpOut[pompaConsOut] = {6, 7};
const int gensetOn = 22;
const int gensetStart = 23;

float sensorD = 320.47; // sensor ke dasar
float sensorN = 190.47; // sensor ke nol blok
float minimum = -60;
float maximum = -50;
float calIn = 90;
float calOut = 79;
int idwl = 22;

String jam;
String timeArr[10] = {"null", "null", "null", "null", "null", "null", "null", "null", "null", "null"};
String timeStr, timeNow = "";
int timer_cnt, log_interval, log_count;
long uptimeInt = 0;
long nyalaInt = 180;
long matiInt = 36;


String serverPath = "http://srs-ssms.com/post-wl-data.php";
String ssid = "Pompa_IoT";
String password = "asdf12345";

const int arIn = 10;
const int arOut = 10;
float avgInAr[arIn], avgOutAr[arOut] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float avgIn, avgOut = 0;
int log_time_unit = 50; // 5 menit
int test_time = 1;      // test time
unsigned long test_siklus = 5000;
unsigned long siklusPompa = 7200000;
int delLogging = 600000;
int del = 100000;
int delRelay = 30000;
int delGenset = 60000;


// declare ultrasonic [trig, echo]
const int ultrasonic1[2] = {2, 4};
const int ultrasonic2[2] = {13, 12};
float distanceCmIn[10], distanceCmOut[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String distanceStrIn, distanceStrOut = "";
float distanceNowIn, distanceNowOut = 0;


void resetWatchdog()
{
  esp_cpu_reset(0);
  ESP.restart();
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
  Serial.println("Gagal");
  String fileNm = "/LOG/" + jam + ".txt";
  int fileNmLg = fileNm.length();
  char fileNmArr[fileNmLg + 1];
  strcpy(fileNmArr, fileNm.c_str());
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
  Serial.println(String(httpCode) + " - " + payload); // Print HTTP return code
  http.end();
  Serial.println("http code:" + String(httpCode));
  if (httpCode == 200)
    success = 1;
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
    delay(500);
    Serial.print(".");
    i++;
    if (i >= 15)
    {
      Serial.println("");
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

void pumpTrig(bool kondisi)
{
  if (kondisi)
  {
    relayTrig(gensetOn, true);
    delay(5000);
    if (!starterGenset)
    {
      relayTrig(gensetStart, true);
      delay(1000);
      relayTrig(gensetStart, false);
      starterGenset = true;
      delay(delGenset);
    }
    for (int i = 0; i < pompaCons; i++)
    {
      delay(delRelay);
      relayTrig(waterPump[i], true);
    }
  }
  else
  {
    deactivate();
  }
}
void relayTrig(int pin, bool kondisi)
{
  if (kondisi)
  {
    Serial.println("Trigger nyala pin=" + String(pin));
    Serial2.println(pin);
  }
  else
  {
    int pinMati = pin * -1;
    Serial.println("Trigger mati pin=" + String(pinMati));
    Serial2.println(pinMati);
  }
}

void deactivate()
{
  for (int i = 0; i < pompaCons; i++)
  {
    relayTrig(waterPump[i], false);
    delay(5000);
  }
  starterGenset = false;
  relayTrig(gensetOn, false);
  // mati
  Serial.println("pompa mati");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(1000);
  if (test)
  {
    siklusPompa = test_siklus;
    log_time_unit = test_time;
    delLogging = 30000;
    nyalaInt = 90;
    matiInt = 30;
    del = 1000;
    delGenset = 5000;
    delRelay = 1000;
  }
  log_count = 0;
  log_interval = log_time_unit * 10;
  timer_cnt = log_interval + 1;

  //  watchdogTicker.attach(WATCHDOG_TIMEOUT, resetWatchdog);
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
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
    distanceNowOut = getDistance(ultrasonic2, calOut);
    avgInFn(distanceNowIn);
    avgOutFn(distanceNowOut);
    i++;
    String a = "pengukuran awal " + String(i);
    Serial.println(a);
    delay(200);
  }
  createDir(SD, "/LOG");
  checkLoop.every(300000, sendChecker);
  simpanData.every(delLogging, Logging);
  kirimDataMSD.every(5000, kirimDataMsd);

}

void loop() {
  //  checkLoop.update();
  simpanData.update();
  kirimDataMSD.update();

  // realtime ultrasonic
  distanceNowIn = getDistance(ultrasonic1, calIn);
  distanceNowOut = getDistance(ultrasonic2, calOut);
  avgInFn(distanceNowIn);
  avgOutFn(distanceNowOut);
  Serial.println("DistanceIn (cm): " + String(distanceNowIn) + "\nDistanceOut (cm) : " + String(distanceNowOut));
  Serial.println("Avg In : " + String(avgIn) + "\nAvg Out : " + String(avgOut));
  Serial.println("ukur realtime ultrasonik");
  timeNow = getTime();
  Serial.println(jam);

  if (timeNow != "1970-1-1 00:00:00" and timer_cnt >= log_interval)
  {
    timer_cnt = 0;  // log_interval values are 10=15secs 40=1min 200=5mins 400=10mins 2400=1hr
    log_count += 1; // Increase loggin event count
    Serial.println("interrupt");
    if (avgIn <= avgOut)
    {
      if (avgIn >= maximum)
        Serial.println("pompa nyala");
      // nyala
      long currentMillis = millis();
      if (!pumpMode)
      {
        pumpTrig(true);
        uptimeInt = (currentMillis / del) + nyalaInt;
        pumpMode = true;
      }
      if ((currentMillis / del) > uptimeInt && !seq)
      {
        seq = true;
        uptimeInt = (currentMillis / del) + matiInt;
        pumpTrig(false);
      }
      else if ((currentMillis / del) > uptimeInt && seq)
      {
        seq = false;
        uptimeInt = (currentMillis / del) + nyalaInt;
        pumpTrig(true);
      }
    }
    else if (pumpMode == true && avgIn < minimum)
    {
      pumpTrig(false);
      // mati
      pumpMode = false;
      Serial.println("pompa mati");
    }
  }
  timer_cnt += 1; // Readings set by value of log_interval each 40 = 1min
  delay(500);
}
