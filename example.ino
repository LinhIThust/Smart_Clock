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
//#include <Adafruit_GFX.h>      // include Adafruit graphics library
//#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver
#include "SSD1306Wire.h"
SSD1306Wire display(0x3c, 4, 5,GEOMETRY_128_32);  // ADDRESS, SDA, SCL
// #define OLED_RESET   5     // define SSD1306 OLED reset at ESP8266 GPIO5 (NodeMCU D1)
// Adafruit_SSD1306 display(OLED_RESET);
 
WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600, 60000);

#define BTN_CONTRAST 14
#define BTN_MODE 12
int countMode =0;
int countBri =0;
// set location and API key
String Location = "Hanoi2";
String API_Key  = "90366e0d41ba5f0fcac7621190876245";
String statusWeather ="";
String lon = "";
String lat = "";
int UTC;
char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
String currentTemp="";
String currentHumidity="";
String dailyWeather[10];
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_;



void showOled();
void showTime(String time,String date);
void showWeather(String temp,String humidity);
void updateWearher();
void configWifi();
void updateTime();
void configOled();
void getDataConfig();
void systemDisplay();
void showWeatherForecast();
 
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
    systemDisplay();
    delay(200);
  }
}

void systemDisplay(){
  if(!digitalRead(BTN_MODE)){
    countMode++;
    while(!digitalRead(BTN_MODE));
    if(countMode >2) countMode =0;
  } 
  if(!digitalRead(BTN_CONTRAST)){
     countBri+=20;
     while(!digitalRead(BTN_CONTRAST));
     if(countBri >255) countBri =0;
     display.setBrightness(countBri);
  }  
  switch(countMode){
      case 0:
        showTime(Time,Date);
        break;
      case 1:
        showWeather(currentTemp,currentHumidity);
        break;
      case 2:
        showWeatherForecast();
        break;
  }                                                                                                                                                                                                    
  
}



     
void showOled(String s){
  display.clear();
  display.drawString(0, 10, s);
  display.display();

}
void showTime(String time,String date){
  display.clear();
  display.drawString(10, 15, "   "+time);
  display.drawString(10, 0, date);
  display.display();
}

void showWeather(String temp,String humidity){
  display.clear();
  display.drawString(0, 15, statusWeather+": "+temp +" ºC");
  display.drawString(0, 0, "Humidity: "+humidity+" %");
  display.display();
}
void showWeatherForecast(){
  for(unsigned char i =0;i<8;i++){
     display.clear();
     display.drawString(0, 15,dailyWeather[i] );
     display.drawString(0, 0, "Weather Forecast");
     display.display();
     delay(500);
  }
}
void display_wday()
{
  switch(wday)
  {
    case 1:  showOled("SUNDAY    "); break;
    case 2:  showOled("MONDAY    "); break;
    case 3:  showOled("TUESDAY   "); break;
    case 4:  showOled("WEDNESDAY "); break;
    case 5:  showOled("THURSDAY  "); break;
    case 6:  showOled("FRIDAY    "); break;
    default: showOled("SATURDAY  ");
  }
 
}
void configOled(){
 
  display.init();
  display.setFont(ArialMT_Plain_16);
  showOled("Duong Tran IT");
}



void configWifi(){
  WiFiManager wifiManager;
  wifiManager.autoConnect("My ESP");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.psk());
  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println("Connected");
  showOled("connected");
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
 
       Serial.println("TIME:");
       Serial.println(Time);
      // display.setCursor(0, 11);
       Serial.println("DATE:");
      // display_wday();
      // display.print(Date);        // display date (format: dd-mm-yyyy)
       Serial.println(Date);
      // display.display();
      //showTime(Time,Date);
 
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
        JsonArray& weathers = root["weather"];
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        }
        float temp = (float)(root["main"]["temp"]) - 273.15;        // get temperature in °C
        int   humidity = root["main"]["humidity"];                  // get humidity in %
        lon = (float)(root["coord"]["lon"]);
        lat = (float)(root["coord"]["lat"]);
        
        Serial.println("LL:"+String(lon)+String(lat));
        for (auto weather : weathers) {
           const char* value = weather["main"];
           Serial.print("thoi tiet:");
           
           Serial.println(value);
           statusWeather =value;
        }
        
         Serial.print("Nhiệt độ:");
         Serial.println(temp);
         Serial.print("Độ ẩm:");
         Serial.println(humidity);
  
        currentTemp = String(temp);
        currentHumidity = String(humidity);
        //showWeather(currentTemp,currentHumidity);
      }
      String request2 = "https://api.openweathermap.org/data/2.5/onecall?lat="+lat+"&lon="+lon+"&exclude=hourly,minutely"+ "&APPID=" + API_Key;
      http.begin(request2);  // !!
      int httpCode2 = http.GET();  // send the request
      if (httpCode2 > 0)  // check the returning code
      {
        String payload = http.getString();   // get the request response payload
        int countDay=0;
        DynamicJsonBuffer jsonBuffer(512);
        // Parse JSON object
        JsonObject& root = jsonBuffer.parseObject(payload);
        JsonArray& dailys = root["daily"];
        
        for (auto daily : dailys) {
           float temp = (float)(daily["temp"]["day"]) - 273.15;   
           Serial.print("ngay tiep theo:");
           Serial.println(String(temp));
           JsonArray& weathers = daily["weather"];
           for (auto weather : weathers) {
              String v = weather["main"];
              dailyWeather[countDay] = v;
              dailyWeather[countDay]+= ",Temp: ";
              dailyWeather[countDay] +=String(temp) ;
              Serial.println(dailyWeather[countDay]);
          }
          countDay++;
        }
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        } 
        String zone = root["timezone"];
        Serial.println(zone);
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
