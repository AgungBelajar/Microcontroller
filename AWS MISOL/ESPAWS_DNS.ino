#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
ESP8266WebServer server(80);
int csPin = 0;
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
WebServer server(80);
int csPin = 5;
#endif

#include <SD.h>
#include <RTClib.h>
#include <Timer.h>

String ssid = "Meeting SRS";
String password = "SRS2022#";
int delayMill = 2000;

File myFile;

int id, testLoop = 0;

Timer sendTimer;

IPAddress staticIP(10, 9, 159, 185);
IPAddress gateway(10, 9, 159, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsServer(192, 168, 1, 22);

void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP8266!");
}

void handlePost() {
  if (server.method() == HTTP_POST) {
    String dateutc, baromabsin, windgustmph, baromrelin, date_jkt, solarradiation, windgustkmh, windspeedkmh, windspeedmph, winddir, rainratein, tempf, tempinf, humidityin, uv, humidity, dateStr = "";
    float temp_in, temp_out = 0;
    DateTime datetimeNow, datetimePlus;

    dateutc = server.arg("dateutc");
    //Serial.println("DATEUTC: " + dateutc);

    uint16_t year = 0;
    uint8_t month, day, hour, minute, second = 0;
    // Parse the input date-time string
    year = dateutc.substring(0, 4).toInt();
    month = dateutc.substring(5, 7).toInt();
    day = dateutc.substring(8, 10).toInt();
    hour = dateutc.substring(11, 13).toInt();
    minute = dateutc.substring(14, 16).toInt();
    second = dateutc.substring(17, 19).toInt();

    String substr = "substr: " + String(year) + "," + String(month) + "," + String(day) + "," + String(hour) + "," + String(minute) + "," + String(second);
    //Serial.println(substr);
    // Create a DateTime object with the parsed values and set the timezone to UTC
    // datetimeNow = DateTime(year, month, day, hour, minute, second); //salah

    // dateStr = datetimeNow.toString("YYYY-MM-DD hh:mm:ss");
    // Serial.println("DATEstr: " + dateStr);

    // // Convert the datetime to Jakarta timezone and format the output
    // datetimePlus = datetimeNow + TimeSpan(0, 7, 0, 0); // Add 7 hours to convert to Jakarta timezone
    // date_jkt = datetimePlus.toString("YYYY-MM-DD hh:mm:ss");

    // Add 7 hours
    hour += 7;

    // Adjust the date and time values if necessary
    if (hour >= 24) {
      hour -= 24;
      day += 1;
    }
    if (month == 2 && day > 28) {  // Adjust for February having 28 days
      day = 1;
      month += 1;
    } else if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {  // Adjust for months with 30 days
      day = 1;
      month += 1;
    } else if (day > 31) {  // Adjust for months with 31 days
      day = 1;
      month += 1;
    }
    if (month > 12) {  // Adjust for changing years
      month = 1;
      year += 1;
    }

    String month_str = zeroDate(month);
    String day_str = zeroDate(day);
    String hour_str = zeroDate(hour);
    String minute_str = zeroDate(minute);
    String second_str = zeroDate(second);

    // Construct the adjusted date string
    String new_date_string = String(year) + "-" + String(month_str) + "-" + String(day_str) + " " + String(hour_str) + ":" + String(minute_str) + ":" + String(second_str);
    // Print the adjusted date string
    //Serial.println(new_date_string);

    tempinf = server.arg("tempinf");
    tempf = server.arg("tempf");
    windspeedmph = server.arg("windspeedmph");
    windgustmph = server.arg("windgustmph");

    windgustmph = windgustmph.toFloat() * 1.60934;
    windspeedkmh = windspeedmph.toFloat() * 1.60934;
    winddir = server.arg("winddir");
    rainratein = server.arg("rainratein");
    temp_in = (5.0 / 9.0) * (tempinf.toFloat() - 32.0);
    humidityin = server.arg("humidityin");
    uv = server.arg("uv");
    temp_out = (5.0 / 9.0) * (tempf.toFloat() - 32.0);
    humidity = server.arg("humidity");
    solarradiation = server.arg("solarradiation");
    baromrelin = server.arg("baromrelin");
    baromabsin = server.arg("baromabsin");

    String data = new_date_string + "," + windspeedkmh + "," + winddir + "," + rainratein + "," + temp_in + "," + temp_out + "," + humidityin + "," + humidity + "," + uv + "," + windgustmph + "," + baromrelin + "," + baromabsin + "," + solarradiation;

    //Serial.println("data awal bos: " + data);
    //Serial.println(".");

    myFile = SD.open("/data.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(data);
    }
    myFile.close();
    server.send(200, "text/plain", "Data saved to SD card.");
    digitalWrite(LED_BUILTIN, LOW);   // Arduino: turn the LED on (HIGH)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // Arduino: turn the LED off (LOW)
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);   // Arduino: turn the LED on (HIGH)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // Arduino: turn the LED off (LOW)
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);   // Arduino: turn the LED on (HIGH)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // Arduino: turn the LED off (LOW)
  }
}

