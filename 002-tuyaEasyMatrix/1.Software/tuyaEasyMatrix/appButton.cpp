#include "appButton.h"
#include "appDisplay.h"
#include "TuyaIoT.h"

#include <Log.h>
#include "OneButton.h"

OneButton button1(BTN1_PIN, true);
OneButton button2(BTN2_PIN, true);
OneButton button3(BTN3_PIN, true);

static void btn1Click(void)
{
  PR_DEBUG("btn1 click");
  enum DisplayPage_E page = appDisplayPageGet();

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (page == TIME) {
    timePagePrev();
  } else if (page == RHYTHM) {
    rhythmPagePrev();
  } else if (page == ANIM) {
    animPagePrev();
  } else if (page == BRIGHT) {
    brightnessDec();
  } else if (page == CLOCK) {
    clockNumberDec();
  }

  return;
}

static void btn1LongClick(void)
{
  PR_DEBUG("btn1 long click");
  enum DisplayPage_E page = appDisplayPageGet();

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (page == CLOCK) {
    clockChooseModeDec();
  }

  return;
}

static void btn2Click(void)
{
  PR_DEBUG("btn2 click");
  enum DisplayPage_E page = appDisplayPageGet();

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (page == TIME) {
    timePageNext();
  } else if (page == RHYTHM) {
    rhythmPageNext();
  } else if (page == ANIM) {
    animPageNext();
  } else if (page == BRIGHT) {
    brightnessInc();
  } else if (page == CLOCK) {
    clockNumberInc();
  }

  return;
}

static void btn2LongClick(void)
{
  PR_DEBUG("btn2 long click");

  enum DisplayPage_E page = appDisplayPageGet();

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (page == CLOCK) {
    clockChooseModeInc();
  }

  return;
}

static uint8_t is_colock_setting = 0;

static void btn3Click(void)
{
  PR_DEBUG("btn3 click");

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (appDisplayPageGet() == CLOCK && is_colock_setting) {
    is_colock_setting = 0;
    clockSave();
  } else {
    appDisplayPageNext();
  }

  return;
}

static void btn3LongClick(void)
{
  PR_DEBUG("btn3 long click");

  enum DisplayPage_E page = appDisplayPageGet();

  if (clockIsBell()) {
    clockBellSet(0);
    PR_DEBUG("cancel bell");
    return;
  }

  if (!powerStatusGet()) {
    PR_DEBUG("power off");
    return;
  }

  if (page == CLOCK) {
    is_colock_setting = 1;
    clockStateSet(!clockStateGet());
    return;
  }

  TuyaIoT.remove();

  return;
}

void appButtonInit(void)
{
  button1.attachClick(btn1Click);
  button1.setDebounceMs(10);
  button1.attachLongPressStart(btn1LongClick);
  button1.setPressMs(1200);

  button2.attachClick(btn2Click);
  button2.setDebounceMs(10);
  button2.attachLongPressStart(btn2LongClick);
  button2.setPressMs(1200);

  button3.attachClick(btn3Click);
  button3.setDebounceMs(10);
  button3.attachLongPressStart(btn3LongClick);
  button3.setPressMs(1500);
}

void appButtonTick(void)
{
  button1.tick();
  button2.tick();
  button3.tick();
}
