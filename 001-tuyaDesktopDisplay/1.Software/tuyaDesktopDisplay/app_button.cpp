#include "app_button.h"

#include "OneButton.h"
#include "TuyaIoT.h"

#define buttonPin BUTTON_BUILTIN
OneButton button(buttonPin);

static void buttonClick()
{
  Serial.println("Button clicked");
}

static void buttonLongPressStart()
{
  Serial.println("Button long press, remove IoT device.");
  TuyaIoT.remove();
}

void app_button_init(void)
{
  button.attachClick(buttonClick);
  button.setPressMs(3*1000);
  button.attachLongPressStart(buttonLongPressStart);
}

void app_button_loop(void)
{
  button.tick();
}
