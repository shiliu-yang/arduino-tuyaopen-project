#include "appDisplay.h"
#include "appStore.h"

#include "Ticker.h"
#include "Log.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>

#include "tal_thread.h"
#include "tal_time_service.h"

#include "MyFont.h"
#include "buzzer.h"

// whale image
#include "img/timeAnim0.h"
#include "img/timeAnim1.h"
#include "img/timeAnim2.h"
#include "img/timeAnim3.h"
#include "img/timeAnim4.h"
#include "img/timeAnim5.h"
#include "img/timeAnim6.h"
#include "img/timeAnim7.h"
#include "img/timeAnim8.h"
#include "img/animHack.h"
#include "img/bell.h"
#include "img/checkingTime.h"

#define APP_DISPLAY_DELAY (10)

// 亮度调节步进
#define BRIGHTNESS_STEP (20)

// settings for the matrix
static uint8_t is_setting_page = 0;
struct displaySettings {
  uint32_t tick;
  uint16_t color;
};

// time page
#define WHALE_ANIM_INTERVAL (200)
Ticker whaleAnimTicker;
enum TimePage_E {
  TIME_H_M_S,
  TIME_H_M,
  TIME_DATE,
  TIME_MAX,
};
static POSIX_TM_S lastTimeinfo;
struct timePageData_S {
  enum TimePage_E timePage;
  uint16_t color;
};

// RHYTHM page
#define FREQ_BAND_NUM             (32)  // frequency band number
#define SAMPLES_NUM               (256) // number of samples
#define AMPLITUDE             1000  //声音强度调整倍率（柱状高度倍率）
#define SAMPLING_FREQ             (10000) // sampling frequency
#define NOISE                 1770   //噪音
int bandValues[FREQ_BAND_NUM] = {0};
uint8_t peak[FREQ_BAND_NUM] = {0};
int oldBarHeights[FREQ_BAND_NUM] = {0};
double vReal[SAMPLES_NUM] = {0};
double vImag[SAMPLES_NUM] = {0};
int colorTime;
unsigned long starTime;
unsigned long peekDecayTime;
unsigned long changeColorTime;
int model2ColorArray[2][3] = {{0, 220, 255}, {240, 45, 255}};
int model4ColorArrar[8][3] = {{240, 45, 255},{253, 98, 248},{253, 169, 205},{255, 196, 123},{253, 214, 200},{253, 192, 255},{249, 175, 255},{0, 220, 255}};
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES_NUM, SAMPLING_FREQ);

enum RhythmPage_E {
  RHYTHM_MODEL_1,
  RHYTHM_MODEL_2,
  RHYTHM_MODEL_3,
  RHYTHM_MODEL_4,
  RHYTHM_MODEL_MAX
};
struct RhythmPage_S {
  uint16_t color;
  enum RhythmPage_E rhythmPage;
};

// anim page
int matrixArray[13][32]; // 点阵二维数组，用来记录一些数据
int animInterval1 = 60; // 动画1每一帧动画间隔
int animInterval2 = 80; // 动画2每一帧动画间隔
int animInterval3 = 100; // 动画3每一帧动画间隔
int hackAnimProbability = 8; // 骇客动画产生的概率，数值越大，新产生的下坠动画就越少
int lightedCount = 0; // 随机点动画已点亮的个数
bool increasing = true; // 随机点动画正在增加状态 

enum AnimPage_E {
  ANIM_MODEL_1,
  ANIM_MODEL_2,
  ANIM_MODEL_3,
  ANIM_MODEL_MAX
};
struct AnimPage_S {
  enum AnimPage_E animPage;
};

enum ClockChoosed_E {
  CLOCK_H,
  CLOCK_M,
  CLOCK_BELL,
  CLOCK_MAX
};

// clock
struct ClockData_S {
  int clockH;
  int clockM;
  int clockBellNum;
  bool clockOpen;
};

struct ClockPage_S {
  uint8_t is_bell;
  struct ClockData_S tmpData;
  struct ClockData_S targetData;
  uint16_t color;
  // uint16_t weekColor;
  enum ClockChoosed_E clockChoosed;
};

struct appDisplayData_S {
  enum DisplayPage_E dispalyPage;
  uint8_t brightness; // 5-145
  uint16_t color;

  // settings for the matrix
  struct displaySettings settings;
  // time page
  struct timePageData_S timePageData;
  // rhythm page
  struct RhythmPage_S rhythmPageData;
  // anim page
  struct AnimPage_S animPageData;
  // clock page
  struct ClockPage_S clockPageData;
};

static void whaleAnimStart(void);

static void whaleAnimStop(void);

static struct appDisplayData_S lastAppDisplayData;

static uint8_t is_power_on = 1; // 0-关机 1-开机，不要放入结构体中避免存放到 flash 中 导致上电黑屏

static struct appDisplayData_S appDisplayData = {
  .dispalyPage = TIME,
  .brightness = 100,
  .color = 0x001F,

  .settings = {
    .tick = 0,
    .color = 0x001F,
  },

  .timePageData = {
    .timePage = TIME_H_M_S,
    .color = 0x001F,
  },

  .rhythmPageData = {
    .rhythmPage = RHYTHM_MODEL_1
  },

  .animPageData = {
    .animPage = ANIM_MODEL_1
  },

  .clockPageData = {
    .is_bell = 0,
    .tmpData = {0},
    .targetData= {0},
    .color = 0x001F,
    .clockChoosed = CLOCK_H,
  }
};

static THREAD_HANDLE displayThreadHandle;
static THREAD_HANDLE songThreadHandle;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_HEIGHT, MATRIX_WIDTH,
  MATRIX_COUNT, 1,
  DATA_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE +
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE, NEO_GRB + 
  NEO_KHZ800);

static void storeRead(void)
{
  struct appDisplayData_S *readData;
  size_t len = 0;
  storeEasyMatrixGet((void **)&readData, &len);
  if (readData != NULL && len == sizeof(struct appDisplayData_S)) {
    PR_DEBUG("storeRead success");
    memcpy(&appDisplayData, readData, sizeof(struct appDisplayData_S));
    storeEasyMatrixFree(readData);
  }
  memcpy(&lastAppDisplayData, &appDisplayData, sizeof(struct appDisplayData_S));
}

