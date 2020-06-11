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
#include "SSD1306Wire.h"
SSD1306Wire display(0x3c, 4, 5,GEOMETRY_128_32);  // ADDRESS, SDA, SCL

WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600, 60000);

#define BTN_CONTRAST 14
#define BTN_MODE 12
int countMode =0;
int countBri =0;

//weather
String Location = "Hanoi2";
String API_Key  = "90366e0d41ba5f0fcac7621190876245";
String statusWeather ="";
String lon = "";
String lat = "";
String currentTemp="";
String currentHumidity="";
String dailyWeather[10];

//time
int UTC;
char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_;
unsigned long unix_epoch;

//am lich
signed char dayd, monthd, yeard;
unsigned char lunar_day, lunar_month, lunar_year;

struct MONTH_INFO
{
  unsigned int N_AL_DT_DL   :5;
  unsigned int T_AL_DT_DL   :4;
  unsigned int SN_CT_AL   :1;
  unsigned int TN_B_THT   :1;
  unsigned int SN_CT_DL   :2;
};

union LUNAR_RECORD
{
  unsigned int Word; 
  struct MONTH_INFO Info;
};

// Lunar Calendar Lookup Table 
// From 1/1/2020 to 31/12/2039
#define BEGINNING_YEAR  20

char *Lunar_year_table[]  = {"Giap Ty", "At Suu", "Binh Dan", "Dinh Mao", "Mau Thin",
                        "Ky Ty", "Canh Ngo", "Tan Mui", "Nham Than", "Quy Dau",
                        "Giap Tuat", "At Hoi", "Binh Ty", " Dinh Suu", "Mau Dan",
                        "Ky Mao", "Canh Thin", "Tan Ty", "Nham Ngo", "Quy Mui",
                        "Giap Than", "At Dau", "Binh Tuat", "Dinh Hoi", "Mau Ty",
                        "Ky Suu", "Canh Dan", "Tan Mao", "Nham Thin", "Quy Ty",
                        "Giap Ngo", "At Mui", "Binh Than", "Dinh Dau", "Mau Tuat",
                        "Ky Hoi", "Canh Ty", "Tan Suu", "Nham Dan", "Quy Mao",
                        "Giap Thin", "At Ty", "Binh Ngo", " Dinh Mui", "Mau Than",
                        "Ky Dau", "Canh Tuat", "Tan Hoi", "Nham Ty", "Quy Suu",
                        "Giap Dan", "At Mao", "Binh Thin", "Dinh Ty", "Mau Ngo",
                        "Ky Mui", "Canh Than", "Tan Dau", "Nham Tuat", "Quy Hoi"};
