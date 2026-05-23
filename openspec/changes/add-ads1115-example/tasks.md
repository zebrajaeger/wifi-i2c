## 1. Research Current API And ADS1115 Basics

- [x] 1.1 Review `README.md` and `src/i2c_rest_api.*` for current endpoint names, JSON payloads, and response shapes.
- [x] 1.2 Review the MCP4725 example structure to mirror README, Node.js CLI, and package script style.
- [x] 1.3 Confirm ADS1115 common address assumptions, pointer/config/conversion register usage, and raw signed conversion format.
- [x] 1.4 Decide default example settings for address, channel, gain/full-scale range, data rate, and conversion wait time.

## 2. Example Content

- [x] 2.1 Create the `examples/ads1115/` subdirectory.
- [x] 2.2 Add a self-contained ADS1115 README under `examples/ads1115/`.
- [x] 2.3 Document required placeholders: controller IP, I2C address, channel, gain, data rate, input wiring, and safe voltage limits.
- [x] 2.4 Include a scan example using `GET /api/i2c/scan`.
- [x] 2.5 Include REST examples for writing the ADS1115 configuration register and reading the conversion register.
- [x] 2.6 Explain how raw ADS1115 bytes become a signed integer and approximate voltage.
- [x] 2.7 Add a runnable `ads1115.js` Node.js CLI for scan and single-shot reads.
- [x] 2.8 Add a `package.json` with direct npm scripts for scanning and reading.

## 3. Verification

- [x] 3.1 Verify all example REST endpoint names match the current firmware routes.
- [x] 3.2 Verify JSON examples are valid and byte values are in the accepted 0-255 range.
- [x] 3.3 Verify the Node.js file parses successfully without third-party dependencies.
- [x] 3.4 Confirm the change does not modify firmware source files or PlatformIO configuration.
- [x] 3.5 Optionally run an I2C scan against a connected controller if hardware is available.