static void autoSave(void)
{
  if (memcmp(&appDisplayData, &lastAppDisplayData, sizeof(struct appDisplayData_S)) == 0) {
    return;
  }

  memcpy(&lastAppDisplayData, &appDisplayData, sizeof(struct appDisplayData_S));

  PR_DEBUG("autoSave");

  storeEasyMatrix(&appDisplayData, sizeof(struct appDisplayData_S));
}

static void fillShow(uint16_t color)
{
  matrix.setCursor(0, 0);
  matrix.fillScreen(color);
  matrix.show();
}

static void drawText(int x, int y, uint16_t color, String text)
{
  matrix.fillScreen(0);
  matrix.setTextColor(color);
  matrix.setCursor(x,y);
  matrix.print(text);
  matrix.show();
}

static void drawSetting(uint8_t is_first)
{
  static uint8_t count = 0;

  uint16_t color = appDisplayData.settings.color;

  if (is_first) {
    appDisplayData.settings.tick = 0;
    count = 0;
  } else {
    appDisplayData.settings.tick++;
  }

  if ((appDisplayData.settings.tick * APP_DISPLAY_DELAY) % 1000 != 0) {
    return;
  }

  count++;
  if (count == 3) {
    count = 0;
  }

  if (whaleAnimTicker.active()) {
    whaleAnimStop();
  }

  // brightness set to max
  matrix.setBrightness(145);

  switch (count % 3) {
    case 0:
      drawText(2, 6, color, "START.");
      break;
    case 1:
      drawText(2, 6, color, "START..");
      break;
    case 2:
      drawText(2, 6, color, "START...");
      break;
    default:
      break;
  }

  return;
}

// time page start
static void whaleAnim()
{
  static uint8_t animIndex = 0;
  switch(animIndex){
      case 0:
        matrix.drawRGBBitmap(0,0,timeAnim0,11,8);
        break;
      case 1:
        matrix.drawRGBBitmap(0,0,timeAnim1,11,8);
        break;
      case 2:
        matrix.drawRGBBitmap(0,0,timeAnim2,11,8);
        break;
      case 3:
        matrix.drawRGBBitmap(0,0,timeAnim3,11,8);
        break;
      case 4:
        matrix.drawRGBBitmap(0,0,timeAnim4,11,8);
        break;
      case 5:
        matrix.drawRGBBitmap(0,0,timeAnim5,11,8);
        break;
      case 6:
        matrix.drawRGBBitmap(0,0,timeAnim6,11,8);
        break;
      case 7:
        matrix.drawRGBBitmap(0,0,timeAnim7,11,8);
        break;
      case 8:
        matrix.drawRGBBitmap(0,0,timeAnim8,11,8);
        break;
      case 9:
        matrix.drawRGBBitmap(0,0,timeAnim0,11,8);
        break;  
      default:
        animIndex = 9;
        break;
    }
    matrix.show();
    animIndex++;
    if(animIndex == 10){
      animIndex = 0;
    }
}

static void whaleAnimStart(void)
{
  whaleAnimTicker.attach_ms(WHALE_ANIM_INTERVAL, whaleAnim);
}

static void whaleAnimStop(void)
{
  whaleAnimTicker.detach();
}

static void drawTimePage(struct timePageData_S *timePageData, uint8_t is_first)
{
  OPERATE_RET rt = OPRT_OK;
  static enum TimePage_E lastTimePage = TIME_MAX;
  uint16_t color = timePageData->color;

  if (is_first) {
    lastTimePage = TIME_MAX;
  }

  if (lastTimePage != timePageData->timePage) {
    lastTimePage = timePageData->timePage;
    memset(&lastTimeinfo, 0, sizeof(POSIX_TM_S));
    // tm_wday 永远不会是 -1,这样可以确保切页后刷新一次
    lastTimeinfo.tm_wday = -1;
    fillShow(0x0000);
  }

  POSIX_TM_S curTimeinfo;

  rt = tal_time_get_local_time_custom(0, &curTimeinfo);
  if (OPRT_OK != rt) {
    PR_ERR("tal_time_get_local_time_custom failed");
    return;
  }

  matrix.setTextColor(color);

  switch (timePageData->timePage) {
    case (TIME_H_M_S): {
      if (whaleAnimTicker.active()) {
        whaleAnimStop();
      }

      if (lastTimeinfo.tm_hour == curTimeinfo.tm_hour && \
          lastTimeinfo.tm_min == curTimeinfo.tm_min && \
          lastTimeinfo.tm_sec == curTimeinfo.tm_sec && \
          lastTimeinfo.tm_wday == curTimeinfo.tm_wday){
        break;
      }
      lastTimeinfo = curTimeinfo;

      matrix.fillScreen(0);
      // draw time
      matrix.setCursor(2, 5);
      String h;
      if (curTimeinfo.tm_hour < 10) {
        h = "0" + String(curTimeinfo.tm_hour);
      } else {
        h = String(curTimeinfo.tm_hour);
      }
      String m;
      if (curTimeinfo.tm_min < 10) {
        m = "0" + String(curTimeinfo.tm_min);
      } else {
        m = String(curTimeinfo.tm_min);
      }
      String s;
      if (curTimeinfo.tm_sec < 10) {
        s = "0" + String(curTimeinfo.tm_sec);
      } else {
        s = String(curTimeinfo.tm_sec);
      }
      matrix.print(h + ":" + m + ":" + s);

      // draw week
      int weekNum = curTimeinfo.tm_wday;
      if(weekNum == 0){
        weekNum = 7;
      }
      for (int i=1; i<=7; i++) {
        matrix.drawFastHLine(2 + (i - 1) * 4, 7, 3, (i == weekNum) ? (~(color)) : color);
      }
      matrix.show();
    } break;
    case (TIME_H_M): {
      if (!whaleAnimTicker.active()) {
        whaleAnimStart();
      }

      if (lastTimeinfo.tm_hour == curTimeinfo.tm_hour && \
          lastTimeinfo.tm_min == curTimeinfo.tm_min && \
          lastTimeinfo.tm_wday == curTimeinfo.tm_wday){
        break;
      }
      lastTimeinfo = curTimeinfo;

      // clear time area
      matrix.fillRect(12, 0, 19, 8, 0);
      matrix.setCursor(14, 5);
      String h;
      if (curTimeinfo.tm_hour < 10) {
        h = "0" + String(curTimeinfo.tm_hour);
      } else {
        h = String(curTimeinfo.tm_hour);
      }
      String m;
      if (curTimeinfo.tm_min < 10) {
        m = "0" + String(curTimeinfo.tm_min);
      } else {
        m = String(curTimeinfo.tm_min);
      }
      matrix.print(h + ":" + m);

      // draw week
      int weekNum = curTimeinfo.tm_wday;
      if(weekNum == 0){
        weekNum = 7;
      }
      for (int i=1; i<=7; i++) {
        matrix.drawFastHLine(12 + (i - 1) * 3, 7, 2, (i == weekNum) ? (~(color)) : color);
      }
      matrix.show();
    } break;
    case (TIME_DATE): {
      if (!whaleAnimTicker.active()) {
        whaleAnimStart();
      }

      if (lastTimeinfo.tm_mon == curTimeinfo.tm_mon && \
          lastTimeinfo.tm_mday == curTimeinfo.tm_mday && \
          lastTimeinfo.tm_wday == curTimeinfo.tm_wday){
        break;
      }
      lastTimeinfo = curTimeinfo;

      // clear time area
      matrix.fillRect(12, 0, 19, 8, 0);
      // write date
      matrix.setCursor(12, 5);
      String month = curTimeinfo.tm_mon < 9 ? "0" : "";
      month = month + (curTimeinfo.tm_mon + 1);
      String day = curTimeinfo.tm_mday < 10 ? "0" : "";
      day = day + curTimeinfo.tm_mday;
      matrix.print(month + "-" + day);

      // draw week
      int weekNum = curTimeinfo.tm_wday;
      if(weekNum == 0){
        weekNum = 7;
      }
      for (int i=1; i<=7; i++) {
        matrix.drawFastHLine(12 + (i - 1) * 3, 7, 2, (i == weekNum) ? (~(color)) : color);
      }
      matrix.show();
    } break;
  }

  

  return;
}

