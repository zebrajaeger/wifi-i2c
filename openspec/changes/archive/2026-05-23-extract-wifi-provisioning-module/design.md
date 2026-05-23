## Context

`src/main.cpp` currently owns WiFi credential storage, station connection, AP fallback, DNS captive portal handling, portal HTML rendering, SSID scanning, credential submission, and reboot handling. The I2C REST API has already been extracted into `src/i2c_rest_api.h/.cpp`, so WiFi provisioning is now the largest remaining responsibility in the firmware entry point.

This change is an internal refactor. The externally visible WiFi provisioning behavior should remain the same: boot attempts saved WiFi, failed or missing credentials start the `wifi-i2c-setup` AP, the captive portal lists nearby SSIDs, credentials can be saved, and reboot remains available.

## Goals / Non-Goals

**Goals:**
- Move WiFi provisioning implementation into `src/wifi_provisioning.h` and `src/wifi_provisioning.cpp`.
- Keep `main.cpp` focused on serial startup, mode selection, and loop dispatch.
- Expose a small module API for attempting station connection, starting provisioning mode, and servicing provisioning loop work.
- Preserve existing captive portal routes, page content, logs, AP SSID/IP, credential namespace/keys, connection timeout, and reboot behavior unless a build issue requires a minimal mechanical adjustment.
- Keep the I2C REST API module independent from WiFi provisioning internals.

**Non-Goals:**
- Changing AP SSID, IP address, captive portal UX, credential storage format, or WiFi connection timeout.
- Adding authentication, new provisioning screens, hidden SSID support, or enterprise WiFi.
- Reworking the I2C REST API module.
- Introducing a new web framework or async server dependency.

## Decisions

1. Use a namespace-style module rather than a class.
   - Proposed interface: `WifiProvisioning::connectToConfiguredWifi()`, `WifiProvisioning::startProvisioningMode(WebServer&)`, and `WifiProvisioning::handleClient()`.
   - Rationale: The current firmware has one global provisioning flow and no need for multiple instances.
   - Alternative considered: a `WifiProvisioning` class holding server/DNS state. That is more flexible but adds constructor/lifetime ceremony without current benefit.

2. Keep the shared `WebServer` owned by `main.cpp`.
   - `main.cpp` already shares one HTTP server lifecycle between provisioning mode and I2C API mode.
   - The provisioning module should register portal routes against the server reference it receives.
   - Alternative considered: provisioning owns its own `WebServer`. That would duplicate server ownership and make mode switching less explicit.

3. Move DNS server ownership into the provisioning module.
   - DNS catch-all is only meaningful in AP/captive portal mode.
   - `WifiProvisioning::handleClient()` should process DNS requests and the HTTP server while provisioning is active.
   - Alternative considered: keep DNS in `main.cpp`. That leaves captive portal implementation details in the entry point, weakening the separation.

4. Keep credential helpers private to the provisioning module.
   - Preferences namespace, keys, load/save behavior, and portal form handling belong to provisioning.
   - `main.cpp` should only ask whether station connection succeeded.

5. Avoid spec behavior changes.
   - The main `wifi-provisioning-captive-portal` behavioral spec should remain valid after this refactor.
   - The new `wifi-provisioning-module-boundary` spec captures the structural boundary so the refactor remains intentional and testable.

## Risks / Trade-offs

- Route registration order can break portal behavior -> keep provisioning route setup identical and verify AP/captive portal still responds.
- Shared `WebServer` pointer/reference lifetime can be mismanaged -> server remains global in `main.cpp` and is passed by reference before `begin()`.
- DNS handling could be skipped after extraction -> make `WifiProvisioning::handleClient()` responsible for `dnsServer.processNextRequest()` and `server.handleClient()`.
- Behavior drift during move -> use mechanical extraction first, then build and re-run API/provisioning smoke checks.
- GitNexus may not know newly extracted symbols until re-indexed -> run impact on known existing symbols before editing and `detect_changes` after.
