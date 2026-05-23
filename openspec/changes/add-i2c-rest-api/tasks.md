## 1. Safety and Structure

- [ ] 1.1 Run GitNexus impact analysis for `setup`, `loop`, and the existing web server route handlers before editing firmware symbols.
- [ ] 1.2 Decide whether to keep the API implementation in `src/main.cpp` or split I2C/API helpers into dedicated files.
- [ ] 1.3 Add required ESP32 Arduino includes and dependencies for I2C master operations and JSON request handling.

## 2. Server Lifecycle

- [ ] 2.1 Add an API mode that starts after successful WiFi station connection.
- [ ] 2.2 Keep captive portal routes limited to provisioning mode.
- [ ] 2.3 Ensure `loop()` services HTTP requests in both provisioning mode and API mode.
- [ ] 2.4 Add logs that distinguish provisioning mode, API mode, and I2C request handling.

## 3. I2C Core

- [ ] 3.1 Initialize the I2C bus as ESP32 master before accepting REST requests.
- [ ] 3.2 Implement a bus scan over valid 7-bit I2C addresses.
- [ ] 3.3 Implement validated byte writes to a target I2C address.
- [ ] 3.4 Implement validated byte reads from a target I2C address.
- [ ] 3.5 Implement write-then-read transactions for register-style device access.

## 4. REST API Contracts

- [ ] 4.1 Implement `GET /api/i2c/scan` returning JSON with discovered addresses.
- [ ] 4.2 Implement `POST /api/i2c/write` accepting address and byte array JSON payloads.
- [ ] 4.3 Implement `POST /api/i2c/read` accepting address and read length JSON payloads.
- [ ] 4.4 Implement `POST /api/i2c/write-read` accepting address, prefix bytes, and read length JSON payloads.
- [ ] 4.5 Return consistent JSON success responses with `ok: true` and operation-specific data.
- [ ] 4.6 Return consistent JSON error responses with `ok: false` and machine-readable error fields.

## 5. Validation and Limits

- [ ] 5.1 Validate I2C addresses before starting bus operations.
- [ ] 5.2 Validate byte arrays so each value is between 0 and 255.
- [ ] 5.3 Validate read and write payload sizes against a conservative maximum such as 32 bytes.
- [ ] 5.4 Reject malformed JSON or missing required fields before touching the I2C bus.

## 6. Verification

- [ ] 6.1 Build the firmware with PlatformIO or the project-standard command.
- [ ] 6.2 Flash the controller and verify API mode starts after connecting to the configured WiFi.
- [ ] 6.3 Verify the I2C scan endpoint returns JSON and handles an empty bus.
- [ ] 6.4 Verify write, read, and write-read endpoint validation with malformed and out-of-range requests.
- [ ] 6.5 Verify at least one real I2C transaction against attached hardware when hardware is available.
- [ ] 6.6 Verify provisioning mode still exposes the captive portal and does not expose I2C API routes.
- [ ] 6.7 Run `gitnexus_detect_changes()` before committing implementation changes.