void timePageNext(void)
{
  whaleAnimStop();

  switch (appDisplayData.timePageData.timePage) {
    case (TIME_H_M_S): appDisplayData.timePageData.timePage = TIME_H_M; break;
    case (TIME_H_M): appDisplayData.timePageData.timePage = TIME_DATE; break;
    case (TIME_DATE): appDisplayData.timePageData.timePage = TIME_H_M_S; break;
    default: break;
  }
}

void timePagePrev(void)
{
  whaleAnimStop();

  switch (appDisplayData.timePageData.timePage) {
    case (TIME_H_M_S): appDisplayData.timePageData.timePage = TIME_DATE; break;
    case (TIME_H_M): appDisplayData.timePageData.timePage = TIME_H_M_S; break;
    case (TIME_DATE): appDisplayData.timePageData.timePage = TIME_H_M; break;
    default: break;
  }
}
// time page end

// RHYTHM light start
// HSV转RGB格式
uint16_t hsv2rgb(uint16_t hue, uint8_t saturation, uint8_t value){
  uint8_t r, g, b;
  uint16_t h = (hue / 60) % 6;
  uint16_t F = 100 * hue / 60 - 100 * h;
  uint16_t P = value * (100 - saturation) / 100;
  uint16_t Q = value * (10000 - F * saturation) / 10000;
  uint16_t T = value * (10000 - saturation * (100 - F)) / 10000;
  switch (h){
    case 0:
        r = value;
        g = T;
        b = P;
        break;
    case 1:
        r = Q;
        g = value;
        b = P;
        break;
    case 2:
        r = P;
        g = value;
        b = T;
        break;
    case 3:
        r = P;
        g = Q;
        b = value;
        break;
    case 4:
        r = T;
        g = P;
        b = value;
        break;
    case 5:
        r = value;
        g = P;
        b = Q;
        break;
    default:
      return matrix.Color(255, 0, 0);
  }
  r = r * 255 / 100;
  g = g * 255 / 100;
  b = b * 255 / 100;
  return matrix.Color(r, g, b);
}

