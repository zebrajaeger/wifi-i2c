## 1. Safety and Structure

- [x] 1.1 Run GitNexus impact analysis for `setup` before changing firmware startup behavior.
- [x] 1.2 Decide whether to keep the first implementation in `src/main.cpp` or split WiFi provisioning into dedicated source/header files.
- [x] 1.3 Add the required ESP32 Arduino includes for WiFi station/AP mode, DNS, HTTP serving, preferences storage, and restart handling.

## 2. Credential Storage

- [x] 2.1 Implement loading saved SSID and password from a dedicated `Preferences` namespace.
- [x] 2.2 Implement saving submitted SSID and password without logging the password over Serial.
- [x] 2.3 Add validation so empty SSID submissions are rejected and do not overwrite stored credentials.

## 3. Boot Connection Flow

- [x] 3.1 Implement bounded boot-time connection attempts when saved credentials exist.
- [x] 3.2 Continue normal controller operation when the station connection succeeds.
- [x] 3.3 Start provisioning mode when credentials are missing or the configured network is unreachable within the timeout.

## 4. Provisioning Access Point

- [x] 4.1 Start a recognizable provisioning access point when provisioning mode is active.
- [x] 4.2 Configure local IP, gateway, and DNS catch-all so connected clients resolve web requests to the controller.
- [x] 4.3 Keep DNS and HTTP request handling active from `loop()` while provisioning mode is running.

## 5. Captive Portal UI and Routes

- [x] 5.1 Implement a WiFi scan that sorts networks by RSSI descending and de-duplicates repeated SSIDs by strongest signal.
- [x] 5.2 Render the portal page with the sorted SSID list, password input, save action, and reboot action.
- [x] 5.3 Handle common captive portal detection paths by serving or redirecting to the portal.
- [x] 5.4 Handle credential form submission with success and error responses.
- [x] 5.5 Handle the reboot action with `ESP.restart()` after returning a response to the client.

## 6. Verification

- [x] 6.1 Build the firmware with PlatformIO or the project-standard command.
- [x] 6.2 Verify the no-credentials boot path starts the provisioning access point.
- [x] 6.3 Verify the portal lists nearby SSIDs sorted strongest to weakest.
- [x] 6.4 Verify saved credentials survive reboot and the controller connects automatically to the configured WiFi.
- [x] 6.5 Verify unreachable or wrong credentials return the controller to provisioning mode.
- [x] 6.6 Run `gitnexus_detect_changes()` before committing implementation changes.
