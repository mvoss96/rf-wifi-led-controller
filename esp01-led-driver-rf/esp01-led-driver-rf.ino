/****************************************************************************************************************************
  esp01-led-driver
  For ESP8266 / ESP-01 boards
  by Marcus Voß
  Licensed under MIT license
 *****************************************************************************************************************************/

#include "defines.h"
extern DoubleResetDetector* drd;

bool LEDpower = false;
uint8_t LEDbrightness = 127;
int LEDcolor = (LED_CHANNEL2 == -1) ? 100 : 50;
unsigned long int timer = millis();
bool      initialConfig = false;

void startServer();
void checkReset();
void rfSetup();
void rfListen();

///////////////////////////////////////////////////

void serialSetup1() {
  Serial.begin(115200); while (!Serial); delay(200); //wait for serial connection
  Serial.print(F("\nStarting up ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
  if (WiFi.SSID() == "") {
    Serial.println(F("No AP credentials"));
    initialConfig = true; //no config: need for setup
  }
}

void serialSetup2() {
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
  unsigned long startedAt = millis();
  Serial.print(F("After waiting "));
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis() - startedAt);
  Serial.print(waited / 1000); Serial.print(F(" secs , Connection result is ")); Serial.println(connRes);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect"));
  }
  else {
    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP());
  }
}

void serialReport() {
  if (millis() - timer > 5000){
    Serial.printf("power: %d brightness: %d color: %d\n", LEDpower, LEDbrightness, LEDcolor);
    timer = millis();
  }
}

void setLED(bool power, uint8_t led1, uint8_t led2) {
  if (power)
  {
    analogWrite(LED_CHANNEL1, led1 );
    analogWrite(LED_CHANNEL2, led2);
  }
  else
  {
    analogWrite(LED_CHANNEL1, 0);
    analogWrite(LED_CHANNEL2, 0);
  }
}



///////////////////////////////////////////////////////////////////////

void setup() {
  analogWriteRange(255);
  pinMode(LED_CHANNEL1, OUTPUT);
  pinMode(LED_CHANNEL2, OUTPUT);
  digitalWrite(LED_CHANNEL1, LOW);
  digitalWrite(LED_CHANNEL2, LOW);
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  serialSetup1();
  rfSetup();
  checkReset();
  delay(100);
  WiFi.softAPdisconnect(true);
  serialSetup2();
  startServer();
  digitalWrite(LED_CHANNEL1, HIGH);
  digitalWrite(LED_CHANNEL2, HIGH);
  delay(500);

}

void loop() {
  drd->loop(); //doubleReset detection background loop
  rfListen();
  serialReport();
  AsyncElegantOTA.loop();
  uint8_t led1 = (LEDcolor / 100.0) * LEDbrightness * LED_MAX_POWER_FACTOR;
  uint8_t led2 = (1 - LEDcolor / 100.0) * LEDbrightness * LED_MAX_POWER_FACTOR;
  setLED(LEDpower, led1, led2);
}