void drawRhythmPage(struct RhythmPage_S *pageData, uint8_t is_first)
{
  matrix.clear();
  memset(bandValues, 0, sizeof(bandValues));
  // sample
  for (int i = 0; i < SAMPLES_NUM; i++) {
    starTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN);
    vImag[i] = 0;
    // Serial.println(micros() - starTime);
    // while ((micros() - starTime) < sampling_period_us) { /* chill */ }
    // Serial.println(vReal[i]);
  }
  // fft compute
  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
  // parse result
  for (int i = 2; i < (SAMPLES_NUM/2); i++){
    if (vReal[i] > NOISE) {
      // Serial.println(vReal[i]);
      // 去除前面6段低频杂音和一些高频尖叫
      if (i>6    && i<=9   ) bandValues[0]   += (int)vReal[i];
      if (i>9    && i<=11  ) bandValues[1]   += (int)vReal[i];
      if (i>11   && i<=13  ) bandValues[2]   += (int)vReal[i];
      if (i>13   && i<=15  ) bandValues[3]   += (int)vReal[i];
      if (i>15   && i<=17  ) bandValues[4]   += (int)vReal[i];
      if (i>17   && i<=19  ) bandValues[5]   += (int)vReal[i];
      if (i>19   && i<=21  ) bandValues[6]   += (int)vReal[i];
      if (i>21   && i<=23  ) bandValues[7]   += (int)vReal[i];
      if (i>23   && i<=25  ) bandValues[8]   += (int)vReal[i];
      if (i>25   && i<=27  ) bandValues[9]   += (int)vReal[i];
      if (i>27   && i<=29  ) bandValues[10]  += (int)vReal[i];
      if (i>29   && i<=31  ) bandValues[11]  += (int)vReal[i];
      if (i>31   && i<=33  ) bandValues[12]  += (int)vReal[i];
      if (i>33   && i<=35  ) bandValues[13]  += (int)vReal[i];
      if (i>35   && i<=38  ) bandValues[14]  += (int)vReal[i];
      if (i>38   && i<=41  ) bandValues[15]  += (int)vReal[i];
      if (i>41   && i<=44  ) bandValues[16]  += (int)vReal[i];
      if (i>44   && i<=47  ) bandValues[17]  += (int)vReal[i];
      if (i>47   && i<=50  ) bandValues[18]  += (int)vReal[i];
      if (i>50   && i<=53  ) bandValues[19]  += (int)vReal[i];
      if (i>53   && i<=56  ) bandValues[20]  += (int)vReal[i];
      if (i>56   && i<=59  ) bandValues[21]  += (int)vReal[i];
      if (i>59   && i<=62  ) bandValues[22]  += (int)vReal[i];
      if (i>62   && i<=65  ) bandValues[23]  += (int)vReal[i];
      if (i>65   && i<=68  ) bandValues[24]  += (int)vReal[i];
      if (i>68   && i<=71  ) bandValues[25]  += (int)vReal[i];
      if (i>71   && i<=74  ) bandValues[26]  += (int)vReal[i];
      if (i>74   && i<=77  ) bandValues[27]  += (int)vReal[i];
      if (i>77   && i<=80  ) bandValues[28]  += (int)vReal[i];
      if (i>80   && i<=83  ) bandValues[29]  += (int)vReal[i];
      if (i>83   && i<=87  ) bandValues[30]  += (int)vReal[i];
      if (i>87   && i<=91  ) bandValues[31]  += (int)vReal[i];
    }
  }
  // 将FFT数据处理为条形高度
  int color = 0;
  int r, g, b;
  for (uint8_t band = 0; band < FREQ_BAND_NUM; band++) {
    // 根据倍率缩放条形图高度
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > MATRIX_TOTAL_HEIGHT) barHeight = MATRIX_TOTAL_HEIGHT;
    // 旧的高度值和新的高度值平均一下
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;
    // 如果条形的高度大于顶点高度，则调整顶点高度
    if (barHeight > peak[band]) {
      peak[band] = min(MATRIX_TOTAL_HEIGHT - 0, barHeight);
    }
    // 绘制操作
    switch (pageData->rhythmPage){
      case RHYTHM_MODEL_1:
        // 绘制条形
        matrix.drawFastVLine((MATRIX_TOTAL_WIDTH - 1 - band), (MATRIX_TOTAL_HEIGHT - barHeight), barHeight, hsv2rgb(color,100,100));
        color += 360 / (FREQ_BAND_NUM + 4);
        // 绘制顶点
        matrix.drawPixel((MATRIX_TOTAL_WIDTH - 1 - band), MATRIX_TOTAL_HEIGHT - peak[band] - 1, matrix.Color(150,150,150));
        break;  
      case RHYTHM_MODEL_2:
        // 绘制条形
        r = model2ColorArray[0][0];
        g = model2ColorArray[0][1];
        b = model2ColorArray[0][2];
        for (int y = MATRIX_TOTAL_HEIGHT; y >= MATRIX_TOTAL_HEIGHT - barHeight; y--) {
          matrix.drawPixel((MATRIX_TOTAL_WIDTH - 1 - band), y, matrix.Color(r, g, b));
          // if (barHeight > 0) {
          //   r+=(model2ColorArray[1][0] - model2ColorArray[0][0]) / barHeight;
          //   g+=(model2ColorArray[1][1] - model2ColorArray[0][1]) / barHeight;
          //   b+=(model2ColorArray[1][2] - model2ColorArray[0][2]) / barHeight;
          // }
          barHeight = (barHeight > 0) ? barHeight : 1;
          r+=(model2ColorArray[1][0] - model2ColorArray[0][0]) / barHeight;
          g+=(model2ColorArray[1][1] - model2ColorArray[0][1]) / barHeight;
          b+=(model2ColorArray[1][2] - model2ColorArray[0][2]) / barHeight;
        }
        // 绘制顶点
        matrix.drawPixel((MATRIX_TOTAL_WIDTH - 1 - band), MATRIX_TOTAL_HEIGHT - peak[band] - 1, matrix.Color(150,150,150));
        break;
      case RHYTHM_MODEL_3:
        // 绘制条形,此模式不绘制顶点
        for (int y = MATRIX_TOTAL_HEIGHT; y >= MATRIX_TOTAL_HEIGHT - barHeight; y--) {
          matrix.drawPixel((MATRIX_TOTAL_WIDTH - 1 - band), y, hsv2rgb(y * (255 / MATRIX_TOTAL_WIDTH / 5) + colorTime, 100, 100));
        }
        break;
      case RHYTHM_MODEL_4:
        // 此模式下，只绘制顶点
        matrix.drawPixel((MATRIX_TOTAL_WIDTH - 1 - band), (MATRIX_TOTAL_HEIGHT - peak[band] - 1), 
        matrix.Color(model4ColorArrar[peak[band]][0], model4ColorArrar[peak[band]][1], model4ColorArrar[peak[band]][2]));
        break;  
      default:
        break;
    }   
    // 将值记录到oldBarHeights
    oldBarHeights[band] = barHeight;
  }
  // 70毫秒降低一次顶点
  if((millis() - peekDecayTime) >= 70){
    for (byte band = 0; band < FREQ_BAND_NUM; band++){
      if (peak[band] > 0) peak[band] -= 1;
    }
    colorTime++;
    peekDecayTime = millis();
  }
  // 10毫秒变换一次颜色
  if ((millis() - changeColorTime) >= 10) {
    colorTime++;
    changeColorTime = millis();
  }
  matrix.show();
}

void rhythmPageNext(void)
{
  switch (appDisplayData.rhythmPageData.rhythmPage) {
    case (RHYTHM_MODEL_1): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_2; break;
    case (RHYTHM_MODEL_2): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_3; break;
    case (RHYTHM_MODEL_3): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_4; break;
    case (RHYTHM_MODEL_4): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_1; break;
    default: break;
  }
}

