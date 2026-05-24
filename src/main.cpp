#include <Arduino.h>
#include <WebServer.h>

#include "gpio_rest_api.h"
#include "i2c_rest_api.h"
#include "wifi_provisioning.h"

namespace {
WebServer webServer(80);
bool apiActive = false;

void startApiMode() {
  apiActive = true;
  I2cRestApi::begin(webServer);
  GpioRestApi::begin(webServer);
  webServer.begin();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("[boot] wifi-i2c controller starting."));

  if (!WifiProvisioning::connectToConfiguredWifi()) {
    WifiProvisioning::startProvisioningMode(webServer);
  } else {
    Serial.println(F("[boot] Normal WiFi station mode active."));
    startApiMode();
  }
}

void loop() {
  WifiProvisioning::handleClient();

  if (apiActive) {
    webServer.handleClient();
  }
}
