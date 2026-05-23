## Context

The project currently contains a minimal ESP32 Arduino firmware entry point. The new provisioning flow will become part of startup behavior: the controller first tries to join a configured WiFi network, and falls back to a local access point with a captive portal when provisioning is needed.

The implementation should use ESP32 Arduino facilities already available through the selected PlatformIO environment where practical: `WiFi` for station/AP modes and scans, `WebServer` for the portal, `DNSServer` for captive portal redirection, and `Preferences` for persistent credential storage.

## Goals / Non-Goals

**Goals:**
- Connect automatically to saved WiFi credentials on boot.
- Start an AP-based provisioning mode when no credentials exist or connection fails.
- Present nearby SSIDs sorted by signal strength in a captive portal.
- Save selected credentials persistently and allow the user to reboot the controller from the portal.
- Keep the first implementation compact enough for the current single-firmware project while leaving room to split modules later.

**Non-Goals:**
- Remote cloud provisioning or account-based setup.
- Enterprise WiFi, certificates, WPA2-Enterprise, or hidden-network manual SSID entry unless added later.
- A full configuration dashboard beyond WiFi credential capture and reboot.
- Guaranteed automatic captive portal pop-up on every operating system; the portal must still be reachable via browser when automatic detection is inconsistent.

## Decisions

1. Use a boot-time WiFi state machine.
   - The firmware will load credentials from non-volatile storage during `setup()`.
   - If credentials exist, it will attempt `WIFI_STA` connection for a bounded timeout.
   - If connection succeeds, normal controller operation continues.
   - If credentials are missing or the timeout expires, provisioning mode starts.
   - Alternative considered: always start AP and station simultaneously. This is more flexible but adds complexity and can make the first implementation harder to reason about.

2. Store credentials with ESP32 `Preferences`.
   - Store SSID and password under a dedicated namespace such as `wifi`.
   - Commit only after the user submits both selected SSID and password.
   - Alternative considered: file-based storage in SPIFFS/LittleFS. `Preferences` is simpler for small key/value configuration and avoids introducing a filesystem for two fields.

3. Implement captive portal with DNS catch-all plus local HTTP routes.
   - In provisioning mode, the ESP32 starts an access point with a recognizable SSID.
   - `DNSServer` resolves all hostnames to the AP IP.
   - `WebServer` serves the portal page for `/` and handles common captive-check paths by redirecting or serving the portal.
   - Alternative considered: external async web server dependency. The synchronous built-in server is sufficient for a small setup portal and keeps dependencies low.

4. Scan and sort WiFi networks before rendering the portal.
   - Use `WiFi.scanNetworks()` in provisioning mode.
   - Sort results by RSSI descending so the strongest signal appears first.
   - De-duplicate repeated SSIDs by keeping the strongest RSSI entry.
   - Alternative considered: display raw scan order. That would be less useful to users and can show duplicate SSIDs.

5. Reboot only after explicit user action.
   - Saving credentials responds with a confirmation state.
   - A reboot button triggers `ESP.restart()` so the next boot follows the normal connection path.
   - Alternative considered: reboot immediately after saving. An explicit button gives the user clearer feedback that credentials were accepted before the device restarts.

## Risks / Trade-offs

- Captive portal detection differs between Android, iOS, Windows, and desktop browsers -> serve common detection paths and keep the portal reachable at the AP gateway IP.
- WiFi scans can block briefly -> run scans only in provisioning mode or behind a refresh action, not during normal controller operation.
- Passwords are stored on-device -> use `Preferences` namespace isolation and avoid logging passwords over Serial.
- Wrong credentials could trap the device in retry/fallback cycles -> use a bounded connection timeout and always return to AP provisioning when connection fails.
- AP provisioning is local and unauthenticated by default -> keep the AP active only when provisioning is required, and avoid exposing unrelated controller controls in the portal.