void rhythmPagePrev(void)
{
  switch (appDisplayData.rhythmPageData.rhythmPage) {
    case (RHYTHM_MODEL_1): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_4; break;
    case (RHYTHM_MODEL_2): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_1; break;
    case (RHYTHM_MODEL_3): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_2; break;
    case (RHYTHM_MODEL_4): appDisplayData.rhythmPageData.rhythmPage = RHYTHM_MODEL_3; break;
    default: break;
  }
}
// RHYTHM light end

// ANIM start
void drawAnimPage(struct AnimPage_S *pageData, uint8_t is_first)
{
  static enum AnimPage_E lastAnimPage = ANIM_MODEL_MAX;
  static unsigned long animTime = 0; // 记录上一次动画的时间 

  if (lastAnimPage != pageData->animPage) {
    lastAnimPage = pageData->animPage;
    matrix.fillScreen(0);
    lightedCount = 0;
    memset(matrixArray, 0, sizeof(matrixArray));
  }

  switch (pageData->animPage) {
    case ANIM_MODEL_1: {
      if((millis() - animTime) < animInterval1) return;
      matrix.fillScreen(0);
      for(int i = 0; i < MATRIX_COUNT * MATRIX_WIDTH; i++) {
        int animCount = 0; // 记录往下移一格后这一列还有几条动画存在（矩阵高度为8，动画高度为6，所以一列最多存在两条动画）
        int index1 = 0; // 记录往下移一格后第一个动画的头部位置
        int index2 = 0; // 记录往下移一格后第二个动画的头部位置
        // 根据矩阵二维数组判断这一列是否已经有动画，动画的起始位置（最上端）的值为1
        for(int j = 0; j < 13; j++){
          if(matrixArray[j][i] == 1){ // 这个位置是有动画的
            // 把这个动画往下移一格
            if((j + 1) <= 12){
              matrix.drawRGBBitmap(i,(j + 1 - 5),animHack,1,6);
              animCount++;
              if(animCount == 1){
                index1 = j + 1;
              }
              if(animCount == 2){
                index2 = j + 1;
              }
            }
          }
          // 将这个位置置为0
          matrixArray[j][i] = 0;
        }
        // 将新位置的值置为1
        if(animCount == 2){
          matrixArray[index1][i] = 1;
          matrixArray[index2][i] = 1;
        }else if(animCount == 1){
          matrixArray[index1][i] = 1;
        }
        int x = random(0, hackAnimProbability); // 随机数，如果是1则产生新动画
        if(x == 1){
          // 如果这一列没有动画，或者只有1个动画，并且这条动画与这条新生成的动画距离>=（6+1），就进入下面的判断
          if(animCount == 0 || (animCount == 1 && index1 >= 7)){
            // 判断左边同一行有没有动画，有的话也不生成新动画
            if(i == 0 || matrixArray[0][i - 1] == 0){
              matrix.drawRGBBitmap(i,-5,animHack,1,6);
              matrixArray[0][i] = 1;
            }
          }
        }
      }
    } break;
    case ANIM_MODEL_2: {
      if((millis() - animTime) < animInterval2) return;
      // matrixArray[0][0]记录大步骤，0：红色→黄色 1：黄色→绿色 2：绿色→蓝色 3：蓝色→紫色 4：紫色→红色
      // matrixArray[0][1]记录每一个大步骤的小步,每一种颜色渐变过程持续50步
      // matrixArray[0][2]记录颜色R
      // matrixArray[0][3]记录颜色G
      // matrixArray[0][4]记录颜色B
      if(matrixArray[0][0] == 0 && matrixArray[0][1] == 0){ // 刚开始设置为红色，初始值250
        matrixArray[0][2] = 250;
      }
      // 循环绘制竖线
      int tmpR = matrixArray[0][2];
      int tmpG = matrixArray[0][3];
      int tmpB = matrixArray[0][4];
      int tmpStep = matrixArray[0][0];
      for(int i = 0; i < MATRIX_WIDTH * MATRIX_COUNT; i++){
        matrix.drawFastVLine(i, 0, 8, matrix.Color(tmpR, tmpG, tmpB));
        // 计算出下一个循环的颜色
        if(i != MATRIX_WIDTH * MATRIX_COUNT - 1){
          if(tmpStep == 0){ // 红色→黄色
            tmpG = tmpG + 5;
            if(tmpG == 250){
              tmpStep++;
            }
          }else if(tmpStep == 1){ // 黄色→绿色
            tmpR = tmpR - 5;
            if(tmpR == 0){
              tmpStep++;
            }
          }else if(tmpStep == 2){ // 绿色→蓝色
            tmpG = tmpG - 5;
            tmpB = tmpB + 5;
            if(tmpG == 0){
              tmpStep++;
            }
          }else if(tmpStep == 3){ // 蓝色→紫色
            tmpR = tmpR + 5;
            if(tmpR == 250){
              tmpStep++;
            }
          }else if(tmpStep == 4){ // 紫色→红色
            tmpB = tmpB - 5;
            if(tmpB == 0){
              tmpStep = 0;
            }
          }
        }
      }
      if(matrixArray[0][0] == 0){ // 红色→黄色
        matrixArray[0][3] = matrixArray[0][3] + 5;
      }else if(matrixArray[0][0] == 1){ // 黄色→绿色
        matrixArray[0][2] = matrixArray[0][2] - 5;
      }else if(matrixArray[0][0] == 2){ // 绿色→蓝色
        matrixArray[0][3] = matrixArray[0][3] - 5;
        matrixArray[0][4] = matrixArray[0][4] + 5;
      }else if(matrixArray[0][0] == 3){ // 蓝色→紫色
        matrixArray[0][2] = matrixArray[0][2] + 5;
      }else if(matrixArray[0][0] == 4){ // 紫色→红色
        matrixArray[0][4] = matrixArray[0][4] - 5;
      }
      matrixArray[0][1] = matrixArray[0][1] + 1;
      if(matrixArray[0][1] == 50){
        matrixArray[0][1] = 0;
        matrixArray[0][0] = matrixArray[0][0] + 1;
        if(matrixArray[0][0] == 5){
          matrixArray[0][0] = 0;
        }
      }
    } break;
    case ANIM_MODEL_3: {
      if((millis() - animTime) < animInterval3) return;
      // 计算是填充过程还是消失过程
      if(lightedCount == 0){
        increasing = true;
      }else if(lightedCount == PIXEL_NUMBER){
        increasing = false;
      }
      if(increasing){
        int num = random(1, 4); // 每次随机填充num个点
        if(num + lightedCount > PIXEL_NUMBER){ // 不能超过灯珠总数量
          num = PIXEL_NUMBER - lightedCount;
        }
        for(int i = 0; i < num; i++){
          int x = random(0, 32);
          int y = random(0, 8);
          while(matrixArray[y][x] == 1){ // 这个随机生成的坐标已经点亮
            x = random(0, 32);
            y = random(0, 8);
          }
          matrixArray[y][x] = 1;
          // 已点亮灯的个数 + 1
          lightedCount++;
          int r = random(10, 256);
          int g = random(10, 256);
          int b = random(10, 256);
          matrix.drawPixel(x, y, matrix.Color(r, g, b));
        }
      }else{
        int num = random(1, 4); // 每次随机消失num个点
        if(num > lightedCount){ // 不能大于剩余已点亮数量
          num = lightedCount;
        }
        for(int i = 0; i < num; i++){
          int x = random(0, 32);
          int y = random(0, 8);
          while(matrixArray[y][x] == 0){ // 这个随机生成的坐标已经熄灭
            x = random(0, 32);
            y = random(0, 8);
          }
          matrixArray[y][x] = 0;
          // 已点亮灯的个数 - 1
          lightedCount--;
          matrix.drawPixel(x, y, 0);
        }
      }
    } break;
    default:break;
  }
  matrix.show();
  animTime = millis();
}

