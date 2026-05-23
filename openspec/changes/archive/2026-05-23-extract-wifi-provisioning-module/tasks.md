## 1. Safety and Boundaries

- [x] 1.1 Run GitNexus impact analysis for `setup`, `loop`, `connectToConfiguredWifi`, `startProvisioningMode`, and captive portal route handlers before editing firmware symbols.
- [x] 1.2 Define the public `WifiProvisioning` module interface in a new header.
- [x] 1.3 Decide which state remains in `main.cpp` and which state moves into the provisioning module.

## 2. Module Extraction

- [x] 2.1 Create `src/wifi_provisioning.h` with the public provisioning API.
- [x] 2.2 Create `src/wifi_provisioning.cpp` and move credential storage helpers into it.
- [x] 2.3 Move boot WiFi connection logic into the provisioning module.
- [x] 2.4 Move WiFi scan, SSID sorting, and duplicate handling into the provisioning module.
- [x] 2.5 Move captive portal HTML rendering and HTTP route handlers into the provisioning module.
- [x] 2.6 Move AP setup and DNS captive portal handling into the provisioning module.

## 3. Main Orchestration

- [x] 3.1 Update `main.cpp` to include `wifi_provisioning.h`.
- [x] 3.2 Update `setup()` to call the provisioning module for WiFi connection and AP fallback.
- [x] 3.3 Update `loop()` to delegate provisioning loop work to the provisioning module.
- [x] 3.4 Keep I2C REST API startup separate from provisioning internals.

## 4. Behavior Preservation

- [x] 4.1 Preserve existing AP SSID, AP IP, gateway, subnet, credential namespace, credential keys, and WiFi connection timeout.
- [x] 4.2 Preserve captive portal routes and common captive-check handling.
- [x] 4.3 Preserve credential save and reboot behavior.
- [x] 4.4 Preserve serial logs or update them only where names clarify the new module boundary.

## 5. Verification

- [x] 5.1 Build the firmware with PlatformIO or the project-standard command.
- [x] 5.2 Flash the controller and verify successful WiFi station boot still starts I2C API mode.
- [x] 5.3 Verify the I2C scan endpoint still returns the attached device.
- [x] 5.4 Verify provisioning/AP mode still starts when credentials are missing or unreachable.
- [x] 5.5 Verify the captive portal still lists nearby SSIDs and can save credentials.
- [x] 5.6 Run `gitnexus_detect_changes()` before committing implementation changes.
