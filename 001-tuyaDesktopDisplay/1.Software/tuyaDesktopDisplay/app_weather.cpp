#include "app_weather.h"
#include "app_display.h"

#include "TuyaIoT.h"
#include "TuyaIoTWeather.h"
#include "Log.h"

#include "tal_workq_service.h"

DELAYED_WORK_HANDLE weatherWorkQueueHandle;

TuyaIoTWeatherClass TuyaWeather;

static void weatherWorkQueueCallback(void *data)
{
  int rt = 0;

  static int nextUpdateCount = 10, cnt = 0;

  struct DesktopWeather weatherData;

  int weather, temp, humi, realFeel, mbar, uvi, highTemp, lowTemp, windLevel;
  int aqi, qualityLevel, pm25, pm10, o3, no2, co, so2;
  String windDir, windSpeed;

  cnt++;

  if (false == TuyaIoT.isTimeSync()) {
    cnt = 0;
    return;
  }

  if (nextUpdateCount > cnt) {
    return;
  }

  rt = TuyaWeather.getCurrentConditions(weather, temp, humi, realFeel, mbar, uvi);
  if (rt != OPRT_OK) {
    PR_ERR("Get current conditions failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.weatherCode = weather;
  weatherData.realFeel = realFeel;
  weatherData.humidity = humi;

  rt = TuyaWeather.getTodayHighLowTemp(highTemp, lowTemp);
  if (rt != OPRT_OK) {
    PR_ERR("Get today high low temp failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.highTemp = highTemp;
  weatherData.lowTemp = lowTemp;

  rt = TuyaWeather.getCurrentAQI(aqi, qualityLevel, pm25, pm10, o3, no2, co, so2);
  if (rt != OPRT_OK) {
    PR_ERR("Get current AQI failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.aqi = aqi;

  rt = TuyaWeather.getCurrentWindCN(windDir, windSpeed, windLevel);
  if (rt != OPRT_OK) {
    PR_ERR("Get current wind failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.windDir = windDir;
  weatherData.windLevel = windLevel;

  String province, city, area;
  rt = TuyaWeather.getCity(province, city, area);
  if (rt != OPRT_OK) {
    PR_ERR("Get city name failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.cityName = city;

  app_display_desktop_weather_set(weatherData);

  nextUpdateCount = 15*60; // 15 minutes
  cnt = 0;
}

void app_weather_init(void)
{
  TIME_MS TmInterval = 1000;

  tal_workq_init_delayed(WORKQ_SYSTEM, weatherWorkQueueCallback, NULL, &weatherWorkQueueHandle);
  tal_workq_start_delayed(weatherWorkQueueHandle, TmInterval, LOOP_CYCLE);
}

