## 1. Safety and Structure

- [ ] 1.1 Run GitNexus impact analysis for `setup` before changing firmware startup behavior.
- [ ] 1.2 Decide whether to keep the first implementation in `src/main.cpp` or split WiFi provisioning into dedicated source/header files.
- [ ] 1.3 Add the required ESP32 Arduino includes for WiFi station/AP mode, DNS, HTTP serving, preferences storage, and restart handling.

## 2. Credential Storage

- [ ] 2.1 Implement loading saved SSID and password from a dedicated `Preferences` namespace.
- [ ] 2.2 Implement saving submitted SSID and password without logging the password over Serial.
- [ ] 2.3 Add validation so empty SSID submissions are rejected and do not overwrite stored credentials.

## 3. Boot Connection Flow

- [ ] 3.1 Implement bounded boot-time connection attempts when saved credentials exist.
- [ ] 3.2 Continue normal controller operation when the station connection succeeds.
- [ ] 3.3 Start provisioning mode when credentials are missing or the configured network is unreachable within the timeout.

## 4. Provisioning Access Point

- [ ] 4.1 Start a recognizable provisioning access point when provisioning mode is active.
- [ ] 4.2 Configure local IP, gateway, and DNS catch-all so connected clients resolve web requests to the controller.
- [ ] 4.3 Keep DNS and HTTP request handling active from `loop()` while provisioning mode is running.

## 5. Captive Portal UI and Routes

- [ ] 5.1 Implement a WiFi scan that sorts networks by RSSI descending and de-duplicates repeated SSIDs by strongest signal.
- [ ] 5.2 Render the portal page with the sorted SSID list, password input, save action, and reboot action.
- [ ] 5.3 Handle common captive portal detection paths by serving or redirecting to the portal.
- [ ] 5.4 Handle credential form submission with success and error responses.
- [ ] 5.5 Handle the reboot action with `ESP.restart()` after returning a response to the client.

## 6. Verification

- [ ] 6.1 Build the firmware with PlatformIO or the project-standard command.
- [ ] 6.2 Verify the no-credentials boot path starts the provisioning access point.
- [ ] 6.3 Verify the portal lists nearby SSIDs sorted strongest to weakest.
- [ ] 6.4 Verify saved credentials survive reboot and the controller connects automatically to the configured WiFi.
- [ ] 6.5 Verify unreachable or wrong credentials return the controller to provisioning mode.
- [ ] 6.6 Run `gitnexus_detect_changes()` before committing implementation changes.