String zeroDate(int zero) {
  String month_str = String(zero);
  if (month_str.length() == 1) {
    month_str = "0" + month_str;
  }
  return month_str;
}

void setup() {
  Serial.begin(115200);

  // Start the SD card
  if (!SD.begin(csPin)) {
    Serial.println("Card failed, or not present");
    //    return;
  }
  Serial.println("Card initialized successfully");

  File myFile;                   // create a file object
  myFile = SD.open("/ssid.txt");  // open the file
  if (myFile) {
    ssid = myFile.readStringUntil('\n');
    Serial.println("ssid = " + String(ssid));
    myFile.close();  // close the file
  } else {
    Serial.println("Error opening file!");
  }

  myFile = SD.open("/pass.txt");  // open the file
  if (myFile) {
    password = myFile.readStringUntil('\n');  // read the first line of the file
    Serial.println("pass = " + String(password));    
    myFile.close();                           // close the file
  } else {
    Serial.println("Error opening file!");
  }

  myFile = SD.open("/id.txt");  // open the file
  if (myFile) {
    id = myFile.readStringUntil('\n').toInt();  // read the first line of the file
    Serial.println("id = " + String(id));
    myFile.close();                             // close the file
  } else {
    Serial.println("Error opening file!");
  }

  myFile = SD.open("/ip.txt");  // open the file
  if (myFile) {
    String ipString = myFile.readStringUntil('\n');  // read the first line of the file
    Serial.println("ip = " + String(ipString));
    myFile.close();                                  // close the file
    char charBuf[ipString.length() + 1];
    ipString.toCharArray(charBuf, ipString.length() + 1);
    // Tokenize the char array using "."
    char *token = strtok(charBuf, ".");
    int ipParts[4];
    int i = 0;
    while (token != NULL) {
      // Convert token to integer and store in array
      ipParts[i++] = atoi(token);
      token = strtok(NULL, ".");
    }
    // Create IP address object
    staticIP = IPAddress(ipParts[0], ipParts[1], ipParts[2], ipParts[3]);
  } else {
    Serial.println("Error opening file!");
  }

  myFile = SD.open("/gateway.txt");  // open the file
  if (myFile) {
    String ipString = myFile.readStringUntil('\n');  // read the first line of the file
    myFile.close();                                  // close the file
    char charBuf[ipString.length() + 1];
    ipString.toCharArray(charBuf, ipString.length() + 1);
    // Tokenize the char array using "."
    char *token = strtok(charBuf, ".");
    int ipParts[4];
    int i = 0;
    while (token != NULL) {
      // Convert token to integer and store in array
      ipParts[i++] = atoi(token);
      token = strtok(NULL, ".");
    }
    // Create IP address object
    gateway = IPAddress(ipParts[0], ipParts[1], ipParts[2], ipParts[3]);
    Serial.println("gateway = " + String(gateway));
  } else {
    Serial.println("Error opening file!");
  }

  myFile = SD.open("/subnet.txt");  // open the file
  if (myFile) {
    String ipString = myFile.readStringUntil('\n');  // read the first line of the file
    myFile.close();                                  // close the file
    char charBuf[ipString.length() + 1];
    ipString.toCharArray(charBuf, ipString.length() + 1);
    // Tokenize the char array using "."
    char *token = strtok(charBuf, ".");
    int ipParts[4];
    int i = 0;
    while (token != NULL) {
      // Convert token to integer and store in array
      ipParts[i++] = atoi(token);
      token = strtok(NULL, ".");
    }
    // Create IP address object
    subnet = IPAddress(ipParts[0], ipParts[1], ipParts[2], ipParts[3]);
    Serial.println("subnet = " + String(subnet));
  } else {
    Serial.println("Error opening file!");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.println();


  connectWiFi();


  // Initialize the server
  server.on("/", handleRoot);
  server.on("/post", handlePost);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Arduino: turn the LED on (HIGH)

  sendTimer.every(delayMill, sendData);
}

void sendData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    connectWiFi();
  }

  String rawText;
  myFile = SD.open("/data.txt");  // open the file
  if (myFile) {
    rawText = myFile.readStringUntil('\n');  // read the first line of the file
    myFile.close();                          // close the file
    if (rawText != "") {
      String values[13];  // an array to store the values
      int index = 0;
      while (rawText.length() > 0) {                // loop until there are no more values
        int separatorIndex = rawText.indexOf(",");  // find the index of the next comma
        if (separatorIndex == -1) {                 // if there are no more commas, use the entire remaining text as the last value
          values[index++] = rawText;
          break;
        }
        values[index++] = rawText.substring(0, separatorIndex);  // extract the value before the comma
        rawText = rawText.substring(separatorIndex + 1);         // remove the value and the comma from the text
      }

      WiFiClient client;
      HTTPClient http;

      // Prepare the data
      String data = "{\"idws\": " + String(id) + ", \"date\": \"" + String(values[0]) + "\", \"windspeedkmh\": " + String(values[1]) + ", \"winddir\": " + String(values[2]) + ", \"rain_rate\": " + String(values[3]) + ", \"temp_in\": " + String(values[4]) + ", \"temp_out\": " + String(values[5]) + ", \"hum_in\": " + String(values[6]) + ", \"hum_out\": " + String(values[7]) + ", \"uv\": " + String(values[8]) + ", \"wind_gust\": " + String(values[9]) + ", \"air_press_rel\": " + String(values[10]) + ", \"air_press_abs\": " + String(values[11]) + ", \"solar_radiation\": " + String(values[12]) + "}";
      //Serial.println("data: " + data);
      // Send the POST request
      http.begin(client, "http://srs-ssms.com/iot/post-data-aws.php");
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(data);

      // Check the response
      if (httpCode == 200) {
        String response = http.getString();
        Serial.println("HTTP response: " + response);
        deleteTopLine();
      } else if (httpCode > 0) {
        String response = http.getString();
        Serial.println("HTTP error response: " + response);
      }

      else {
        Serial.print("HTTP error: ");
        Serial.println(httpCode);
      }
      http.end();
    }
  } else {
    Serial.println("Error opening file!");
  }

  Serial.print("loop ke ");
  testLoop++;
  Serial.println(testLoop);
}

void connectWiFi() {
  WiFi.config(staticIP, gateway, subnet, dnsServer);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to ssid ");
  Serial.println(ssid);
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet mask: ");
  Serial.println(WiFi.subnetMask());
}

void deleteTopLine() {
  File originalFile, newFile;

  originalFile = SD.open("data.txt", FILE_READ);

  // create new file for writing
  newFile = SD.open("new_file.txt", FILE_WRITE);

  // read and discard the first line
  originalFile.readStringUntil('\n');

  // read each subsequent line and write to new file
  while (originalFile.available()) {
    String line = originalFile.readStringUntil('\n');
    newFile.println(line);
  }

  // close both files
  originalFile.close();
  newFile.close();

  // remove original file
  SD.remove("data.txt");

  // rename new file to original file name
  SD.rename("new_file.txt", "data.txt");
}

void loop() {
  server.handleClient();
  sendTimer.update();
}
