#pragma once

#include <Arduino.h>

struct DesktopWeather {
  int weatherCode;
  int realFeel;
  int humidity;
  int aqi;
  int highTemp;
  int lowTemp;
  String cityName;
  String windDir;
  int windLevel;
};

void app_display_init(void);
void app_display_loop(void);

void app_display_desktop_weather_set(struct DesktopWeather weather);
