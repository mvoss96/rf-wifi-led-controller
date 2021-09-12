#include <SPI.h>
#include <NRFLite.h>

const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;


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
  _radioData.FromRadioId = RADIO_ID;
}

void rfSleep() {
  //Serial.println("rfsleep");
  digitalWrite(PIN_RADIO_CE, LOW);
}

void rfWakeup() {
  //Serial.println("rfwakeup");
  digitalWrite(PIN_RADIO_CE, HIGH);
}

void rfSend(uint8_t type, uint8_t value) {
  _radioData.msgType = type;
  _radioData.value = value;

  //Serial.print("Sending ");
  //Serial.print(_radioData.msgType);
  //Serial.print(" ");
  //Serial.print(_radioData.value);
  if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))) // Note how '&' must be placed in front of the variable name.
  {
    //Serial.println("...Success");
    while (_radio.hasAckData()) // Look to see if the receiver provided the ACK data.
    {
      RadioPacket ackData;
      _radio.readData(&ackData);
      String msg = "  Received ACK data from id: ";
      msg += ackData.FromRadioId;
      msg += ", type: ";
      msg += ackData.msgType;
      msg += ", value: ";
      msg += ackData.value;
      msg += ", extra: ";
      msg += (int)ackData.extra[0];
      msg += " ";
      msg += (int)ackData.extra[1];
      //Serial.println(msg);
    }
  }
  else
  {
    //Serial.println("...Failed");
  }
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
  //Serial.println(msg);
}
