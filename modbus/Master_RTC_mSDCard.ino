#include "FS.h"
#include "SD.h"
#include "SPI.h"
const char* printWord;

#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include <Wire.h>
#define SLAVE_ADDR 9
#define ANSWERSIZE 5
String response = "";

void Rtc();
void master();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
String readLine(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void deleteFile(fs::FS &fs, const char * path);
struct LinesAndPositions FindLinesAndPositions(char *filename);
void CopyFiles(char *ToFile, char *FromFile);
void DeleteLineFromFile(char *filename, int Line);
void DeleteSelectedLinesFromFile(char *filename, char *StrOfLines);
void DeleteSelectedLinesFromFile(char *filename, char *StrOfLines);
 
void setup () {
  Wire.begin();
  Serial.begin(115200);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
   if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  createDir(SD, "/DIR Test");
}


void loop() {
  // put your main code here, to run repeatedly:
Rtc();
master();
String cetak = response;
int fileNmLg = cetak.length();
char fileNmArr[fileNmLg + 1];
strcpy(fileNmArr,cetak.c_str());
writeFile(SD,fileNmArr,fileNmArr);
delay(3000);
}

void Rtc()
{
  DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

void master()
{
  Serial.println("Write data to slave");
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(0);
  Wire.endTransmission(); 
  Serial.println("Receive data");
  
  Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  

  while (Wire.available()) {
      char b = Wire.read();
      response += b;
  } 
  Serial.println(response);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
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
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.println(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
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
