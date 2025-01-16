#include "TuyaIoT.h"
#include <Log.h>

#include "hardwareConfig.h"
#include "appButton.h"
#include "appDisplay.h"
#include "appStore.h"

// Tuya license
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "nuoIWxxxxxxxxxxxxxxxxxxxxxxNoCgd"

#define DPID_SWITCH_LED   (20)
#define DPID_COLOUR_DATA  (24)

void tuyaIoTEventCallback(tuya_event_msg_t *event);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Log.begin();

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);

  // license
  tuya_iot_license_t license;
  int rt = TuyaIoT.readBoardLicense(&license);
  if (OPRT_OK != rt) {
    license.uuid = TUYA_DEVICE_UUID;
    license.authkey = TUYA_DEVICE_AUTHKEY;
    Serial.println("Replace the TUYA_DEVICE_UUID and TUYA_DEVICE_AUTHKEY contents, otherwise the demo cannot work");
  }
  Serial.print("uuid: "); Serial.println(license.uuid);
  Serial.print("authkey: "); Serial.println(license.authkey);
  TuyaIoT.setLicense(license.uuid, license.authkey);

  // The "PROJECT_VERSION" comes from the "PROJECT_VERSION" field in "appConfig.json"
  TuyaIoT.begin("j1yn1vt1nygu9crz", PROJECT_VERSION);

  appButtonInit();
  appDisplayInit();
}

void loop() {
  // put your main code here, to run repeatedly:
  appButtonTick();

  delay(10);
}

void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
  tuya_event_id_t event_id = TuyaIoT.eventGetId(event);

  switch (event_id) {
    case TUYA_EVENT_BIND_START: {
      PR_DEBUG("Start bind mode");
      appDisplaySetting();
    } break;
    case TUYA_EVENT_ACTIVATE_SUCCESSED: {
      PR_DEBUG("Device activated");
    } break;
    case TUYA_EVENT_MQTT_CONNECTED: {
      // Update all DP
      Serial.println("---> TUYA_EVENT_MQTT_CONNECTED");
      int ledState = powerStatusGet();
      TuyaIoT.write(DPID_SWITCH_LED, ledState);
      String colourDataStr = colorGet();
      TuyaIoT.write(DPID_COLOUR_DATA, (char *)colourDataStr.c_str());
    } break;
    case TUYA_EVENT_TIMESTAMP_SYNC: {
      tal_time_set_posix(event->value.asInteger, 1);
      appDisplaySettingExit();
    } break;
    case TUYA_EVENT_RESET_COMPLETE: {
      // Erase all app data
      storeEasyMatrixErase();
      tal_system_reset();
    } break;
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
      uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
      for (uint16_t i = 0; i < dpNum; i++) {
        uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
        switch (dpid) {
          case DPID_SWITCH_LED: {
            int ledState = 0;
            TuyaIoT.read(event, DPID_SWITCH_LED, ledState);
            Serial.print("Receive DPID_SWITCH_LED: "); Serial.println(ledState);
            powerStatusSet(ledState);
            TuyaIoT.write(DPID_SWITCH_LED, ledState);
          } break;
          case DPID_COLOUR_DATA: {
            char * colourData = NULL;
            uint16_t h,s,v;
            TuyaIoT.read(event, DPID_COLOUR_DATA, colourData);
            Serial.print("Receive DPID_COLOUR_DATA: "); Serial.println(colourData);
            String colourDataStr = String(colourData);
            // 前四字节是 h, 中间四字节是 s, 后四字节是 v，为 hex 格式
            h = strtoul(colourDataStr.substring(0, 4).c_str(), NULL, 16);
            s = strtoul(colourDataStr.substring(4, 8).c_str(), NULL, 16);
            v = strtoul(colourDataStr.substring(8, 12).c_str(), NULL, 16);
            PR_DEBUG("h: %d, s: %d, v: %d", h, s, v);
            colorSet(h, s/10, v/10);
            // colorSet
            TuyaIoT.write(DPID_COLOUR_DATA, colourData);
          } break;
          default : break;
        }
      }
    } break;
    default: break;
  }
}
