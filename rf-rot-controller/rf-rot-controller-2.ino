#include "Button2.h";
#include "ESPRotary.h";
#include "LowPower.h"

#define ROTARY_PIN1  3
#define ROTARY_PIN2 4
#define BUTTON_PIN  2
#define CLICKS_PER_STEP   2

#define TYPE_POWER  0
#define TYPE_BR_SET  1
#define TYPE_BR_DEC  2
#define TYPE_BR_INC  3
#define TYPE_CO_SET  4
#define TYPE_CO_DEC  5
#define TYPE_CO_INC  6
#define TYPE_PRESET  7

#define POWER_OFF 0
#define POWER_ON 1
#define POWER_TOGGLE 2
#define VALUE_PER_CLICK 8
#define SLEEP_AFTER 2000

ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
Button2 b = Button2(BUTTON_PIN);

long globalTimer;
bool sleeping;

void rfSetup();
void rfSleep();
void rfWakeup();
void rfSend(uint8_t type, uint8_t value);

void wakeUp()
{
  globalTimer = millis();
  if (sleeping) {
    sleeping = false;
    rfWakeup();
  }

}

void rotLeft() {
  //Serial.print("left ");
  bool pressed = !digitalRead(BUTTON_PIN);
  //Serial.println(pressed);
  if (pressed)
    rfSend(TYPE_CO_DEC, VALUE_PER_CLICK);
  else
    rfSend(TYPE_BR_DEC, VALUE_PER_CLICK);
}

void rotRight() {
  //Serial.println("right ");
  bool pressed = !digitalRead(BUTTON_PIN);
  //Serial.println(pressed);
  if (pressed)
    rfSend(TYPE_CO_INC, VALUE_PER_CLICK);
  else
    rfSend(TYPE_BR_INC, VALUE_PER_CLICK);
}

void rotClick() {
  rfSend(TYPE_POWER, POWER_TOGGLE);
}

void rotDoubleClick() {
  //Serial.println("doubleclick ");
  rfSend(TYPE_PRESET, 0);
}





void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("RF_Controller");
  rfSetup();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ROTARY_PIN1, INPUT);
  pinMode(ROTARY_PIN1, INPUT);

  r.setLeftRotationHandler(rotLeft);
  r.setRightRotationHandler(rotRight);
  b.setClickHandler(rotClick);
  b.setDoubleClickHandler(rotDoubleClick);

  attachInterrupt(0, wakeUp, FALLING);
  attachInterrupt(1, wakeUp, CHANGE);

  globalTimer = millis();
  sleeping = false;
}

void loop() {
  // put your main code here, to run repeatedly:
  r.loop();
  b.loop();

  if (millis() - globalTimer > SLEEP_AFTER) {
    sleeping = true;
    //Serial.println("sleep");
    rfSleep();
    //delay(10);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
}
