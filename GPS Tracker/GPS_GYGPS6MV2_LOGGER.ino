#include <TinyGPS.h>
#include <RTClib.h>
#include <SD.h>

#define gpsSerial Serial2
RTC_DS3231 rtc;
File myFile;

float lat = 1.111111 , lon = 2.222222; // create variable for latitude and longitude object
TinyGPS gps; // create gps object

void setup() {
  Serial.begin(115200); // connect serial
  gpsSerial.begin(9600); // connect gps sensor
  if (!SD.begin(5))
  {
    Serial.println("Card failed, or not present");
  }
  else
  {
    Serial.println("Card initialized successfully");
  }
  
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
  Serial.println("inisialisasi RTC");
}

void loop() {
  String timenow = getTime();
  Serial.println(timenow);
  
  while (gpsSerial.available()) { // check for gps data
    if (gps.encode(gpsSerial.read())) // encode gps data
    {
      gps.f_get_position(&lat, &lon); // get latitude and longitude
      // display position
      Serial.print("Position: ");
      Serial.print("Latitude:");
      Serial.print(lat, 6);
      Serial.print(";");
      Serial.print("Longitude:");
      Serial.println(lon, 6);    
    }
  }
  String latitude = String(lat, 6);
  String longitude = String(lon, 6);
  String loc = "Latitude = " + String(latitude) + "; " + "Longitude = " + String(longitude);
  Serial.println(loc);
  String payload = timenow + loc;
  appendFile(SD, "/data.txt", payload);
  delay(1000);
}

String getTime()
{
  DateTime now = rtc.now();
  String timeS = String(now.year(), DEC) + "-" + String(now.month(), DEC) + "-" + String(now.day(), DEC) + " " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  return timeS;
}

void appendFile(fs::FS &fs, const char *path, String message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.println(message))
  {
    Serial.println("- message appended");

    // getDataSD(path);
  }
  else
  {
    Serial.println("- append failed");
  }
  file.close();
}
