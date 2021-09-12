#include <SPI.h>
#include <math.h>
#include <NRFLite.h>
#include "defines.h"

#define TYPE_POWER  0
#define TYPE_BR_SET  1
#define TYPE_BR_DEC  2
#define TYPE_BR_INC  3
#define TYPE_CO_SET  4
#define TYPE_CO_DEC  5
#define TYPE_CO_INC  6
#define TYPE_PRESET  7
#define TYPE_ACK_DATA  255

const static uint8_t RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE = 5;
const static uint8_t PIN_RADIO_CSN = 4;

extern bool LEDpower;
extern uint8_t LEDbrightness;
extern int LEDcolor;

struct __attribute__((packed)) RadioPacket // Any packet up to 32 bytes can be sent.
{
  uint8_t FromRadioId;
  uint8_t msgType;
  uint8_t value;
  byte extra[2];
};

NRFLite _radio;
RadioPacket _radioData;

void rfSetup() {
  while (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, 100))
  {
    Serial.println("Cannot communicate with radio");
    delay(1000);
  }
}

int convert(uint8_t input) {
  return ceil((input / 255.0) * 100);
}

void printRF(RadioPacket& pck);
void printRF(RadioPacket& pck) {
  String msg = "-> id: ";
  msg += pck.FromRadioId;
  msg += ", type: ";
  msg += pck.msgType;
  msg += ", value: ";
  msg += pck.value;
  msg += ", extra: ";
  msg += (int)pck.extra[0];
  msg += " ";
  msg += (int)pck.extra[1];
  Serial.println(msg);
}

int parseRF(RadioPacket& pck);
int parseRF(RadioPacket& pck) {
  switch (pck.msgType) {
      {
      case TYPE_POWER:
        if (pck.value == 0)
          LEDpower = false;
        else if (pck.value == 1)
          LEDpower = true;
        else if (pck.value == 2)
          LEDpower = (LEDpower) ? false : true;
        else
          return -1;
        return 0;
      }
    case TYPE_BR_SET:
      {
        LEDbrightness = pck.value;
        if (LEDbrightness < MIN_BRIGHTNESS)
          LEDbrightness = MIN_BRIGHTNESS;
        return 0;
      }
    case TYPE_BR_DEC:
      {
        if (LEDbrightness - pck.value >= MIN_BRIGHTNESS)
          LEDbrightness -= pck.value;
        else
          LEDbrightness = MIN_BRIGHTNESS;
        return 0;
      case TYPE_BR_INC:
        if (LEDbrightness + pck.value <= 255)
          LEDbrightness += pck.value;
        else
          LEDbrightness = 255;
        return 0;
      }
    case TYPE_CO_SET:
      {
        int newVal = convert(pck.value);
        if (newVal > 100)
          LEDcolor = 100;
        else if (newVal < 0)
          LEDcolor = 0;
        else
          LEDcolor = newVal;
        return 0;
      }
    case TYPE_CO_INC:
      {
        if (LEDcolor + convert(pck.value) > 100)
          LEDcolor = 100;
        else
          LEDcolor += convert(pck.value);
        return 0;
      }
    case TYPE_CO_DEC:
      {
        if (LEDcolor - convert(pck.value) < 0)
          LEDcolor = 0;
        else
          LEDcolor -= convert(pck.value);
        return 0;
      }
    case TYPE_PRESET:
      {
        LEDcolor = 50;
        return 0;
      }
    default:
      {
        return -1;
      }
  }
}


void rfListen() {
  while (_radio.hasData())
  {
    _radio.readData(&_radioData); // Note how '&' must be placed in front of the variable name.
    printRF(_radioData);
    if (parseRF(_radioData) == -1)
      Serial.println("Error parsing previous packet");
    RadioPacket ackData;
    ackData.FromRadioId = RADIO_ID;
    ackData.msgType = TYPE_ACK_DATA;
    ackData.value = LEDpower;
    ackData.extra[0] = LEDbrightness;
    ackData.extra[1] = (LEDcolor * 255) / 100;
    _radio.addAckData(&ackData, sizeof(ackData));
  }
}
