## 1. Example Structure

- [ ] 1.1 Create `examples/ina219/` with a README, `ina219.js`, and `package.json`.
- [ ] 1.2 Keep the example dependency-free and runnable with Node.js 18 or newer.

## 2. Documentation

- [ ] 2.1 Document INA219 assumptions: controller IP, default I2C address, shunt resistor value, current LSB, wiring, and safety limits.
- [ ] 2.2 Document I2C scan usage through `GET /api/i2c/scan` and common INA219 addresses.
- [ ] 2.3 Document register-style reads for config, shunt-voltage, bus-voltage, current, and power registers through `POST /api/i2c/write-read`.
- [ ] 2.4 Document optional calibration through `POST /api/i2c/write` before current and power reads.
- [ ] 2.5 Document conversion formulas for shunt voltage, bus voltage, current, and power.
- [ ] 2.6 Document Node.js usage for scan, voltage reads, calibrated reads, address override, shunt resistor override, and current LSB override.

## 3. Node.js CLI

- [ ] 3.1 Implement argument parsing for host, address, scan mode, calibration mode, shunt resistor value, current LSB, and config read.
- [ ] 3.2 Implement REST helpers for scan, INA219 register `write-read` transactions, and calibration register writes.
- [ ] 3.3 Implement INA219 conversion helpers for shunt voltage, bus voltage, current, power, and calibration register calculation.
- [ ] 3.4 Print readable table and JSON output for measurements, calibration details, and scan results.

## 4. Verification

- [ ] 4.1 Run a syntax check for `examples/ina219/ina219.js`.
- [ ] 4.2 Verify the example does not change firmware source files or PlatformIO configuration.
- [ ] 4.3 If a controller IP is available, scan the I2C bus and confirm an INA219-compatible address appears.
- [ ] 4.4 If a controller IP is available, read INA219 voltage measurements through the Node.js example.