void animPageNext(void)
{
  switch (appDisplayData.animPageData.animPage) {
    case (ANIM_MODEL_1): appDisplayData.animPageData.animPage = ANIM_MODEL_2; break;
    case (ANIM_MODEL_2): appDisplayData.animPageData.animPage = ANIM_MODEL_3; break;
    case (ANIM_MODEL_3): appDisplayData.animPageData.animPage = ANIM_MODEL_1; break;
    default: break;
  }
}

void animPagePrev(void)
{
  switch (appDisplayData.animPageData.animPage) {
    case (ANIM_MODEL_1): appDisplayData.animPageData.animPage = ANIM_MODEL_3; break;
    case (ANIM_MODEL_2): appDisplayData.animPageData.animPage = ANIM_MODEL_1; break;
    case (ANIM_MODEL_3): appDisplayData.animPageData.animPage = ANIM_MODEL_2; break;
    default: break;
  }
}

// ANIM end

// clock start
void drawClockPage(struct ClockPage_S *pageData, uint8_t is_first)
{
  matrix.fillScreen(0);

  if (pageData->tmpData.clockOpen) {
    matrix.setCursor(2, 5);
    String h;
    if (pageData->tmpData.clockH < 10) {
      h = "0" + String(pageData->tmpData.clockH);
    } else {
      h = String(pageData->tmpData.clockH);
    }
    String m;
    if (pageData->tmpData.clockM < 10) {
      m = "0" + String(pageData->tmpData.clockM);
    } else {
      m = String(pageData->tmpData.clockM);
    }
    matrix.print(h + ":" + m);
    // 绘制闹钟图标
    matrix.drawBitmap(22, 1, bell, 8, 6, pageData->color);
    // 绘制指示线
    if(pageData->clockChoosed == ClockChoosed_E::CLOCK_H){
      matrix.drawFastHLine(2, 7, 7, (~(pageData->color)));
    }else if(pageData->clockChoosed == ClockChoosed_E::CLOCK_M){
      matrix.drawFastHLine(12, 7, 7, (~(pageData->color)));
    }else if(pageData->clockChoosed == ClockChoosed_E::CLOCK_BELL){
      matrix.drawFastHLine(22, 7, 8, (~(pageData->color)));
    }  
  } else {
    matrix.drawBitmap(8, 1, bell, 8, 6, pageData->color);
    matrix.setCursor(20, 6);
    matrix.setTextColor(matrix.Color(255, 0, 0));
    matrix.print("X");  
  }

  matrix.show();
}

void clockStateSet(uint8_t is_open)
{
  if (is_open) {
    appDisplayData.clockPageData.tmpData.clockOpen = 1;
  } else {
    appDisplayData.clockPageData.tmpData.clockOpen = 0;
  }
}

uint8_t clockStateGet(void)
{
  return appDisplayData.clockPageData.tmpData.clockOpen;
}

void clockChooseModeInc(void)
{
  if (!appDisplayData.clockPageData.tmpData.clockOpen) {
    return;
  }

  if (appDisplayData.clockPageData.clockChoosed == CLOCK_H) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_M;
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_M) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_BELL;
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_BELL) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_H;
  }
}

void clockChooseModeDec(void)
{
  if (!appDisplayData.clockPageData.tmpData.clockOpen) {
    return;
  }

  if (appDisplayData.clockPageData.clockChoosed == CLOCK_H) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_BELL;
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_M) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_H;
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_BELL) {
    appDisplayData.clockPageData.clockChoosed = CLOCK_M;
  }
}

void clockNumberInc(void)
{
  if (!appDisplayData.clockPageData.tmpData.clockOpen) {
    return;
  }

  if (appDisplayData.clockPageData.clockChoosed == CLOCK_H) {
    if (appDisplayData.clockPageData.tmpData.clockH < 23) {
      appDisplayData.clockPageData.tmpData.clockH++;
    } else {
      appDisplayData.clockPageData.tmpData.clockH = 0;
    }
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_M) {
    if (appDisplayData.clockPageData.tmpData.clockM < 59) {
      appDisplayData.clockPageData.tmpData.clockM++;
    } else {
      appDisplayData.clockPageData.tmpData.clockM = 0;
    }
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_BELL) {
    // TODO:
  }
}

