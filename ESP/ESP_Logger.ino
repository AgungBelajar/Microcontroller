#include <Wire.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <HTTPClient.h>

//buat rata rata data
const int numReadings = 10;   //berapa banyak data yang mau di rata2
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int jedarata2 = numReadings*1000;
int jedakirim = (1000*60*60) - jedarata2;


//kirim data ke Website
const char *ssid = "WaterLevelPLE";  //Nama Wifi
const char *password = "qwerty12345"; // pass wifi
//int idwl = 1; //Idwl untuk test
int idwl = 77; //Idwl untuk 78 PLE; 77 office
String serverPath = "http://srs-ssms.com/post-wl-data.php";
String timeArr , postData, postDataid;
const char* postData1;
int avgIn, jarak;

//microSD
File myFile;
const int I2C_ADDRESS = 0x68;
const char* AMPM = 0;
 

//RTC
const char* days[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
const char* months[] = {"01", "02", "03", "04", "05", "06", "07", "08","09", "10", "11", "12"};
byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 0;
byte monthday = 0;
byte month = 0;
byte year = 0;

//Sonar
#if defined(__AVR__) || defined(ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, -1);
#else
#define mySerial Serial2
#endif
#define CONTROL_PIN 5   // This is the YELLOW wire, can be any data line
int16_t distance;  // The last measured distance
bool newData = false; // Whether new data is available from the sensor
uint8_t buffer[4];  // our buffer for storing data
uint8_t idx = 0;  // our idx into the storage buffer

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void writeFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path);
void appendFile(fs::FS &fs, const char * path, const char * message);
void loop_sonar();
void membacaWaktu(); //Print data RTC
void averageData();
void kirim_data();

void setup() {
  //setup kirim File ke Server
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting");
  int i=0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print(".");
    i++;
    if(i>10)
    {
     Serial.println(".");
     break;
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  }
  //setup RTC
  Wire.begin(21, 22);
//  mengaturWaktu();

  //setup Sonar
  Serial.begin(115200);
  while (!Serial) 
  { delay(10);} // wait for serial port to connect. Needed for native USB port only
  mySerial.begin(9600);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

  //setup SD Card
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;}
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC)
  { Serial.println("MMC"); } 
  else if(cardType == CARD_SD)
  { Serial.println("SDSC"); } 
  else if(cardType == CARD_SDHC)
  {Serial.println("SDHC"); } 
  else 
  { Serial.println("UNKNOWN");  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);
//  createDir(SD, "/mydir");
//  listDir(SD, "/", 0);
//  removeDir(SD, "/mydir");
//  listDir(SD, "/", 2);
//  writeFile(SD, "/Data Water Level.txt", " ");
  appendFile(SD, "/Data Water Level.txt", "");
  readFile(SD, "/Data Water Level.txt");
//  deleteFile(SD, "/foo.txt");
//  renameFile(SD, "/hello.txt", "/foo.txt");
//  readFile(SD, "/foo.txt");
//  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  
  //setup rata rata
  for (int thisReading = 0; thisReading < numReadings; thisReading++) 
  { readings[thisReading] = 0; }
  }

void loop()
{
  //loop sonar
  loop_sonar();
  membacaWaktu(); //Print data RTC
  if (newData) 
  {
    int jarak = (distance/10)-200;
    String timeArr = "20"+String(year)+"-"+String(months[month-1])+"-"+String(monthday)+" "+
                   String(hour , DEC)+":"+String(minute , DEC)+":"+String(second , DEC);
    String postData = "lvl_in= " + String(jarak) + "; &d=" + timeArr;
    String postDataid = String(postData) + "&idwl=" + String(idwl);
    postData1 = postData.c_str();
    newData = false;
    if (readIndex >= numReadings) 
    {
      kirim_data();
      appendFile(SD, "/Data Water Level.txt", postData1);
      readIndex = 0;
      delay(jedakirim);
    } 
    else
   {    averageData();
        Serial.print("baca data : ke-"); 
        Serial.print(readIndex);
        Serial.print("   distance : "); 
        Serial.print(distance);
        Serial.print("   average : "); 
        Serial.print(average);
        Serial.print("  Tinggi air : "); 
        Serial.println(jarak);
   }    
// delay(1000);
  }
}
 
//void rata - ratain data
void averageData()
{
  int jarak = (distance/10)- 1270;
  total = total - readings[readIndex];
  readings[readIndex] = jarak;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  average = total / numReadings;
  int avgIn = average;
}