const long int LUNAR_CALENDAR_LOOKUP_TABLE[] = {
 //2020 
0x1B87,0x0828,0x1A48,0x1269,0x1A89,0x108A,0x1AAB,0x18CC,0x10EE,0x1B0F,0x1130,0x1951,
  //2021 
0x1B73,0x0394,0x1832,0x1254,0x1A74,0x1095,0x1AB6,0x18D7,0x12F9,0x1919,0x133B,0x195B,
  //2022 
0x1B7D,0x0021,0x1A3D,0x1061,0x1A81,0x12A3,0x1AC3,0x18E4,0x1306,0x1926,0x1348,0x1968,
  //2023  
0x1B8A,0x002B,0x1A4A,0x104B,0x186C,0x128E,0x1AAE,0x18CF,0x12F1,0x1B11,0x1132,0x1B53,
  //2024  
0x1974,0x0B96,0x1835,0x1257,0x1877,0x1099,0x1ABA,0x18DB,0x12FD,0x1B1D,0x1141,0x1B61,
 //2025  
0x1982,0x0224,0x1842,0x1264,0x1884,0x10A6,0x1AC7,0x18C8,0x12EA,0x190A,0x132C,0x1B4C,
  //2026
0x1B6D,0x018E,0x1A2D,0x104E,0x1A6F,0x1090,0x18B1,0x1AD3,0x10F4,0x1915,0x1337,0x1B57,
  //2027  
0x1B78,0x0199,0x1A38,0x1259,0x1879,0x129B,0x18BB,0x18DD,0x1301,0x1922,0x1344,0x1B64,
 //2028 
0x1985,0x0A27,0x1A46,0x1267,0x1887,0x12A9,0x18A9,0x18CB,0x12ED,0x190D,0x112F,0x1B50,
 //2029 
0x1B71,0x0192,0x1A31,0x1252,0x1872,0x1294,0x18B4,0x1AD6,0x10F7,0x1B18,0x1139,0x195A,
  //2030  
0x1B7C,0x019D,0x1A3C,0x125D,0x187D,0x12A1,0x1AC1,0x1AE3,0x1104,0x1B25,0x1146,0x1B67,
  //2031  
0x1988,0x002A,0x1A49,0x106A,0x1A6B,0x128C,0x18AC,0x1ACE,0x12EF,0x190F,0x1331,0x1951,
  //2032  
0x1B73,0x0994,0x1834,0x1256,0x1876,0x1298,0x18B8,0x1ADA,0x12FB,0x191B,0x133D,0x1B5D,
  //2033 
0x1981,0x0022,0x1841,0x1062,0x1883,0x12A5,0x18C5,0x1AE7,0x1108,0x1B29,0x134A,0x1B6A,
  //2034 
0x196B,0x038D,0x182B,0x124D,0x186D,0x108F,0x1AB0,0x18D1,0x10F3,0x1B14,0x1335,0x1B55,
  //2035
0x1976,0x0398,0x1A36,0x1057,0x1A78,0x1099,0x18BA,0x1ADC,0x10FD,0x1921,0x1342,0x1962,
  //2036 
0x1B84,0x0A25,0x1A44,0x1065,0x1A86,0x10A7,0x18C8,0x1ACA,0x10EB,0x190C,0x132E,0x194E,
  //2037
0x1B70,0x0391,0x1A2F,0x1050,0x1A71,0x1292,0x18B2,0x18D4,0x12F6,0x1916,0x1138,0x1B59,
  //2038
0x197A,0x039C,0x1A3A,0x105B,0x1A7C,0x129D,0x18BD,0x1AE1,0x1303,0x1923,0x1145,0x1B66,
  //2039
0x1987,0x0229,0x1A47,0x1068,0x1A89,0x12AA,0x18AA,0x1ACC,0x10ED,0x1B0E,0x112F,0x1950,
};

void getDataConfig();
void updateTime();
void configWifi();
void configOled();
void systemDisplay();
void showWeatherForecast();
void showOled();
void showTime(String time,String date);
void showWeather(String temp,String humidity);
void updateWearher();
void settingAmLich();
void displayAmLich();
void parseTime(unsigned long unix_epoch);


//----------------------------------------------------
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
  }else{
    showOled("Wifi is disconnected!");
  }
}

//--------------------------------------------------------------

