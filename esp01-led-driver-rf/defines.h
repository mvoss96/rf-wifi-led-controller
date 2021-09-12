/****************************************************************************************************************************
  esp01-led-driver
  For ESP8266 / ESP-01 boards
  Built by Marcus Voß
  Licensed under MIT license
 *****************************************************************************************************************************/

#if defined(ESP8266)
#define USE_LITTLEFS            true
#define ESP_DRD_USE_LITTLEFS    true
#else
#error This code is intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif
#include <ESPAsync_WiFiManager.h>               //https://github.com/khoih-prog/ESPAsync_WiFiManager
#define DRD_TIMEOUT             5
#define DRD_ADDRESS             0
#define REQUIRE_ONE_SET_SSID_PW true
#include <ESP_DoubleResetDetector.h>            //https://github.com/khoih-prog/ESP_DoubleResetDetector
#include <AsyncElegantOTA.h>


#define LED_CHANNEL1                 16
#define LED_CHANNEL2                 0
#define LED_MAX_POWER_FACTOR         1
#define MIN_BRIGHTNESS               5