void clockNumberDec(void)
{
  if (!appDisplayData.clockPageData.tmpData.clockOpen) {
    return;
  }

  if (appDisplayData.clockPageData.clockChoosed == CLOCK_H) {
    if (appDisplayData.clockPageData.tmpData.clockH > 0) {
      appDisplayData.clockPageData.tmpData.clockH--;
    } else {
      appDisplayData.clockPageData.tmpData.clockH = 23;
    }
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_M) {
    if (appDisplayData.clockPageData.tmpData.clockM > 0) {
      appDisplayData.clockPageData.tmpData.clockM--;
    } else {
      appDisplayData.clockPageData.tmpData.clockM = 59;
    }
  } else if (appDisplayData.clockPageData.clockChoosed == CLOCK_BELL) {
    // TODO:
  }
}

void clockSave(void)
{
  appDisplayData.clockPageData.targetData = appDisplayData.clockPageData.tmpData;
  PR_DEBUG("clockSave %d:%d", appDisplayData.clockPageData.targetData.clockH, appDisplayData.clockPageData.targetData.clockM);
}

// clock end

// brightness start
void drawbrightnessPage(uint8_t brightness, uint8_t is_first)
{
  uint16_t color = appDisplayData.color;

  if (appDisplayData.brightness == brightness && !is_first) {
    return;
  }
  appDisplayData.brightness = brightness;
  matrix.setBrightness(brightness);

  matrix.fillScreen(0);
  matrix.setTextColor(color);

  // 亮度图标
  matrix.drawFastVLine(13, 3, 2, color);
  matrix.drawPixel(14, 2, color);
  matrix.drawPixel(14, 5, color);
  matrix.drawPixel(15, 1, color);
  matrix.drawPixel(15, 6, color);
  matrix.drawFastVLine(16, 1, 6, color);
  matrix.drawFastVLine(17, 2, 4, color);
  matrix.drawFastVLine(18, 3, 2, color);

  if (appDisplayData.brightness > 5) {
    matrix.setCursor(3, 6);
    matrix.print("-");
  }

  if(appDisplayData.brightness < 145){
    matrix.setCursor(26, 6);
    matrix.print("+");
  }

  matrix.show();
}

void brightnessSet(uint8_t brightness)
{
  if (brightness > 145) {
    brightness = 145;
  }

  if (brightness < 5) {
    brightness = 5;
  }

  if (appDisplayData.dispalyPage == BRIGHT) {
    drawbrightnessPage(brightness, 0);
  } else {
    appDisplayData.brightness = brightness;
    matrix.setBrightness(brightness);
    matrix.show();
  }

  return;
}

void brightnessInc(void)
{
  uint8_t brightness = appDisplayData.brightness;

  if (brightness >= 145 - BRIGHTNESS_STEP) {
    brightness = 145;
  } else {
    brightness += BRIGHTNESS_STEP;
  }

  brightnessSet(brightness);
}

void brightnessDec(void)
{
  uint8_t brightness = appDisplayData.brightness;

  if (brightness <= 5 + BRIGHTNESS_STEP) {
    brightness = 5;
  } else {
    brightness -= BRIGHTNESS_STEP;
  }

  brightnessSet(brightness);
}

// brightness end

static void diaplayTask(void *args)
{
  static uint8_t last_power_state = 0xFF;
  static uint8_t is_first = 1;
  static enum DisplayPage_E lastPage = SETTING;

  uint16_t lastColor = 0;

  PR_DEBUG("displayTask start");

  storeRead();

  matrix.begin();
  matrix.setFont(&MyFont);
  matrix.setTextWrap(false);
  matrix.setBrightness(appDisplayData.brightness);

  // fillShow(matrix.Color(0, 255, 0));
  // delay(1000);

  for (;;) {
    if (!is_power_on) {
      if (last_power_state != 0) {
        lastPage = SETTING;
        last_power_state = 0;
        if (appDisplayData.dispalyPage == TIME) {
          whaleAnimStop();
        }
        // 可能是因为接触不良，有时候关闭，最后一个灯珠无法关闭
        // fillShow(matrix.Color(0, 0, 0)); // 关闭屏幕
        PR_DEBUG("displayTask power off");
      }
      fillShow(matrix.Color(0, 0, 0)); // 关闭屏幕
      delay(APP_DISPLAY_DELAY);
      continue;
    }
    last_power_state = 1;

    if (is_setting_page) {
      lastPage = SETTING;
      drawSetting(0);
      delay(APP_DISPLAY_DELAY);
      continue;
    }

    // auto save
    autoSave();

    if (lastPage != appDisplayData.dispalyPage) {
      is_first = 1;
      lastPage = appDisplayData.dispalyPage;
    }

    if (lastColor != appDisplayData.color) {
      lastColor = appDisplayData.color;
      matrix.setTextColor(appDisplayData.color);
      is_first = 1;
    }

    if (is_first) {
      fillShow(0x0000);
    }

    // PR_DEBUG("display page %d", appDisplayData.dispalyPage);

    switch (appDisplayData.dispalyPage) {
      case TIME:
        drawTimePage(&appDisplayData.timePageData, is_first);
        break;
      case RHYTHM:
        drawRhythmPage(&appDisplayData.rhythmPageData, is_first);
        break;
      case ANIM:
        drawAnimPage(&appDisplayData.animPageData, is_first);
        break;
      case CLOCK:
        drawClockPage(&appDisplayData.clockPageData, is_first);
        break;
      case BRIGHT:
        drawbrightnessPage(appDisplayData.brightness, is_first);
        break;
      default:
        break;
    }

    if (is_first) {
      is_first = 0;
    }

    delay(APP_DISPLAY_DELAY);
  }
}

uint8_t clockIsBell(void)
{
  return appDisplayData.clockPageData.is_bell;
}

void clockBellSet(uint8_t is_bell)
{
  PR_DEBUG("clockBellSet %d", is_bell);
  appDisplayData.clockPageData.is_bell = is_bell;
  if (!is_bell) {
    PR_DEBUG("clockBellSet stop");
    noTone(BUZZER_PIN);
  }
}

