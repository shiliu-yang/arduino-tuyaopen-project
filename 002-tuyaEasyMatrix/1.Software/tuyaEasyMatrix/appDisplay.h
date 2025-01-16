#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include "hardwareConfig.h"

// 定义页面枚举 SETTING-配网页面  TIME-时间页面  RHYTHM-节奏灯页面  ANIM-动画页面  CLOCK-闹钟设置页面  BRIGHT-亮度调节
enum DisplayPage_E{
  SETTING, TIME, RHYTHM, ANIM, CLOCK, BRIGHT
};

void appDisplayInit(void);

void appDisplaySetting(void);
void appDisplaySettingExit(void);

void appDisplayPageSet(enum DisplayPage_E page);
enum DisplayPage_E appDisplayPageGet(void);

void appDisplayPageNext(void);
void appDisplayPagePrev(void);

// time page
void timePageNext(void);
void timePagePrev(void);

// rhythm page
void rhythmPageNext(void);
void rhythmPagePrev(void);

void animPageNext(void);
void animPagePrev(void);

void brightnessSet(uint8_t brightness);
void brightnessInc(void);
void brightnessDec(void);

void clockStateSet(uint8_t is_open);
uint8_t clockStateGet(void);

void clockChooseModeInc(void);
void clockChooseModeDec(void);
void clockNumberInc(void);
void clockNumberDec(void);
void clockSave(void);
uint8_t clockIsBell(void);
void clockBellSet(uint8_t is_bell);

void powerStatusSet(uint8_t status);
uint8_t powerStatusGet(void);

void colorSet(uint16_t h, uint16_t s, uint16_t v);
String colorGet(void);

#endif // __APP_DISPLAY_H__
