#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD.h"
#include "SPI.h"
#include <HTTPClient.h>
#include <WiFiMulti.h> // Built-in
#include <string>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include "BluetoothSerial.h"

//float Sensor;
//String SensorStr = "";
bool debugMode = true;
String jam;
const int arIn = 10;
float avgInAr[arIn]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float avgIn = 0;

const int arKelembaban = 10;
float avgKelembabanAr[arKelembaban] = {0,0,0,0,0,0,0,0,0,0};
float avgKelembaban = 0;

const int arSuhu = 10;
float avgSuhuAr[arSuhu] = {0,0,0,0,0,0,0,0,0,0};
float avgSuhu = 0;

int idsm = 1;
int log_time_unit = 1; // 5 menit
int test_time = 1;      // test time

// setting id water level dan php
String serverPath = "http://srs-ssms.com/post-sm-data.php";

// password wifi1 default
const char *ssid =  "HUAWEI-9D50";
const char *password = "79AH10J9LAG";
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

//Kelembaban
#define AOUT_PIN 35 // Tiang 2
float kelembabanTanah; //Tiang 2
String kelembabanStr = "";
float kelembabanArr[10] = {0,0,0,0,0,0,0,0,0,0};

#define BOUT_PIN 34 // Tiang 2
float kelembabanTanah1; //Tiang 2
String kelembabanStr1 = "";
float kelembabanArr1[10] = {0,0,0,0,0,0,0,0,0,0};

//Suhu
const int oneWireBus = 4;  
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
float temperatureC;
//bluetooth
//#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
//#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
//#endif
//char incomingChar;
//int calIn=0;
//String message = "";
//BluetoothSerial SerialBT;

void avgSuhuFn(float nilaiSuhuBaru);
void avgInFn(float nilaiInBaru);
void avgKelembabanFn(float nilaiKelembabanBaru);
void debug(String noDebug);
void connectToWifi();
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
  // set awal interrupt
  log_count = 0;
  log_interval = log_time_unit * 10;
  timer_cnt = log_interval + 1;

  Serial.begin(9600);  // Starts the serial communication
//  SerialBT.begin("Moisture Soil"); //Bluetooth device name
//  Serial.println("The device started, now you can pair it with bluetooth!");
  sensors.begin();
  // set awal jam
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
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
  writeFile(SPIFFS,"/wifi.txt", ssid);
  writeFile(SPIFFS,"/pass.txt", password);
  ssidStr = readLine(SPIFFS, "/wifi.txt"); // todo
  pwdStr = readLine(SPIFFS, "/pass.txt");  // todo
  debug("inisalisasi password");

  // Connect to Wi-Fi
  connectToWifi();
  debug("inisalisasi konek wifi");

  // Start server
  server.begin();
  debug("inisalisasi server");
  

  createDir(SD, "/LOG");
}

void loop()
{
  int value = analogRead(AOUT_PIN); // read the analog value from sensor
  kelembabanTanah = (0.109*value)+0.575; //(100-((value/1023.00)*100));
  avgInFn(kelembabanTanah);
  debug("kelembabanTanah : " +String(kelembabanTanah));
  debug("Avg kelembaban : " + String(avgIn));
 
  int value1 = analogRead(BOUT_PIN); // read the analog value from sensor
  kelembabanTanah1 = (0.109*value)+0.575; //(100-((value/1023.00)*100));
  avgKelembabanFn(kelembabanTanah1);
  debug("kelembabanTanah : " +String(kelembabanTanah1));
  debug("Avg kelembaban : " + String(avgKelembaban));
  
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  avgSuhuFn(temperatureC);
  debug("Suhu : " + String(temperatureC) + "ºC");
  debug("Avg Suhu : " + String(temperatureC));
  timeNow = getTime();
  debug("dapatkan waktu");
  
  // implementasi interrupt
  if (timeNow != "1970-1-1 00:00:00" and timer_cnt >= log_interval)
  {
    timer_cnt = 0;  // log_interval values are 10=15secs 40=1min 200=5mins 400=10mins 2400=1hr
    log_count += 1; // Increase loggin event count
    debug("interrupt");

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

void avgKelembabanFn(float nilaiKelembabanBaru)
{
  float totalKelembaban = 0;
  for (int i = arKelembaban - 1; i > 0; i--)
  {
    avgKelembabanAr[i] = avgKelembabanAr[i - 1];
    totalKelembaban += avgKelembabanAr[i];
  }
  avgKelembabanAr[0] = nilaiKelembabanBaru;
  totalKelembaban += nilaiKelembabanBaru;
  avgKelembaban = totalKelembaban / arKelembaban;
}

void avgSuhuFn(float nilaiSuhuBaru)
{
  float totalSuhu = 0;
  for (int i = arSuhu - 1; i > 0; i--)
  {
    avgSuhuAr[i] = avgSuhuAr[i - 1];
    totalSuhu += avgSuhuAr[i];
  }
  avgSuhuAr[0] = nilaiSuhuBaru;
  totalSuhu += nilaiSuhuBaru;
  avgSuhu = totalSuhu / arSuhu;
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
    kelembabanArr1[j] = kelembabanArr1[j - 1];
    kelembabanArr[j] = kelembabanArr[j - 1];
    timeArr[j] = timeArr[j - 1];
  }
  kelembabanArr1[0] = kelembabanTanah1;
  kelembabanArr[0] = kelembabanTanah;
  timeArr[0] = getTime();

  // geser array nilai string
  kelembabanStr1 = "[";
  kelembabanStr = "[";
  timeStr = "[";
  for (int i = 0; i < 10; i++)
  {
    kelembabanStr1 += kelembabanArr1[i];
    kelembabanStr += kelembabanArr[i];
    timeStr += timeArr[i];
    if (i < 9)
    {
      kelembabanStr1 += ", ";
      kelembabanStr += ", ";
      timeStr += ",";
    }
  }
  kelembabanStr1 += "]";
  kelembabanStr += "]";
  timeStr += "]";

  // Prints the distance in the Serial Monitor
  Serial.println("Humidity 1: " + String(kelembabanArr[0]));
  Serial.println("Humidity1 Array: " + String(kelembabanStr));
  Serial.println("Humidity 2: " + String(kelembabanArr1[0]));
  Serial.println("Humidity2 Array: " + String(kelembabanStr1));
  
  Serial.println("Time: " + String(timeArr[0]));
  Serial.println("Time Array: " + String(timeStr));

  if (wifiMulti.run() == WL_CONNECT_FAILED)
  {
    connectToWifi();
  }

  String payload = "hum1= " + String(avgIn) + " &hum2= " + String(avgKelembaban) + " &temp= " + String(avgSuhu) + " &datetime= " + String(timeArr[0]);
  String appendPayload = payload + " &idsm=" + String(idsm);
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

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
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

void readFile(fs::FS &fs, const char *path)
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

String readLine(fs::FS &fs, const char *path)
{
  String line = "";
  Serial.printf("Reading line: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    ESP.restart();
    return line;
  }

  Serial.println("- read from line:");
  line = file.readStringUntil('\n');
  Serial.println(line);

  file.close();
  return line;
}

void createDir(fs::FS &fs, const char *path)
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

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    ESP.restart();
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
    ESP.restart();
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
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

void deleteFile(fs::FS &fs, const char *path)
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
  String isi = postRequest ;//+ "&idsm=" + idsm;
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
