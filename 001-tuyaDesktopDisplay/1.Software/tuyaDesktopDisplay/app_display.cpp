#include "app_display.h"

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TuyaIoT.h>

#include "TJpg_Decoder.h"

#include "number.h"
#include "weathernum.h"

#include "font/simhei20.h"

#include "image/temperature.h"
#include "image/humidity.h"

#include "image/pangzi/i0.h"
#include "image/pangzi/i1.h"
#include "image/pangzi/i2.h"
#include "image/pangzi/i3.h"
#include "image/pangzi/i4.h"
#include "image/pangzi/i5.h"
#include "image/pangzi/i6.h"
#include "image/pangzi/i7.h"
#include "image/pangzi/i8.h"
#include "image/pangzi/i9.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);

enum {
  DISPLAY_STATE_FIRST = 0,
  DISPLAY_STATE_ACTIVITING,
  DISPLAY_STATE_CONNECTING,
  DISPLAY_STATE_CONNECTED,
  DISPLAY_STATE_DISCONNECTED,
  DISPLAY_STATE_UNKNOW,
};

static void display_first(void)
{
  TFT_eSprite text = TFT_eSprite(&tft);

  text.setColorDepth(8);
  text.loadFont(simhei20);

  text.createSprite(240, 240);
  text.fillSprite(TFT_BLACK);
  text.setTextDatum(CC_DATUM);
  text.setTextColor(TFT_WHITE);
  text.drawString("Hello, Tuya IoT.", 120, 120);
  text.pushSprite(0, 0);
  text.deleteSprite();

  text.unloadFont();
}

static void display_activiting(uint8_t isFirst)
{
  static uint8_t count = 0;
  TFT_eSprite activeSprite = TFT_eSprite(&tft);

  count++;
  if (count >= 6) {
    count = 0;
  }
  activeSprite.setColorDepth(8);
  activeSprite.loadFont(simhei20);

  if (isFirst) {
    tft.fillScreen(TFT_BLACK);

    activeSprite.createSprite(240, 50);

    activeSprite.fillSprite(TFT_BLACK);
    activeSprite.setTextDatum(CC_DATUM);
    activeSprite.setTextColor(TFT_WHITE);

    activeSprite.drawString("Use the Tuya app ", 120, 8);
    activeSprite.drawString("to connect the device.", 120, 8+8+16);

    activeSprite.pushSprite(0, 80);
    activeSprite.deleteSprite();
  }

  activeSprite.createSprite(240, 30);
  activeSprite.fillSprite(TFT_BLACK);
  activeSprite.setTextDatum(CC_DATUM);
  activeSprite.setTextColor(TFT_WHITE);

  if (count == 0) {
    activeSprite.drawString("Activiting.", 120, 16);
  } else if (count == 1) {
    activeSprite.drawString("Activiting..", 120, 16);
  } else if (count == 2) {
    activeSprite.drawString("Activiting...", 120, 16);
  } else if (count == 3) {
    activeSprite.drawString("Activiting....", 120, 16);
  } else if (count == 4) {
    activeSprite.drawString("Activiting.....", 120, 16);
  } else if (count == 5) {
    activeSprite.drawString("Activiting......", 120, 16);
  }

  activeSprite.pushSprite(0, 130);
  activeSprite.deleteSprite();

  activeSprite.unloadFont();
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;

  tft.pushImage(x, y, w, h, bitmap);

  return true;
}

// Display desktop start
static struct DesktopWeather weatherData, weatherDataPrev;

Number      dig;
WeatherNum  wrat;

void weather_data_init(void)
{
  weatherDataPrev.weatherCode = 0;
  weatherDataPrev.realFeel = 0;
  weatherDataPrev.humidity = 0;
  weatherDataPrev.aqi = 0;
  weatherDataPrev.highTemp = 0;
  weatherDataPrev.lowTemp = 0;
  weatherDataPrev.cityName = "";
  weatherDataPrev.windDir = "";
  weatherDataPrev.windLevel = 0;

  weatherData.weatherCode = 120;
  weatherData.realFeel = 26;
  weatherData.humidity = 70;
  weatherData.aqi = 34;
  weatherData.highTemp = 30;
  weatherData.lowTemp = 22;
  weatherData.cityName = "杭州";
  weatherData.windDir = "NE";
  weatherData.windLevel = 2;
}

