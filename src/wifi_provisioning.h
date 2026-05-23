#pragma once

#include <WebServer.h>

namespace WifiProvisioning {

bool connectToConfiguredWifi();
void startProvisioningMode(WebServer &server);
void handleClient();

}  // namespace WifiProvisioning