void systemDisplay(){
  if(!digitalRead(BTN_MODE)){
    countMode++;
    while(!digitalRead(BTN_MODE));
    if(countMode >3) countMode =0;
  } 
  if(!digitalRead(BTN_CONTRAST)){
     countBri+=50;
     while(!digitalRead(BTN_CONTRAST));
     if(countBri >255) countBri =0;
     display.setBrightness(countBri);
  }  
  switch(countMode){
      case 0:
        showTime(Time,Date);
        break;
      case 1:
        displayAmLich();
        break;
      case 2:
        showWeather(currentTemp,currentHumidity);
        break;
      case 3:
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
void displayAmLich(){
  settingAmLich(day_, month_, year_);  
  String al = String(lunar_day)+"/"+String(lunar_month)+"-"+String(Lunar_year_table[(((lunar_year) + 2000 - 3)%60)-1]);
  display.clear();
  display.drawString(10, 15, al);
  display.drawString(10, 0, "Am Lich");
  display.display();
}

void showWeather(String temp,String humidity){
  display.clear();
  display.drawString(0, 15, statusWeather+": "+temp +" ºC");
  display.drawString(0, 0, "Humidity: "+humidity+" %");
  display.display();
}
void showWeatherForecast(){
  display.clear();
  display.drawString(0, 0, "Weather Forecast");
  display.display();
  delay(1000);
  for(unsigned char i =0;i<8;i++){
     unix_epoch+=86400; // display date
     parseTime(unix_epoch);
     display.clear();
     display.drawString(0, 15,dailyWeather[i] );
     display.drawString(10, 0, Date);
     display.display();
     delay(1000);
  }
  countMode=0;
  
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
  delay(1000);
}



void configWifi(){
  WiFiManager wifiManager;
  wifiManager.autoConnect("My ESP");
  showOled("  ~~Ready~~");
}

void updateTime(){
    timeClient.update();
    unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
    unix_epoch+=(UTC-1)*3600;
    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    if (last_second != second_)  {        // update time & date every 1 second
      parseTime(unix_epoch);
    } 
  
 }
 void parseTime(unsigned long unix_epoch){
   
      second_ = second(unix_epoch);  
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
      last_second = second_;
    
  
  
  
 }

void updateWeather(){
  if (last_minute != minute_)       // update weather every 1 minute
    {
      HTTPClient http;  // declare an object of class HTTPClient
      // specify request destination
      //firebase();
     
      String request = "http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key;
    
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
        
   
        for (auto weather : weathers) {
           const char* value = weather["main"];

           statusWeather =value;
        }
        currentTemp = String(temp);
        currentHumidity = String(humidity);
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
           JsonArray& weathers = daily["weather"];
           for (auto weather : weathers) {
              String v = weather["main"];
              dailyWeather[countDay] = v;
              dailyWeather[countDay]+= ",T: ";
              dailyWeather[countDay] +=String(temp) ;
              dailyWeather[countDay] +=" ºC" ;
          }
          countDay++;
        }
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        } 
        String zone = root["timezone"];
      }
      http.end();   // close connection
 
      last_minute = minute_;
    }
}
void settingAmLich(unsigned char SolarDate, unsigned char SolarMonth, unsigned char SolarYear){
 unsigned char N_AL_DT_DL; 
  unsigned char T_AL_DT_DL; 
  unsigned char SN_CT_AL;
  unsigned char TN_B_THT;
  unsigned char N_AL_DT_DL_TT;
  unsigned char T_AL_DT_DL_TT;

  union LUNAR_RECORD lr;

  lr.Word = LUNAR_CALENDAR_LOOKUP_TABLE[(SolarYear-BEGINNING_YEAR)
*12+SolarMonth -1]; 
  N_AL_DT_DL = lr.Info.N_AL_DT_DL;
  T_AL_DT_DL = lr.Info.T_AL_DT_DL;
  SN_CT_AL = lr.Info.SN_CT_AL + 29;
  TN_B_THT =  lr.Info.TN_B_THT;

  lr.Word = LUNAR_CALENDAR_LOOKUP_TABLE[(SolarYear-BEGINNING_YEAR)
*12+SolarMonth];
  N_AL_DT_DL_TT = lr.Info.N_AL_DT_DL;
  T_AL_DT_DL_TT = lr.Info.T_AL_DT_DL;

  // Tinh ngay & thang
  if(N_AL_DT_DL == SN_CT_AL && N_AL_DT_DL_TT == 2)
  {
    if(SolarDate==1)
    {
      (lunar_day) = N_AL_DT_DL;
      (lunar_month) = T_AL_DT_DL;
    }
    else if(SolarDate==31)
    {
      (lunar_day) = 1;
      (lunar_month) = T_AL_DT_DL_TT;
    }
    else
    {
      (lunar_day) = SolarDate - 1;
      if(TN_B_THT)
      {
        (lunar_month) = T_AL_DT_DL;
      }
      else
      {
        (lunar_month) = T_AL_DT_DL==12?1:
(T_AL_DT_DL + 1);
      } 
    }
  }
  else
  {
    (lunar_day) = SolarDate + N_AL_DT_DL - 1;
    if((lunar_day)<= SN_CT_AL)
    {
      (lunar_month) = T_AL_DT_DL;
    }
    else
    {
      (lunar_day) -= SN_CT_AL;

      (lunar_month) = T_AL_DT_DL + 1 - TN_B_THT;
      if((lunar_month) == 13) (lunar_month) = 1;
    }
  }

  // Tinh Nam
  if(SolarMonth >= (lunar_month))
  {
    (lunar_year) = SolarYear;
  }
  else
  {
    (lunar_year) = SolarYear - 1;
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
  if (Firebase.getString(firebaseData, path + "/Location")){
      Location=firebaseData.stringData();

  }
  if (Firebase.getString(firebaseData, path + "/API_KEY")){
      API_Key=firebaseData.stringData();
  }
  if (Firebase.getInt(firebaseData, path + "/UTC")){
      UTC=firebaseData.intData();
  }

 }