void CitySet(String cityName)
{
  TFT_eSprite citySprite = TFT_eSprite(&tft);

  citySprite.setColorDepth(8);
  citySprite.loadFont(simhei20);

  citySprite.createSprite(94, 30); 
  citySprite.fillSprite(TFT_BLACK);
  citySprite.setTextDatum(CC_DATUM);
  citySprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  citySprite.drawString(cityName,44,16);
  citySprite.pushSprite(15,15);
  citySprite.deleteSprite();

  citySprite.unloadFont();

  return;
}

void aqiSet(int aqi)
{
  uint16_t pm25BgColor = tft.color565(156,202,127);
  String aqiTxt = "优";

  TFT_eSprite aqiSprite = TFT_eSprite(&tft);

  if(aqi>200){
    pm25BgColor = tft.color565(136,11,32);
    aqiTxt = "重度";
  }else if(aqi>150){
    pm25BgColor = tft.color565(186,55,121);
    aqiTxt = "中度";
  }else if(aqi>100){
    pm25BgColor = tft.color565(242,159,57);
    aqiTxt = "轻度";
  }else if(aqi>50){
    pm25BgColor = tft.color565(247,219,100);
    aqiTxt = "良";
  }

  aqiSprite.setColorDepth(8);
  aqiSprite.loadFont(simhei20);

  aqiSprite.createSprite(56, 24); 
  aqiSprite.fillSprite(TFT_BLACK);
  aqiSprite.fillRoundRect(0, 0, 50, 24, 4, pm25BgColor);
  aqiSprite.setTextDatum(CC_DATUM);
  aqiSprite.setTextColor(0x0000); 
  aqiSprite.drawString(aqiTxt, 25, 13);
  aqiSprite.pushSprite(104, 18);
  aqiSprite.deleteSprite();

  aqiSprite.unloadFont();
}

#define SCROLL_TEXT_NUM 2
String scrollText[SCROLL_TEXT_NUM];
int currentIndex = 0;

void scrollBanner(void)
{
  TFT_eSprite clkb = TFT_eSprite(&tft);

  static unsigned long _scrollBannerlastTime = 0;

  if(millis() - _scrollBannerlastTime > 3*1000 || 0 == _scrollBannerlastTime) { // Switch every 3 seconds
    _scrollBannerlastTime = millis();
    _scrollBannerlastTime = (_scrollBannerlastTime == 0) ? 1 : _scrollBannerlastTime;

    // Scroll banner update
    scrollText[0] = "最低温度 " + String(weatherData.lowTemp) + " ℃";
    scrollText[1] = "最高温度 " + String(weatherData.highTemp) + " ℃";

    if(scrollText[currentIndex])
    {
      clkb.setColorDepth(8);
      clkb.loadFont(simhei20);
      clkb.createSprite(150, 30); 
      clkb.fillSprite(TFT_BLACK);
      clkb.setTextWrap(false);
      clkb.setTextDatum(CC_DATUM);
      clkb.setTextColor(TFT_WHITE, TFT_BLACK); 
      clkb.drawString(scrollText[currentIndex], 74, 16);
      clkb.pushSprite(10,45);

      clkb.deleteSprite();
      clkb.unloadFont();

      currentIndex++;
      currentIndex = currentIndex % SCROLL_TEXT_NUM;
    }
  }
}

void TemperatureSet(int temperature)
{
  TFT_eSprite tempSprite = TFT_eSprite(&tft);

  uint16_t tempcol = 0;

  tempSprite.setColorDepth(8);
  tempSprite.loadFont(simhei20);

  tempSprite.createSprite(58, 24);
  tempSprite.fillSprite(TFT_BLACK);
  tempSprite.setTextDatum(CC_DATUM);
  tempSprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  tempSprite.drawString(String(temperature) + "℃",28,13);
  tempSprite.pushSprite(100,184);
  tempSprite.deleteSprite();

  temperature = temperature+10;
  if(temperature<10)
    tempcol=0x00FF;
  else if(temperature<28)
    tempcol=0x0AFF;
  else if(temperature<34)
    tempcol=0x0F0F;
  else if(temperature<41)
    tempcol=0xFF0F;
  else if(temperature<49)
    tempcol=0xF00F;
  else
  {
    tempcol=0xF00F;
    temperature=50;
  }

  // Temperature progress bar
  tempSprite.createSprite(52, 6);
  tempSprite.fillSprite(0x0000);
  tempSprite.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  tempSprite.fillRoundRect(1, 1, temperature, 4, 2, tempcol);
  tempSprite.pushSprite(45, 192);
  tempSprite.deleteSprite();

  tempSprite.unloadFont();

  return;
}

