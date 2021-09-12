/****************************************************************************************************************************
  esp01-led-driver
  For ESP8266 / ESP-01 boards
  Built by Marcus Voß
  Licensed under MIT license
 *****************************************************************************************************************************/
#include "defines.h"

AsyncWebServer webServer(80);
DNSServer dnsServer;
DoubleResetDetector* drd;

extern bool LEDpower;
extern uint8_t LEDbrightness;
extern int LEDcolor;

void checkReset() {
  if (drd->detectDoubleReset()) {
    Serial.println(F("DRD"));
    initialConfig = true;
  }
  if (initialConfig) {
    Serial.println(F("Starting Config Portal"));
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "LED-driver");
    ESPAsync_wifiManager.setConfigPortalTimeout(120);
    if (!ESPAsync_wifiManager.startConfigPortal("LED-driver-setup")) {
      Serial.println(F("Not connected to WiFi"));
    }
    else {
      Serial.println(F("connected"));
    }
  }
}

bool handleBodyPower(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  Serial.printf("[REQUEST POWER]\t%.*s\r\n", len, (const char*)data);
  if (memcmp(data, "off", 3) == 0 || memcmp(data, "OFF", 3) == 0)
  {
    LEDpower = false;
    return 1;
  }
  else if (memcmp(data, "on", 2) == 0 || memcmp(data, "ON", 2) == 0)
  {
    LEDpower = true;
    return 1;
  }
  else
  {
    return 0;
  }
}

bool handleBodyBrightness(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  data[len] = '\0';
  Serial.printf("[REQUEST BRIGHTNESS]\t%.*s\r\n", len, (const char*)data);
  int val = atoi((char *)data);
  Serial.printf("parsed: %d\n", val);
  if (val >= 0 && val <= 255)
  {
    LEDbrightness = val;
    return 1;
    if (LEDbrightness < MIN_BRIGHTNESS)
          LEDbrightness = MIN_BRIGHTNESS;
  }
  else
    return 0;
  {
  }
}

bool handleBodyColor(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  data[len] = '\0';
  Serial.printf("[REQUEST COLOR]\t%.*s\r\n", len, (const char*)data);
  if (LED_CHANNEL2 == -1) {
    return 0;
  }
  int val = atoi((char *)data);
  if (val >= 0 && val <= 100)
  {
    LEDcolor = val;
    return 1;
  }
  else
  {
    return 0;
  }
}

bool handleBodyColorMired(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  data[len] = '\0';
  Serial.printf("[REQUEST COLOR-MIRED]\t%.*s\r\n", len, (const char*)data);
  if (LED_CHANNEL2 == -1) {
    return 0;
  }
  int val = atoi((char *)data);
  if (val >= 50 && val <= 400)
  {
    LEDcolor = (int)((val - 50) / 3.5);
    return 1;
  }
  else
  {
    return 0;
  }
}

void startServer() {
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->redirect("/update");
  });
  webServer.on("/power", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send(200, "text/plain", (LEDpower) ? "on" : "off");
  });
  webServer.on("/brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    char msg[4];
    sprintf(msg, "%2d", LEDbrightness);
    request->send(200, "text/plain", msg);
  });
  webServer.on("/color", HTTP_GET, [](AsyncWebServerRequest * request) {
    char msg[4];
    if (LED_CHANNEL2 != -1) {
      sprintf(msg, "%2d", LEDcolor);
      request->send(200, "text/plain", msg);
    }
    else {
      request->send(400, "text/plain", "not supported");
    }
  });
  webServer.on("/color-mired", HTTP_GET, [](AsyncWebServerRequest * request) {
    char msg[4];
    if (LED_CHANNEL2 != -1) {
      sprintf(msg, "%2d", (int)(50 + 3.5 * LEDcolor));
      request->send(200, "text/plain", msg);
    }
    else {
      request->send(400, "text/plain", "not supported");
    }
  });

  webServer.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (request->url() == "/power") {
      if (!handleBodyPower(request, data, len)) request->send(400, "text/plain", "");
      request->send(200, "text/plain", "ok");
    }
    else if (request->url() == "/brightness") {
      if (!handleBodyBrightness(request, data, len)) request->send(400, "text/plain", "");
      request->send(200, "text/plain", "ok");
    }
    else if (request->url() == "/color") {
      if (!handleBodyColor(request, data, len)) request->send(400, "text/plain", "");
      request->send(200, "text/plain", "ok");
    }
    else if (request->url() == "/color-mired") {
      if (!handleBodyColorMired(request, data, len)) request->send(400, "text/plain", "");
      request->send(200, "text/plain", "ok");
    }

  });

  AsyncElegantOTA.begin(&webServer);    // Start ElegantOTA
  webServer.begin();
  Serial.println("OTA-server started");
}