//void kirim data ke Server
void kirim_data() {
  String timeArr = "20"+String(year)+"-"+String(months[month-1])+"-"+String(monthday)+" "+
                   String(hour , DEC)+":"+String(minute , DEC)+":"+String(second , DEC);
  String postData = "lvl_in= " + String(jarak) + "; &d=" + timeArr;
  String postDataid = String(postData) + "&idwl=" + String(idwl);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverPath);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  auto httpCode = http.POST(postDataid);
  String payload = http.getString();
  Serial.println(postDataid);
  Serial.println(payload);
  http.end();
}

//void sonar
void loop_sonar() { // run over and over
  if (mySerial.available()) {
    uint8_t c = mySerial.read();
    if (idx == 0 && c == 0xFF) 
    { buffer[idx++] = c;  }
    else if ((idx == 1) || (idx == 2)) 
    { buffer[idx++] = c; }
    else if (idx == 3) {
      uint8_t sum = 0;
      sum = buffer[0] + buffer[1] + buffer[2];
      if (sum == c) 
      { distance = ((uint16_t)buffer[1] << 8) | buffer[2];
        newData = true;}
      idx = 0;}
   }
}
// void RTC
void printTime() {
  char buffer[3];
  const char* AMPM = 0;
  membacaWaktu();
  Serial.print("20");
  Serial.print(year);
  Serial.print("-");
  Serial.print(months[month-1]);
  Serial.print("-");
  Serial.print(monthday);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  sprintf(buffer, "%02d", minute);
  Serial.print(buffer);
  Serial.print(":");
  sprintf(buffer, "%02d", second);
  Serial.print(buffer);
  Serial.println(""); }
  
void membacaWaktu() {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDRESS, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}

void mengaturWaktu() {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(byte(0));
  Wire.write(decToBcd(58)); // second
  Wire.write(decToBcd(3)); // minute
  Wire.write(decToBcd(16)); // hour
  Wire.write(decToBcd(4));  // weekday
  Wire.write(decToBcd(10)); // date
  Wire.write(decToBcd(8));  // month
  Wire.write(decToBcd(22)); // year
  Wire.write(byte(0));
  Wire.endTransmission();
}

byte decToBcd(byte val) 
{ return ((val/10*16) + (val%10));}
byte bcdToDec(byte val) 
{ return ((val/16*10) + (val%16));}

//print di SDCARD
void tulis_SDcard()
{
    myFile.print("jarak : ");
    myFile.print(distance);
    myFile.print(" mm");
    myFile.print(" ; waktu : ");
    myFile.print(days[weekday-1]);
    myFile.print(" ");
    myFile.print(monthday);
    myFile.print(" ");
    myFile.print(months[month-1]);
    myFile.print(" 20");
    myFile.print(year);
    myFile.print(" ");
  if (hour > 12) {
    hour -= 12;
    AMPM = " PM";}
  else AMPM = " AM";
  myFile.print(hour);
  myFile.print(":");
  myFile.print(minute);
  myFile.print(":");
  myFile.print(second);
  myFile.print(" ");
  myFile.println(AMPM);
} 
//void MicroSD
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return; }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels)
      {  listDir(fs, file.name(), levels -1);}
    } 
    else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size()); }
    file = root.openNextFile();
   }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path))
  { Serial.println("Dir created"); } 
    else 
    { Serial.println("mkdir failed");}
  }

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path))
  {Serial.println("Dir removed");} 
  else 
  { Serial.println("rmdir failed"); }
  }

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;  }
  Serial.print("Read from file: ");
  while(file.available())
  { Serial.write(file.read()); }
  file.close();
  }

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return; }
  if(file.print(message))
  { Serial.println("File written"); } 
  else 
  { Serial.println("Write failed");}
  file.close();
  }

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;  }
  if(file.println(message))
  {Serial.println("Message appended");} 
  else 
  {Serial.println("Append failed");}
  file.close();
  }

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) 
  {  Serial.println("File renamed");} 
  else 
  {  Serial.println("Rename failed");}
  }

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path))
  {  Serial.println("File deleted");} 
  else 
  { Serial.println("Delete failed");}
 }

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
   }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();} 
    else 
    {Serial.println("Failed to open file for reading");}

  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return; }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++)
  { file.write(buf, 512); }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}