static void songTask(void *args)
{
  OPERATE_RET rt = OPRT_OK;
  POSIX_TM_S curTime;
  uint8_t checkClock = 0;

  for (;;) {
    if (!appDisplayData.clockPageData.targetData.clockOpen) {
      if (clockIsBell()) {
        clockBellSet(0);
      }
      tal_system_sleep(500);
      continue;
    }

    if (clockIsBell()) {
      playSongs(appDisplayData.clockPageData.targetData.clockBellNum);
      continue;
    }

    rt = tal_time_get_local_time_custom(0, &curTime);
    if (OPRT_OK != rt) {
      PR_ERR("tal_time_get_local_time_custom failed");
      tal_system_sleep(500);
      continue;
    }

    if (checkClock) {
      if (curTime.tm_hour != appDisplayData.clockPageData.targetData.clockH || \
          curTime.tm_min != appDisplayData.clockPageData.targetData.clockM) {
        checkClock = 0;
      }
    }

    if (curTime.tm_hour == appDisplayData.clockPageData.targetData.clockH && \
        curTime.tm_min == appDisplayData.clockPageData.targetData.clockM && \
        !checkClock) {
      checkClock = 1;
      clockBellSet(1);
      continue;
    }

    tal_system_sleep(500);
  }
}

void appDisplayInit(void)
{
  THREAD_CFG_T thrd_param = {1024 * 10, THREAD_PRIO_1, "displayTask"};
  tal_thread_create_and_start(&displayThreadHandle, NULL, NULL, diaplayTask, NULL, &thrd_param);

  thrd_param.stackDepth = 1024 * 4;
  thrd_param.priority = THREAD_PRIO_2;
  thrd_param.thrdname = "songTask";
  tal_thread_create_and_start(&songThreadHandle, NULL, NULL, songTask, NULL, &thrd_param);
}

void appDisplaySetting(void)
{
  appDisplayData.settings.tick = 0;
  is_setting_page = 1;
}

void appDisplaySettingExit(void)
{
  is_setting_page = 0;
}

void appDisplayPageSet(enum DisplayPage_E page)
{
  if (appDisplayData.dispalyPage == SETTING) {
    return; // 这里不能设置界面为 SETTING，要不然无法恢复到上一个界面
  }

  if (appDisplayData.dispalyPage == TIME) {
    whaleAnimStop();
  }

  appDisplayData.dispalyPage = page;
}

enum DisplayPage_E appDisplayPageGet(void)
{
  return appDisplayData.dispalyPage;
}

void appDisplayPageNext(void)
{
  switch (appDisplayData.dispalyPage) {
    case (TIME): {
      whaleAnimStop();
      appDisplayData.dispalyPage = RHYTHM; 
    }break;
    case (RHYTHM): appDisplayData.dispalyPage = ANIM; break;
    case (ANIM): {
      appDisplayData.clockPageData.clockChoosed = CLOCK_H;
      appDisplayData.dispalyPage = CLOCK;
    } break;
    case (CLOCK): appDisplayData.dispalyPage = BRIGHT; break;
    case (BRIGHT): appDisplayData.dispalyPage = TIME; break;
    default: appDisplayData.dispalyPage = TIME; break;
  }
}

void appDisplayPagePrev(void)
{
  switch (appDisplayData.dispalyPage) {
    case (TIME): {
      whaleAnimStop();
      appDisplayData.dispalyPage = BRIGHT;
    } break;
    case (BRIGHT): {
      appDisplayData.clockPageData.clockChoosed = CLOCK_H;
      appDisplayData.dispalyPage = CLOCK;
    } break;
    case (CLOCK): appDisplayData.dispalyPage = ANIM; break;
    case (ANIM): appDisplayData.dispalyPage = RHYTHM; break;
    case (RHYTHM): appDisplayData.dispalyPage = TIME; break;
    default: appDisplayData.dispalyPage = TIME; break;
  }
}

void powerStatusSet(uint8_t status)
{
  PR_DEBUG("powerStatusSet %d", status);
  is_power_on = status;
}

uint8_t powerStatusGet(void)
{
  PR_DEBUG("powerStatusGet %d", is_power_on);
  return is_power_on;
}

void colorSet(uint16_t h, uint16_t s, uint16_t v)
{
  appDisplayData.color = hsv2rgb(h, s, v);
  PR_DEBUG("colorSet %04x", appDisplayData.color);
  appDisplayData.timePageData.color = appDisplayData.color;
  appDisplayData.clockPageData.color = appDisplayData.color;

  uint32_t brightness = 0;
  brightness  = map(v, 0, 100, 5, 145);
  PR_DEBUG("brightness %d", brightness);
  appDisplayData.brightness = (uint8_t)brightness;
  matrix.setBrightness(appDisplayData.brightness);
}

void rgb2hsv(uint16_t color, uint16_t *h, uint16_t *s, uint16_t *v)
{
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  float r_norm = r / 31.0;
  float g_norm = g / 63.0;
  float b_norm = b / 31.0;

  float max = fmax(fmax(r_norm, g_norm), b_norm);
  float min = fmin(fmin(r_norm, g_norm), b_norm);
  float delta = max - min;

  // Calculate V
  *v = max * 1000;

  // Calculate S
  if (max != 0) {
    *s = (delta / max) * 1000;
  } else {
    *s = 0;
    *h = 0;
    return;
  }

  // Calculate H
  if (delta == 0) {
    *h = 0;
  } else if (max == r_norm) {
    *h = 60 * (fmod(((g_norm - b_norm) / delta), 6));
  } else if (max == g_norm) {
    *h = 60 * (((b_norm - r_norm) / delta) + 2);
  } else if (max == b_norm) {
    *h = 60 * (((r_norm - g_norm) / delta) + 4);
  }

  if (*h < 0) {
    *h += 360;
  }
}

String colorGet(void)
{
  char buf[13] = {0};
  // appDisplayData.color, 转换成 HSV 格式，hex，uint16, HHHHSSSSVVVV
  uint16_t h, s, v;
  rgb2hsv(appDisplayData.color, &h, &s, &v);

  // String color = String(h) + String(s) + String(v);
  snprintf(buf, sizeof(buf), "%04x%04x%04x", h, s, v);
  String color = String(buf);
  PR_DEBUG("colorGet %s", color.c_str());

  return color;
}