void HumiditySet(int humidity)
{
  TFT_eSprite humiSprite = TFT_eSprite(&tft);

  uint16_t humicol = 0;

  humiSprite.setColorDepth(8);
  humiSprite.loadFont(simhei20);

  humiSprite.createSprite(58, 24); 
  humiSprite.fillSprite(TFT_BLACK);
  humiSprite.setTextDatum(CC_DATUM);
  humiSprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  humiSprite.drawString(String(humidity) + "%",28,13);
  humiSprite.pushSprite(100,214);
  humiSprite.deleteSprite();

  if(humidity>90)
    humicol=0x00FF;
  else if(humidity>70)
    humicol=0x0AFF;
  else if(humidity>40)
    humicol=0x0F0F;
  else if(humidity>20)
    humicol=0xFF0F;
  else
    humicol=0xF00F;

  // Humidity progress bar
  humiSprite.createSprite(52, 6);
  humiSprite.fillSprite(0x0000);
  humiSprite.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  humiSprite.fillRoundRect(1, 1, humidity/2 , 4, 2, humicol);
  humiSprite.pushSprite(45, 222);
  humiSprite.deleteSprite();

  humiSprite.unloadFont();

  return;
}

void digitalClockDisplay(uint8_t month, uint8_t day, uint8_t week, uint8_t hour, uint8_t minute, uint8_t second)
{
  static uint8_t lastHour = -1, lastMinute = -1, lastSecond = -1;
  static uint8_t lastMonth = -1, lastDay = -1, lastWeek = -1;

  TFT_eSprite clkSprite = TFT_eSprite(&tft);

  int timey=82;

  if (lastHour != hour) {
    dig.printfW3660(20,timey,hour/10);
    dig.printfW3660(60,timey,hour%10);
    lastHour = hour;
  }

  if (lastMinute != minute) {
    dig.printfO3660(101,timey,minute/10);
    dig.printfO3660(141,timey,minute%10);
    lastMinute = minute;
  }

  if (lastSecond != second) {
    dig.printfW1830(182,timey+30,second/10);
    dig.printfW1830(202,timey+30,second%10);
    lastSecond = second;
  }

  const char *weekStr[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
  if (lastWeek != week || lastMonth != month || lastDay != day) {
    clkSprite.setColorDepth(8);
    clkSprite.loadFont(simhei20);

    //星期
    clkSprite.createSprite(58, 30);
    clkSprite.fillSprite(TFT_BLACK);
    clkSprite.setTextDatum(CC_DATUM);
    clkSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    clkSprite.drawString(weekStr[week],29,16);
    clkSprite.pushSprite(102,150);
    clkSprite.deleteSprite();
    lastWeek = week;

    // 月日
    clkSprite.createSprite(95, 30);
    clkSprite.fillSprite(TFT_BLACK);
    clkSprite.setTextDatum(CC_DATUM);
    clkSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    clkSprite.drawString(String(month) + "月" + String(day) + "日",49,16);
    clkSprite.pushSprite(5,150);
    clkSprite.deleteSprite();
    lastMonth = month;
    lastDay = day;

    clkSprite.unloadFont();
  }
}

void imgAnim()
{
  static int Anim = 0;
  static int AprevTime = 0;

  int x=160,y=160;
  if(millis() - AprevTime > 100)
  {
    Anim++;
    AprevTime = millis();
  }
  if(Anim==10)
    Anim=0;

  switch(Anim)
  {
    case 0:
      TJpgDec.drawJpg(x,y,i0, sizeof(i0));
      break;
    case 1:
      TJpgDec.drawJpg(x,y,i1, sizeof(i1));
      break;
    case 2:
      TJpgDec.drawJpg(x,y,i2, sizeof(i2));
      break;
    case 3:
      TJpgDec.drawJpg(x,y,i3, sizeof(i3));
      break;
    case 4:
      TJpgDec.drawJpg(x,y,i4, sizeof(i4));
      break;
    case 5:
      TJpgDec.drawJpg(x,y,i5, sizeof(i5));
      break;
    case 6:
      TJpgDec.drawJpg(x,y,i6, sizeof(i6));
      break;
    case 7:
      TJpgDec.drawJpg(x,y,i7, sizeof(i7));
      break;
    case 8: 
      TJpgDec.drawJpg(x,y,i8, sizeof(i8));
      break;
    case 9: 
      TJpgDec.drawJpg(x,y,i9, sizeof(i9));
      break;
    default:
      Serial.println("Diaplay Anim error");
      break;
  }
}

void app_display_desktop_weather_set(struct DesktopWeather weather)
{
  weatherData.weatherCode = weather.weatherCode;
  weatherData.realFeel = weather.realFeel;
  weatherData.humidity = weather.humidity;
  weatherData.aqi = weather.aqi;
  weatherData.highTemp = weather.highTemp;
  weatherData.lowTemp = weather.lowTemp;
  weatherData.cityName = weather.cityName;
  weatherData.windDir = weather.windDir;
  weatherData.windLevel = weather.windLevel;

  return;
}

void app_display_desktop(uint8_t isFirst)
{
  int rt = OPRT_OK;

  if (isFirst) {
    tft.fillScreen(TFT_BLACK);

    // Temperature icon
    TJpgDec.drawJpg(15,183,temperature, sizeof(temperature));
    // Humidity icon
    TJpgDec.drawJpg(15,213,humidity, sizeof(humidity));
  }

  if (weatherData.cityName != weatherDataPrev.cityName) {
    CitySet(weatherData.cityName);
    weatherDataPrev.cityName = weatherData.cityName;
  }

  if (weatherData.aqi != weatherDataPrev.aqi) {
    aqiSet(weatherData.aqi);
    weatherDataPrev.aqi = weatherData.aqi;
  }

  if (weatherData.weatherCode != weatherDataPrev.weatherCode) {
    wrat.printfweather(170, 15, weatherData.weatherCode);
    weatherDataPrev.weatherCode = weatherData.weatherCode;
  }

  if (weatherData.realFeel != weatherDataPrev.realFeel) {
    TemperatureSet(weatherData.realFeel);
    weatherDataPrev.realFeel = weatherData.realFeel;
  }

  if (weatherData.humidity != weatherDataPrev.humidity) {
    HumiditySet(weatherData.humidity);
    weatherDataPrev.humidity = weatherData.humidity;
  }

  POSIX_TM_S posixTime;
  uint8_t Hour = 0 , Minute = 0 , Second = 0;
  uint8_t Month = 9 , Day = 27 , Week = 5;
  rt = tal_time_get_local_time_custom(0, &posixTime);
  if (rt == OPRT_OK) {
    Month = posixTime.tm_mon + 1;
    Day = posixTime.tm_mday;
    Week = posixTime.tm_wday;
    Hour = posixTime.tm_hour;
    Minute = posixTime.tm_min;
    Second = posixTime.tm_sec;
    digitalClockDisplay(Month, Day, Week, Hour, Minute, Second);
  }

  scrollBanner();

  imgAnim();
}

// Display desktop end

void app_display_init(void)
{
  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(0);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  display_first();
  weather_data_init();
}

void app_display_loop(void)
{
  static uint8_t isFirst = 1;
  static int state = DISPLAY_STATE_FIRST;
  static unsigned long last_refresh = 0, refresh_interval = 0;

  unsigned long now = millis();
  if (now - last_refresh < refresh_interval && now > last_refresh) {
    return;
  }

  switch (state) {
    case DISPLAY_STATE_FIRST:
      // Display for 2 seconds
      last_refresh = now;
      refresh_interval = 1000*2;
      state = TuyaIoT.isActivated() ? DISPLAY_STATE_CONNECTING : DISPLAY_STATE_ACTIVITING;
      isFirst = 1;
      break;
    case DISPLAY_STATE_ACTIVITING:
      display_activiting(isFirst);
      isFirst = 0;

      last_refresh = now;
      refresh_interval = 1000*1;

      if (TuyaIoT.isTimeSync()) {
        state = DISPLAY_STATE_CONNECTED;
        isFirst = 1;
      }
      break;
    case DISPLAY_STATE_CONNECTING:
    case DISPLAY_STATE_CONNECTED:
      app_display_desktop(isFirst);
      isFirst = 0;

      last_refresh = now;
      refresh_interval = 100;
      break;
    default:
      break;
  }
}
