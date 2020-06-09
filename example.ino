// ESP8266 WiFi main library
#define FIREBASE_HOST "smart-clock-e87b8.firebaseio.com"
#define FIREBASE_AUTH  "kbhTUdV2cw1UN7kPoONCmjpBd6yGJ6tt5dYwJDWA"
#include "FirebaseESP8266.h"
FirebaseData firebaseData;
FirebaseJson json;
#include <ESP8266WiFi.h>
 
// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library
#include <WiFiManager.h>    
 
// Libraries for internet weather
#include <ESP8266HTTPClient.h>  // http web access library
#include <ArduinoJson.h>        // JSON decoding library
 
// Libraries for SSD1306 OLED display
#include <Wire.h>              // include wire library (for I2C devices such as the SSD1306 display)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver
 
#define OLED_RESET   5     // define SSD1306 OLED reset at ESP8266 GPIO5 (NodeMCU D1)
Adafruit_SSD1306 display(OLED_RESET);
 
WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600, 60000);
 
// set location and API key
String Location = "Hanoi2";
String API_Key  = "90366e0d41ba5f0fcac7621190876245";
int UTC;
char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_;

void updateWearher();
void configWifi();
void updateTime();
void configOled();
void getDataConfig();
 
void setup(void)
{
  Serial.begin(9600);
  delay(1000); 
  configOled();
  configWifi();
  getDataConfig();
  timeClient.begin();
  
  delay(1000);
}
 

 
void loop()
{
  if (WiFi.status() == WL_CONNECTED)  // check WiFi connection status
  {
    updateTime();
    updateWeather();
    delay(200);
  }
}


void display_wday()
{
  switch(wday)
  {
    case 1:  display.print("SUNDAY    "); break;
    case 2:  display.print("MONDAY    "); break;
    case 3:  display.print("TUESDAY   "); break;
    case 4:  display.print("WEDNESDAY "); break;
    case 5:  display.print("THURSDAY  "); break;
    case 6:  display.print("FRIDAY    "); break;
    default: display.print("SATURDAY  ");
  }
 
}
void configOled(){
 
  Wire.begin(4, 0);           // set I2C pins [SDA = GPIO4 (D2), SCL = GPIO0 (D3)], default clock is 100kHz
 
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
 
  Wire.setClock(400000L);   // set I2C clock to 400kHz
 
  display.clearDisplay();   // clear the display buffer
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.display();  
}

void configWifi(){
  WiFiManager wifiManager;
  //reset settings - for testing
  wifiManager.autoConnect("My ESP");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.psk());
  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println("connected");
  display.print("connected");
  display.display();
}

void updateTime(){
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
    unix_epoch+=(UTC-1)*3600;
 
    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    if (last_second != second_)          // update time & date every 1 second
    {
      minute_ = minute(unix_epoch);      // get minutes (0 - 59)
      hour_   = hour(unix_epoch);        // get hours   (0 - 23)
      wday    = weekday(unix_epoch);     // get minutes (1 - 7 with Sunday is day 1)
      day_    = day(unix_epoch);         // get month day (1 - 31, depends on month)
      month_  = month(unix_epoch);       // get month (1 - 12 with Jan is month 1)
      year_   = year(unix_epoch) - 2000; // get year with 4 digits - 2000 results 2 digits year (ex: 2018 --> 18)
 
      Time[7] = second_ % 10 + '0';
      Time[6] = second_ / 10 + '0';
      Time[4] = minute_ % 10 + '0';
      Time[3] = minute_ / 10 + '0';
      Time[1] = hour_   % 10 + '0';
      Time[0] = hour_   / 10 + '0';
      Date[9] = year_   % 10 + '0';
      Date[8] = year_   / 10 + '0';
      Date[4] = month_  % 10 + '0';
      Date[3] = month_  / 10 + '0';
      Date[1] = day_    % 10 + '0';
      Date[0] = day_    / 10 + '0';
 
      display.setCursor(0, 0);
      display.print("Time:");
      Serial.println("TIME:");
      display.setCursor(60, 0);
      display.print(Time);        // display time (format: hh:mm:ss)
      Serial.println(Time);
      display.setCursor(0, 11);
      Serial.println("DATE:");
      display_wday();
      display.print(Date);        // display date (format: dd-mm-yyyy)
      Serial.println(Date);
      display.display();
 
      last_second = second_;
    } 
  
 }
void updateWeather(){
  if (last_minute != minute_)       // update weather every 1 minute
    {
      HTTPClient http;  // declare an object of class HTTPClient
      // specify request destination
      //firebase();
      Serial.println(Location);
     
      String request = "http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key;
      Serial.println(request);
      http.begin(request);  // !!
      int httpCode = http.GET();  // send the request
 
      if (httpCode > 0)  // check the returning code
      {
        String payload = http.getString();   // get the request response payload
        DynamicJsonBuffer jsonBuffer(512);
        // Parse JSON object
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        }
        float temp = (float)(root["main"]["temp"]) - 273.15;        // get temperature in °C
        int   humidity = root["main"]["humidity"];                  // get humidity in %
        display.setCursor(0, 24);
        Serial.print("Nhiệt độ:");
        display.printf("Temperature: %5.2f C\r\n", temp);
        Serial.println(temp);
        Serial.print("Độ ẩm:");
        display.printf("Humidity   : %d %%\r\n", humidity);
        Serial.println(humidity);
        display.drawRect(109, 24, 3, 3, WHITE);     // put degree symbol ( ° )
        display.drawRect(97, 56, 3, 3, WHITE);
        display.display();
      }
 
      http.end();   // close connection
 
      last_minute = minute_;
    }
}

void getDataConfig(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  String path = "/Config";
  Serial.println("Read status...");
  if (Firebase.getString(firebaseData, path + "/Location")){
      Serial.print("VALUE: ");
      Serial.println(firebaseData.stringData());
      Location=firebaseData.stringData();
      Serial.println("------------------------------------");
       Serial.println();
  }
  if (Firebase.getString(firebaseData, path + "/API_KEY")){
      Serial.print("VALUE: ");
      Serial.println(firebaseData.stringData());
      API_Key=firebaseData.stringData();
      Serial.println("------------------------------------");
       Serial.println();
  }
  if (Firebase.getInt(firebaseData, path + "/UTC")){
      Serial.print("VALUE: ");
      Serial.println(firebaseData.intData());
      UTC=firebaseData.intData();
      Serial.println("------------------------------------");
      Serial.println();
  }

 }
