## 1. Research Current Behavior

- [x] 1.1 Review `platformio.ini` for the supported PlatformIO environment and device workflow commands.
- [x] 1.2 Review `src/main.cpp`, `src/wifi_provisioning.*`, and `src/i2c_rest_api.*` for current boot behavior, provisioning mode, and REST routes.
- [x] 1.3 Review existing OpenSpec specs to align README terminology with documented capabilities.

## 2. README Content

- [x] 2.1 Create top-level `README.md` with a concise project overview.
- [x] 2.2 Document firmware operating modes: WiFi station/API mode and AP/captive portal provisioning mode.
- [x] 2.3 Document PlatformIO build, upload, and serial monitor commands.
- [x] 2.4 Document provisioning usage, including AP SSID, portal flow, credential save, and reboot expectation.
- [x] 2.5 Document I2C REST API endpoints with practical request examples.
- [x] 2.6 Document verification steps for confirming WiFi/API mode and I2C scan behavior.

## 3. Verification

- [x] 3.1 Verify README endpoint names and command examples match the current code and project configuration.
- [x] 3.2 Check the README for clear placeholders where IP addresses or attached I2C devices are environment-specific.
- [x] 3.3 Confirm no firmware code or build configuration changes are required for this documentation change.
