## 1. Example Structure

- [ ] 1.1 Create `examples/ina3221/` with a README, `ina3221.js`, and `package.json`.
- [ ] 1.2 Keep the example dependency-free and runnable with Node.js 18 or newer.

## 2. Documentation

- [ ] 2.1 Document INA3221 assumptions: controller IP, default I2C address, channel numbering, shunt resistor values, wiring, and safety limits.
- [ ] 2.2 Document I2C scan usage through `GET /api/i2c/scan` and common INA3221 addresses.
- [ ] 2.3 Document register-style reads for config, shunt-voltage, and bus-voltage registers through `POST /api/i2c/write-read`.
- [ ] 2.4 Document conversion formulas for signed shunt voltage, bus voltage, and optional current from shunt resistance.
- [ ] 2.5 Document Node.js usage for scan, single-channel reads, all-channel reads, address override, and shunt resistor override.

## 3. Node.js CLI

- [ ] 3.1 Implement argument parsing for host, address, channel, all-channel mode, scan mode, and optional shunt resistor value.
- [ ] 3.2 Implement REST helpers for scan and INA3221 register `write-read` transactions.
- [ ] 3.3 Implement INA3221 conversion helpers for bus voltage, shunt voltage, and optional current.
- [ ] 3.4 Print readable JSON or tabular output for channel measurements and scan results.

## 4. Verification

- [ ] 4.1 Run a syntax check for `examples/ina3221/ina3221.js`.
- [ ] 4.2 Verify the example does not change firmware source files or PlatformIO configuration.
- [ ] 4.3 If a controller IP is available, scan the I2C bus and confirm the connected INA3221 address appears.
- [ ] 4.4 If a controller IP is available, read at least one INA3221 channel through the Node.js example.
