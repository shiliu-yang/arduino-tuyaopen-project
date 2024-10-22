#include <Arduino.h>

#include "Log.h"
#include "Ticker.h"
#include "tal_memory.h"

#include "app_button.h"
#include "app_tuya_iot.h"
#include "app_weather.h"
#include "app_display.h"

// Tuya license
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

// Print free heap
Ticker heapTicker;

void heapCallback() {
  PR_NOTICE("Free heap: %d", tal_system_get_free_heap_size());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // close built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // display init
  app_display_init();

  // button init
  app_button_init();

  // Tuya IoT init
  app_iot_init(TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY);

  // weather init, need to be called after app_iot_init()
  // app_iot_init() will initialize the work queue
  app_weather_init();

  heapTicker.attach(5, heapCallback);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println("Hello World");

  app_button_loop();
  app_display_loop();

  delay(10);
}

