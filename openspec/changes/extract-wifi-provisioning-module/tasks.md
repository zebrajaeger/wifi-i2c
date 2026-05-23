## 1. Safety and Boundaries

- [ ] 1.1 Run GitNexus impact analysis for `setup`, `loop`, `connectToConfiguredWifi`, `startProvisioningMode`, and captive portal route handlers before editing firmware symbols.
- [ ] 1.2 Define the public `WifiProvisioning` module interface in a new header.
- [ ] 1.3 Decide which state remains in `main.cpp` and which state moves into the provisioning module.

## 2. Module Extraction

- [ ] 2.1 Create `src/wifi_provisioning.h` with the public provisioning API.
- [ ] 2.2 Create `src/wifi_provisioning.cpp` and move credential storage helpers into it.
- [ ] 2.3 Move boot WiFi connection logic into the provisioning module.
- [ ] 2.4 Move WiFi scan, SSID sorting, and duplicate handling into the provisioning module.
- [ ] 2.5 Move captive portal HTML rendering and HTTP route handlers into the provisioning module.
- [ ] 2.6 Move AP setup and DNS captive portal handling into the provisioning module.

## 3. Main Orchestration

- [ ] 3.1 Update `main.cpp` to include `wifi_provisioning.h`.
- [ ] 3.2 Update `setup()` to call the provisioning module for WiFi connection and AP fallback.
- [ ] 3.3 Update `loop()` to delegate provisioning loop work to the provisioning module.
- [ ] 3.4 Keep I2C REST API startup separate from provisioning internals.

## 4. Behavior Preservation

- [ ] 4.1 Preserve existing AP SSID, AP IP, gateway, subnet, credential namespace, credential keys, and WiFi connection timeout.
- [ ] 4.2 Preserve captive portal routes and common captive-check handling.
- [ ] 4.3 Preserve credential save and reboot behavior.
- [ ] 4.4 Preserve serial logs or update them only where names clarify the new module boundary.

## 5. Verification

- [ ] 5.1 Build the firmware with PlatformIO or the project-standard command.
- [ ] 5.2 Flash the controller and verify successful WiFi station boot still starts I2C API mode.
- [ ] 5.3 Verify the I2C scan endpoint still returns the attached device.
- [ ] 5.4 Verify provisioning/AP mode still starts when credentials are missing or unreachable.
- [ ] 5.5 Verify the captive portal still lists nearby SSIDs and can save credentials.
- [ ] 5.6 Run `gitnexus_detect_changes()` before committing implementation changes.
