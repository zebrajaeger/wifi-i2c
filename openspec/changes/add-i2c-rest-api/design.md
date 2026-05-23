## Context

The firmware already provides WiFi provisioning and runs an ESP32 Arduino `WebServer` while in provisioning mode. After successful WiFi station connection, the controller currently does not expose application functionality. This change adds the controller's primary external interface: REST endpoints that let network clients operate the ESP32 as I2C master.

The controller should keep provisioning concerns separate from operational API concerns. Captive portal routes remain available only during provisioning. I2C REST routes are started only after the controller has joined the configured WiFi network.

## Goals / Non-Goals

**Goals:**
- Initialize the ESP32 I2C master bus during normal startup.
- Expose REST endpoints for bus scan, write, read, and register-style write-then-read.
- Return structured JSON responses for success and error cases.
- Validate all externally supplied addresses, byte arrays, and read lengths before touching the I2C bus.
- Keep I2C operations bounded so a bad request or unresponsive device cannot stall the controller indefinitely.

**Non-Goals:**
- I2C slave mode.
- Long-running streaming, subscriptions, or WebSocket transport.
- Multi-master arbitration beyond what the ESP32/Wire stack provides.
- Device-specific drivers or semantic decoding of sensor/register values.
- Authentication or authorization in this change; the API is intended for the trusted configured WLAN unless a later security change adds access control.

## Decisions

1. Run one HTTP server instance in either provisioning mode or API mode.
   - Provisioning mode keeps the existing captive portal behavior.
   - Station mode starts REST routes on the same `WebServer` type after WiFi connects.
   - Alternative considered: separate server instances for portal and API. That adds lifecycle complexity without a current need because only one mode is active at a time.

2. Use ESP32 Arduino `Wire` as the I2C master implementation.
   - `Wire.begin()` is available in the current framework and keeps dependencies low.
   - The first implementation should use default board SDA/SCL pins unless explicit project pin definitions are added during implementation.
   - Alternative considered: lower-level ESP-IDF I2C APIs. They offer more control but are heavier for this REST bridge and less consistent with the existing Arduino-style firmware.

3. Use compact JSON contracts with decimal or hex-compatible input.
   - Address fields accept numeric values. Implementation may also accept strings such as `"0x3c"` if this can be done cleanly.
   - Byte arrays are JSON arrays of integers from 0 to 255.
   - Responses include `ok`, request-specific data, and error details when applicable.
   - Alternative considered: raw binary HTTP bodies. JSON is easier for curl, browser tools, and test scripts.

4. Define REST endpoints around I2C primitives.
   - `GET /api/i2c/scan` scans 7-bit addresses and returns discovered addresses.
   - `POST /api/i2c/write` writes bytes to an address.
   - `POST /api/i2c/read` reads a requested byte count from an address.
   - `POST /api/i2c/write-read` writes prefix/register bytes without releasing the bus when supported, then reads bytes.
   - Alternative considered: one generic transaction endpoint only. Separate primitive endpoints are easier to test and document; a combined endpoint still covers common register reads.

5. Bound request size and I2C read length.
   - The API should reject empty writes, overly large byte arrays, invalid addresses, and read lengths outside configured limits.
   - Recommended initial maximum read/write payload is 32 bytes because this aligns with common Arduino Wire buffer limits.
   - Alternative considered: dynamically allocating larger buffers. That is riskier on a small controller and not required for typical I2C transactions.

## Risks / Trade-offs

- Unauthenticated API on the WLAN -> limit exposure to station mode and document that the network must be trusted until an auth change is added.
- Wire buffer limits vary by core/version -> validate payload size against a conservative constant and report a clear JSON error.
- I2C devices can hold the bus or NACK unpredictably -> expose status codes from `endTransmission()` and use bounded read lengths.
- Register-read semantics vary by device -> provide raw write-read primitives only, without device-specific interpretation.
- Existing `WebServer` code may grow too large in `main.cpp` -> implementation can start in one file but should split API/I2C helpers if readability drops.
